#include "DatabaseManager.h"
#include <iostream>

DatabaseManager::DatabaseManager(const std::string& databaseFilename) : db_name(databaseFilename), db(nullptr) {
    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
        std::cerr << "[DB Error] Cannot open database: " << sqlite3_errmsg(db) << "\n";
    } else {
        std::cout << "[DB] Connected to SQLite database: " << db_name << "\n";
    }
}

DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(db);
        std::cout << "[DB] Connection closed.\n";
    }
}

bool DatabaseManager::executeQuery(const std::string& query) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "[DB Error] " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::initializeTables() {
    std::string createHourlyStats = 
        "CREATE TABLE IF NOT EXISTS hourly_stats ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "hour INTEGER NOT NULL, "
        "unique_visitors INTEGER NOT NULL);";

    std::string createTopDomains = 
        "CREATE TABLE IF NOT EXISTS top_domains ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "hour INTEGER NOT NULL, "
        "domain TEXT NOT NULL, "
        "frequency INTEGER NOT NULL);";

    std::string createSuspiciousIPs = 
        "CREATE TABLE IF NOT EXISTS suspicious_ips ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "ip_address TEXT NOT NULL, "
        "timestamp TEXT NOT NULL);";

    return executeQuery(createHourlyStats) && 
           executeQuery(createTopDomains) && 
           executeQuery(createSuspiciousIPs);
}

bool DatabaseManager::saveHourlyStats(int hour, int uniqueVisitors) {
    std::string sql = "INSERT INTO hourly_stats (hour, unique_visitors) VALUES (?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, hour);
    sqlite3_bind_int(stmt, 2, uniqueVisitors);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

bool DatabaseManager::saveTopDomains(int hour, const std::vector<std::pair<std::string, uint32_t>>& topDomains) {
    std::string sql = "INSERT INTO top_domains (hour, domain, frequency) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    // Wrap the inserts in a transaction for massive performance gains
    executeQuery("BEGIN TRANSACTION;");
    
    for (const auto& entry : topDomains) {
        sqlite3_bind_int(stmt, 1, hour);
        sqlite3_bind_text(stmt, 2, entry.first.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, entry.second);

        sqlite3_step(stmt);
        sqlite3_reset(stmt); // Reset the statement for the next loop iteration
    }

    executeQuery("COMMIT;");
    sqlite3_finalize(stmt);
    return true;
}

bool DatabaseManager::logSuspiciousIP(const std::string& ipAddress, const std::string& timestamp) {
    std::string sql = "INSERT INTO suspicious_ips (ip_address, timestamp) VALUES (?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, ipAddress.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, timestamp.c_str(), -1, SQLITE_STATIC);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}



// Fetch total unique users per hour
std::vector<std::pair<int, int>> DatabaseManager::getHourlyStats() {
    std::vector<std::pair<int, int>> stats;
    std::string sql = "SELECT hour, unique_visitors FROM hourly_stats ORDER BY hour ASC;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.push_back({sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1)});
        }
    }
    sqlite3_finalize(stmt);
    return stats;
}

// Fetch top domains across all hours to build your frontend line graph
std::vector<nlohmann::json> DatabaseManager::getTopDomainsGrouped() {
    std::vector<nlohmann::json> domainsList;
    std::string sql = "SELECT hour, domain, frequency FROM top_domains ORDER BY hour ASC, frequency DESC;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            nlohmann::json row;
            row["hour"] = sqlite3_column_int(stmt, 0);
            row["domain"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            row["frequency"] = sqlite3_column_int(stmt, 2);
            domainsList.push_back(row);
        }
    }
    sqlite3_finalize(stmt);
    return domainsList;
}

// Fetch all flagged malicious IPs
std::vector<std::pair<std::string, std::string>> DatabaseManager::getSuspiciousIPs() {
    std::vector<std::pair<std::string, std::string>> ips;
    std::string sql = "SELECT ip_address, timestamp FROM suspicious_ips ORDER BY id DESC;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ips.push_back({
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
            });
        }
    }
    sqlite3_finalize(stmt);
    return ips;
}