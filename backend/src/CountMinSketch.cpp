#include "CountMinSketch.h"
#include "HashUtils.h"
#include <cmath>
#include <algorithm>
#include <iostream>

CountMinSketch::CountMinSketch(double epsilon, double delta) {
    // Calculate optimal width (columns) and depth (rows)
    // width = ceil(e / epsilon), depth = ceil(ln(1 / delta))
    width = static_cast<uint32_t>(std::ceil(std::exp(1.0) / epsilon));
    depth = static_cast<uint32_t>(std::ceil(std::log(1.0 / delta)));

    // Initialize the flattened table with zeros
    table.resize(width * depth, 0);

    std::cout << "[Count-Min Sketch] Initialized with depth (hashes): " 
              << depth << ", width: " << width 
              << " (" << (table.size() * sizeof(uint32_t)) / 1024 << " KB)\n";
}

void CountMinSketch::add(const std::string& domain) {
    for (uint32_t i = 0; i < depth; ++i) {
        uint32_t hash = HashUtils::getEnhancedHash(domain, i);
        uint32_t col = hash % width;
        
        // 1D indexing for a 2D structure: (row * width) + col
        table[i * width + col]++;
    }
}

uint32_t CountMinSketch::estimate(const std::string& domain) const {
    uint32_t minCount = UINT32_MAX;

    for (uint32_t i = 0; i < depth; ++i) {
        uint32_t hash = HashUtils::getEnhancedHash(domain, i);
        uint32_t col = hash % width;
        
        uint32_t count = table[i * width + col];
        minCount = std::min(minCount, count);
    }

    return minCount;
}

void CountMinSketch::reset() {
    std::fill(table.begin(), table.end(), 0);
}