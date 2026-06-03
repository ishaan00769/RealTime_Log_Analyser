#include <iostream>
#include <array>
#include <string>
#include <regex>
#include <thread> // REQUIRED FOR REAL-TIME MULTITHREADING
#include "json.hpp" 
#include "BloomFilter.h"
#include "CountMinSketch.h"
#include "SpaceSaving.h"
#include "LogReceiver.h"
#include "HyperLogLog.h"
#include "DatabaseManager.h"
#include "httplib.h"

using json = nlohmann::json;

// Define the contents of an individual hourly bucket
struct HourlyBucket {
    CountMinSketch cms;
    SpaceSaving topDomains;
    HyperLogLog uniqueUsers; 
    HourlyBucket() : cms(0.001, 0.01), topDomains(10) {}
};

// Global instances
std::array<HourlyBucket, 24> dailyBuckets;
BloomFilter suspiciousIpFilter(100000, 0.01);
DatabaseManager db("analytics.db"); 

// Helper function to extract the hour from an ISO 8601 timestamp string
int extractHourFromTimestamp(const std::string& timestamp) {
    std::regex hourRegex("T(\\d{2}):");
    std::smatch match;
    if (std::regex_search(timestamp, match, hourRegex) && match.size() > 1) {
        return std::stoi(match[1].str());
    }
    return 0; 
}

// Processing pipeline executing for every valid log entry incoming
void pipelineProcessor(const std::string& rawJson) {
    try {
        auto data = json::parse(rawJson);

        std::string website_domain = data.at("website_domain").get<std::string>();
        std::string user_ip        = data.at("user_ip").get<std::string>();
        std::string timestamp      = data.at("timestamp").get<std::string>();

        int hour = extractHourFromTimestamp(timestamp);
        if (hour < 0 || hour > 23) hour = 0;

        // 1. Update frequency metrics for that hour
        dailyBuckets[hour].cms.add(website_domain);
        dailyBuckets[hour].topDomains.add(website_domain);
        dailyBuckets[hour].uniqueUsers.add(user_ip); 

        // 2. Evaluate suspicious IP state via Bloom Filter
        bool isSuspicious = suspiciousIpFilter.possiblyContains(user_ip);

        if (isSuspicious) {
            std::cout << "[ALERT] Suspicious access detected from flagged IP: " << user_ip << "\n";
            db.logSuspiciousIP(user_ip, timestamp); 
        }

        std::cout << "[Processed] Hour: " << hour 
                  << " | Domain: " << website_domain 
                  << " | User IP: " << user_ip << "\n";

    } catch (const std::exception& e) {
        std::cerr << "[Pipeline Error] Failed parsing log: " << e.what() << "\n";
    }
}

int main() {
    // 1. Setup Database
    if (!db.initializeTables()) {
        std::cerr << "Failed to initialize database tables.\n";
        return 1;
    }

    // 2. Setup Security
    suspiciousIpFilter.add("192.168.1.50");

    // 3. MULTITHREADING: Push log ingestion to a background thread
    std::thread log_ingestion_thread([&]() {
        std::string line;
        int logCounter = 0;

        while (std::getline(std::cin, line)) {
            if (line == "FLUSH") {
                std::cout << "\n[*] Triggering end-of-day database flush...\n";
                for (int i = 0; i < 24; ++i) {
                    db.saveHourlyStats(i, static_cast<int>(dailyBuckets[i].uniqueUsers.estimate()));
                    auto topList = dailyBuckets[i].topDomains.getTopK();
                    for (auto& entry : topList) {
                        entry.second = dailyBuckets[i].cms.estimate(entry.first);
                    }
                    db.saveTopDomains(i, topList);
                }
                std::cout << "[+] Database flush complete.\n";
                break; 
            }
            
            pipelineProcessor(line);
            logCounter++;

            // REAL-TIME FIX: Flush to database every 50 logs so the frontend has live data
            if (logCounter % 50 == 0) {
                for (int i = 0; i < 24; ++i) {
                    db.saveHourlyStats(i, static_cast<int>(dailyBuckets[i].uniqueUsers.estimate()));
                    auto topList = dailyBuckets[i].topDomains.getTopK();
                    for (auto& entry : topList) {
                        entry.second = dailyBuckets[i].cms.estimate(entry.first);
                    }
                    db.saveTopDomains(i, topList);
                }
            }
        }
    });

    // 4. Start the web server on the MAIN thread (0.0.0.0 is crucial for Docker)
    std::cout << "\n[API Server] Starting Dashboard Backend on port 8080 ...\n";
    httplib::Server svr;

    auto set_cors = [](httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    };

    svr.Get("/api/visitors", [&](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        auto data = db.getHourlyStats();
        nlohmann::json j = nlohmann::json::array();
        for (const auto& p : data) {
            j.push_back({{"hour", p.first}, {"unique_visitors", p.second}});
        }
        res.set_content(j.dump(), "application/json");
    });

    svr.Get("/api/domains", [&](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        auto data = db.getTopDomainsGrouped();
        res.set_content(nlohmann::json(data).dump(), "application/json");
    });

    svr.Get("/api/suspicious", [&](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        auto data = db.getSuspiciousIPs();
        nlohmann::json j = nlohmann::json::array();
        for (const auto& p : data) {
            j.push_back({{"ip_address", p.first}, {"timestamp", p.second}});
        }
        res.set_content(j.dump(), "application/json");
    });

    svr.Options(R"(/api/.*)", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 200;
    });

    svr.listen("0.0.0.0", 8080);
    
    // Clean up thread on exit (though the server runs forever)
    if (log_ingestion_thread.joinable()) {
        log_ingestion_thread.join();
    }

    return 0;
}