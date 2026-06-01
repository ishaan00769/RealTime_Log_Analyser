#ifndef SPACE_SAVING_H
#define SPACE_SAVING_H

#include <string>
#include <unordered_map>
#include <vector>
#include <utility>
#include <cstdint>

class SpaceSaving {
private:
    uint32_t k; // Number of top elements to track (e.g., Top 10)
    
    // We use a hash map for O(1) lookups. 
    std::unordered_map<std::string, uint32_t> counts;

public:
    // Initialize with the maximum number of heavy hitters to track
    SpaceSaving(uint32_t k_elements);

    // Process a new domain from the log stream
    void add(const std::string& domain);

    // Retrieve the sorted top-K list for the frontend API
    std::vector<std::pair<std::string, uint32_t>> getTopK() const;

    // Reset the bucket for a new day/hour
    void reset();
};

#endif // SPACE_SAVING_H