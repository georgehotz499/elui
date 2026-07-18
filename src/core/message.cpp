#include "message.h"
#include "log.h"

namespace {
#define WHAT "what"
#define MSG  "msg"
}

Message Message::Parse(const std::string& msg) {
    Json json;
    Message msg_obj;
    if (!JsonManager::ParseJson(msg, json)) {
        LOGE("Failed to parse msg......");
        return msg_obj;
    }

    // 解析m_what
    int what{0};
    JsonManager::Parse(json, WHAT, what);
    msg_obj.m_what = (MsgType)what;

    // 解析msg
    JsonManager::Parse(json, MSG, msg_obj.m_msg);

    return msg_obj;
}

void Message::SetWhat(const MsgType& what) {
    m_what = what;
}

void Message::SetMsg(const Json& msg) {
    m_msg = msg;
}

std::string Message::ToJsonString() {
    Json json;
    json[WHAT] = int(m_what);
    json[MSG] = m_msg;

    return json.dump();
}
