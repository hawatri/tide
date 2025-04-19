#pragma once

#include <string>

// Color pair definitions
enum ColorPairs {
    NORMAL = 1,
    KEYWORD,
    STRING,
    COMMENT,
    PREPROCESSOR,
    NUMBER
};

// Editor constants
const bool SHOW_LINE_NUMBERS_DEFAULT = true;
constexpr const char* DEFAULT_FILENAME = "untitled.txt";
const std::string APP_NAME = "Tide";
