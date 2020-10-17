#include <stdio.h>

#include "render.hpp"

namespace ui {

#define MOVE_CURSOR(x, y) \
    printf("\033[%d;%dH", (x), (y))

struct text_render_t: public render_t {
    text_render_t(const conf_t conf): render_t(conf) {
        conf_.width_ += 2;
        conf_.height_ += 2;
        screen_ = new char[conf_.width_ * conf_.height_];
        for (int i = 0; i < conf_.width_ * conf_.height_; ++i) screen_[i] = ' ';
        init_background();
    }

    ~text_render_t() {
        clear_display();
        delete[] screen_;
    }

    int draw() const override {
        clear_display();

        for (int y = 0; y < conf_.height_; ++y) {
            for (int x = 0; x < conf_.width_; ++x) {
                MOVE_CURSOR(x+1, y+1);
                printf("%c", get(y, x));
            }
        }
                
        fflush(0);
        return 0;
    }

    int add_point(int x, int y, char symbol = 'X') override {
        return set(x + 1, y + 1, symbol);
    }
    int rm_point(int x, int y) override { return set(x + 1, y + 1, ' '); }

    void add_entry(const proto::entry_t &e) override {
        if (std::holds_alternative<proto::snake_t>(e)) {
            const auto s = std::get<proto::snake_t>(e);
            const char symbol = s.part == proto::snake_part_t::BODY ? 'x' : '@';
            add_point(s.x, s.y, symbol);
        } else if (std::holds_alternative<proto::loot_t>(e)) {
            const auto l = std::get<proto::loot_t>(e);
            add_point(l.x, l.y, '$');
        } else if (std::holds_alternative<proto::clear_t>(e)) {
            const auto c = std::get<proto::clear_t>(e);
            rm_point(c.x, c.y);
        }
    }

protected:
    int clear_display() const {
        // clear screen
        printf("\033[2J");
        // set cursor to 1 1
        MOVE_CURSOR(0, 0);
        return 0;
    }
    int set(int x, int y, char value) {
        screen_[y * conf_.width_ + x] = value;
        return 0;
    }
    char get(int x, int y) const {
        return screen_[y * conf_.width_ + x];
    }

    int init_background() {
        const auto height = conf_.height_;
        const auto width = conf_.width_;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (y == 0 || y == height - 1) {
                    if (x == 0 || x == width - 1) {
                        set(x, y, '+');
                    } else {
                        set(x, y, '-');
                    }
                } else {
                    if (x == 0 || x == width - 1) {
                        set(x, y, '|');
                    } else {
                        set(x, y, ' ');
                    }
                }
            }
        }
        return 0;
    }
    

    char *screen_ = nullptr;
};
 
}
