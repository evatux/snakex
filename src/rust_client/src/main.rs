extern crate termion;
extern crate websocket;

mod utypes;
mod drawer;

use std::{thread, time};

use termion::event::Key;
use termion::input::TermRead;

use websocket::client::ClientBuilder;

use std::sync::mpsc;

use utypes::Pos;
use drawer::Drawer;

const SYMBOL_EMPTY: &str = " ";
const SYMBOL_BORDER: &str = "█";
const SYMBOL_BORDER_PERIODIC: &str = "▒";
const SYMBOL_SNAKE_BODY: &str = "o";
const SYMBOL_SNAKE_HEAD: &str = "@";
const SYMBOL_LOOT: &str = "¤";

struct Game {
    drawer: Drawer,
    board: Pos,
}

impl Game {
    pub fn new(board: &Pos) -> Game {
        let mut drawer = Drawer::new();
        drawer.init();

        let mut game = Game { drawer:drawer, board:*board };
        game.draw_board(&board);

        game
    }

    fn print_at_pos(&mut self, pos: Pos, s: &str) {
        self.drawer.print_at_pos(Pos{x:pos.x, y:self.board.y - 1 - pos.y}, s);
    }

    fn draw_board(&mut self, board: &Pos) {
        let bs = if false {
            SYMBOL_BORDER_PERIODIC
        } else {
            SYMBOL_BORDER
        };

        self.drawer.set_color(drawer::Color::Blue);

        for x in -1 .. board.x + 1 {
            self.print_at_pos(Pos{x, y: -1}, &bs);
        }

        for y in 0 .. board.y {
            self.print_at_pos(Pos{x: -1, y}, &bs);
            self.print_at_pos(Pos{x: board.x, y}, &bs);
        }

        for x in -1 .. board.x + 1 {
            self.print_at_pos(Pos{x, y: board.y}, &bs);
        }

        self.drawer.reset_color();
        self.drawer.flush();
    }

    fn clean(&mut self, cmd: &str) {
        let mut iter = cmd.strip_prefix("CL ").unwrap()
            .split(' ')
            .map(|x| x.parse::<i32>().unwrap());
        let x = iter.next().unwrap();
        let y = iter.next().unwrap();
        assert_eq!(iter.next(), None);

        self.drawer.reset_color();
        self.print_at_pos(Pos{x, y}, &SYMBOL_EMPTY);
        self.drawer.flush();
    }

    fn loot(&mut self, cmd: &str) {
        let mut iter = cmd.strip_prefix("LO ").unwrap()
            .split(' ')
            .map(|x| x.parse::<i32>().unwrap());
        let id = iter.next().unwrap();
        let x = iter.next().unwrap();
        let y = iter.next().unwrap();
        assert_eq!(iter.next(), None);
        assert_eq!(id, 0);

        self.drawer.set_color(drawer::Color::Cyan);
        self.print_at_pos(Pos{x, y}, &SYMBOL_LOOT);
        self.drawer.reset_color();
        self.drawer.flush();
    }

    fn snake(&mut self, cmd: &str) {
        let mut iter = cmd.strip_prefix("SN ").unwrap().split(' ');

        let id = iter.next().unwrap().parse::<i32>().unwrap();
        let bh = iter.next().unwrap();
        assert!(bh == "b" || bh == "h");

        let x = iter.next().unwrap().parse::<i32>().unwrap();
        let y = iter.next().unwrap().parse::<i32>().unwrap();

        let color = match id {
            0 => drawer::Color::Green,
            1 => drawer::Color::Red,
            2 => drawer::Color::Black,
            _ => drawer::Color::Cyan,
        };
        self.drawer.set_color(color);
        if bh == "b" {
            self.print_at_pos(Pos{x, y}, &SYMBOL_SNAKE_BODY);
        } else {
            self.print_at_pos(Pos{x, y}, &SYMBOL_SNAKE_HEAD);
        }
        self.drawer.reset_color();
        self.drawer.flush();
    }
}

impl Drop for Game {
    fn drop(&mut self) {
        self.drawer.fini();
    }
}

fn process_from_server(game: &mut Game, message: &String) {
    // println!("@@@ server message recieved: {:?}", message);

    let mut cmds = message.split(';');
    for cmd in cmds {
        match &cmd.get(..2) {
            Some("CL") => game.clean(cmd),
            Some("LO") => game.loot(cmd),
            Some("SN") => game.snake(cmd),
            _ => (),
        }
    }
}

use websocket::sender::Writer;
fn process_client_command<W>(
    sender: &mut websocket::sender::Writer<W>,
    cmd: &str)
    where W: std::io::Write
{
    match &cmd[..] {
        "Q" => {
            let message = websocket::OwnedMessage::Text(String::from("EG;"));
            sender.send_message(&message).unwrap();
        }
        _ => {
            let mut message = String::from("MO ;");
            message.insert_str(3, &cmd.to_string());
            let message = websocket::OwnedMessage::Text(message);
            sender.send_message(&message).unwrap();
        }
    }
}

fn play() {
    const CONNECTION: &'static str = "ws://127.0.0.1:1234";
    println!("Connecting to {}", CONNECTION);

    let client = ClientBuilder::new(CONNECTION)
        .unwrap()
        .connect_insecure()
        .unwrap();
    println!("Successfully connected");

    let (mut receiver, mut sender) = client.split().unwrap();
    let (tx, rx) = mpsc::channel(); // String

    let receive_loop = thread::spawn(move || {
        for message in receiver.incoming_messages() {
            let message = match message {
                Ok(m) => m,
                Err(e) => {
                    println!("Receive Loop: {:?}", e);
                    return;
                }
            };
            match message {
                websocket::OwnedMessage::Text(message_str) => {
                    // println!("Receive Loop: Text: {:?}", message_str);
                    tx.send(message_str);
                },
                websocket::OwnedMessage::Close(_) => {
                    println!("Receive Loop: Close");
                    tx.send(String::from("close"));
                },
                _ => {
                    println!("Receive Loop: unknown message {:?}", message);
                    return;
                },
            }
        }
    });

    let mut id = 0i32;
    let mut board = Pos{x:0, y:0};

    // handshake
    {
        let message: String = rx.recv().unwrap();
        let mut cmds = message.split(';');
        // 1. ID
        {
            let cmd = cmds.next().unwrap();
            id = cmd.strip_prefix("ID ").unwrap().parse::<i32>().unwrap();
            println!("id:{}", id);
        }
        // 2. ST <X> <Y>
        {
            let cmd = cmds.next().unwrap();
            let mut wxy_iter = cmd.strip_prefix("ST ").unwrap()
                                  .split(' ')
                                  .map(|x| x.parse::<i32>().unwrap());
            board.x = wxy_iter.next().unwrap();
            board.y = wxy_iter.next().unwrap();
            assert_eq!(wxy_iter.next(), None);
            println!("wx:{} wy:{}", board.x, board.y);
            let window = Drawer::get_max_size();
            if window.x < board.x || window.y < board.y { panic!("window is too small"); }
        }
        // 3. NA <id> <name>
        {
            let mut message = String::from("NA  Rust;EM;");
            message.insert_str(3, &id.to_string());
            let message = websocket::OwnedMessage::Text(message);
            sender.send_message(&message).unwrap();
        }
    }

    let mut stdin = termion::async_stdin().keys();
    let mut game = Game::new(&board);

    loop {
        // process all messages from server
        loop {
            match rx.try_recv() {
                Ok(message) => { process_from_server(&mut game, &message); },
                Err(mpsc::TryRecvError::Disconnected) => { panic!("disconnected"); },
                Err(mpsc::TryRecvError::Empty) => break,
            }
        }

        let key = stdin.by_ref().last();
        match key {
            Some(Ok(Key::Char('q'))) => break,
            Some(Ok(Key::Left)) => process_client_command(&mut sender, "L"),
            Some(Ok(Key::Right)) => process_client_command(&mut sender, "R"),
            Some(Ok(Key::Up)) => process_client_command(&mut sender, "U"),
            Some(Ok(Key::Down)) => process_client_command(&mut sender, "D"),
            _ => ()
        };

        thread::sleep(time::Duration::from_millis(20));
    }
}

fn main() {
    play();
}
