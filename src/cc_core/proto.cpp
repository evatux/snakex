#include <assert.h>
#include <stdio.h>

#include <exception>
#include <string>

#include "proto.hpp"

namespace proto {

#if 0
    Command       Serialization
    ------------- -------------------
    clear         CL <x> <y>
    end_game      EG
    loot          LO <id> <x> <y>
    move          MO [URDL]
    name          NA <id> <name>
    score_change  SC <id> <s>
    setup         ST <x> <y>
    snake         SN <id> b <x> <y>
                  SN <id> h <x> <y> [URDL]
    end_message   EM


Communication protocol:
    Dir     #       Command             Meaning
    ------- ------- ------------------- ----------------------
    S->C    1       ID <id>             Assign id to client
            2       ST <x> <y>          Specify the field size
    C->S        1   NA <id> <name>      acK
                2   EG                  Fail
    S->C    3       NA <id> <name>      Broadcast names

    S->C    4+      CL | LO | SC | SN   Game scene changes
    C->S        4+  MO                  Move

    S->C    N       EG                  End game // either server
    C->S    N       EG                  End game // or client
#endif

std::string to_string(const entry_t &entry) {
    std::string res;
    if (std::holds_alternative<clear_t>(entry)) {
        const auto &v = std::get<clear_t>(entry);
        res += std::string("CL");
        res += std::string(" ") + std::to_string(v.x);
        res += std::string(" ") + std::to_string(v.y);
    } else if (std::holds_alternative<end_game_t>(entry)) {
        res += std::string("EG");
    } else if (std::holds_alternative<id_t>(entry)) {
        const auto &v = std::get<id_t>(entry);
        res += std::string("ID");
        res += std::string(" ") + std::to_string(v.id);
    } else if (std::holds_alternative<loot_t>(entry)) {
        const auto &v = std::get<loot_t>(entry);
        res += std::string("LO");
        res += std::string(" ") + std::to_string(v.id);
        res += std::string(" ") + std::to_string(v.x);
        res += std::string(" ") + std::to_string(v.y);
    } else if (std::holds_alternative<move_t>(entry)) {
        const auto &v = std::get<move_t>(entry);
        res += std::string("MO");
        res += std::string(" ") + static_cast<char>(v.dir);
    } else if (std::holds_alternative<name_t>(entry)) {
        const auto &v = std::get<name_t>(entry);
        res += std::string("NA");
        res += std::string(" ") + std::to_string(v.id);
        res += std::string(" ") + v.name;
    } else if (std::holds_alternative<score_change_t>(entry)) {
        const auto &v = std::get<score_change_t>(entry);
        res += std::string("SC");
        res += std::string(" ") + std::to_string(v.id);
        res += std::string(" ") + std::to_string(v.score);
    } else if (std::holds_alternative<setup_t>(entry)) {
        const auto &v = std::get<setup_t>(entry);
        res += std::string("ST");
        res += std::string(" ") + std::to_string(v.wx);
        res += std::string(" ") + std::to_string(v.wy);
    } else if (std::holds_alternative<snake_t>(entry)) {
        const auto &v = std::get<snake_t>(entry);
        res += std::string("SN");
        res += std::string(" ") + std::to_string(v.id);
        res += std::string(" ") + static_cast<char>(v.part);
        res += std::string(" ") + std::to_string(v.x);
        res += std::string(" ") + std::to_string(v.y);
        if (v.part == snake_part_t::HEAD)
            res += std::string(" ") + static_cast<char>(v.dir);
    }
    res += ";";
    return res;
}

std::string to_string(const message_t &message) {
    std::string res;
    for (auto &entry: message)
        res += to_string(entry);
    res += "EM;";
    return res;
}

clear_t clear_from_string(const std::string &str, size_t &i) {
    int read;
    int x, y;
    sscanf(str.data() + i + 2, " %d %d;%n", &x, &y, &read);
    i += 2 + read;
    return {x, y};
}

end_game_t end_game_from_string(const std::string &str, size_t &i) {
    int read;
    sscanf(str.data() + i + 2, ";%n", &read);
    i += 2 + read;
    return {};
}

id_t id_from_string(const std::string &str, size_t &i) {
    int read;
    int id;
    sscanf(str.data() + i + 2, " %d;%n", &id, &read);
    i += 2 + read;
    return {id};
}

loot_t loot_from_string(const std::string &str, size_t &i) {
    int read;
    int id, x, y;
    sscanf(str.data() + i + 2, " %d %d %d;%n", &id, &x, &y, &read);
    i += 2 + read;
    return {id, x, y};
}

move_t move_from_string(const std::string &str, size_t &i) {
    int read;
    char c;

    sscanf(str.data() + i + 2, " %c;%n", &c, &read);
    i += 2 + read;

    return {static_cast<dir_t>(c)};
}

name_t name_from_string(const std::string &str, size_t &i) {
    int read;
    int id;
    sscanf(str.data() + i + 2, " %d %n", &id, &read);
    i += 2 + read;

    std::string name;
    while (true) {
        const char *c = str.data() + i;
        assert(*c != '\0');
        if (*c == ';') break;
        name += *c;
        ++i;
    }
    i++;

    return {id, name};
}

score_change_t score_change_from_string(const std::string &str, size_t &i) {
    int read;
    int id, score;
    sscanf(str.data() + i + 2, " %d %d;%n", &id, &score, &read);
    i += 2 + read;
    return {id, score};
}

setup_t setup_from_string(const std::string &str, size_t &i) {
    int read;
    int wx, wy;
    sscanf(str.data() + i + 2, " %d %d;%n", &wx, &wy, &read);
    i += 2 + read;
    return {wx, wy};
}

snake_t snake_from_string(const std::string &str, size_t &i) {
    int read;
    int id, x, y;
    char c;

    snake_part_t part;
    dir_t dir = dir_t::UNDEF;

    sscanf(str.data() + i + 2, " %d %c %d %d%n", &id, &c, &x, &y, &read);
    i += 2 + read;

    part = static_cast<snake_part_t>(c);

    if (part == snake_part_t::HEAD) {
        sscanf(str.data() + i, " %c;", &c);
        dir = static_cast<dir_t>(c);
        i += 3;
    } else {
        i += 1; // ;
    }
    return {id, part, x, y, dir};
}

message_t message_from_string(const std::string &str) {
    message_t message;

    size_t i = 0;
    while (i < str.length()) {
        assert(i + 2 <= str.length());
        std::string command = str.substr(i, 2);
        if (command == "CL")
            message.emplace_back(clear_from_string(str, i));
        else if (command == "EG")
            message.emplace_back(end_game_from_string(str, i));
        else if (command == "ID")
            message.emplace_back(id_from_string(str, i));
        else if (command == "LO")
            message.emplace_back(loot_from_string(str, i));
        else if (command == "MO")
            message.emplace_back(move_from_string(str, i));
        else if (command == "NA")
            message.emplace_back(name_from_string(str, i));
        else if (command == "SC")
            message.emplace_back(score_change_from_string(str, i));
        else if (command == "ST")
            message.emplace_back(setup_from_string(str, i));
        else if (command == "SN")
            message.emplace_back(snake_from_string(str, i));
        else if (command == "EM") {
            assert(i + 3 == str.length());
            assert(str[i + 2] == ';');
            break;
        } else {
            std::string rest = str.substr(i);
            throw std::runtime_error(
                    "unknown command (" + std::to_string(i) +
                    ") rest:'" + rest + "'");
        }
    }

    return message;
}

} // namespace proto
