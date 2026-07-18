# elui

elui 是一个基于 LVGL 9 的 C++ UI 封装和 Windows 示例工程。`src/widget`
把常用 LVGL 控件包装成较统一的 C++ 接口，`src/core` 提供日志、资源、图片、
JSON、线程、HTTP、MQTT 和 Socket 等基础能力。工程采用 UI（Surface）与业务逻辑
（Logic）分离的方式组织页面。

## 目录结构

```text
.
|-- main.cpp                 # 初始化 LVGL Windows 显示和事件循环
|-- src/
|   |-- elui.cpp             # elui 入口
|   |-- core/                # 基础功能
|   |-- widget/              # LVGL C++ 控件封装
|   |-- surface/             # 页面布局和控件创建
|   `-- logic/               # 事件处理和业务逻辑
|-- lvgl/                    # LVGL 头文件、配置和源码
|-- third_party/msvc/x64/    # x64 第三方头文件、库和运行时 DLL
`-- licenses/                # 第三方许可证
```

程序的启动顺序为：

```text
lv_init()
  -> lv_windows_create_display()
  -> EluiEntry()
  -> MainLogic / TemplateLogic
  -> TemplateSurface
  -> lv_timer_handler() 循环
```

## 构建和运行

环境要求：

- Windows 10/11
- Visual Studio 2022，安装“使用 C++ 的桌面开发”工作负载
- MSVC v143 和 Windows 10 SDK
- 使用 `x64` 配置；当前 `Win32` 配置没有完整的第三方依赖

用 Visual Studio 打开 `elui.sln`，选择 `Debug | x64` 或 `Release | x64` 后构建。
也可以在 Visual Studio Developer PowerShell 中运行：

```powershell
msbuild .\elui.sln /m /p:Configuration=Debug /p:Platform=x64
cd .\x64\Debug
.\elui.exe
```

运行时资源使用相对路径，因此建议从可执行文件目录启动。默认目录约定如下：

```text
x64/Debug/
|-- elui.exe
|-- curl-ca-bundle.crt       # HTTPS CA 证书
|-- data/                    # 页面图片和 GIF
`-- res/ttf/2312_v9.ttf     # 默认字体
```

`Resources::GetDataPath()` 返回 `./data/`，`Resources::GetFontPath()` 返回
`./res/ttf/2312_v9.ttf`。新增资源时需要保证它们存在于程序的工作目录中。

## 控件模型

`Object` 提供父子关系和定时器，`Widget` 在其上增加位置、尺寸、样式、显示状态和
触摸能力。`Page` 是顶层页面容器，其他控件通常都需要传入一个非空的
`Widget* parent`。子控件的位置相对于父控件。

所有控件共有的常用接口包括：

```cpp
widget->SetGeometry(x, y, width, height);
widget->SetBgColor(Color(0x20, 0x24, 0x2A));
widget->SetBgOpacity(100);  // 0~100
widget->SetRadius(8);
widget->SetTouchable(true);
widget->Show();
widget->Hide();
```

### 创建一个简单页面

```cpp
#include "widget/page.h"
#include "widget/label.h"
#include "widget/button.h"
#include "widget/slider.h"
#include "core/log.h"

class DemoPage : public Page {
public:
    DemoPage()
        : Page("DemoPage", nullptr, SCREEN_MGR->GetActiveLayer()) {
        SetGeometry(0, 0, 1280, 800);
        SetBgColor(Color(0xF5, 0xF6, 0xF8));
        SetBgOpacity(100);

        auto* title = new Label("Title", this);
        title->SetGeometry(40, 30, 400, 50);
        title->SetText("elui demo");
        title->SetTextColor(Color(0x20, 0x24, 0x2A));
        title->SetFontSize(28);

        auto* button = new Button("ConfirmButton", this);
        button->SetGeometry(40, 110, 160, 52);
        button->SetText("Confirm");
        button->AddClickedCallback([](Button* sender) {
            LOGI("%s clicked", sender->GetName().c_str());
        });

        auto* slider = new Slider("VolumeSlider", this);
        slider->SetGeometry(40, 200, 280, 32);
        slider->SetRange(0, 100);
        slider->SetProgress(50);
        slider->AddProgressCallback([](Slider*, int value) {
            LOGI("slider value: %d", value);
        });
    }
};

// LVGL 和显示设备初始化完成后创建页面。
auto* page = new DemoPage();
```

### 控件概览

| 类 | 用途 | 常用接口 |
| --- | --- | --- |
| `Page` | 顶层页面和页面定时器 | `AddTimeoutCallback`、`MoveLayer` |
| `Label` | 文本显示 | `SetText`、`SetTextColor`、`SetFontSize` |
| `Image` | JPEG/PNG 显示和图片挖空 | `SetImage`、`SetClipArea` |
| `Button` | 多状态文字/图片按钮 | `SetText`、`SetImageNormal`、`AddClickedCallback` |
| `List` | 横向或纵向复用列表 | `SetCell`、`AddItem`、`Refresh` |
| `Slider` | 横向/纵向滑条 | `SetRange`、`SetProgress`、`AddProgressCallback` |
| `Arc` | 弧形滑条 | `SetStartAngle`、`SetRangeAngle`、`SetProgress` |
| `Lineinput` | 单行文本输入 | `SetPlaceholderText`、`SetMaxLength`、`AddTextChangedCallback` |
| `Canvas` | 矩形、线段和圆弧绘制 | `DrawRect`、`DrawLine`、`DrawArc` |
| `Led` | LED 状态和亮度 | `SetColor`、`SetBrightness`、`On`、`Off` |
| `Dashboard` | 图片式仪表盘 | `SetImageMain`、`SetImagePointer`、`SetValue` |
| `Gif` | GIF 动画 | `SetImage` |
| `QrCode` | 二维码 | `SetContent`、`SetQrcodeColor` |
| `BarCode` | Code 128 条形码 | `SetContent`、`SetScale`、`SetDirection` |

图片、二维码、画布和输入框的简单用法：

```cpp
auto* image = new Image("Logo", page);
image->SetGeometry(380, 40, 180, 120);
image->SetImage(Resources::GetDataPath() + "logo.png");

auto* qr = new QrCode("Qr", page);
qr->SetGeometry(380, 190, 160, 160);
qr->SetContent("https://example.com");
qr->SetQrcodeColor(Color(0x00, 0x00, 0x00));
qr->SetQrBgColor(Color(0xFF, 0xFF, 0xFF));

auto* canvas = new Canvas("Canvas", page);
canvas->SetGeometry(600, 40, 260, 180);
canvas->Clear();
canvas->DrawRect(10, 10, 100, 60, 0x00A86B, 8);
canvas->DrawLine(20, 100, 220, 100, 3, 0x333);  // DrawLine 当前使用 0xRGB

auto* input = new Lineinput("NameInput", page);
input->SetGeometry(600, 260, 260, 48);
input->SetPlaceholderText("Input text");
input->SetMaxLength(64);
input->AddTextChangedCallback([](Lineinput*, const std::string& text) {
    LOGI("input: %s", text.c_str());
});
```

### 列表

`List` 使用固定数量的 `Button` 作为可复用视图，通过回调提供数据。下面创建一个
横向、3 行 2 列、循环滚动的列表：

```cpp
auto* list = new List("MenuList", page);
list->SetGeometry(40, 300, 360, 220);
list->SetCell(3, 2);
list->SetRowSpan(8);
list->SetColumnSpan(8);
list->SetScrollDir(List::kHor);
list->SetCycle(true);

const int extra = list->GetRowCount();
const int view_count = list->GetRowCount() * list->GetColumnCount() + extra;
for (int i = 0; i < view_count; ++i) {
    auto* item = new Button("MenuItem" + std::to_string(i), list);
    item->SetTouchable(false);
    list->AddItem(item, i);
}

list->AddGetListitemCountCallback([] { return 21; });
list->AddListUpdateCallback([](Button* item, int index) {
    item->SetText("item " + std::to_string(index));
});
list->AddListitemClickedCallback([](Button*, int index) {
    LOGI("list item %d clicked", index);
});
list->Refresh();
```

### 页面定时器

定时器 ID 在同一个对象内必须唯一，单位为毫秒。回调返回 `true` 时自动停止该
定时器，返回 `false` 时继续运行。

```cpp
page->AddTimeoutCallback([](int timer_id) {
    LOGD("timer: %d", timer_id);
    return false;
});
page->StartTimer(1, 1000);
// page->StopTimer(1);
```

## core 基础功能

| 模块 | 主要能力 |
| --- | --- |
| `log` | `LOGD`、`LOGI`、`LOGW`、`LOGE` |
| `resources` | 获取 `data`、`res` 和默认字体路径 |
| `file_helper` | 文件读写、目录创建、文件枚举和删除 |
| `string_helper` | 字符串转换、分割、替换和十六进制输出 |
| `json_manager` / `message` | JSON 解析和消息序列化 |
| `image_ins` | JPEG/PNG 解析与图片缓存 |
| `main_looper` | 把后台任务投递到 LVGL 主线程 |
| `thread` / `thread_pool` | 线程、互斥锁和任务队列 |
| `http_client` | GET、POST、上传、下载、URL 编码和 MD5 |
| `mqtt` | MQTT 连接、订阅、发布和消息回调 |
| `socket_tcp` / `socket_udp` | TCP、UDP 通信 |
| `net_manager` | 本机 IP、联网状态和广播地址 |
| `base64` | Base64 编码和解码 |
| `color` / `point` / `size` / `rect` | UI 基础数据类型 |

### 日志、文件、字符串和 JSON

```cpp
#include "core/log.h"
#include "core/file_helper.h"
#include "core/string_helper.h"
#include "core/json_manager.h"
#include "core/base64.h"

LOGI("application started");

FileHelper::WriteFile("hello", "./data/example.txt");
std::string content = FileHelper::ReadFile("./data/example.txt");

auto parts = StringHelper::SplitString("alpha,beta,gamma", ",");
std::string encoded = base64_encode(content.data(), content.size());
std::string decoded = base64_decode(encoded);

Json json;
if (JsonManager::ParseJson(R"({"name":"elui","version":1})", json)) {
    std::string name;
    int version = 0;
    JsonManager::Parse(json, "name", name);
    JsonManager::Parse(json, "version", version);
    LOGI("%s version %d", name.c_str(), version);
}
```

### 图片缓存

重复使用的图片可以先缓存。缓存由 `ImageIns` 管理；不再需要时按路径移除。

```cpp
const std::string path = Resources::GetDataPath() + "icon.png";
ImageIns::CacheImage(path);

auto* icon = new Image("Icon", page);
icon->SetGeometry(40, 560, 64, 64);
icon->SetImage(path);

// 页面确定不再使用该图片后调用：
// ImageIns::RemoveCacheImage(path);
```

### 后台任务与主线程更新

LVGL 及 elui 控件接口应在 LVGL 主线程调用。后台任务完成后，通过 `MAINLOOPER`
把 UI 更新投递回主线程：

```cpp
THREADPOOL_MGR->Enqueue([label] {
    std::string result = FileHelper::ReadFile("./data/status.txt");

    MAINLOOPER->AddMainMessageQueue([label, result] {
        label->SetText(result);
    });
});
```

投递的回调执行前，捕获的控件必须仍然存活。页面退出时应停止产生新任务，或者使用
业务层状态避免访问已销毁的控件。

### HTTP

在发起请求前初始化 libcurl；应用结束并且所有请求线程退出后再清理。HTTPS 请求
依赖工作目录下的 `curl-ca-bundle.crt`。

```cpp
Http::GlobalInit();

Http http;
Header header;
header.Put("Content-Type: application/json");
Response response = http.Post(
    "https://example.com/api/items",
    header,
    R"({"name":"elui"})",
    10);

if (response.m_code == 0 && response.m_status >= 200 && response.m_status < 300) {
    LOGI("response: %s", response.m_msg.c_str());
} else {
    LOGE("curl=%d http=%ld message=%s",
         response.m_code, response.m_status, response.m_msg.c_str());
}

// 应用退出且没有请求正在执行时调用：
// Http::GlobalCleanup();
```

HTTP 是同步接口，网络请求应放到 `ThreadPool` 中，结果再通过 `MAINLOOPER` 回到
UI 线程。`Header` 内的数据会在构建请求时取出，同一个对象再次请求前需要重新
添加请求头。

### MQTT

```cpp
auto* mqtt = new Mqtt();
mqtt->SetMqttHost("tcp://127.0.0.1");
mqtt->SetMqttPort("1883");
mqtt->SetClientId("elui-client");
mqtt->SetTopic("elui/status");

mqtt->AddConnServerCallback([mqtt] {
    mqtt->Publish("elui/status", "online");
});

mqtt->AddMsgRecvCallback([](const MqttMsg& msg) {
    MAINLOOPER->AddMainMessageQueue([msg] {
        LOGI("mqtt [%s] %s", msg.m_topic.c_str(), msg.m_message.c_str());
    });
});

mqtt->ConnectServer();  // 内部线程连接并在断线后重试
```

MQTT 回调不在 LVGL 主线程中，不能在回调里直接修改控件。服务器地址会与端口以
`host:port` 方式拼接，请按照所用 Paho 传输协议填写，例如 `tcp://127.0.0.1`。

## 生命周期约定

- 页面和控件通常用 `new` 创建，并通过 `parent` 建立父子关系。
- 销毁顶层页面时调用 `page->Destroy()`；它会递归销毁子控件并删除对应 LVGL 对象。
- `Destroy()` 完成后对象指针已经失效，不要再访问，也不要紧接着再次 `delete`。
- 页面销毁前应停止业务线程、网络回调和可能捕获控件指针的异步任务。
- 页面内对象名称建议唯一，定时器 ID 必须在该对象内唯一。

## 完整示例

工程自带的模板页面覆盖了按钮、列表、横纵滑条、弧形滑条、LED、输入框、仪表盘
和 GIF：

- [`src/surface/template_surface.cpp`](src/surface/template_surface.cpp)：创建和布局控件
- [`src/logic/template_logic.cpp`](src/logic/template_logic.cpp)：设置数据、绑定回调和定时器
- [`main.cpp`](main.cpp)：Windows LVGL 初始化和主循环

第三方依赖及版本见 [`licenses/README.md`](licenses/README.md)。
