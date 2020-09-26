#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ctime>

#include "game.hpp"

namespace core {

void game_t::generate_players(int num_players) {
    enum { start_snake_length = 3 };

    assert(size_.x >= start_snake_length * 2 + 1);
    assert(size_.y >= start_snake_length * 2 + 1);

    const pos_t center{size_.x / 2, size_.y / 2};
    const int lx = size_.x / 4;
    const int ly = size_.y / 4;

    pos_t offset[] = {ly * DOWN, ly * UP, lx * LEFT, lx * RIGHT};
    pos_t directions[] = {RIGHT, LEFT, DOWN, UP};

    for (int i = 0; i < num_players; ++i) {
        pos_t head = center + offset[i];
        pos_t direction = directions[i];
        snake_t snake(head, direction, start_snake_length);

        player_t player{true, std::move(snake), 0};
        players_.emplace_back(player);
    }
}

void game_t::generate_loots(int num_loots) {
    std::srand(std::time(nullptr));

    for (int i = 0; i < num_loots; ++i) {
        while (true) {
            int rand_val = std::rand() % (size_.x * size_.y);
            const pos_t pos{rand_val % size_.x, rand_val / size_.x};
            if (is_empty_cell(pos)) {
                loot_t loot{loot_t::type_t::FOOD, pos};
                loots_.emplace_back(loot);
                break;
            }
        }
    }
}

bool game_t::is_empty_cell(const pos_t &pos) const {
    for (const auto &player: players())
        if (player.is_active && player.snake.contains(pos)) return false;

    for (const auto &loot: loots())
        if (loot.pos == pos) return false;

    return true;
}

void game_t::set_snake_head_direction(int id, const pos_t &head_direction) {
    assert(players_[id].is_active);
    assert(0 <= id && id < (int)players_.size());
    assert(head_direction.is_direction());
    players_[id].snake.set_head_direction(head_direction);
}

state_t game_t::step() {
    int num_loots = num_loots_on_board;

    for (auto &player: players_) {
        if (!player.is_active) continue;

        auto &snake = player.snake;
        const pos_t dir = snake.head_direction();
        const pos_t target = snake.head() + dir;

        bool stop = false;

        // check for loot
        auto loot = std::find_if(loots_.begin(), loots_.end(),
                [&](const loot_t &loot) { return loot.pos == target; });
        if (loot != loots_.end()) {
            snake.step(dir, true);
            player.score += 1;
            loots_.erase(loot);
            stop = true;
        }
        if (stop) continue;

        // check for collision
        for (const auto &other_player: players_) {
            if (other_player.snake.contains(target)) {
                player.is_active = false;
                stop = true;
            }
            if (stop) break;
        }
        if (stop) continue;

        // oK to step
        snake.step(dir, false);
    }

    generate_loots(num_loots);

    return state_t::PLAY;
}

} // namespace core
