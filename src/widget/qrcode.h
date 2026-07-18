#pragma once
#include "widget.h"
#include "core/color.h"


class QrCode : public Widget {
public:
    QrCode(std::string name, Widget* parent);
    ~QrCode();
private:
    /**
    * @brief 创建对象
    */
    void Create(ScreenLayer layer) override;

public:
    /**
    * @brief 设置二维码宽高
    * @param size 宽度以及高度
    */
    void SetSize(int size);

    /**
    * @brief 设置二维码颜色
    */
    void SetQrcodeColor(Color color);

    /**
    * @brief 设置底部背景颜色
    */
    void SetQrBgColor(Color color);

    /**
    * @brief 设置二维码显示内容
    */
    void SetContent(std::string str);

    /**
    * @brief 设置二维码几何属性
    */
    void SetGeometry(int x, int y, int w, int h);
};
