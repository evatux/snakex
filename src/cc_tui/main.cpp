#include <unistd.h>

// TODO: remove me, use termios keyboard
#include <ncurses.h>

#include <cmath>
#include <chrono>

#include "cc_core/game.hpp"
#include "cc_core/proto.hpp"

#define fatal(...) do { endwin(); fprintf(stderr, __VA_ARGS__); exit(2); } while(0)

#include "text_render.hpp" 
#include "termios_keyboard.hpp"

bool game_finished, fast_exit;

void draw(ui::render_t &r, const proto::message_t &m) {
    for (const auto &e: m) r.add_entry(e);
    r.draw();
}

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

void play_game(ui::render_t &r, int nplayers) {
    core::game_t game({r.width(), r.height()}, nplayers);
    draw(r, game.state_message());

    do {
        handle_input(game);

        // usleep(1000 * 1000);
        proto::message_t message = game.step();
        draw(r, message);
    } while (!game_finished);
}

void init_curses() {
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
}

int main(int argc, char *argv[]) {
    fast_exit = game_finished = false;

    init_curses();


    int size_x = argc > 1 ? atoi(argv[1]) : 30;
    int size_y = argc > 2 ? atoi(argv[2]) : size_x;
    int nplayers = argc > 3 ? atoi(argv[3]) : 2;

    auto render = new ui::text_render_t({size_x, size_y});
    auto input = new ui::termios_keyboard_t();

    if (nplayers < 1 || nplayers > 2)
        fatal("nplayers:%d != 1 or 2\n", nplayers);

    play_game(*render, nplayers);

    // wrap-up
    if (!fast_exit) { timeout(-1); getch(); }
    endwin();


    delete render;
    delete input;
    return 0;
}

void example() {
    auto render = new ui::text_render_t({20, 20});
    auto input = new ui::termios_keyboard_t();

    // generate message
    proto::message_t test_msg;
    test_msg.emplace_back(proto::snake_t{0, proto::snake_part_t::BODY, 2, 3});
    test_msg.emplace_back(proto::snake_t{0, proto::snake_part_t::BODY, 3, 3});
    test_msg.emplace_back(proto::snake_t{0, proto::snake_part_t::BODY, 4, 3});
    test_msg.emplace_back(proto::snake_t{0, proto::snake_part_t::BODY, 5, 3});
    test_msg.emplace_back(proto::snake_t{0, proto::snake_part_t::HEAD, 5, 4});
    test_msg.emplace_back(proto::loot_t{0, 8, 8});
    test_msg.emplace_back(proto::loot_t{0, 1, 1});

    // fill scene
    for (auto &e : test_msg) {
        render->add_entry(e);
    }

    // render
    render->draw();

    sleep(5);

    // clean-up
    delete render;
    delete input;
}
