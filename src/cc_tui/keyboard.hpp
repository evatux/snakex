namespace ui {

struct keyboard_t {
    keyboard_t() = default;
    virtual ~keyboard_t() = default;

    virtual bool is_hit() const = 0;
    virtual char get_key() const = 0;
};

}
