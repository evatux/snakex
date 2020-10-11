#include <ncurses.h>
#include <unistd.h>

#include <cmath>

#include "cc_core/game.hpp"
#include "cc_core/proto.hpp"

#define fatal(...) do { endwin(); fprintf(stderr, __VA_ARGS__); exit(2); } while(0)

int size_x, size_y;
bool game_finished;

void nprint(int x, int y, const char *s) {
    mvprintw(size_y - y, x + 1, s);
}

void nprint_score(int id, int score) {
    char str[16];
    snprintf(str, sizeof(str), "P%d: %3d%s",
            id, std::abs(score), score >= 0 ? "" : " DEAD");
    mvprintw(2 + id * 2, size_x + 3, str);
}

void nprint_endgame() {
    const int sx = size_x / 2 - 6;
    const int sy = size_y / 2;
    nprint(sx, sy + 1, "=============");
    nprint(sx, sy + 0, "| GAME OVER |");
    nprint(sx, sy - 1, "=============");
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
        const auto &v = std::get<snake_t>(e);
        const char *c = v.part == snake_part_t::BODY ? "x" : "@";
        nprint(v.x, v.y, c);
    } else if (std::holds_alternative<loot_t>(e)) {
        const auto &v = std::get<loot_t>(e);
        nprint(v.x, v.y, "$");
    } else if (std::holds_alternative<clear_t>(e)) {
        const auto &v = std::get<clear_t>(e);
        nprint(v.x, v.y, " ");
    } else if (std::holds_alternative<setup_t>(e)) {
        const auto &v = std::get<setup_t>(e);
        if (v.wx != size_x || v.wy != size_y)
            fatal("screen:(%d,%d) doesn't match game_screen:(%d,%d)\n",
                    size_x, size_y, v.wx, v.wy);
        draw_border(v.wx, v.wy);
    } else if (std::holds_alternative<score_change_t>(e)) {
        const auto &v = std::get<score_change_t>(e);
        nprint_score(v.id, v.score);
    } else if (std::holds_alternative<end_game_t>(e)) {
        game_finished = true;
        nprint_endgame();
    }
}

void draw(const proto::message_t &m) { for (const auto &e: m) draw(e); }

void play_game() {
    core::game_t game({size_x, size_y}, 1);
    draw(game.state_message());

    timeout(1000);
    do {
        int c = getch();
        switch(c) {
            case 'w': case KEY_UP: game.set_snake_head_direction(0, core::UP); break;
            case 's': case KEY_DOWN: game.set_snake_head_direction(0, core::DOWN); break;
            case 'a': case KEY_LEFT: game.set_snake_head_direction(0, core::LEFT); break;
            case 'd': case KEY_RIGHT: game.set_snake_head_direction(0, core::RIGHT); break;
        }

        // usleep(1000 * 1000);
        proto::message_t message = game.step();
        draw(message);
        refresh();
    } while (!game_finished);
}

int main(int argc, char *argv[]) {
    game_finished = false;

    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);

    size_x = argc > 1 ? atoi(argv[1]) : 30;
    size_y = argc > 2 ? atoi(argv[2]) : size_x;

    if (size_x + 2 + 16 > max_x || size_y + 2 > max_y)
        fatal("window:(%d,%d) requested_size:(%d,%d). Cannot fit\n",
                max_x, max_y, size_x, size_y);

    clear();
    play_game();

    // wrap-up
    timeout(-1);
    getch();
    endwin();

    return 0;
}
