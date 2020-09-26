#ifndef _GAME_HPP
#define _GAME_HPP

#include <algorithm>
#include <cassert>
#include <list>
#include <vector>

#include "utypes.hpp"

namespace core {

struct snake_t {
    snake_t() = default;
    snake_t(const snake_t &) = default;
    snake_t(snake_t &&) = default;

    snake_t(const pos_t &head, const pos_t &head_direction, int length) {
        head_direction_ = head_direction;
        pos_t e = head;
        for (int i = 0; i < length; ++i) {
            body_.push_back(e);
            e -= head_direction_;
        }
    }

    snake_t &operator=(snake_t &&rhs) {
        body_ = std::move(rhs.body_);
        head_direction_ = rhs.head_direction_;
        return *this;
    }

    int length() const { return (int)body_.size(); }

    const pos_t &head() const { assert(!body_.empty()); return *body_.cbegin(); }
    const pos_t &tail() const { assert(!body_.empty()); return *body_.cend(); }
    const pos_t &head_direction() const { return head_direction_; }

    std::list<pos_t>::const_iterator cbegin() const { return body_.cbegin(); }
    std::list<pos_t>::const_iterator cend() const { return body_.cend(); }

    bool contains(const pos_t &pos) const {
        return std::any_of(
                cbegin(), cend(), [&](const pos_t &p) { return p == pos; });
    }

    // modifiers
    void set_head_direction(const pos_t &new_head_direction) {
        assert(new_head_direction.is_direction());
        head_direction_ = new_head_direction;
    }
    void step(const pos_t &direction, bool grow) {
        assert(direction.is_direction());
        body_.push_front(head() + direction);
        if (!grow) body_.pop_back();
    }

private:
    std::list<pos_t> body_;
    pos_t head_direction_;
};

enum class state_t { PLAY, END };

struct loot_t {
    enum class type_t { FOOD };
    type_t type;
    pos_t pos;
};

struct game_t {
    struct player_t {
        bool is_active;
        snake_t snake;
        int score;
    };

    game_t(const pos_t size, int num_players): size_(size) {
        generate_players(num_players);
        generate_loots(num_loots_on_board);
    }

    const pos_t &size() const { return size_; }

    const std::vector<player_t> &players() const { return players_; }
    bool is_player_active(int id) const { return players()[id].is_active; }

    const std::list<loot_t> &loots() const { return loots_; }

    bool is_empty_cell(const pos_t &pos) const;

    // modifiers
    void set_snake_head_direction(int id, const pos_t &direction);
    state_t step();

private:
    pos_t size_;

    std::vector<player_t> players_;

    std::list<loot_t> loots_;
    const int num_loots_on_board = 1;

    void generate_players(int num_players);
    void generate_loots(int num_loots);
};

} // namespace core

#endif
