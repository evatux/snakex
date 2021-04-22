// Draw on the screen

extern crate termion;

use std::fmt;
use std::io::{Write, stdout};

use termion::raw::IntoRawMode;

use crate::utypes;
use utypes::Pos;

type Screen = termion::raw::RawTerminal<std::io::Stdout>;

pub enum Color {
    Black,
    White,
    Blue,
    Green,
    Red,
    Yellow,
    Cyan,
}

impl Pos {
    fn into_cursor_pos(&self) -> termion::cursor::Goto {
        termion::cursor::Goto((self.x + 1) as u16, (self.y + 1) as u16)
    }
}

impl fmt::Display for Color {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        use termion::color;
        match self {
            Color::Black => color::Fg(color::Black).fmt(f),
            Color::White => color::Fg(color::White).fmt(f),
            Color::Blue => color::Fg(color::Blue).fmt(f),
            Color::Green => color::Fg(color::Green).fmt(f),
            Color::Red => color::Fg(color::Red).fmt(f),
            Color::Yellow => color::Fg(color::Yellow).fmt(f),
            Color::Cyan => color::Fg(color::Cyan).fmt(f),
        }
    }
}

pub struct Drawer {
    screen: Screen,
    offset: Pos,
}

impl Drawer {
    pub fn get_max_size() -> Pos {
        let terminal_sizes = termion::terminal_size().unwrap();
        let reserve = Pos{x: 10, y: 10};
        Pos{x: terminal_sizes.0 as i32, y: terminal_sizes.1 as i32} - reserve
    }

    pub fn new() -> Drawer {
        Drawer {
            screen: stdout().into_raw_mode().unwrap(),
            offset: Pos{x: 3, y: 3},
        }
    }

    pub fn init(&mut self) {
        write!(self.screen, "{}{}",
               termion::clear::All,
               termion::cursor::Hide).unwrap();
        self.flush();
    }

    pub fn fini(&mut self) {
        write!(self.screen, "{}{}\r\n",
               termion::style::Reset, termion::cursor::Show).unwrap();
    }

    pub fn flush(&mut self) {
        self.screen.flush().unwrap();
    }

    pub fn set_color(&mut self, color: Color) {
        write!(self.screen, "{}", color).unwrap();
    }

    pub fn reset_color(&mut self) {
        write!(self.screen, "{}", Color::White).unwrap();
    }

    pub fn print_at_pos(&mut self, pos: Pos, s: &str) {
        let pos = self.offset + pos;
        self.print_at_pos_raw(pos, s);
    }

    pub fn print_at_pos_raw(&mut self, pos: Pos, s: &str) {
        write!(self.screen, "{}{}", pos.into_cursor_pos(), s).unwrap();
    }
}
