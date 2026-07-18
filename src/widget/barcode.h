#pragma once
#include "widget.h"
#include "core/color.h"


class BarCode : public Widget {
public:
    BarCode(std::string name, Widget* parent);
    ~BarCode();

private:
    /**
    * @brief 创建对象
    */
    void Create(ScreenLayer layer) override;

public:
    /**
    * @brief 设置条形码颜色
    */
    void SetQrcodeColor(Color color);

    /**
    * @brief 设置底部背景颜色
    */
    void SetQrBgColor(Color color);

    /**
    * @brief 设置条形码显示内容
    */
    void SetContent(std::string str);

    /**
    * @brief 设置条形码放大倍数
    * @param scale 放大倍数（默认值为1）
    */
    void SetScale(uint16_t scale);

    /**
    * @brief 设置条形码显示方向（在线验证：https://www.jsbarcode.com/barcodeDecode?ref=67tool.com）
    * @param direction 0:横向；1:纵向
    */
    void SetDirection(int direction);

    /**
    * @brief 获取宽度
    */
    int GetWidth();

    /**
    * @brief 获取高度
    */
    int GetHeight();
};

