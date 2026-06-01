#include "SpaceSaving.h"
#include <algorithm>
#include <iostream>

SpaceSaving::SpaceSaving(uint32_t k_elements) : k(k_elements) {
    std::cout << "[Space-Saving] Initialized to track top " << k << " elements.\n";
}

void SpaceSaving::add(const std::string& domain) {
    // Case 1: The domain is already being tracked
    if (counts.find(domain) != counts.end()) {
        counts[domain]++;
    } 
    // Case 2: We haven't hit our K limit yet, just add it
    else if (counts.size() < k) {
        counts[domain] = 1;
    } 
    // Case 3: The tracker is full. We must evict the minimum element.
    else {
        auto min_it = counts.begin();
        for (auto it = counts.begin(); it != counts.end(); ++it) {
            if (it->second < min_it->second) {
                min_it = it;
            }
        }
        
        // The core Space-Saving eviction logic:
        // Replace the evicted item's frequency with min_count + 1
        uint32_t min_count = min_it->second;
        counts.erase(min_it);
        counts[domain] = min_count + 1;
    }
}

std::vector<std::pair<std::string, uint32_t>> SpaceSaving::getTopK() const {
    // Copy to a vector so we can sort it for the JSON response
    std::vector<std::pair<std::string, uint32_t>> top_elements(counts.begin(), counts.end());
    
    // Sort in descending order based on frequency
    std::sort(top_elements.begin(), top_elements.end(),
              [](const std::pair<std::string, uint32_t>& a, const std::pair<std::string, uint32_t>& b) {
                  return a.second > b.second; // Compare counts
              });
              
    return top_elements;
}

void SpaceSaving::reset() {
    counts.clear();
}