#pragma once

struct lv_obj_t;
using ScreenLayer = lv_obj_t*;


class Screen {
public:
    Screen();

    /**
     * @brief 获取单例对象
    */
    static Screen* GetInstance();

    /**
     * @brief 获取屏幕bottom层
    */
    ScreenLayer GetBottomLayer();

    /**
     * @brief 获取屏幕active层
    */
    ScreenLayer GetActiveLayer();
    
    /**
     * @brief 获取屏幕top层
    */
    ScreenLayer GetTopLayer();

    /**
     * @brief 获取屏幕system层
    */
    ScreenLayer GetSystemLayer();
};

#define SCREEN_MGR Screen::GetInstance()
