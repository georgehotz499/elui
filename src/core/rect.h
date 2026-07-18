#pragma once

class Rect {
public:
    Rect() {};
    Rect(int x, int y, int w, int h)
        :m_x(x), m_y(y), m_width(w), m_height(h){};

    Rect& operator=(const Rect& other) {
        // 赋值逻辑
        this->m_x = other.m_x;
        this->m_y = other.m_y;
        this->m_width = other.m_width;
        this->m_height = other.m_height;
        return *this;  // 返回自身引用，支持链式赋值
    }

    bool operator==(const Rect& other) const {
        // 比较逻辑：返回 true 表示相等，false 表示不等
        return (this->m_x == other.m_x && this->m_y == other.m_y &&
            this->m_width == other.m_width && this->m_height == other.m_height);
    }

    bool operator!=(const Rect& other) const {
        // 比较逻辑：返回 true 表示相等，false 表示不等
        return !(*this == other);
    }

public:
    int m_x{0};
    int m_y{0};
    int m_width{0};
    int m_height{0};
};
