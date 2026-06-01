#include "HyperLogLog.h"
#include "HashUtils.h"
#include <cmath>
#include <algorithm>
#include <iostream>

HyperLogLog::HyperLogLog(uint8_t b_bits) : b(b_bits) {
    m = 1 << b; // 2^b
    registers.resize(m, 0);

    // Calculate the alpha constant to correct systematic bias
    double alpha_m;
    if (m == 16) alpha_m = 0.673;
    else if (m == 32) alpha_m = 0.697;
    else if (m == 64) alpha_m = 0.709;
    else alpha_m = 0.7213 / (1.0 + 1.079 / m);

    alphaMM = alpha_m * m * m;

    std::cout << "[HyperLogLog] Initialized with " << m << " registers (" 
              << (m / 1024.0) << " KB). Est. Error: " 
              << (1.04 / std::sqrt(m)) * 100 << "%\n";
}

void HyperLogLog::add(const std::string& item) {
    // Generate a robust 32-bit hash
    uint32_t hash = HashUtils::murmurHash3(item, 0x42);

    // 1. Extract the top 'b' bits to determine the register index
    uint32_t index = hash >> (32 - b);

    // 2. Shift the hash left to isolate the remaining (32-b) bits
    uint32_t w = hash << b;

    // 3. Count leading zeros.
    // If w is 0, the rank is the maximum possible zeros (32 - b) + 1.
    // Otherwise, we use GCC's built-in 'Count Leading Zeros' for maximum CPU efficiency.
    uint8_t rank = (w == 0) ? (32 - b + 1) : __builtin_clz(w) + 1;

    // 4. Store the maximum rank observed in this register
    registers[index] = std::max(registers[index], rank);
}

double HyperLogLog::estimate() const {
    double sum = 0.0;
    uint32_t zero_registers = 0;

    // Calculate the harmonic mean of the registers
    for (uint32_t i = 0; i < m; ++i) {
        sum += 1.0 / (1ULL << registers[i]);
        if (registers[i] == 0) zero_registers++;
    }

    double estimate = alphaMM / sum;

    // Linear Counting optimization for small cardinalities
    if (estimate <= 2.5 * m) {
        if (zero_registers > 0) {
            estimate = m * std::log(static_cast<double>(m) / zero_registers);
        }
    } 

    return estimate;
}

void HyperLogLog::reset() {
    std::fill(registers.begin(), registers.end(), 0);
}