#ifndef _PROTO_HPP
#define _PROTO_HPP

#include <iterator>
#include <string>
#include <variant>
#include <vector>

namespace proto {

// aux
enum class command_t: char { SNAKE = 's', LOOT = 'l', CLEAR = 'c', SCREEN = 'S', END = 'e' };
enum class snake_part_t: char { BODY = 'b', HEAD = 'h' };
enum class dir_t: char { UNDEF = '\0', UP = 'U', RIGHT = 'R', DOWN = 'D', LEFT = 'L' };

// entries
struct clear_t { int x, y; };
struct end_game_t {};
struct id_t { int id; };
struct loot_t { int id; int x, y; };
struct move_t { dir_t dir; };
struct name_t { int id; std::string name; };
struct score_change_t { int id; int score; };
struct setup_t { int wx, wy; };
struct snake_t { int id; snake_part_t part; int x, y; dir_t dir = dir_t::UNDEF; };

// entry and message
using entry_t = std::variant<
    clear_t,
    end_game_t,
    id_t,
    loot_t,
    move_t,
    name_t,
    score_change_t,
    setup_t,
    snake_t
        >;
using message_t = std::vector<entry_t>;

inline message_t &concatenate(message_t &self, message_t &&rhs) {
    self.insert(self.end(),
            std::make_move_iterator(rhs.begin()),
            std::make_move_iterator(rhs.end()));
    return self;
}

std::string to_string(const entry_t &entry);
std::string to_string(const message_t &message);
message_t message_from_string(const std::string &str);

} // namespace proto

#endif
