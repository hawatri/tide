#include "tide.hpp"
#include <sstream>

Tide::Tide(const char* filename) :
    cursor_x(0), cursor_y(0), filename(filename),
    show_line_numbers(SHOW_LINE_NUMBERS_DEFAULT),
    should_exit(false) {
    buffer.push_back("");
    mode = COMMAND;
}

void Tide::run() {
    initscr();
    raw();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();

    init_pair(NORMAL, COLOR_WHITE, COLOR_BLACK);
    init_pair(KEYWORD, COLOR_BLUE, COLOR_BLACK);
    init_pair(STRING, COLOR_GREEN, COLOR_BLACK);
    init_pair(COMMENT, COLOR_CYAN, COLOR_BLACK);
    init_pair(PREPROCESSOR, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(NUMBER, COLOR_YELLOW, COLOR_BLACK);

    load_file();

    while(!should_exit) {
        update_line_number_width();
        clear();
        draw_line_numbers();
        draw_buffer();
        draw_status_bar();
        refresh();

        int ch = getch();
        switch(mode) {
            case COMMAND: handle_command_mode(ch); break;
            case INSERT: handle_insert_mode(ch); break;
            case EX: {
                std::string ex_cmd;
                handle_ex_mode(ex_cmd, ch);
                break;
            }
        }
    }
    endwin();
}
void Tide::handle_ex_command(const std::string& cmd) {
    if (cmd == "q") should_exit = true;
    else if (cmd == "w") save_file();
    else if (cmd == "wq") { save_file(); should_exit = true; }
    else if (cmd == "set number") show_line_numbers = true;
    else if (cmd == "set nonumber") show_line_numbers = false;
}

void Tide::handle_ex_mode(std::string& command, int ch) {
    if (ch == '\n') {
        handle_ex_command(command);
        command.clear();
        mode = COMMAND;
    } else if (ch == 27) {  // ESC
        command.clear();
        mode = COMMAND;
    } else if (ch == 127 || ch == KEY_BACKSPACE) {
        if (!command.empty()) command.pop_back();
    } else {
        command += static_cast<char>(ch);
    }
}
// [Rest of the implementations...]
// Continue with other method implementations from previous versions
// (load_file, save_file, draw_status_bar, etc)
// Follow the same pattern as original code but using Tide class name
