#include "syntax.hpp"
#include "config.hpp"
#include <cctype>

SyntaxHighlighter::SyntaxHighlighter() {
    cpp_keywords = {
        "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
        "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t",
        "class", "compl", "concept", "const", "consteval", "constexpr", "const_cast",
        "continue", "co_await", "co_return", "co_yield", "decltype", "default", "delete",
        "do", "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern",
        "false", "float", "for", "friend", "goto", "if", "inline", "int", "long",
        "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator",
        "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast",
        "requires", "return", "short", "signed", "sizeof", "static", "static_assert",
        "static_cast", "struct", "switch", "template", "this", "thread_local", "throw",
        "true", "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
        "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
    };
}

std::vector<int> SyntaxHighlighter::highlight(const std::string& line, SyntaxState& state) {
    std::vector<int> colors(line.size(), NORMAL);

    for(size_t i = 0; i < line.size(); i++) {
        if(state.in_comment) {
            colors[i] = COMMENT;
            if(i > 0 && line[i-1] == '*' && line[i] == '/') {
                state.in_comment = false;
            }
            continue;
        }

        if(state.in_string || state.in_char) {
            colors[i] = STRING;
            if(line[i] == '\\' && !state.escape) {
                state.escape = true;
            } else {
                if((state.in_string && line[i] == '"') ||
                   (state.in_char && line[i] == '\'')) {
                    if(!state.escape) {
                        state.in_string = false;
                        state.in_char = false;
                    }
                }
                state.escape = false;
            }
            continue;
        }

        if(line[i] == '/' && i+1 < line.size()) {
            if(line[i+1] == '/') {
                std::fill(colors.begin()+i, colors.end(), COMMENT);
                break;
            }
            if(line[i+1] == '*') {
                state.in_comment = true;
                colors[i] = COMMENT;
                colors[i+1] = COMMENT;
                i++;
                continue;
            }
        }

        if(line[i] == '"') {
            state.in_string = true;
            colors[i] = STRING;
            continue;
        }

        if(line[i] == '\'') {
            state.in_char = true;
            colors[i] = STRING;
            continue;
        }

        if(line[i] == '#') {
            colors[i] = PREPROCESSOR;
            size_t end = line.size();
            std::fill(colors.begin()+i, colors.begin()+end, PREPROCESSOR);
            break;
        }

        if(isdigit(line[i]) && (i == 0 || !isalnum(line[i-1]))) {
            colors[i] = NUMBER;
            size_t end = i + 1;
            while(end < line.size() && (isdigit(line[end]) || line[end] == '.' || line[end] == 'x')) {
                end++;
            }
            std::fill(colors.begin()+i, colors.begin()+end, NUMBER);
            i = end - 1;
            continue;
        }

        if(isalpha(line[i]) || line[i] == '_') {
            size_t start = i;
            while(i < line.size() && (isalnum(line[i]) || line[i] == '_')) {
                i++;
            }
            std::string word = line.substr(start, i - start);
            if(cpp_keywords.count(word)) {
                std::fill(colors.begin()+start, colors.begin()+i, KEYWORD);
            }
            i--;
            continue;
        }
    }
    return colors;
}
