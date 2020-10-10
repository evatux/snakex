#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ctime>

#include "game.hpp"
#include "proto.hpp"

namespace core {

namespace {
proto::dir_t to_proto(const pos_t &dir) {
    assert(dir.is_direction());
    if (dir == UP) return proto::dir_t::UP;
    if (dir == RIGHT) return proto::dir_t::RIGHT;
    if (dir == DOWN) return proto::dir_t::DOWN;
    if (dir == LEFT) return proto::dir_t::LEFT;
    return proto::dir_t::UNDEF;
}
}

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

proto::message_t game_t::generate_loots(int num_loots) {
    proto::message_t message;

    std::srand(std::time(nullptr));

    for (int i = 0; i < num_loots; ++i) {
        while (true) {
            int rand_val = std::rand() % (size_.x * size_.y);
            const pos_t pos{rand_val % size_.x, rand_val / size_.x};
            if (is_empty_cell(pos)) {
                auto type = loot_t::type_t::FOOD;
                loot_t loot{type, pos};
                message.emplace_back(proto::loot_t{(int)type, pos.x, pos.y});
                loots_.emplace_back(loot);
                break;
            }
        }
    }

    return message;
}

proto::message_t game_t::state_message() const {
    proto::message_t message;
    message.emplace_back(proto::setup_t{size_.x, size_.y});

    for (int id = 0; id < (int)players_.size(); ++id) {
        const auto &snake = players_[id].snake;
        auto it = snake.begin();
        // head
        {
            proto::dir_t dir = to_proto(snake.head_direction());
            message.emplace_back(proto::snake_t{
                    id, proto::snake_part_t::HEAD, it->x, it->y, dir});
            ++it;
        }
        // body
        for (; it != snake.end(); ++it) {
            message.emplace_back(proto::snake_t{
                    id, proto::snake_part_t::BODY, it->x, it->y});
        }
    }

    for (const auto &loot: loots_)
        message.emplace_back(proto::loot_t{(int)loot.type, loot.pos.x, loot.pos.y});

    return message;
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

proto::message_t game_t::remove_player(int id) {
    proto::message_t message;

    assert(players_[id].is_active == true);
    players_[id].is_active = false;

    for (const auto &b: players_[id].snake)
        message.emplace_back(proto::clear_t{b.x, b.y});
    return message;
}

proto::message_t game_t::step() {
    proto::message_t message;

    int num_loots_ate = 0;

    for (int id = 0; id < num_players(); ++id) {
        auto &player = players_[id];
        if (!player.is_active) continue;

        auto &snake = player.snake;
        const pos_t dir = snake.head_direction();
        const pos_t target = maybe_mirror(snake.head() + dir);

        bool stop = false;

        // check out of bounds
        if (!check_pos_inside(target)) {
            proto::concatenate(message, remove_player(id));
            stop = true;
        }
        if (stop) continue;

        // check for loot
        auto loot = std::find_if(loots_.begin(), loots_.end(),
                [&](const loot_t &loot) { return loot.pos == target; });
        if (loot != loots_.end()) {
            pos_t p = snake.head();
            message.emplace_back(proto::snake_t{
                    id, proto::snake_part_t::BODY, p.x, p.y});
            p = target;
            message.emplace_back(proto::snake_t{
                    id, proto::snake_part_t::HEAD, p.x, p.y, to_proto(dir)});

            snake.step(target, true);
            player.score += 1;
            num_loots_ate += 1;
            loots_.erase(loot);
            stop = true;
        }
        if (stop) continue;

        // check for collision
        for (const auto &other_player: players_) {
            if (other_player.snake.contains(target)) {
                proto::concatenate(message, remove_player(id));
                stop = true;
            }
            if (stop) break;
        }
        if (stop) continue;

        // oK to step
        {
            pos_t p = snake.head();
            message.emplace_back(proto::snake_t{
                    id, proto::snake_part_t::BODY, p.x, p.y});
            p = target;
            message.emplace_back(proto::snake_t{
                    id, proto::snake_part_t::HEAD, p.x, p.y, to_proto(dir)});
            p = snake.tail();
            message.emplace_back(proto::clear_t{p.x, p.y});

            snake.step(target, false);
        }
    }

    proto::concatenate(message, generate_loots(num_loots_ate));

    return message;
}

pos_t game_t::maybe_mirror(const pos_t &pos) const {
    if (with_mirror_ == false) return pos;

    pos_t res = pos;
    if (res.x < 0) res.x += size_.x;
    if (res.x >= size_.x) res.x -= size_.x;
    if (res.y < 0) res.y += size_.y;
    if (res.y >= size_.y) res.y -= size_.y;

    return res;
}

bool game_t::check_pos_inside(const pos_t &pos) const {
    if (with_mirror_) return true;
    return pos.x >= 0 && pos.x < size_.x && pos.y >= 0 && pos.y < size_.y;
}

} // namespace core
