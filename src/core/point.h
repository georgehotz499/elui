#pragma once

class Point {
public:
    Point() {}
    Point(int x, int y):m_x(x), m_y(y) {}

    Point& operator=(const Point& other) {
        // 赋值逻辑
        this->m_x = other.m_x;
        this->m_y = other.m_y;
        return *this;  // 返回自身引用，支持链式赋值
    }

    // == 重载：判断两个点是否相等
    bool operator==(const Point& other) const {
        return (this->m_x == other.m_x) && (this->m_y == other.m_y);
    }

    // != 重载：判断两个点是否不相等
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }

public:
    int m_x{ 0 };
    int m_y{ 0 };
};
