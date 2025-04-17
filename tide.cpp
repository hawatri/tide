#include <ncurses.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <cctype>

class MiniVim {
private:
    std::vector<std::string> buffer;
    int cursor_x, cursor_y;
    std::string filename;
    enum Mode { COMMAND, INSERT, EX } mode;
    bool show_line_numbers;
    int line_num_width;
    bool should_exit;

    // Syntax highlighting components
    std::unordered_set<std::string> cpp_keywords = {
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

    enum SyntaxType {
        NORMAL,
        KEYWORD,
        STRING,
        COMMENT,
        PREPROCESSOR,
        NUMBER
    };

    struct SyntaxState {
        bool in_string = false;
        bool in_char = false;
        bool in_comment = false;
        bool escape = false;
    };

public:
    MiniVim(const char* filename) : cursor_x(0), cursor_y(0), filename(filename),
                                  show_line_numbers(true), should_exit(false) {
        buffer.push_back("");
        mode = COMMAND;
    }

    void run() {
        initscr();
        raw();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);

        // Initialize colors
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_BLUE, COLOR_BLACK);
        init_pair(3, COLOR_GREEN, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6, COLOR_YELLOW, COLOR_BLACK);

        load_file();
        main_loop();
        endwin();
    }

private:
    void load_file() {
        std::ifstream file(filename);
        if (file) {
            buffer.clear();
            std::string line;
            while (getline(file, line)) {
                buffer.push_back(line);
            }
            if (buffer.empty()) buffer.push_back("");
        }
    }

    void save_file() {
        std::ofstream file(filename);
        if (file) {
            for (const auto& line : buffer) {
                file << line << "\n";
            }
        }
    }

    void update_line_number_width() {
        line_num_width = std::to_string(buffer.size()).length() + 2;
    }

    void draw_status() {
        attron(A_REVERSE);
        std::string mode_str;
        switch(mode) {
            case COMMAND: mode_str = "COMMAND"; break;
            case INSERT: mode_str = "INSERT"; break;
            case EX: mode_str = "EX"; break;
        }
        mvprintw(LINES-1, 0, " %s | %s | Line: %d Col: %d ",
                mode_str.c_str(), filename.c_str(), cursor_y+1, cursor_x+1);
        clrtoeol();
        attroff(A_REVERSE);
    }

    void draw_line_numbers() {
        if(!show_line_numbers) return;

        attron(COLOR_PAIR(1) | A_DIM);
        for (int y = 0; y < LINES-1 && y < buffer.size(); y++) {
            std::stringstream ss;
            ss << y+1;
            mvprintw(y, line_num_width - ss.str().length() - 2, "%2d ", y+1);
        }
        attroff(COLOR_PAIR(1) | A_DIM);
    }

    void draw_buffer() {
        SyntaxState state;

        for (int y = 0; y < LINES-1 && y < buffer.size(); y++) {
            int draw_x = line_num_width;
            std::vector<int> colors = highlight_syntax(buffer[y], state);

            for (size_t x = 0; x < buffer[y].size(); x++) {
                int color_pair = 1;
                if (colors[x] != NORMAL) {
                    color_pair = colors[x] + 1;
                }

                if (y == cursor_y && x == cursor_x) {
                    attron(A_REVERSE | COLOR_PAIR(1));
                    mvaddch(y, draw_x, buffer[y][x]);
                    attroff(A_REVERSE | COLOR_PAIR(1));
                } else {
                    attron(COLOR_PAIR(color_pair));
                    mvaddch(y, draw_x, buffer[y][x]);
                    attroff(COLOR_PAIR(color_pair));
                }
                draw_x++;
            }

            if (y == cursor_y && cursor_x >= buffer[y].size()) {
                attron(A_REVERSE | COLOR_PAIR(1));
                mvaddch(y, draw_x, ' ');
                attroff(A_REVERSE | COLOR_PAIR(1));
            }
            clrtoeol();
        }
    }

    std::vector<int> highlight_syntax(const std::string& line, SyntaxState& state) {
        std::vector<int> colors(line.size(), NORMAL);

        for (size_t i = 0; i < line.size(); i++) {
            if (state.in_comment) {
                colors[i] = COMMENT;
                if (i > 0 && line[i-1] == '*' && line[i] == '/') {
                    state.in_comment = false;
                }
                continue;
            }

            if (state.in_string || state.in_char) {
                colors[i] = STRING;
                if (line[i] == '\\' && !state.escape) {
                    state.escape = true;
                } else {
                    if ((state.in_string && line[i] == '"') ||
                        (state.in_char && line[i] == '\'')) {
                        if (!state.escape) {
                            state.in_string = false;
                            state.in_char = false;
                        }
                    }
                    state.escape = false;
                }
                continue;
            }

            if (line[i] == '/' && i+1 < line.size()) {
                if (line[i+1] == '/') {
                    std::fill(colors.begin()+i, colors.end(), COMMENT);
                    break;
                }
                if (line[i+1] == '*') {
                    state.in_comment = true;
                    colors[i] = COMMENT;
                    colors[i+1] = COMMENT;
                    i++;
                    continue;
                }
            }

            if (line[i] == '"') {
                state.in_string = true;
                colors[i] = STRING;
                continue;
            }

            if (line[i] == '\'') {
                state.in_char = true;
                colors[i] = STRING;
                continue;
            }

            if (line[i] == '#') {
                colors[i] = PREPROCESSOR;
                size_t end = line.size();
                std::fill(colors.begin()+i, colors.begin()+end, PREPROCESSOR);
                break;
            }

            if (isdigit(line[i]) && (i == 0 || !isalnum(line[i-1]))) {
                colors[i] = NUMBER;
                size_t end = i + 1;
                while (end < line.size() && (isdigit(line[end]) || line[end] == '.' || line[end] == 'x')) {
                    end++;
                }
                std::fill(colors.begin()+i, colors.begin()+end, NUMBER);
                i = end - 1;
                continue;
            }

            if (isalpha(line[i]) || line[i] == '_') {
                size_t start = i;
                while (i < line.size() && (isalnum(line[i]) || line[i] == '_')) {
                    i++;
                }
                std::string word = line.substr(start, i - start);
                if (cpp_keywords.count(word)) {
                    std::fill(colors.begin()+start, colors.begin()+i, KEYWORD);
                }
                i--;
                continue;
            }
        }
        return colors;
    }

    void handle_command_mode() {
        int ch = getch();
        switch(ch) {
            case 'i': mode = INSERT; break;
            case ':': mode = EX; break;
            case 'q': should_exit = true; break;

            case KEY_UP:
                if (cursor_y > 0) cursor_y--;
                adjust_cursor_x();
                break;
            case KEY_DOWN:
                if (cursor_y < buffer.size()-1) cursor_y++;
                adjust_cursor_x();
                break;
            case KEY_LEFT:
                if (cursor_x > 0) cursor_x--;
                break;
            case KEY_RIGHT:
                if (cursor_x < buffer[cursor_y].length()) cursor_x++;
                break;
        }
    }

    void handle_insert_mode() {
        int ch = getch();
        switch(ch) {
            case 27: mode = COMMAND; break;

            case 127: case KEY_BACKSPACE:
                handle_backspace();
                break;

            case '\n':
                handle_newline();
                break;

            case KEY_UP:
                if (cursor_y > 0) cursor_y--;
                adjust_cursor_x();
                break;
            case KEY_DOWN:
                if (cursor_y < buffer.size()-1) cursor_y++;
                adjust_cursor_x();
                break;
            case KEY_LEFT:
                if (cursor_x > 0) cursor_x--;
                break;
            case KEY_RIGHT:
                if (cursor_x < buffer[cursor_y].length()) cursor_x++;
                break;

            default:
                if (cursor_x <= buffer[cursor_y].size()) {
                    buffer[cursor_y].insert(cursor_x, 1, ch);
                    cursor_x++;
                }
                break;
        }
    }

    void adjust_cursor_x() {
        cursor_x = std::min(cursor_x, (int)buffer[cursor_y].size());
    }

    void handle_backspace() {
        if (cursor_x > 0) {
            buffer[cursor_y].erase(--cursor_x, 1);
        }
        else if (cursor_y > 0) {
            cursor_x = buffer[cursor_y-1].size();
            buffer[cursor_y-1] += buffer[cursor_y];
            buffer.erase(buffer.begin() + cursor_y);
            cursor_y--;
        }
    }

    void handle_newline() {
        std::string new_line = buffer[cursor_y].substr(cursor_x);
        buffer[cursor_y] = buffer[cursor_y].substr(0, cursor_x);
        buffer.insert(buffer.begin() + cursor_y + 1, new_line);
        cursor_y++;
        cursor_x = 0;
    }

    void main_loop() {
        std::string ex_command;
        while (!should_exit) {
            clear();
            update_line_number_width();

            draw_line_numbers();
            draw_buffer();
            draw_status();

            switch(mode) {
                case COMMAND:
                    handle_command_mode();
                    break;

                case INSERT:
                    handle_insert_mode();
                    break;

                case EX: {
                    mvprintw(LINES-1, 0, ":%s", ex_command.c_str());
                    clrtoeol();
                    int ch = getch();
                    if (ch == '\n') {
                        handle_ex_command(ex_command);
                        ex_command.clear();
                        mode = COMMAND;
                    } else if (ch == 27) {
                        ex_command.clear();
                        mode = COMMAND;
                    } else if (ch == 127 || ch == KEY_BACKSPACE) {
                        if (!ex_command.empty()) ex_command.pop_back();
                    } else {
                        ex_command += ch;
                    }
                    break;
                }
            }

            refresh();
        }
    }

    void handle_ex_command(const std::string& cmd) {
        if (cmd == "q") {
            should_exit = true;
        }
        else if (cmd == "w") {
            save_file();
        }
        else if (cmd == "wq") {
            save_file();
            should_exit = true;
        }
        else if (cmd == "set number") {
            show_line_numbers = true;
        }
        else if (cmd == "set nonumber") {
            show_line_numbers = false;
        }
    }
};

int main(int argc, char** argv) {
    const char* filename = argc > 1 ? argv[1] : "untitled.txt";
    MiniVim editor(filename);
    editor.run();
    return 0;
}
