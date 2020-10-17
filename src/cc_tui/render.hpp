#include "proto.hpp"

namespace ui {

struct render_t {
    struct conf_t {
        conf_t(int height, int width) : height_(height), width_(width) {}
        int height_ = 0;
        int width_ = 0;
    };

    render_t(const conf_t &conf): conf_(conf) {}

    virtual ~render_t() = default;

    virtual int draw() const = 0;
    virtual int add_point(int x, int y, char s) = 0;
    virtual int rm_point(int x, int y) = 0;
    virtual void add_entry(const proto::entry_t &e) = 0;

    render_t(const render_t &rhs) = delete;
    render_t &operator=(const render_t &rhs) = delete;

protected:
    conf_t conf_;
};

}
