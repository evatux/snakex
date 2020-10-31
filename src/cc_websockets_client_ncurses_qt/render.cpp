#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>

#include <cmath>
#include <chrono>

#include <QtCore/QDebug>

#include "cc_core/proto.hpp"

#include "render.h"

#define fatal(...) do { endwin(); fprintf(stderr, __VA_ARGS__); exit(2); } while(0)
#define log(...) do { if (log_file) fprintf(log_file, __VA_ARGS__), fflush(log_file); } while(0)

namespace impl {

int size_x, size_y;
int nplayers;
bool game_finished, fast_exit;

FILE *log_file;

void nprint(int x, int y, const char *s) {
    mvprintw(size_y - y, x + 1, s);
}

void nprint_score(int id, int score) {
    char str[32];
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

char handle_input() {
    const int tout = 500;

    auto start = std::chrono::high_resolution_clock::now();
    while (1) {
        auto stop = std::chrono::high_resolution_clock::now();

        int past = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
        if (past >= tout) return '\0';
        timeout(tout - past);
        int c = getch();
        switch(c) {
            // Player 1
            case KEY_UP:    return 'W';
            case KEY_DOWN:  return 'S';
            case KEY_LEFT:  return 'A';
            case KEY_RIGHT: return 'D';

            // Player 2
            case 'w': case 's': case 'a': case 'd': return c;

            // Control
            case 'q': fast_exit = game_finished = true; return '\0';
        }
    }
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

int init() {
    fast_exit = game_finished = false;
    log_file = nullptr;

    init_curses();

    int max_y = 0, max_x = 0;
    getmaxyx(stdscr, max_y, max_x);

    size_x = 30;
    size_y = size_x;
    nplayers = 1;
    const char *log_name = nullptr;
    if (log_name != nullptr) log_file = fopen(log_name, "w");

    if (size_x + 2 + 16 > max_x || size_y + 2 > max_y)
        fatal("window:(%d,%d) requested_size:(%d,%d). Cannot fit\n",
                max_x, max_y, size_x, size_y);
    if (nplayers < 1 || nplayers > 2)
        fatal("nplayers:%d != 1 or 2\n", nplayers);

    clear();
    return 0;
}

void finish() {
    // wrap-up
    if (!fast_exit) { timeout(-1); getch(); }
    attroff(A_BOLD);
    endwin();

    if (log_file) fclose(log_file);
}

} // namespace impl

QT_USE_NAMESPACE

Render::Render(int id) : id_(id) {}

void Render::receiveMessage(QString qstr) {
    std::string str = qstr.toStdString();
    impl::draw(proto::message_from_string(str));
    refresh();
}

void Render::run() {
    impl::init();
    emit messageSent(QString('0' + id_));

    do {
        char c = impl::handle_input();
        if (c != '\0') emit messageSent(QString(c));
    } while (!impl::game_finished);

    impl::finish();
    emit gameFinished();
}
