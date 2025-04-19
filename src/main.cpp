#include "tide.hpp"
#include "config.hpp"

int main(int argc, char** argv) {
    std::string filename = argc > 1 ? argv[1] : DEFAULT_FILENAME;
    Tide editor(filename.c_str());
    return 0;
}
