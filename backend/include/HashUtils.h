#ifndef HASH_UTILS_H
#define HASH_UTILS_H

#include <string>
#include <cstdint>

class HashUtils {
public:
    // Standard MurmurHash3 32-bit implementation
    static uint32_t murmurHash3(const std::string& key, uint32_t seed);

    // Helper to generate 'k' different hash values for a single key (used in Bloom Filter / CMS)
    // Employs Kirsch-Mitzenmacher optimization to generate multiple hashes using only two hash runs
    static uint32_t getEnhancedHash(const std::string& key, uint32_t index);
};

#endif // HASH_UTILS_H