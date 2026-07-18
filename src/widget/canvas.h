#pragma once
#include "widget.h"


class Canvas : public Widget {
public:
    Canvas(std::string name, Widget* parent);
    ~Canvas();

private:
    /**
    * @brief 创建对象
    */
    void Create(ScreenLayer layer) override;

public:
    /**
    * @brief 设置几何属性
    */
    void SetGeometry(int x, int y, int w, int h);

    /**
    * @brief 清空画布
    */
    void Clear();

    /**
     * @brief 绘制矩形
     * @param x 坐标
     * @param y 坐标
     * @param w 宽度
     * @param h 高度
     * @param radius 圆角半径
     */
    void DrawRect(int x, int y, int w, int h, uint32_t color, int radius = 0);

    /**
     * @brief 绘制弧形
     * @param center_x 中心x坐标
     * @param center_y 中心y坐标
     * @param border_width 弧的宽度
     * @param start_angle 起始角度（默认顺时针方向）
     * @param end_angle 停止角度（默认顺时针方向）
     * @param color 颜色
     */
    void DrawArc(int center_x, int center_y, int radius, int border_width, int start_angle, int end_angle, uint32_t color);

    /**
     * @brief 绘制线段
     * @param x1 起点x坐标
     * @param y1 起点y坐标
     * @param x2 终点x坐标
     * @param y2重点y坐标
     * @param width 线段宽度
     * @param color 颜色
     * @param radius 圆角
     */
    void DrawLine(int x1, int y1, int x2, int y2, int width, uint32_t color, int radius = 1);

private:
    // 图像绘制buffer
    uint8_t* m_canvas_buf{ nullptr };
};
