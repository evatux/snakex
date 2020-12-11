// u-data types

use std::ops::Add;
use std::ops::Sub;
use std::ops::Mul;

#[derive(Debug, PartialEq, Copy, Clone)]
pub struct Pos {
    pub x: i32,
    pub y: i32,
}

pub type Board = Pos;

impl Add for Pos {
    type Output = Pos;

    fn add(self, other: Pos) -> Pos {
        Pos { x: self.x + other.x, y: self.y + other.y }
    }
}

impl Sub for Pos {
    type Output = Pos;

    fn sub(self, other: Pos) -> Pos {
        Pos { x: self.x - other.x, y: self.y - other.y }
    }
}

impl Mul<Pos> for i32 {
    type Output = Pos;

    fn mul(self, other: Pos) -> Pos {
        Pos { x: self * other.x, y: self * other.y }
    }
}

#[test]
fn pos_simple_test() {
    assert_eq!(Pos{x: 1, y: 2}, Pos{x: 2, y: 0} + Pos{x: -1, y: 2});
    assert_eq!(Pos{x: 1, y: 2}, Pos{x: 2, y: 1} - Pos{x: 1, y: -1});
    assert_eq!(Pos{x: 4, y: -6}, 2 * Pos{x: 2, y: -3});
    assert_eq!(Pos{x: -4, y: 6}, -2 * Pos{x: 2, y: -3});
}
