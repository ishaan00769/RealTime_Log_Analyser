#include "BloomFilter.h"
#include "HashUtils.h"
#include <cmath>
#include <iostream>

BloomFilter::BloomFilter(uint64_t expectedElements, double falsePositiveRate) {
    // Calculate optimal array size (m) and number of hash functions (k)
    // Formula: m = -(n * ln(p)) / (ln(2)^2)
    double m = -(expectedElements * std::log(falsePositiveRate)) / std::pow(std::log(2), 2);
    numBits = static_cast<uint64_t>(m);

    // Formula: k = (m / n) * ln(2)
    double k = (numBits / static_cast<double>(expectedElements)) * std::log(2);
    numHashes = static_cast<uint8_t>(std::ceil(k));

    // Initialize the space-optimized bit array
    bitArray.resize(numBits, false);
    
    std::cout << "[Bloom Filter] Initialized with " << numBits << " bits (" 
              << (numBits / 8) / 1024 << " KB) and " 
              << static_cast<int>(numHashes) << " hash functions.\n";
}

void BloomFilter::add(const std::string& ipAddress) {
    for (uint8_t i = 0; i < numHashes; ++i) {
        uint32_t hash = HashUtils::getEnhancedHash(ipAddress, i);
        uint64_t index = hash % numBits;
        bitArray[index] = true;
    }
}

bool BloomFilter::possiblyContains(const std::string& ipAddress) const {
    for (uint8_t i = 0; i < numHashes; ++i) {
        uint32_t hash = HashUtils::getEnhancedHash(ipAddress, i);
        uint64_t index = hash % numBits;
        
        // If any bit is 0, the item is definitely not in the set
        if (!bitArray[index]) {
            return false;
        }
    }
    // If all bits are 1, it's possibly in the set
    return true;
}

void BloomFilter::reset() {
    // Quickly zero out the underlying memory
    std::fill(bitArray.begin(), bitArray.end(), false);
}