#include "Encrypt.h"
#include <sstream>
#include <iomanip>

// Caesars cypher. 
// A strong password with unpredictable output is fairly strong
std::string encrypt(std::string_view str, std::string_view key)
{
    std::stringstream ss;
    for (size_t i = 0; i < str.length(); ++i)
        ss << (char) static_cast<unsigned short>(str[i] ^ key[i < key.length() ? i : i % key.length()]);
    return ss.str();
}

uint32_t SHA256::rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

void SHA256::transform(const uint8_t* chunk) {
    uint32_t w[64];
    for (int i = 0; i < 16; ++i)
        w[i] = (chunk[i * 4] << 24) | (chunk[i * 4 + 1] << 16) | (chunk[i * 4 + 2] << 8) | chunk[i * 4 + 3];

    for (int i = 16; i < 64; ++i) {
        uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    uint32_t a, b, c, d, e, f, g, h0;
    a = h[0]; b = h[1]; c = h[2]; d = h[3];
    e = h[4]; f = h[5]; g = h[6]; h0 = h[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t temp1 = h0 + S1 + ch + k[i] + w[i];
        uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;

        h0 = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    h[0] += a; h[1] += b; h[2] += c; h[3] += d;
    h[4] += e; h[5] += f; h[6] += g; h[7] += h0;
}

void SHA256::update(const std::string& input) {
    for (uint8_t byte : input) {
        data[datalen++] = byte;
        bitlen += 8;
        if (datalen == 64) {
            transform(data);
            datalen = 0;
        }
    }
}

std::string SHA256::finalize() {
    data[datalen++] = 0x80;
    while (datalen < 56)
        data[datalen++] = 0;

    uint64_t bitlen_be = bitlen;
    for (int i = 7; i >= 0; --i)
        data[datalen++] = static_cast<uint8_t>((bitlen_be >> (i * 8)) & 0xFF);

    transform(data);

    std::ostringstream oss;
    for (uint32_t val : h)
        oss << std::hex << std::setw(8) << std::setfill('0') << val;

    return oss.str();
}

std::string SHA256::hash(const std::string& input) {
    SHA256 sha256;
    sha256.update(input);
    return sha256.finalize();
}