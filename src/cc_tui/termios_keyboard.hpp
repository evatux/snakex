#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

#include "keyboard.hpp"

namespace ui {

struct termios_keyboard_t: public keyboard_t {
    termios_keyboard_t(): keyboard_t() {
        tcgetattr(0, &original_terminal_mode_);
        termios new_terminal_mode;
        memcpy(&new_terminal_mode, &original_terminal_mode_, sizeof(new_terminal_mode));

        cfmakeraw(&new_terminal_mode);
        tcsetattr(0, TCSANOW, &new_terminal_mode);
    }

    ~termios_keyboard_t() {
        tcsetattr(0, TCSANOW, &original_terminal_mode_);
    }

    bool is_hit() const override {
        timeval wait = { 0L, 0L };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        return select(1, &fds, NULL, NULL, &wait);
    }

    char get_key() const override {
        int bytes = 0;
        char key = 0;
        if ((bytes = read(0, &key, sizeof(key))) < 0) {
            return 0;
        } else {
            return key;
        }
    }

protected:
    termios original_terminal_mode_;
};

}
