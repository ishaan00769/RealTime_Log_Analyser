#ifndef COUNT_MIN_SKETCH_H
#define COUNT_MIN_SKETCH_H

#include <vector>
#include <string>
#include <cstdint>

class CountMinSketch {
private:
    uint32_t width;
    uint32_t depth;
    std::vector<uint32_t> table; // Flattened 1D array for better cache locality

public:
    // Initialize with epsilon (error factor) and delta (certainty)
    // e.g., epsilon = 0.001, delta = 0.01
    CountMinSketch(double epsilon, double delta);

    // Increment the count for a specific domain
    void add(const std::string& domain);

    // Get the estimated frequency of a domain
    uint32_t estimate(const std::string& domain) const;

    // Reset the sketch (useful if you ever want to reuse the bucket)
    void reset();
};

#endif // COUNT_MIN_SKETCH_H