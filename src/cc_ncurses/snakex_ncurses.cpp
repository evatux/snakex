#include <ncurses.h>
#include <unistd.h>

#include <cmath>
#include <chrono>

#include "cc_core/game.hpp"
#include "cc_core/proto.hpp"

#define fatal(...) do { endwin(); fprintf(stderr, __VA_ARGS__); exit(2); } while(0)

int size_x, size_y;
int nplayers;
bool game_finished, fast_exit;

void nprint(int x, int y, const char *s) {
    mvprintw(size_y - y, x + 1, s);
}

void nprint_score(int id, int score) {
    char str[16];
    snprintf(str, sizeof(str), "P%d: %3d%s",
            id, std::abs(score), score >= 0 ? "" : " DEAD");
    attron(COLOR_PAIR(id + 1));
    mvprintw(2 + id * 2, size_x + 3, str);
    attroff(COLOR_PAIR(id + 1));
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
        const char *c = v.part == snake_part_t::BODY ? "o" : "@";
        attron(COLOR_PAIR(v.id + 1));
        nprint(v.x, v.y, c);
        attroff(COLOR_PAIR(v.id + 1));
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

void handle_input(core::game_t &game) {
    const int tout = 500;

    auto start = std::chrono::high_resolution_clock::now();
    while (1) {
        auto stop = std::chrono::high_resolution_clock::now();

        int past = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
        if (past >= tout) return;
        timeout(tout - past);
        int c = getch();
        switch(c) {
            // Player 1
            case KEY_UP:    game.set_snake_head_direction(0, core::UP);    break;
            case KEY_DOWN:  game.set_snake_head_direction(0, core::DOWN);  break;
            case KEY_LEFT:  game.set_snake_head_direction(0, core::LEFT);  break;
            case KEY_RIGHT: game.set_snake_head_direction(0, core::RIGHT); break;
            // Player 2
            case 'w': game.set_snake_head_direction(1, core::UP);    break;
            case 's': game.set_snake_head_direction(1, core::DOWN);  break;
            case 'a': game.set_snake_head_direction(1, core::LEFT);  break;
            case 'd': game.set_snake_head_direction(1, core::RIGHT); break;
            // Control
            case 'q': fast_exit = game_finished = true; return;
        }
    }
}

void play_game() {
    core::game_t game({size_x, size_y}, nplayers);
    draw(game.state_message());

    do {
        handle_input(game);

        // usleep(1000 * 1000);
        proto::message_t message = game.step();
        draw(message);
        refresh();
    } while (!game_finished);
}

void init_curses() {
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    clear();

    attron(A_BOLD);
}

int main(int argc, char *argv[]) {
    fast_exit = game_finished = false;

    init_curses();

    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);

    size_x = argc > 1 ? atoi(argv[1]) : 30;
    size_y = argc > 2 ? atoi(argv[2]) : size_x;
    nplayers = argc > 3 ? atoi(argv[3]) : 2;

    if (size_x + 2 + 16 > max_x || size_y + 2 > max_y)
        fatal("window:(%d,%d) requested_size:(%d,%d). Cannot fit\n",
                max_x, max_y, size_x, size_y);
    if (nplayers < 1 || nplayers > 2)
        fatal("nplayers:%d != 1 or 2\n", nplayers);

    clear();
    play_game();

    // wrap-up
    if (!fast_exit) { timeout(-1); getch(); }
    attroff(A_BOLD);
    endwin();

    return 0;
}
