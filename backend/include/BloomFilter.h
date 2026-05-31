#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <vector>
#include <string>
#include <cstdint>

class BloomFilter {
private:
    std::vector<bool> bitArray;
    uint64_t numBits;
    uint8_t numHashes;

public:
    // Initialize with expected number of IPs and desired false positive probability (e.g., 0.01 for 1%)
    BloomFilter(uint64_t expectedElements, double falsePositiveRate);

    // Add an IP address to the filter
    void add(const std::string& ipAddress);

    // Check if an IP address might be suspicious
    bool possiblyContains(const std::string& ipAddress) const;

    // Clear the filter (useful for daily log rotation)
    void reset();
};

#endif // BLOOM_FILTER_H