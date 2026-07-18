#pragma once
#include "widget/page.h"
#include "widget/label.h"
#include "widget/image.h"
#include "widget/qrcode.h"
#include "widget/barcode.h"
#include "widget/canvas.h"
#include "widget/gif.h"
#include "widget/button.h"
#include "widget/list.h"
#include "widget/led.h"
#include "widget/slider.h"
#include "widget/arc.h"
#include "widget/dashboard.h"
#include "widget/lineinput.h"

#include "core/log.h"


class TemplateSurface : public Page {
public:
    TemplateSurface(std::string name, Widget* parent, ScreenLayer layer);
    virtual ~TemplateSurface();

    /**
     * @brief 创建ui控件时执行
     */
    void CreateWidget();

    /**
     * @brief 显示页面时执行
     */
    void Show();

    /**
     * @brief 页面隐藏时执行
     */
    void Hide();

    /**
     * @brief 页面关闭退出时执行
     */
    void Quit();

    /**
     * @brief 启动定时器
     * @param timer_list 定时器数组
     * @param count 定时器个数
     */
    void InitTimer(const Object::Timer timer_list[], int count);

    /**
    * @brief 绑定控件回调函数
    * @param logic xxxLogic 类指针
    */
    void Connect(void* logic);

public:
    // 按键控件
    Button* ButtonPtr{ nullptr };
    // 列表控件
    List* ListPtr{ nullptr };
    // 横向滑条
    Slider* SliderHorPtr{ nullptr };
    // 纵向滑条
    Slider* SliderVerPtr{ nullptr };
    // 弧形滑条
    class Arc* ArcPtr{ nullptr };
    // LED控件
    Led* LedPtr{ nullptr };
    // 单行输入框
    Lineinput* LineinputPtr{ nullptr };
    // 仪表盘
    Dashboard* DashboardPtr{ nullptr };
    // GIP动图
    Gif* GifPtr{ nullptr };
};
