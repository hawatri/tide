#pragma once
#include <vector>
#include <string>
#include <unordered_set>

struct SyntaxState {
    bool in_string = false;
    bool in_char = false;
    bool in_comment = false;
    bool escape = false;
};

class SyntaxHighlighter {
public:
    SyntaxHighlighter();
    std::vector<int> highlight(const std::string& line, SyntaxState& state);

private:
    std::unordered_set<std::string> cpp_keywords;
    void init_keywords();
    void handle_strings(char current_char, SyntaxState& state);
};
