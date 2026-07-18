#include "template_surface.h"

#include "logic/template_logic.h"


TemplateSurface::TemplateSurface(std::string name, Widget* parent, ScreenLayer layer) :Page(name, parent, layer) {
    // 构建页面以及控件
    CreateWidget();
    // 显示页面
    Show();
}

TemplateSurface::~TemplateSurface() {
    Quit();
}

void TemplateSurface::CreateWidget() {
    // 设置页面大小
    SetGeometry(0, 0, 1280, 800);
    // 设置页面不透明度
    SetBgOpacity(100);
    // 设置页面背景色为白色
    SetBgColor(Color(0XFF, 0XFF, 0XFF));

    ButtonPtr = new Button("ButtonPtr", this);
    ButtonPtr->SetGeometry(80, 80, 150, 60);

    ListPtr = new List("ListPtr", this);
    ListPtr->SetGeometry(400, 80, 320, 210);
    // 设置行列数
    ListPtr->SetCell(3, 2);
    // 设置行间距
    ListPtr->SetRowSpan(10);
    // 设置列间距
    ListPtr->SetColumnSpan(10);
    // 设置滑动方向
    ListPtr->SetScrollDir(List::kHor);
    ListPtr->SetBgColor(Color(0XEE, 0XCC, 0XCC));
    // 设置是否循环滚动
    ListPtr->SetCycle(true);
    // 获取额外行列数量
    const int ListPtrExtra = List::kHor == ListPtr->GetScrollDir() ? ListPtr->GetRowCount() : ListPtr->GetColumnCount();
    Button* ListPtrItem{ nullptr };
    for (int level1 = 0; level1 < ListPtr->GetColumnCount() * ListPtr->GetRowCount() + ListPtrExtra; ++level1) {
        ListPtrItem = new Button("ItemPtr", ListPtr);
        ListPtrItem->SetTouchable(false);
        ListPtr->AddItem(ListPtrItem, level1);
    }

    SliderHorPtr = new Slider("SliderHorPtr", this);
    // 设置圆角
    SliderHorPtr->SetRadius(10);
    // 设置内边距
    SliderHorPtr->SetPadding(5);
    SliderHorPtr->SetGeometry(80, 200, 200, 30);

    SliderVerPtr = new Slider("SliderVerPtr", this);
    // 设置圆角
    SliderVerPtr->SetRadius(10);
    // 设置内边距
    SliderVerPtr->SetPadding(5);
    // 设置方向垂直
    SliderVerPtr->SetOrientation(Slider::kVer);
    SliderVerPtr->SetGeometry(300, 80, 30, 200);

    ArcPtr = new Arc("ArcPtr", this);
    // 设置内边距
    ArcPtr->SetPadding(5);
    // 设置不可触摸区域
    ArcPtr->SetUntouchableRadius(70);
    ArcPtr->SetGeometry(80, 250, 200, 200);

    LedPtr = new Led("LedPtr", this);
    LedPtr->SetGeometry(400, 320, 70, 70);

    LineinputPtr = new Lineinput("LineinputPtr", this);
    LineinputPtr->SetGeometry(80, 500, 240, 60);

    DashboardPtr = new Dashboard("DashboardPtr", this);
    DashboardPtr->SetGeometry(450, 330, 260, 260);
    DashboardPtr->SetValue(72);

    GifPtr = new Gif("GifPtr", this);
    GifPtr->SetGeometry(750, 0, 240, 240);
}

void TemplateSurface::Show() {
    Page::Show();
}

void TemplateSurface::Hide() {
    Page::Hide();
}

void TemplateSurface::Quit() {

}

void TemplateSurface::InitTimer(const Object::Timer timer_list[], int count) {
    for (int level1 = 0; level1 < count; ++level1) {
        StartTimer(timer_list[level1].m_timer_id, timer_list[level1].m_interval);
    }
}

void TemplateSurface::Connect(void* logic) {
    // 注册单击回调
    ButtonPtr->AddClickedCallback(std::bind(&TemplateLogic::ButtonClicked, (TemplateLogic*)logic, std::placeholders::_1));

    // 注册list获取子项数量回调
    ListPtr->AddGetListitemCountCallback(std::bind(&TemplateLogic::ListItemCount, (TemplateLogic*)logic));
    // 注册list子项刷新回调
    ListPtr->AddListUpdateCallback(std::bind(&TemplateLogic::ListItemUpdate, (TemplateLogic*)logic, std::placeholders::_1, std::placeholders::_2));
    // 注册list点击子项回调
    ListPtr->AddListitemClickedCallback(std::bind(&TemplateLogic::ListItemClicked, (TemplateLogic*)logic, std::placeholders::_1, std::placeholders::_2));
    // 刷新控件
    ListPtr->Refresh();

    // 注册滑动回调函数
    SliderHorPtr->AddProgressCallback(std::bind(&TemplateLogic::SliderProgressNotify, (TemplateLogic*)logic, std::placeholders::_1, std::placeholders::_2));

    // 注册滑动回调函数
    SliderVerPtr->AddProgressCallback(std::bind(&TemplateLogic::SliderProgressNotify, (TemplateLogic*)logic, std::placeholders::_1, std::placeholders::_2));

    // 注册滑动回调函数
    ArcPtr->AddProgressCallback(std::bind(&TemplateLogic::ArcProgressNotify, (TemplateLogic*)logic, std::placeholders::_1, std::placeholders::_2));

    // 注册文本变化回调
    LineinputPtr->AddTextChangedCallback(std::bind(&TemplateLogic::LineinputTextChanged, (TemplateLogic*)logic, std::placeholders::_1, std::placeholders::_2));
}
