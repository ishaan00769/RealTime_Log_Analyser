#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <sqlite3.h>
#include <vector>
#include <utility>
#include <cstdint> // Required header
#include "json.hpp" // <-- 1. Add this line so the file knows about JSON




class DatabaseManager {
private:
    sqlite3* db;
    std::string db_name;

    // Helper to execute raw SQL (used for table creation)
    bool executeQuery(const std::string& query);

public:

// Add these to the public: section
    std::vector<std::pair<int, int>> getHourlyStats();
  std::vector<nlohmann::json> getTopDomainsGrouped();
    std::vector<std::pair<std::string, std::string>> getSuspiciousIPs();
    // Constructor opens the connection to the SQLite file
    DatabaseManager(const std::string& databaseFilename);
    ~DatabaseManager();

    // Create tables if they don't already exist
    bool initializeTables();

    // Save the HyperLogLog unique visitor count for a specific hour
    bool saveHourlyStats(int hour, int uniqueVisitors);

    // Save the top domains from the Space-Saving/CMS structures
    bool saveTopDomains(int hour, const std::vector<std::pair<std::string, uint32_t>>& topDomains);

    // Immediately log a flagged IP from the Bloom Filter
    bool logSuspiciousIP(const std::string& ipAddress, const std::string& timestamp);
};

#endif // DATABASE_MANAGER_H