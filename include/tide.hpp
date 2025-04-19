#pragma once
#include <ncurses.h>
#include <vector>
#include <string>
#include <fstream>
#include "config.hpp"
#include "syntax.hpp"

class Tide {
public:
    Tide(const char* filename);
    void run();

private:
    enum Mode { COMMAND, INSERT, EX } mode;
    std::vector<std::string> buffer;
    int cursor_x, cursor_y;
    std::string filename;
    bool show_line_numbers;
    bool should_exit;
    SyntaxHighlighter highlighter;
    int line_num_width;
    SyntaxState syntax_state;

    // File operations
    void load_file();
    void save_file();

    // UI components
    void update_line_number_width();
    void draw_status_bar();
    void draw_line_numbers();
    void draw_buffer();
    void handle_ex_command(const std::string& cmd);

    // Mode handlers
    void handle_command_mode(int ch);
    void handle_insert_mode(int ch);
    void handle_ex_mode(std::string& command, int ch);

    // Cursor operations
    void adjust_cursor_x();
    void handle_backspace();
    void handle_newline();
};
