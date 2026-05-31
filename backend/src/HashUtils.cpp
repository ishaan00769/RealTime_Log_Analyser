#include "HashUtils.h"

uint32_t HashUtils::murmurHash3(const std::string& key, uint32_t seed) {
    const uint8_t* data = reinterpret_cast<const uint8_t*>(key.data());
    const int nblocks = key.length() / 4;
    uint32_t h1 = seed;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    // Body
    const uint32_t* blocks = reinterpret_cast<const uint32_t*>(data);
    for (int i = 0; i < nblocks; i++) {
        uint32_t k1 = blocks[i];

        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= c2;

        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> 19);
        h1 = h1 * 5 + 0xe6546b64;
    }

    // Tail
    const uint8_t* tail = reinterpret_cast<const uint8_t*>(data + nblocks * 4);
    uint32_t k1 = 0;
    switch (key.length() & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1;
                k1 = (k1 << 15) | (k1 >> 17);
                k1 *= c2;
                h1 ^= k1;
    };

    h1 ^= key.length();
    h1 ^= (h1 >> 16);
    h1 *= 0x85ebca6b;
    h1 ^= (h1 >> 13);
    h1 *= 0xc2b2ae35;
    h1 ^= (h1 >> 16);

    return h1;
}

uint32_t HashUtils::getEnhancedHash(const std::string& key, uint32_t index) {
    // Generate two fundamental hashes using distinct seeds
    uint32_t hash1 = murmurHash3(key, 0x12345678);
    uint32_t hash2 = murmurHash3(key, 0x87654321);
    
    // Kirsch-Mitzenmacher optimization: hash_i = hash1 + i * hash2
    return hash1 + index * hash2;
}