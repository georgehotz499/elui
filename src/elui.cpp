#include "elui.h"

#include "core/log.h"
#include "core/main_looper.h"
#include "core/screen.h"

#include "logic/main_logic.h"


void EluiEntry(void* arg) {
    LOGI("start elui!!!");
    LOGI("Compiled in %s %s", __DATE__, __TIME__);

    // 设置主线程id
    MAINLOOPER->SetMainThreadId(Thread::GetId()); 

    // 进入mainlogic
    new MainLogic("MainLogic1", SCREEN_MGR->GetActiveLayer());

    LOGI("show byui!!!");
}
