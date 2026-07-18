#pragma once

#include <cstddef>
#include <string>

std::string base64_encode(const char* data, size_t length);
std::string base64_decode(const std::string& input);
