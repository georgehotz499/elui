#include "base64.h"

#include <array>
#include <cctype>

namespace {
constexpr char kEncodeTable[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int DecodeValue(unsigned char ch) {
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A';
    }
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + 26;
    }
    if (ch >= '0' && ch <= '9') {
        return ch - '0' + 52;
    }
    if (ch == '+') {
        return 62;
    }
    if (ch == '/') {
        return 63;
    }
    return -1;
}
}

std::string base64_encode(const char* data, size_t length) {
    if (!data || length == 0) {
        return {};
    }

    std::string output;
    output.reserve(((length + 2) / 3) * 4);

    for (size_t i = 0; i < length; i += 3) {
        const unsigned int octet_a = static_cast<unsigned char>(data[i]);
        const unsigned int octet_b = (i + 1 < length) ? static_cast<unsigned char>(data[i + 1]) : 0;
        const unsigned int octet_c = (i + 2 < length) ? static_cast<unsigned char>(data[i + 2]) : 0;
        const unsigned int triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        output.push_back(kEncodeTable[(triple >> 18) & 0x3f]);
        output.push_back(kEncodeTable[(triple >> 12) & 0x3f]);
        output.push_back((i + 1 < length) ? kEncodeTable[(triple >> 6) & 0x3f] : '=');
        output.push_back((i + 2 < length) ? kEncodeTable[triple & 0x3f] : '=');
    }

    return output;
}

std::string base64_decode(const std::string& input) {
    std::string output;
    output.reserve((input.size() / 4) * 3);

    std::array<int, 4> block{};
    int count = 0;
    int padding = 0;
    bool saw_padding = false;

    for (unsigned char ch : input) {
        if (std::isspace(ch)) {
            continue;
        }

        if (ch == '=') {
            saw_padding = true;
            block[count++] = 0;
            ++padding;
        }
        else {
            const int value = DecodeValue(ch);
            if (value < 0 || saw_padding) {
                return {};
            }
            block[count++] = value;
        }

        if (count == 4) {
            if (padding > 2) {
                return {};
            }

            output.push_back(static_cast<char>((block[0] << 2) | (block[1] >> 4)));
            if (padding < 2) {
                output.push_back(static_cast<char>(((block[1] & 0x0f) << 4) | (block[2] >> 2)));
            }
            if (padding < 1) {
                output.push_back(static_cast<char>(((block[2] & 0x03) << 6) | block[3]));
            }

            count = 0;
            padding = 0;
            saw_padding = false;
        }
    }

    if (count != 0) {
        return {};
    }

    return output;
}
