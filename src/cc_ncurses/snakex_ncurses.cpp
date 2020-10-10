#include <ncurses.h>
#include <unistd.h>

#include "cc_core/game.hpp"
#include "cc_core/proto.hpp"

#define fatal(...) do { endwin(); fprintf(stderr, __VA_ARGS__); exit(2); } while(0)

void draw_border(int size) {
    for (int i = 0; i < size + 2; ++i) mvprintw(0, i, "-");
    for (int i = 0; i < size + 2; ++i) mvprintw(size + 1, i, "-");

    for (int i = 0; i < size; ++i) mvprintw(i + 1, 0, "|");
    for (int i = 0; i < size; ++i) mvprintw(i + 1, size + 1, "|");
}

void draw(const proto::entry_t &e) {
    using namespace proto;

    if (std::holds_alternative<snake_t>(e)) {
        const auto v = std::get<snake_t>(e);
        const char *c = v.part == snake_part_t::BODY ? "x" : "@";
        mvprintw(v.y, v.x, c);
    } else if (std::holds_alternative<loot_t>(e)) {
        const auto v = std::get<loot_t>(e);
        mvprintw(v.y, v.x, "$");
    } else if (std::holds_alternative<clear_t>(e)) {
        const auto v = std::get<clear_t>(e);
        mvprintw(v.y, v.x, " ");
    } else if (std::holds_alternative<screen_t>(e)) {
        const auto v = std::get<screen_t>(e);
        if (v.x != v.y) fatal("screen:(%d,%d) not square\n", v.x, v.y);
        draw_border(v.x);
    }
}

void draw(const proto::message_t &m) { for (const auto &e: m) draw(e); }

void game(int size) {
    core::game_t game({size, size}, 1);
    draw(game.state_message());

    while(1) {
        proto::message_t message = game.step();
        draw(message);

        refresh();
        usleep(1000 * 1000);
    }
}

int main(int argc, char *argv[]) {
    initscr();
    noecho();
    curs_set(FALSE);

    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);

    const int size = argc > 1 ? atoi(argv[1]) : 30;
    if (size + 2 > max_x || size + 2 > max_y)
        fatal("window:(%d,%d) requested_size:%d. cannot fit\n", max_x, max_y, size);

    clear();
    game(size);

    endwin();
}
