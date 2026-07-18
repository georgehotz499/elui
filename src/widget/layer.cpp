#include "layer.h"

#include "core/log.h"

extern "C" {
#include "lvgl/lvgl.h"
}


void Layer::AddPage(Page* page, Level level) {
    // 查找层级信息
    auto layer = m_layers.find(level);
    // 未查找到层级信息
    if (m_layers.end() == layer) {
        std::list<Page*> list_obj;
        // 插入对象
        list_obj.push_back(page);
        // 将对象插入所在层级
        m_layers[level] = list_obj;
    }
    // 查找到层级信息
    else {
        // 插入对象
        layer->second.push_back(page);
    }
}

void Layer::RemovePage(Page* page, Level level) {
    // 查找层级信息
    auto layer = m_layers.find(level);
    // 未查找到层级信息
    if (m_layers.end() == layer) {
        LOGE("Failed to find layer %d", level);
        return;
    }

    for (auto iter : layer->second) {
        LOGI("widget name %s", iter->GetName().c_str());
    }
    // 移除控件
    layer->second.remove(page);
    for (auto iter : layer->second) {
        LOGI("widget name %s", iter->GetName().c_str());
    }
}

void Layer::MoveToTop(Page* page) {
    // 获取层级
    const Level level = static_cast<Level>(page->GetLevel());
    // 移除控件
    RemovePage(page, level);
    // 再次添加控件
    AddPage(page, level);
}

void Layer::MoveToBottom(Page* page) {
    // 获取层级
    const Level level = static_cast<Level>(page->GetLevel());
    // 移除控件
    RemovePage(page, level);

    // 查找层级信息
    auto layer = m_layers.find(level);
    // 未查找到层级信息
    if (m_layers.end() == layer) {
        LOGE("Failed to find level %d", static_cast<int>(level));
        return;
    }
    // 查找到层级信息
    else {
        // 插入对象
        layer->second.push_front(page);
    }
}

void Layer::MoveLayer(Page* page, Level level) {
    // 获取层级
    const Level cur_level = static_cast<Level>(page->GetLevel());
    // 移除控件
    RemovePage(page, cur_level);
    // 再次添加控件
    AddPage(page, level);
}

void Layer::UpdateLayer() {
    // 调整kLow层控件
    AdjustLayer(m_layers, kLow);
    // 调整kMedium层控件
    AdjustLayer(m_layers, kMedium);
    // 调整kHeight层控件
    AdjustLayer(m_layers, kHeight);
    // 调整kTop层控件
    AdjustLayer(m_layers, kTop);
}

void Layer::AdjustLayer(const std::map<Level, std::list<Page*>>& layer_vec, Level level) {
    // 查找图层信息
    auto layer = layer_vec.find(level);
    if (layer_vec.end() != layer) {
        LOGI("level %d size:%d", static_cast<int>(level), layer->second.size());
        for (auto iter : layer->second) {
            LOGI("widget name %s", iter->GetName().c_str());
        }
    }
    // 查找到图层信息
    if (layer_vec.end() != layer) {
        for (auto page : layer->second) {
            auto widgets = page->GetWidgets();
            // 先调整page层级
            MoveToTop(page->GetLayer());
            for (auto widget : widgets) {
                // 调整page子控件层级
                MoveToTop(widget->GetLayer());
            }
        }
    }
    LOGI("Adjust layer had done.");
}

void Layer::MoveToTop(ScreenLayer object) {
    if (object == NULL) return;

    ScreenLayer parent = lv_obj_get_parent(object);
    if (parent == NULL) return; // 根对象（如屏幕）没有父对象，无需移动

    uint32_t child_count = lv_obj_get_child_count(parent);
    if (child_count <= 1) return; // 只有一个子对象时，无需移动

    // 移动到最后一个位置（最顶层）
    lv_obj_move_to_index(object, child_count - 1);
}

void Layer::MoveToBottom(ScreenLayer object) {
    if (object == NULL) return;

    ScreenLayer parent = lv_obj_get_parent(object);
    if (parent == NULL) return;

    uint32_t child_count = lv_obj_get_child_count(parent);
    if (child_count <= 1) return; // 只有一个子对象时，无需移动

    // 移动到第一个位置（最底层）
    lv_obj_move_to_index(object, 0);
}

LayerManager* LayerManager::GetInstance() {
    static LayerManager ins;
    return &ins;
}
