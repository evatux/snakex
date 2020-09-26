#ifndef _UTYPES_HPP
#define _UTYPES_HPP

#include <cassert>
#include <cmath>
#include <cstdlib>

namespace core {

struct pos_t {
    int x, y;

    pos_t(): x(0), y(0) {}
    pos_t(int x, int y): x(x), y(y) {}
    pos_t(const pos_t &rhs): x(rhs.x), y(rhs.y) {}

    pos_t &operator=(const pos_t &rhs) { x = rhs.x; y = rhs.y; return *this; }

    pos_t &operator+=(const pos_t &rhs) { x += rhs.x; y += rhs.y; return *this; }
    pos_t &operator-=(const pos_t &rhs) { x -= rhs.x; y -= rhs.y; return *this; }
    pos_t &operator*=(int a) { x *= a; y *= a; return *this; }

    bool is_direction() const { return std::abs(x) + std::abs(y) == 1; }
};

static const pos_t LEFT{-1, 0}, RIGHT{+1, 0}, UP{0, +1}, DOWN{0, -1};

bool operator==(const pos_t &lhs, const pos_t &rhs) { return lhs.x == rhs.x && lhs.y == rhs.y; }
bool operator!=(const pos_t &lhs, const pos_t &rhs) { return !operator==(lhs, rhs); }
pos_t operator-(const pos_t &rhs) { return pos_t(-rhs.x, -rhs.y); }
pos_t operator+(const pos_t &lhs, const pos_t &rhs) { pos_t res{lhs}; return res += rhs; }
pos_t operator-(const pos_t &lhs, const pos_t &rhs) { pos_t res{lhs}; return res -= rhs; }
pos_t operator*(const pos_t &lhs, int a) { pos_t res{lhs}; return res *= a; }
pos_t operator*(int a, const pos_t &rhs) { pos_t res{rhs}; return res *= a; }

} // namespace core

#endif
