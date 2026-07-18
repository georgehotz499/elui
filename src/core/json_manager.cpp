#include "core/json_manager.h"
#include "core/log.h"

bool JsonManager::ParseJson(const std::string& json_str, Json& json) {
    try
    {
        json = Json::parse(json_str);
        return true;
    }
    catch (const std::exception&)
    {
        LOGE("Failed to Parse json:\n%s", json_str.c_str());
    }
    return false;
}

bool JsonManager::Parse(const Json & json, const std::string& key, std::string & value) {
    auto object = json.find(key);
    if (object != json.end()) {
        if (object.value().is_string()) {
            value = object.value();
            return true;
        }
    }
    return false;
}

bool JsonManager::Parse(const Json& json, const std::string& key, Json& value) {
    auto object = json.find(key);
    if (object != json.end()) {
        auto object_value = object.value();
        if (object_value.is_object()) {
            value = object_value;
            return true;
        }
    }
    return false;
}

bool JsonManager::Parse(const Json& json, const std::string& key, long long& value) {
    auto object = json.find(key);
    if (object != json.end()) {
        auto object_value = object.value();
        if (object_value.is_number_integer()) {
            value = object_value;
            return true;
        }
    }
    return false;
}

bool JsonManager::Parse(const Json& json, const std::string& key, int& value) {
    auto object = json.find(key);
    if (object != json.end()) {
        auto object_value = object.value();
        if (object_value.is_number_integer()) {
            value = object_value;
            return true;
        }
    }
    return false;
}

bool JsonManager::Parse(const Json& json, const std::string& key, bool& value) {
    auto object = json.find(key);
    if (object != json.end()) {
        auto object_value = object.value();
        if (object_value.is_boolean()) {
            value = object_value;
            return true;
        }
    }
    return false;
}

bool JsonManager::ParseArray(const Json& json, const std::string& key, Json& value) {
    auto object = json.find(key);
    if (object != json.end()) {
        auto object_value = object.value();
        if (object_value.is_array()) {
            value = object_value;
            return true;
        }
    }
    return false;
}
