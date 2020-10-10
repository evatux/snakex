#include <ncurses.h>
#include <unistd.h>

#include "cc_core/game.hpp"
#include "cc_core/proto.hpp"

#define fatal(...) do { endwin(); fprintf(stderr, __VA_ARGS__); exit(2); } while(0)

int size_x, size_y;

void nprint(int x, int y, const char *s) {
    mvprintw(size_y - y, x + 1, s);
}

void draw_border(int wx, int wy) {
    nprint(-1, -1, "+");
    nprint(-1, wy, "+");
    nprint(wx, -1, "+");
    nprint(wx, wy, "+");

    for (int i = 0; i < wx; ++i) nprint(i, wy, "-");
    for (int i = 0; i < wx; ++i) nprint(i, -1, "-");

    for (int i = 0; i < wy; ++i) nprint(-1, i, "|");
    for (int i = 0; i < wy; ++i) nprint(wx, i, "|");
}

void draw(const proto::entry_t &e) {
    using namespace proto;

    if (std::holds_alternative<snake_t>(e)) {
        const auto v = std::get<snake_t>(e);
        const char *c = v.part == snake_part_t::BODY ? "x" : "@";
        nprint(v.x, v.y, c);
    } else if (std::holds_alternative<loot_t>(e)) {
        const auto v = std::get<loot_t>(e);
        nprint(v.x, v.y, "$");
    } else if (std::holds_alternative<clear_t>(e)) {
        const auto v = std::get<clear_t>(e);
        nprint(v.x, v.y, " ");
    } else if (std::holds_alternative<screen_t>(e)) {
        const auto v = std::get<screen_t>(e);
        if (v.x != size_x || v.y != size_y)
            fatal("screen:(%d,%d) doesn't match game_screen:(%d,%d)\n",
                    size_x, size_y, v.x, v.y);
        draw_border(v.x, v.y);
    }
}

void draw(const proto::message_t &m) { for (const auto &e: m) draw(e); }

void play_game() {
    core::game_t game({size_x, size_y}, 1);
    draw(game.state_message());

    while(1) {
        proto::message_t message = game.step();
        draw(message);

        timeout(1000);
        int c = getch();
        switch(c) {
            case 'w': game.set_snake_head_direction(0, core::UP); break;
            case 's': game.set_snake_head_direction(0, core::DOWN); break;
            case 'a': game.set_snake_head_direction(0, core::LEFT); break;
            case 'd': game.set_snake_head_direction(0, core::RIGHT); break;
        }

        refresh();
        // usleep(1000 * 1000);
    }
}

int main(int argc, char *argv[]) {
    initscr();
    noecho();
    curs_set(FALSE);

    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);

    size_x = argc > 1 ? atoi(argv[1]) : 30;
    size_y = argc > 2 ? atoi(argv[2]) : size_x;

    if (size_x + 2 > max_x || size_y + 2 > max_y)
        fatal("window:(%d,%d) requested_size:(%d,%d). Cannot fit\n",
                max_x, max_y, size_x, size_y);

    clear();
    play_game();

    endwin();
}
