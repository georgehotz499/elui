#pragma once
#include "page.h"

#include "core/screen.h"

#include <map>


class Layer {
public:
    // UI层级结构，由低到高
    enum Level{kLow = 0, kMedium, kHeight, kTop};

public:
    /**
     * @brief 添加页面控件到图层管理
     * @param page 页面指针
     * @param level 所在层级,默认处于最底层
     */
    void AddPage(Page* page, Level level = kLow);

    /**
     * @brief 从页面图层管理中移除控件
     * @param page 页面指针
     * @param level 所在层级,默认处于最底层
     */
    void RemovePage(Page* page, Level level);

    /**
     * @brief 将控件移动到父控件容器的顶层
     * @param page 页面指针
     */
    void MoveToTop(Page* page);

    /**
     * @brief 将控件移动到父控件容器的顶层
     * @param page 页面指针
     */
    void MoveToBottom(Page* page);

    /**
     * @brief 移动页面所在图层
     * @param page 页面指针
     * @param level 移动到的图层
     */
    void MoveLayer(Page* page, Level level);

    /**
     * @brief 更新图层层级
     */
    void UpdateLayer();

private:
    /**
    * @brief 调整图层
    * @param layer_vec 图层信息
    * @param level 图层层级
    */
    void AdjustLayer(const std::map<Level, std::list<Page*>>& layer_vec, Level level);

    /**
     * @brief 将控件移动到父控件容器的顶层
     * @param object 控件对象图层
     */
    void MoveToTop(ScreenLayer object);

    /**
     * @brief 将控件移动到父控件容器的顶层
     * @param object 控件对象图层
     */
    void MoveToBottom(ScreenLayer object);

private:
    // 键值关系为：层级->各控件
    std::map<Level, std::list<Page*>> m_layers;
};

class LayerManager : public Layer {
public:
    static LayerManager* GetInstance();
};
#define LAYER_MGR LayerManager::GetInstance()
