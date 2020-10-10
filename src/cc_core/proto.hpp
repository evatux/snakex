#ifndef _PROTO_HPP
#define _PROTO_HPP

#include <iterator>
#include <variant>
#include <vector>

namespace proto {

// aux
enum class command_t: char { SNAKE = 's', LOOT = 'l', CLEAR = 'c', SCREEN = 'S', END = 'e' };
enum class snake_part_t: char { BODY = 'b', HEAD = 'h' };
enum class dir_t: char { UNDEF = '\0', UP = 'U', RIGHT = 'R', DOWN = 'D', LEFT = 'L' };

// entries
struct setup_t { int wx, wy; };
struct snake_t { int id; snake_part_t part; int x, y; dir_t dir = dir_t::UNDEF; };
struct loot_t { int id; int x, y; };
struct clear_t { int x, y; };

// entry and message
using entry_t = std::variant<snake_t, loot_t, clear_t, setup_t>;
using message_t = std::vector<entry_t>;

inline message_t &concatenate(message_t &self, message_t &&rhs) {
    self.insert(self.end(),
            std::make_move_iterator(rhs.begin()),
            std::make_move_iterator(rhs.end()));
    return self;
}

} // namespace proto

#endif
