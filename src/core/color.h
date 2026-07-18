#pragma once
#include <stdint.h>


class Color {
public:
    Color() {};

    Color(int color) {
        m_a = color >> 24;
        m_r = color >> 16;
        m_g = color >> 8;
        m_b = color;
    };

    Color(uint8_t r, uint8_t g, uint8_t b) {
        m_r = r;
        m_g = g;
        m_b = b;
    };

    Color(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
        m_a = a;
        m_r = r;
        m_g = g;
        m_b = b;
    };

public:
    uint8_t m_a{ 0XFF };
    uint8_t m_r{ 0X00 };
    uint8_t m_g{ 0X00 };
    uint8_t m_b{ 0X00 };
};
