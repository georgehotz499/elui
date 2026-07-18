#include "template_logic.h"

#include "core/resources.h"
#include "core/image_ins.h"


#define DEBUG LOGD("DEBUG!!");

namespace {
    /**
    * @brief 缓存图片
    */
    static void CacheImage() {
        ImageIns::CacheImage(Resources::GetDataPath() + "01.png");
        ImageIns::CacheImage(Resources::GetDataPath() + "02.png");
        ImageIns::CacheImage(Resources::GetDataPath() + "03.png");
    }
}

// 定时器列表
static const Object::Timer timer_list[] = {
    {0, 50}, 
    {1, 10},
};


TemplateLogic::TemplateLogic(std::string name, Widget* parent, ScreenLayer layer) {
    LOGI("%s surface layer:%p", name.c_str(), layer);
    // 构建UI对象
    ui = new TemplateSurface(name, parent, layer);
    // 注册定时器回调
    ui->AddTimeoutCallback(std::bind(&TemplateLogic::TimerCallback, this, std::placeholders::_1));
    // 启动定时器
    ui->InitTimer(timer_list, sizeof(timer_list)/sizeof(Object::Timer));
    // 绑定控件回调函数
    ui->Connect(this);

    // 初始化UI数据
    InitUi();
}

TemplateLogic::~TemplateLogic() {
    // 注销定时器回调
    ui->AddTimeoutCallback(nullptr);
    // 析构控件对象
    ui->Destroy();
}

void TemplateLogic::InitUi() {
    // 缓存图片
    CacheImage();

    ui->ButtonPtr->SetImageNormal(Resources::GetDataPath() + "01.png");
    ui->ButtonPtr->SetImageNormalPress(Resources::GetDataPath() + "02.png");
    ui->ButtonPtr->SetImageClicked(Resources::GetDataPath() + "02.png");
    ui->ButtonPtr->SetImageClickedPress(Resources::GetDataPath() + "01.png");
    ui->ButtonPtr->SetImageDisable(Resources::GetDataPath() + "03.png");
    ui->ButtonPtr->SetText("这是按键");
    
    // 设置背景图片
    ui->SliderHorPtr->SetImageMain(Resources::GetDataPath() + "image_main_hor.png");
    // 设置进度条图片
    ui->SliderHorPtr->SetImageIndicator(Resources::GetDataPath() + "image_indicator_hor.png");
    // 设置knob图片
    ui->SliderHorPtr->SetImageKnob(Resources::GetDataPath() + "image_knob.png");

    // 设置背景图片
    ui->SliderVerPtr->SetImageMain(Resources::GetDataPath() + "image_main_ver.png");
    // 设置进度条图片
    ui->SliderVerPtr->SetImageIndicator(Resources::GetDataPath() + "image_indicator_ver.png");
    // 设置knob图片
    ui->SliderVerPtr->SetImageKnob(Resources::GetDataPath() + "image_knob.png");

    // 设置背景图片
    ui->ArcPtr->SetImageMain(Resources::GetDataPath() + "arc_main.jpg");
    // 设置进度条图片
    ui->ArcPtr->SetImageIndicator(Resources::GetDataPath() + "arc_indicator.jpg");
    // 设置knob图片
    ui->ArcPtr->SetImageKnob(Resources::GetDataPath() + "image_knob.png");
    
    // 设置光晕宽度
    ui->LedPtr->SetShadowWidth(5);
    // 设置光晕扩散
    ui->LedPtr->SetShadowSpread(2);
    // 设置颜色
    ui->LedPtr->SetColor(0XFFFF00);

    // 设置单行输入框占位文本
    ui->LineinputPtr->SetPlaceholderText("请输入中英文或符号");
    // 设置单行输入框文本
    ui->LineinputPtr->SetText("测试ABC123!");
    // 清除可输入字符限制
    ui->LineinputPtr->ClearAcceptedChars();
    // 设置最大输入长度
    ui->LineinputPtr->SetMaxLength(64);
    // 设置文本颜色
    ui->LineinputPtr->SetTextColor(Color(0X22, 0X33, 0X44));
    // 设置占位文本颜色
    ui->LineinputPtr->SetPlaceholderColor(Color(0X88, 0X88, 0X88));
    // 设置边框颜色
    ui->LineinputPtr->SetBorderColor(Color(0X00, 0XA8, 0X6B));
    // 设置边框宽度
    ui->LineinputPtr->SetBorderWidth(2);
    // 设置背景不透明度
    ui->LineinputPtr->SetBgOpacity(85);

    // 设置背景弧图片
    ui->DashboardPtr->SetImageMain(Resources::GetDataPath() + "dashbord_main.png");
    // 设置前景弧图片
    ui->DashboardPtr->SetImageIndicator(Resources::GetDataPath() + "dashbord_indicator.png");
    // 设置指针图片
    ui->DashboardPtr->SetImagePointer(Resources::GetDataPath() + "dashbord_pointer.png");
    // 设置起始角度
    ui->DashboardPtr->SetStartAngle(125);
    // 设置可转动角度
    ui->DashboardPtr->SetRangeAngle(300);

    // 设置Gif图片
    ui->GifPtr->SetImage(Resources::GetDataPath() + "xiangou.gif");
}

bool TemplateLogic::TimerCallback(int timer_id) {
    switch (timer_id) {
    // 修改仪表盘进度
    case 0:
    {
        // 旋转方向(true:顺时针 false:逆时针)
        static bool diraction = true;

        static int value = 0;
        if (diraction) {
            ++value;
        }
        else {
            --value;
        }
        ui->DashboardPtr->SetValue(value);

        diraction = (100 == value) ? false : (0 == value) ? true : diraction;
    }
    break;
    // 修改LED灯亮度
    case 1:
    {
        // led亮暗状态(true:变亮 false:变暗)
        static bool status{ true };

        static int brightness{ 0 };
        if (status) {
            brightness += 1;
        }
        else {
            brightness -= 1;
        }
        ui->LedPtr->SetBrightness(brightness);

        status = (100 == brightness) ? false : (0 == brightness) ? true : status;
    }
    break;
    default:
        LOGW("Unsolved timer %d", timer_id);
        break;
    }
    return false;
}

void TemplateLogic::ButtonClicked(Button* btn) {
    LOGI("button %s clicked!!", btn->GetName().c_str());
}

int TemplateLogic::ListItemCount(void) {
    return 21;
}

void TemplateLogic::ListItemUpdate(Button* item, int index) {
    //LOGI("List index %d update!!", index);
    char tmp[32]{ 0 };
    snprintf(tmp, sizeof(tmp), "item %d", index);
    item->SetText(tmp);

    // 设置Button1Ptr背景图片
    item->SetImageNormal(Resources::GetDataPath() + "01.png");
    item->SetImageNormalPress(Resources::GetDataPath() + "02.png");
    item->SetImageClicked(Resources::GetDataPath() + "02.png");
    item->SetImageClickedPress(Resources::GetDataPath() + "01.png");
    item->SetImageDisable(Resources::GetDataPath() + "03.png");
}

void TemplateLogic::ListItemClicked(Button* item, int index) {
    LOGI("list item index %d clicked!!", index);
}

void TemplateLogic::SliderProgressNotify(Slider* slider, int progress) {
    LOGI("%s progress is %d", slider->GetName().c_str(), progress);
}

void TemplateLogic::ArcProgressNotify(class Arc* arc, int progress) {
    LOGI("%s progress is %d", arc->GetName().c_str(), progress);
}

void TemplateLogic::LineinputTextChanged(class Lineinput* input, const std::string& text) {
    LOGI("%s text:%s", input->GetName().c_str(), text.c_str());
}

void TemplateLogic::Hide() {
    ui->Hide();
}

void TemplateLogic::Show() {
    ui->Show();
}
