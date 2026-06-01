#ifndef HYPER_LOG_LOG_H
#define HYPER_LOG_LOG_H

#include <vector>
#include <string>
#include <cstdint>

class HyperLogLog {
private:
    uint8_t b;             // Number of bits for the register index (e.g., 14)
    uint32_t m;            // Number of registers (2^b)
    std::vector<uint8_t> registers;
    double alphaMM;        // Precomputed constant (alpha_m * m^2)

public:
    // Initialize with 'b' bits (default 14 yields ~0.81% error rate)
    HyperLogLog(uint8_t b_bits = 14);

    // Add an item (like a user_ip) to track uniqueness
    void add(const std::string& item);

    // Get the estimated number of unique items
    double estimate() const;

    // Reset the registers for a new time period
    void reset();
};

#endif // HYPER_LOG_LOG_H