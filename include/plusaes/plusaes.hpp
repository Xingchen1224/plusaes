// Copyright (C) 2015 kkAyataka
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef PLUSAES_HPP__
#define PLUSAES_HPP__

#include <algorithm>
#include <bitset>
#include <cmath>
#include <stdexcept>
#include <vector>

/** Version number of plusaes.
 * 0x01020304 -> 1.2.3.4 */
#define PLUSAES_VERSION 0x00090100

/** AES cipher APIs */
namespace plusaes {
namespace detail {

const int kWordSize = 4;
typedef unsigned int Word;

const int kBlockSize = 4;
/** @private */
typedef struct {
    Word w[4];
    Word & operator[](const int index) {
        return w[index];
    }
    const Word & operator[](const int index) const {
        return w[index];
    }
} State;

const int kStateSize = 16; // Word * BlockSize
typedef State RoundKey;
typedef std::vector<RoundKey> RoundKeys;

inline void add_round_key(const RoundKey &key, State &state) {
    for (int i = 0; i < kBlockSize; ++i) {
        state[i] ^= key[i];
    }
}

const unsigned char kSbox[] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

const unsigned char kInvSbox[] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

inline Word sub_word(const Word w) {
    return kSbox[(w >>  0) & 0xFF] <<  0 |
           kSbox[(w >>  8) & 0xFF] <<  8 |
           kSbox[(w >> 16) & 0xFF] << 16 |
           kSbox[(w >> 24) & 0xFF] << 24;
}

inline Word inv_sub_word(const Word w) {
    return kInvSbox[(w >>  0) & 0xFF] <<  0 |
           kInvSbox[(w >>  8) & 0xFF] <<  8 |
           kInvSbox[(w >> 16) & 0xFF] << 16 |
           kInvSbox[(w >> 24) & 0xFF] << 24;
}

inline void sub_bytes(State &state) {
    for (int i = 0; i < kBlockSize; ++i) {
        state[i] = sub_word(state[i]);
    }
}

inline void inv_sub_bytes(State &state) {
    for (int i = 0; i < kBlockSize; ++i) {
        state[i] = inv_sub_word(state[i]);
    }
}

inline void shift_rows(State &state) {
    const State ori = { state[0], state[1], state[2], state[3] };
    for (int r = 1; r < kWordSize; ++r) {
        const Word m2 = 0xFF << (r * 8);
        const Word m1 = ~m2;
        for (int c = 0; c < kBlockSize; ++c) {
            state[c] = (state[c] & m1) | (ori[(c + r) % kBlockSize] & m2);
        }
    }
}

inline void inv_shift_rows(State &state) {
    const State ori = { state[0], state[1], state[2], state[3] };
    for (int r = 1; r < kWordSize; ++r) {
        const Word m2 = 0xFF << (r * 8);
        const Word m1 = ~m2;
        for (int c = 0; c < kBlockSize; ++c) {
            state[c] = (state[c] & m1) | (ori[(c + kBlockSize - r) % kWordSize] & m2);
        }
    }
}

inline unsigned char mul2(const unsigned char b) {
    unsigned char m2 = b << 1;
    if (b & 0x80) {
        m2 ^= 0x011B;
    }

    return m2;
}

inline unsigned char mul(const unsigned char b, const unsigned char m) {
    unsigned char v = 0;
    unsigned char t = b;
    for (int i = 0; i < 8; ++i) { // 8-bits
        if ((m >> i) & 0x01) {
            v ^= t;
        }

        t = mul2(t);
    }

    return v;
}

inline void mix_columns(State &state) {
    for (int i = 0; i < kBlockSize; ++i) {
        const unsigned char v0_1 = (state[i] >>  0) & 0xFF;
        const unsigned char v1_1 = (state[i] >>  8) & 0xFF;
        const unsigned char v2_1 = (state[i] >> 16) & 0xFF;
        const unsigned char v3_1 = (state[i] >> 24) & 0xFF;

        const unsigned char v0_2 = mul2(v0_1);
        const unsigned char v1_2 = mul2(v1_1);
        const unsigned char v2_2 = mul2(v2_1);
        const unsigned char v3_2 = mul2(v3_1);

        const unsigned char v0_3 = v0_2 ^ v0_1;
        const unsigned char v1_3 = v1_2 ^ v1_1;
        const unsigned char v2_3 = v2_2 ^ v2_1;
        const unsigned char v3_3 = v3_2 ^ v3_1;

        state[i] =
            (v0_2 ^ v1_3 ^ v2_1 ^ v3_1) <<  0 |
            (v0_1 ^ v1_2 ^ v2_3 ^ v3_1) <<  8 |
            (v0_1 ^ v1_1 ^ v2_2 ^ v3_3) << 16 |
            (v0_3 ^ v1_1 ^ v2_1 ^ v3_2) << 24;
    }
}

inline void inv_mix_columns(State &state) {
    for (int i = 0; i < kBlockSize; ++i) {
        const unsigned char v0 = (state[i] >>  0) & 0xFF;
        const unsigned char v1 = (state[i] >>  8) & 0xFF;
        const unsigned char v2 = (state[i] >> 16) & 0xFF;
        const unsigned char v3 = (state[i] >> 24) & 0xFF;

        state[i] =
            (mul(v0, 0x0E) ^ mul(v1, 0x0B) ^ mul(v2, 0x0D) ^ mul(v3, 0x09)) <<  0 |
            (mul(v0, 0x09) ^ mul(v1, 0x0E) ^ mul(v2, 0x0B) ^ mul(v3, 0x0D)) <<  8 |
            (mul(v0, 0x0D) ^ mul(v1, 0x09) ^ mul(v2, 0x0E) ^ mul(v3, 0x0B)) << 16 |
            (mul(v0, 0x0B) ^ mul(v1, 0x0D) ^ mul(v2, 0x09) ^ mul(v3, 0x0E)) << 24;
    }
}

inline Word rot_word(const Word v) {
    return ((v >> 8) & 0x00FFFFFF) | ((v & 0xFF) << 24);
}

/**
 * @private
 * @throws std::invalid_argument
 */
inline unsigned int get_round_count(const int key_size) {
    switch (key_size) {
    case 16:
        return 10;
    case 24:
        return 12;
    case 32:
        return 14;
    default:
        throw std::invalid_argument("Invalid key size");
    }
}

/**
 * @private
 * @throws std::invalid_argument
 */
inline RoundKeys expand_key(const unsigned char *key, const int key_size) {
    if (key_size != 16 && key_size != 24 && key_size != 32) {
        throw std::invalid_argument("Invalid key size");
    }

    const Word rcon[] = {
        0x00, 0x01, 0x02, 0x04, 0x08, 0x10,
        0x20, 0x40, 0x80, 0x1b, 0x36
    };

    const int nb = kBlockSize;
    const int nk = key_size / nb;
    const int nr = get_round_count(key_size);

    std::vector<Word> w(nb * (nr + 1));
    for (int i = 0; i < nk; ++ i) {
        memcpy(&w[i], key + (i * kWordSize), kWordSize);
    }

    for (int i = nk; i < nb * (nr + 1); ++i) {
        Word t = w[i - 1];
        if (i % nk == 0) {
            t = sub_word(rot_word(t)) ^ rcon[i / nk];
        }
        else if (nk > 6 && i % nk == 4) {
            t = sub_word(t);
        }

        w[i] = t ^ w[i - nk];
    }

    RoundKeys keys(nr + 1);
    memcpy(&keys[0], &w[0], w.size() * kWordSize);

    return keys;
}

inline void copy_bytes_to_state(const unsigned char data[16], State &state) {
    memcpy(&state[0], data +  0, kWordSize);
    memcpy(&state[1], data +  4, kWordSize);
    memcpy(&state[2], data +  8, kWordSize);
    memcpy(&state[3], data + 12, kWordSize);
}

inline void copy_state_to_bytes(const State &state, unsigned char buf[16]) {
    memcpy(buf +  0, &state[0], kWordSize);
    memcpy(buf +  4, &state[1], kWordSize);
    memcpy(buf +  8, &state[2], kWordSize);
    memcpy(buf + 12, &state[3], kWordSize);
}

inline void xor_data(unsigned char data[kStateSize], const unsigned char v[kStateSize]) {
    for (int i = 0; i < kStateSize; ++i) {
        data[i] ^= v[i];
    }
}

/** increment counter (128-bit int) by 1 */
inline void incr_counter(unsigned char counter[kStateSize]) {
    unsigned n = kStateSize, c = 1;
    do {
        --n;
        c += counter[n];
        counter[n] = c;
        c >>= 8;
    } while (n);
}

inline void encrypt_state(const RoundKeys &rkeys, const unsigned char data[16], unsigned char encrypted[16]) {
    State s;
    copy_bytes_to_state(data, s);

    add_round_key(rkeys[0], s);

    for (unsigned int i = 1; i < rkeys.size() - 1; ++i) {
        sub_bytes(s);
        shift_rows(s);
        mix_columns(s);
        add_round_key(rkeys[i], s);
    }

    sub_bytes(s);
    shift_rows(s);
    add_round_key(rkeys.back(), s);

    copy_state_to_bytes(s, encrypted);
}

inline void decrypt_state(const RoundKeys &rkeys, const unsigned char data[16], unsigned char decrypted[16]) {
    State s;
    copy_bytes_to_state(data, s);

    add_round_key(rkeys.back(), s);
    inv_shift_rows(s);
    inv_sub_bytes(s);

    for (std::size_t i = rkeys.size() - 2; i > 0; --i) {
        add_round_key(rkeys[i], s);
        inv_mix_columns(s);
        inv_shift_rows(s);
        inv_sub_bytes(s);
    }

    add_round_key(rkeys[0], s);

    copy_state_to_bytes(s, decrypted);
}

template<int KeyLen>
std::vector<unsigned char> key_from_string(const char (*key_str)[KeyLen]) {
    std::vector<unsigned char> key(KeyLen - 1);
    memcpy(&key[0], *key_str, KeyLen - 1);
    return key;
}

inline bool is_valid_key_size(const unsigned long key_size) {
    if (key_size != 16 && key_size != 24 && key_size != 32) {
        return false;
    }
    else {
        return true;
    }
}


namespace gcm {

const int kBlockBitSize = 128;
const int kBlockByteSize = kBlockBitSize / 8;
//typedef std::bitset<kBlockBitSize> Block;

class Block {
public:
    unsigned char v[16];

    Block() {
        memset(v, 0, sizeof(v));
    }

    Block(const unsigned char * bytes, const unsigned long bytes_size) {
        memset(v, 0, sizeof(v));
        copy_to_v(bytes, bytes_size);
    }

    Block(const std::vector<unsigned char> & bytes) {
        memset(v, 0, sizeof(v));
        copy_to_v(&bytes[0], bytes.size());
    }

    Block(const std::bitset<128> & bits) {
        memset(v, 0, sizeof(v));
        const std::bitset<128> mask(0xFF);
        for (int i = 0; i < 16; ++i) {
            v[15 - i] |= static_cast<unsigned char>(((bits >> (i * 8)) & mask).to_ulong());
        }
    }

    inline unsigned char * data() {
        return reinterpret_cast<unsigned char *>(v);
    }

    inline std::bitset<128> to_bits() const {
        std::bitset<128> bits;
        for (int i = 0; i < 16; ++i) {
            bits << 8;
            bits |= v[i];
        }

        return bits;
    }

    /*
    bool bit(const unsigned int i) const {
        const unsigned int vi = i / 32;
        const unsigned int vb = i % 32;

    }*/

    /*
    Block & operator=(const Block & b) {
        if (this != &b) {
            memcpy(v, b.v, sizeof(v));
        }

        return *this;
    }
    */

private:
    inline void copy_to_v(const unsigned char * bytes, const unsigned long bytes_size) {
        const unsigned long cs = (std::min)(bytes_size, static_cast<unsigned long>(16));
        for (unsigned int i = 0; i < cs; ++i) {
            v[i] = bytes[i];
        }
    }
};

inline Block operator^(const Block & b1, const Block & b2) {
    Block b;
    for (int i = 0; i < 16; ++i) {
        b.v[i] = b1.v[i] ^ b2.v[i];
    }

    return b;
}


template<typename T>
unsigned long ceil(const T v) {
    return static_cast<unsigned long>(std::ceil(v) + 0.5);
}

template<std::size_t N1, std::size_t N2>
std::bitset<N1 + N2> concat(const std::bitset<N1> &v1, const std::bitset<N2> &v2) {
    std::bitset<N1 + N2> ret(v1.to_string() + v2.to_string());
    return ret;
}

template<std::size_t N1, std::size_t N2>
std::bitset<N1 + N2> operator||(const std::bitset<N1> &v1, const std::bitset<N2> &v2) {
    std::bitset<N1 + N2> ret(v1.to_string() + v2.to_string());
    return ret;
}

template<std::size_t S, std::size_t N>
std::bitset<S> lsb(const std::bitset<N> &X) {
    std::bitset<S> r;
    for (int i = 0; i < S; ++i) {
        r[i] = X[i];
    }
    return r;
}

template<std::size_t S, std::size_t N>
std::bitset<S> msb(const std::bitset<N> &X) {
    std::bitset<S> r;
    for (int i = 0; i < S; ++i) {
        r[S - 1 - i] = X[X.size() - 1 - i];
    }
    return r;
}

template<std::size_t S = 32, std::size_t N>
std::bitset<N> inc(const std::bitset<N> X) {
    const auto a = msb<N - S>(X);
    const std::bitset<S> b((lsb<S>(X).to_ulong() + 1) % static_cast<unsigned long>(std::pow(2, S)));

    return concat(a, b);
}

/** Algorithm 1 @private */
inline Block mul_blocks(const Block X, const Block Y) {
    const std::bitset<128> R = Block(std::bitset<8>("11100001") || std::bitset<120>()).to_bits();

    std::bitset<128> X_bits = X.to_bits();
    std::bitset<128> Z;
    std::bitset<128> V = Y.to_bits();
    for (int i = 0; i < kBlockBitSize; ++i) {
        // Z
        if (X_bits[i] == false) {
            Z = Z;
        }
        else {
            Z = Z ^ V;
        }

        // V
        if (V[127] == false) {
            V = V >> 1;
        }
        else {
            V = (V >> 1) ^ R;
        }
    }

    return Z;
}

inline std::vector<Block> ghash(const Block &H, const std::vector<Block> X) {
    std::vector<Block> Y(X.size());
    for (int i = 0; i < X.size(); ++i) {
        Y[i] = mul_blocks((Y[i - 1] ^ X[i]), H);
    }

    return Y;
}

template<std::size_t N>
std::bitset<N * 8> make_bitset(const unsigned char (*bytes)[N]) {
    std::bitset<N * 8> bits;
    for (int i = N - 1; i >= 0; --i) {
        bits <<= 8;
        bits |= (*bytes)[i];
    }
    return bits;
}

//inline Block make_block(const std::vector<unsigned char> &bytes, const unsigned long offset = 0) {
/*
inline Block make_block(const unsigned char *bytes, const std::size_t offset = 0) {
    const unsigned int len = kBlockBitSize / 8;
    Block b;
    for (int i = len - 1; i >= 0; --i) {
        b <<= 8;
        b |= bytes[offset + i];
    }

    return b;
}

inline Block make_padded_block(const unsigned char *bytes, const std::size_t bytes_size) {
    Block b;
    for (std::size_t i = bytes_size - 1; i >= 0; --i) {
        b <<= 8;
        b |= bytes[i];;
    }
    return b;
}

inline std::vector<unsigned char> make_bytes(const Block &block) {
    std::vector<unsigned char> bytes(kBlockByteSize);
    const unsigned int psize = 32;
    for (int i = 0; i < kBlockBitSize / psize; ++i) {
        unsigned long v = 0;
        for (int j = 0; j < psize; ++j) {
            v |= block[i * psize + j] << j;
        }
        //const unsigned long d32 = (block >> (i * psize)).to_ulong() & 0xFFFFFFFF;
        memcpy(&bytes[i * 4], &v, 4);
    }

    return bytes;
}
 */

//inline std::vector<unsigned char> gctr(const detail::RoundKeys rkeys, const Block &ICB, const std::vector<unsigned char> X) {
inline std::vector<unsigned char> gctr(const detail::RoundKeys rkeys, const Block &ICB, const unsigned char *X, const unsigned long X_size) {
    if (!X || X_size == 0) {
        return std::vector<unsigned char>();
    }
    else {
        const unsigned long n = ceil(X_size * 8.0 / kBlockBitSize);
        std::vector<unsigned char> Y(X_size);

        std::vector<Block> CB(n);
        CB[0] = ICB;
        for (int i = 1; i < n; ++i) {
            CB[i] = inc<32>(CB[i - 1].to_bits());
        }
        for (int i = 0; i < n; ++i) {
            //std::vector<unsigned char> CB_bytes = make_bytes(CB[i]);
            Block eCB = CB[i];
            encrypt_state(rkeys, CB[i].data(), eCB.data());

            Block Yi = Block(X + i * kBlockBitSize / 8, 16) ^ eCB;
            //std::vector<unsigned char> Yi_bytes = make_bytes(Yi);
            //memcpy(&Y[i * kBlockByteSize], &(make_bytes(Yi))[0], kBlockByteSize);
            memcpy(&Y[i * kBlockByteSize], Yi.data(), kBlockByteSize);
        }

        // adjust size
        //const unsigned long rm_bytes = X_size % kBlockByteSize;
        //Y.resize(Y.size() - rm_bytes);

        return Y;
    }
}
/*
inline std::vector<Block> gctr(const detail::RoundKeys rkeys, const Block &ICB, const std::vector<Block> X) {
    if (X.empty()) {
        return std::vector<Block>();
    }
    else {
        const unsigned long n = X.size();
        std::vector<Block> Y(n);

        std::vector<Block> CB(n);
        CB[0] = ICB;
        for (int i = 1; i < n; ++i) {
            CB[i] = inc<32>(CB[-1]);
        }
        for (int i = 0; i < n; ++i) {
            std::vector<unsigned char> CB_bytes = make_bytes(CB[i]);
            encrypt_state(rkeys, &CB_bytes[0], &CB_bytes[0]);

            Y[i] = X[i] ^ make_block(&CB_bytes[0]);
        }

        return Y;
    }
}*/

/*
template<std::size_t N>
void push_back(std::vector<unsigned char> &bytes, const std::bitset<N> &bits) {
    static_assert(N % 8 == 0, "Invalid bitset size");

    const std::size_t bits_byte_size = N / 8;
    for (int i = 0; i < bits_byte_size; ++i) {
        bytes.push_back((bits >> (i * 8)).to_ulong() & 0xFF);
    }
}

inline void push_back(std::vector<unsigned char> &bytes, const unsigned char *data, const std::size_t data_size) {
    bytes.insert(bytes.end(), data, data + data_size);
}

inline void push_back(std::vector<unsigned char> &bytes, const std::size_t zero_size) {
    for (int i = 0; i < zero_size; ++i) {
        bytes.push_back(0);
    }
}
 */
/*
inline void push_back(std::vector<Block> &blocks, const unsigned char *data, const std::size_t data_size) {
    const int n = ceil(static_cast<double>(data_size / kBlockByteSize));
    for (int i = 0; i < (n - 1); ++i) {
        blocks.push_back(make_block(data, i * kBlockSize));
    }
}
*/

inline void push_back(std::vector<Block> &blocks, const Block &block) {
    blocks.push_back(block);
}

inline void encrypt_gcm(
    const unsigned char * data,
    const unsigned long data_size,
    const unsigned char * aadata,
    const unsigned long aadata_size,
    const unsigned char * key,
    const unsigned long key_size,
    const unsigned char (*iv)[12],
    unsigned char *encrypted
) {
    const detail::RoundKeys rkeys = detail::expand_key(key, static_cast<int>(key_size));

    std::vector<unsigned char> H_raw(kBlockByteSize);
    encrypt_state(rkeys, &H_raw[0], &H_raw[0]);
    //const Block H = make_block(&H_raw[0]);
    const Block H = Block(H_raw); // 66e94bd4ef8a2c3b884cfa59ca342b2e
    const std::bitset<96> iv_bits = make_bitset(iv);
    const Block J0 = iv_bits || std::bitset<31>() || std::bitset<1>(1);
    //const Block J0 =  std::bitset<1>(1) || std::bitset<31>() || iv_bits;
    //const detail::RoundKeys rkeys2 = detail::expand_key(&H_raw[0], H_raw.size());
    const std::vector<unsigned char> C = gctr(rkeys, inc<32>(J0.to_bits()), data, data_size);
    // 58e2fccefa7e3061367f1d57a4e7455a
    //7A, 5F, 32, D3, 5F, 6A, 7D, 18, EE, 22, 61, B6, 2B, 1B, 74, D4,

    const unsigned long lenC = data_size * 8;
    const unsigned long lenA = aadata_size * 8;
    const std::size_t u = 128 * ceil(lenC / 128) - lenC;
    const std::size_t v = 128 * ceil(lenA / 128) - lenA;
    /*
    std::vector<unsigned char> ghash_in;
    ghash_in.reserve((aadata_size + v / 8) + (data_size + u / 8) + 8 + 8);
    push_back(ghash_in, std::bitset<64>(lenC));
    push_back(ghash_in, std::bitset<64>(lenA));
    push_back(ghash_in, u);
    push_back(ghash_in, data, data_size);
    push_back(ghash_in, v);
    push_back(ghash_in, aadata, aadata_size);
     */
/*
    const std::size_t remC = kBlockByteSize - (data_size % kBlockByteSize);
    const std::size_t remA = kBlockByteSize - (aadata_size % kBlockByteSize);
    std::vector<Block> ghash_in;
    ghash_in.reserve(((lenC + u) + (lenA + v) + 128) / 8 / kBlockByteSize);
    //push_back(ghash_in, aadata, aadata_size);
    //push_back(ghash_in, make_padded_block(aadata + aadata_size - remA, remA));
    push_back(ghash_in, data, data_size);
    //push_back(ghash_in, make_padded_block(data + data_size - remC, remC));
    push_back(ghash_in, std::bitset<64>(lenA) || std::bitset<64>(lenC));

    const std::vector<Block> S = ghash(H, ghash_in);
    const std::bitset<32> T = msb<32>(gctr(rkeys, J0, S)[0]);
    const unsigned long tag = T.to_ulong();
*/
    int i = 0;
    ++i;
}

} // namespce detail::gcm

} // namespace detail

/** Version number of plusaes. */
inline unsigned int version() {
    return PLUSAES_VERSION;
}

/** Create 128-bit key from string. */
inline std::vector<unsigned char> key_from_string(const char (*key_str)[17]) {
    return detail::key_from_string<17>(key_str);
}

/** Create 192-bit key from string. */
inline std::vector<unsigned char> key_from_string(const char (*key_str)[25]) {
    return detail::key_from_string<25>(key_str);
}

/** Create 256-bit key from string. */
inline std::vector<unsigned char> key_from_string(const char (*key_str)[33]) {
    return detail::key_from_string<33>(key_str);
}

/** Calculates encrypted data size when padding is enabled. */
inline unsigned long get_padded_encrypted_size(const unsigned long data_size) {
    return data_size + detail::kStateSize - (data_size % detail::kStateSize);
}

/** Error code */
typedef enum {
    kErrorOk = 0,
    kErrorInvalidDataSize = 1,
    kErrorInvalidKeySize,
    kErrorInvalidBufferSize,
    kErrorInvalidKey,
    kErrorInvalidNonceSize
} Error;

namespace detail {

inline Error check_encrypt_cond(
    const unsigned long data_size,
    const unsigned long key_size,
    const unsigned long encrypted_size,
    const bool pads) {
    // check data size
    if (!pads && (data_size % kStateSize != 0)) {
        return kErrorInvalidDataSize;
    }

    // check key size
    if (!detail::is_valid_key_size(key_size)) {
        return kErrorInvalidKeySize;
    }

    // check encrypted buffer size
    if (pads) {
        const unsigned long required_size = get_padded_encrypted_size(data_size);
        if (encrypted_size < required_size) {
            return kErrorInvalidBufferSize;
        }
    }
    else {
        if (encrypted_size < data_size) {
            return kErrorInvalidBufferSize;
        }
    }
    return kErrorOk;
}

inline Error check_decrypt_cond(
    const unsigned long data_size,
    const unsigned long key_size,
    const unsigned long decrypted_size,
    const unsigned long * padded_size
    ) {
    // check data size
    if (data_size % 16 != 0) {
        return kErrorInvalidDataSize;
    }

    // check key size
    if (!detail::is_valid_key_size(key_size)) {
        return kErrorInvalidKeySize;
    }

    // check decrypted buffer size
    if (!padded_size) {
        if (decrypted_size < data_size) {
            return kErrorInvalidBufferSize;
        }
    }
    else {
        if (decrypted_size < (data_size - kStateSize)) {
            return kErrorInvalidBufferSize;
        }
    }

    return kErrorOk;
}

inline bool check_padding(const unsigned long padding, const unsigned char data[kStateSize]) {
    if (padding > kStateSize) {
        return false;
    }

    for (unsigned long i = 0; i < padding; ++i) {
        if (data[kStateSize - 1 - i] != padding) {
            return false;
        }
    }

    return true;
}

} // namespace detail

/**
 * Encrypts data with ECB mode.
 * @param [in]  data Data.
 * @param [in]  data_size Data size.
 *  If the pads is false, data size must be multiple of 16.
 * @param [in]  key key bytes. The key length must be 16 (128-bit), 24 (192-bit) or 32 (256-bit).
 * @param [in]  key_size key size.
 * @param [out] encrypted Encrypted data buffer.
 * @param [in]  encrypted_size Encrypted data buffer size.
 * @param [in]  pads If this value is true, encrypted data is padded by PKCS.
 *  Encrypted data size must be multiple of 16.
 *  If the pads is true, encrypted data is padded with PKCS.
 *  So the data is multiple of 16, encrypted data size needs additonal 16 bytes.
 * @since 1.0.0
 */
inline Error encrypt_ecb(
    const unsigned char * data,
    const unsigned long data_size,
    const unsigned char * key,
    const unsigned long key_size,
    unsigned char *encrypted,
    const unsigned long encrypted_size,
    const bool pads
    ) {
    const Error e = detail::check_encrypt_cond(data_size, key_size, encrypted_size, pads);
    if (e != kErrorOk) {
        return e;
    }

    const detail::RoundKeys rkeys = detail::expand_key(key, static_cast<int>(key_size));

    const unsigned long bc = data_size / detail::kStateSize;
    for (unsigned long i = 0; i < bc; ++i) {
        detail::encrypt_state(rkeys, data + (i * detail::kStateSize), encrypted + (i * detail::kStateSize));
    }

    if (pads) {
        const int rem = data_size % detail::kStateSize;
        const char pad_v = detail::kStateSize - rem;

        std::vector<unsigned char> ib(detail::kStateSize, pad_v), ob(detail::kStateSize);
        memcpy(&ib[0], data + data_size - rem, rem);

        detail::encrypt_state(rkeys, &ib[0], &ob[0]);
        memcpy(encrypted + (data_size - rem), &ob[0], detail::kStateSize);
    }

    return kErrorOk;
}

/**
 * Decrypts data with ECB mode.
 * @param [in]  data Data bytes.
 * @param [in]  data_size Data size.
 * @param [in]  key Key bytes.
 * @param [in]  key_size Key size.
 * @param [out] decrypted Decrypted data buffer.
 * @param [in]  decrypted_size Decrypted data buffer size.
 * @param [out] padded_size If this value is NULL, this function does not remove padding.
 *  If this value is not NULL, this function removes padding by PKCS
 *  and returns padded size using padded_size.
 * @since 1.0.0
 */
inline Error decrypt_ecb(
    const unsigned char * data,
    const unsigned long data_size,
    const unsigned char * key,
    const unsigned long key_size,
    unsigned char * decrypted,
    const unsigned long decrypted_size,
    unsigned long * padded_size
    ) {
    const Error e = detail::check_decrypt_cond(data_size, key_size, decrypted_size, padded_size);
    if (e != kErrorOk) {
        return e;
    }

    const detail::RoundKeys rkeys = detail::expand_key(key, static_cast<int>(key_size));

    const unsigned long bc = data_size / detail::kStateSize - 1;
    for (unsigned long i = 0; i < bc; ++i) {
        detail::decrypt_state(rkeys, data + (i * detail::kStateSize), decrypted + (i * detail::kStateSize));
    }

    unsigned char last[detail::kStateSize] = {};
    detail::decrypt_state(rkeys, data + (bc * detail::kStateSize), last);

    if (padded_size) {
        *padded_size = last[detail::kStateSize - 1];
        const unsigned long cs = detail::kStateSize - *padded_size;

        if (!detail::check_padding(*padded_size, last)) {
            return kErrorInvalidKey;
        }
        else if (decrypted_size >= (bc * detail::kStateSize) + cs) {
            memcpy(decrypted + (bc * detail::kStateSize), last, cs);
        }
        else {
            return kErrorInvalidBufferSize;
        }
    }
    else {
        memcpy(decrypted + (bc * detail::kStateSize), last, sizeof(last));
    }

    return kErrorOk;
}

/**
 * Encrypt data with CBC mode.
 * @param [in]  data Data.
 * @param [in]  data_size Data size.
 *  If the pads is false, data size must be multiple of 16.
 * @param [in]  key key bytes. The key length must be 16 (128-bit), 24 (192-bit) or 32 (256-bit).
 * @param [in]  key_size key size.
 * @param [in]  iv Initialize vector.
 * @param [out] encrypted Encrypted data buffer.
 * @param [in]  encrypted_size Encrypted data buffer size.
 * @param [in]  pads If this value is true, encrypted data is padded by PKCS.
 *  Encrypted data size must be multiple of 16.
 *  If the pads is true, encrypted data is padded with PKCS.
 *  So the data is multiple of 16, encrypted data size needs additonal 16 bytes.
 * @since 1.0.0
 */
inline Error encrypt_cbc(
    const unsigned char * data,
    const unsigned long data_size,
    const unsigned char * key,
    const unsigned long key_size,
    const unsigned char (* iv)[16],
    unsigned char * encrypted,
    const unsigned long encrypted_size,
    const bool pads
    ) {
    const Error e = detail::check_encrypt_cond(data_size, key_size, encrypted_size, pads);
    if (e != kErrorOk) {
        return e;
    }

    const detail::RoundKeys rkeys = detail::expand_key(key, static_cast<int>(key_size));

    unsigned char s[detail::kStateSize] = {}; // encrypting data

    // calculate padding value
    const bool ge16 = (data_size >= detail::kStateSize);
    const int rem = data_size % detail::kStateSize;
    const unsigned char pad_v = detail::kStateSize - rem;

    // encrypt 1st state
    if (ge16) {
        memcpy(s, data, detail::kStateSize);
    }
    else {
        memset(s, pad_v, detail::kStateSize);
        memcpy(s, data, data_size);
    }
    if (iv) {
        detail::xor_data(s, *iv);
    }
    detail::encrypt_state(rkeys, s, encrypted);

    // encrypt mid
    const unsigned long bc = data_size / detail::kStateSize;
    for (unsigned long i = 1; i < bc; ++i) {
        const long offset = i * detail::kStateSize;
        memcpy(s, data + offset, detail::kStateSize);
        detail::xor_data(s, encrypted + offset - detail::kStateSize);

        detail::encrypt_state(rkeys, s, encrypted + offset);
    }

    // enctypt last
    if (pads && ge16) {
        std::vector<unsigned char> ib(detail::kStateSize, pad_v), ob(detail::kStateSize);
        memcpy(&ib[0], data + data_size - rem, rem);

        detail::xor_data(&ib[0], encrypted + (bc - 1) * detail::kStateSize);

        detail::encrypt_state(rkeys, &ib[0], &ob[0]);
        memcpy(encrypted + (data_size - rem), &ob[0], detail::kStateSize);
    }

    return kErrorOk;
}

/**
 * Decrypt data with CBC mode.
 * @param [in]  data Data bytes.
 * @param [in]  data_size Data size.
 * @param [in]  key Key bytes.
 * @param [in]  key_size Key size.
 * @param [in]  iv Initialize vector.
 * @param [out] decrypted Decrypted data buffer.
 * @param [in]  decrypted_size Decrypted data buffer size.
 * @param [out] padded_size If this value is NULL, this function does not remove padding.
 *  If this value is not NULL, this function removes padding by PKCS
 *  and returns padded size using padded_size.
 * @since 1.0.0
 */
inline Error decrypt_cbc(
    const unsigned char * data,
    const unsigned long data_size,
    const unsigned char * key,
    const unsigned long key_size,
    const unsigned char (* iv)[16],
    unsigned char * decrypted,
    const unsigned long decrypted_size,
    unsigned long * padded_size
    ) {
    const Error e = detail::check_decrypt_cond(data_size, key_size, decrypted_size, padded_size);
    if (e != kErrorOk) {
        return e;
    }

    const detail::RoundKeys rkeys = detail::expand_key(key, static_cast<int>(key_size));

    // decrypt 1st state
    detail::decrypt_state(rkeys, data, decrypted);
    if (iv) {
        detail::xor_data(decrypted, *iv);
    }

    // decrypt mid
    const unsigned long bc = data_size / detail::kStateSize - 1;
    for (unsigned long i = 1; i < bc; ++i) {
        const long offset = i * detail::kStateSize;
        detail::decrypt_state(rkeys, data + offset, decrypted + offset);
        detail::xor_data(decrypted + offset, data + offset - detail::kStateSize);
    }

    // decrypt last
    unsigned char last[detail::kStateSize] = {};
    if (data_size > detail::kStateSize) {
        detail::decrypt_state(rkeys, data + (bc * detail::kStateSize), last);
        detail::xor_data(last, data + (bc * detail::kStateSize - detail::kStateSize));
    }
    else {
        memcpy(last, decrypted, data_size);
        memset(decrypted, 0, decrypted_size);
    }

    if (padded_size) {
        *padded_size = last[detail::kStateSize - 1];
        const unsigned long cs = detail::kStateSize - *padded_size;

        if (!detail::check_padding(*padded_size, last)) {
            return kErrorInvalidKey;
        }
        else if (decrypted_size >= (bc * detail::kStateSize) + cs) {
            memcpy(decrypted + (bc * detail::kStateSize), last, cs);
        }
        else {
            return kErrorInvalidBufferSize;
        }
    }
    else {
        memcpy(decrypted + (bc * detail::kStateSize), last, sizeof(last));
    }

    return kErrorOk;
}

/**
 * @note
 * This is BETA API. I might change API in the future.
 *
 * Encrypts or decrypt data in-place with CTR mode.
 * @param [in,out]  data Data.
 * @param [in,out]  data_size Data size.
 * @param [in]  key key bytes. The key length must be 16 (128-bit), 24 (192-bit) or 32 (256-bit).
 * @param [in]  key_size key size.
 * @param [in]  nonce 16 bytes.
 * @since 1.0.0
 */
inline Error crypt_ctr(
    unsigned char *data,
    unsigned long data_size,
    const unsigned char *key,
    const unsigned long key_size,
    const unsigned char *nonce,
    const unsigned long nonce_size
) {
    if (nonce_size > detail::kStateSize) return kErrorInvalidNonceSize;
    if (!detail::is_valid_key_size(key_size)) return kErrorInvalidKeySize;
    const detail::RoundKeys rkeys = detail::expand_key(key, static_cast<int>(key_size));

    unsigned long pos = 0;
    unsigned long blkpos = detail::kStateSize;
    unsigned char blk[detail::kStateSize];
    unsigned char counter[detail::kStateSize] = {};
    memcpy(counter, nonce, nonce_size);

    while (pos < data_size) {
        if (blkpos == detail::kStateSize) {
            detail::encrypt_state(rkeys, counter, blk);
            detail::incr_counter(counter);
            blkpos = 0;
        }
        data[pos++] ^= blk[blkpos++];
    }

    return kErrorOk;
}

} // namespace plusaes

#endif // PLUSAES_HPP__
