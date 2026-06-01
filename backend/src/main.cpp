#include <iostream>
#include <array>
#include <string>
#include <regex>
#include "json.hpp" // nlohmann/json
#include "BloomFilter.h"
#include "CountMinSketch.h"
#include "SpaceSaving.h"
#include "LogReceiver.h"

using json = nlohmann::json;

// Define the contents of an individual hourly bucket
struct HourlyBucket {
    CountMinSketch cms;
    SpaceSaving topDomains;

    // Initialize CMS with standard bounds, and track Top 10 elements via SpaceSaving
    HourlyBucket() : cms(0.001, 0.01), topDomains(10) {}
};

// Global Pipeline Variables
std::array<HourlyBucket, 24> dailyBuckets;
// Global Bloom filter for suspicious user IPs (Expects up to 100K bad IPs, 1% false-pos rate)
BloomFilter suspiciousIpFilter(100000, 0.01);

// Helper function to extract the hour from an ISO 8601 timestamp string (e.g., "2026-05-29T14:35:00Z")
int extractHourFromTimestamp(const std::string& timestamp) {
    // Basic regex extraction for the hour component "THH:"
    std::regex hourRegex("T(\\d{2}):");
    std::smatch match;
    if (std::regex_search(timestamp, match, hourRegex) && match.size() > 1) {
        return std::stoi(match[1].str());
    }
    return 0; // Fallback to bucket 0 if format parsing fails
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

        // 2. Check and evaluate suspicious IP state via Bloom Filter
        // For testing visualization, we simulate flag conditions
        bool isSuspicious = suspiciousIpFilter.possiblyContains(user_ip);
        
        // Simulating logic: if it's already flagged, warn. 
        // In full prod, you would update an IP strike dictionary or trigger database rules.
        if (isSuspicious) {
            std::cout << "[ALERT] Suspicious access detected from flagged IP: " << user_ip << "\n";
        }

        // Output proof-of-work status to the console
        std::cout << "[Processed] Hour: " << hour 
                  << " | Domain: " << website_domain 
                  << " (Est. Hour Freq: " << dailyBuckets[hour].cms.estimate(website_domain) << ")"
                  << " | User IP: " << user_ip << "\n";

    } catch (const std::exception& e) {
        std::cerr << "[Pipeline Error] Failed parsing log: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "=== Log Analyzer Backend Bootstraping ===\n";

    // Pre-seed a dummy suspicious IP into our Bloom Filter for validation testing
    suspiciousIpFilter.add("192.168.1.50");
    std::cout << "[Setup] Pre-loaded suspicious target IP '192.168.1.50' into Bloom Filter.\n";

    // Initialize HTTP microservice listener on Port 8080
    LogReceiver receiver("0.0.0.0", 8080, pipelineProcessor);
    receiver.start();

    return 0;
}