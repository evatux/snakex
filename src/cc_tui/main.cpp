#include <unistd.h>

#include "proto.hpp"
#include "text_render.hpp" 
#include "termios_keyboard.hpp"

int main() {
    auto render = new ui::text_render_t({20, 20});
    auto input = new ui::termios_keyboard_t();

    proto::message_t test_msg;

//    test_msg.emplace_back(proto::screen_t{20, 20});
    test_msg.emplace_back(proto::snake_t{0, proto::snake_part_t::BODY, 2, 3});
    test_msg.emplace_back(proto::snake_t{0, proto::snake_part_t::BODY, 3, 3});
    test_msg.emplace_back(proto::snake_t{0, proto::snake_part_t::BODY, 4, 3});
    test_msg.emplace_back(proto::snake_t{0, proto::snake_part_t::BODY, 5, 3});
    test_msg.emplace_back(proto::snake_t{0, proto::snake_part_t::HEAD, 5, 4});
    test_msg.emplace_back(proto::loot_t{0, 8, 8});
    test_msg.emplace_back(proto::loot_t{0, 1, 1});

    for (auto &e : test_msg) {
        render->add_entry(e);
    }

    render->draw();

    sleep(5);

    delete render;
    delete input;
}
