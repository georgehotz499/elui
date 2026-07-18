#pragma once

class Size {
public:
    Size() {};
    Size(int w, int h):m_width(w), m_height(h){};

    Size& operator=(const Size& other) {
        // 赋值逻辑
        this->m_width = other.m_width;
        this->m_height = other.m_height;
        return *this;  // 返回自身引用，支持链式赋值
    }

    bool operator==(const Size& other) const {
        // 比较逻辑：返回 true 表示相等，false 表示不等
        return (this->m_width == other.m_width && this->m_height == other.m_height);
    }

    bool operator!=(const Size& other) const {
        // 比较逻辑：返回 true 表示相等，false 表示不等
        return !(*this == other);
    }

public:
    int m_width{0};
    int m_height{0};
};
