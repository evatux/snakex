"use strict;"

let scale = 10;

// Get the canvas element
const snakeboard = document.getElementById("snakeboard");
const snakeboard_ctx = snakeboard.getContext("2d");

let socket = null;

// 0 -- game not started
// 1 -- game started
// 2 -- game finished
let game_state = 0;

let id = -1;

let swiper;

main();

function setScale(wx, wy) {
    const w = window.innerWidth * 2 / 3;
    const h = window.innerHeight * 2 / 3;

    const scale_x = Math.floor(w / wx);
    const scale_y = Math.floor(h / wy);
    scale = Math.min(scale_x, scale_y);
    log(`@client [init] scale:${scale}`);
}

function main() {
    document.addEventListener("keydown", key_listener);

    swiper = new Swipe('snakeboard');
    swiper.onLeft(function() { input_process("L"); });
    swiper.onRight(function() { input_process("R"); });
    swiper.onUp(function() { input_process("U"); });
    swiper.onDown(function() { input_process("D"); });
    swiper.run();
}

function end_game() {
    log('@client [close]');
    if (game_state == 1) {
        send("EG;EM;");
        game_state = 2;
    }
    socket.close();
    socket.onclose = null; // why?
    id = -1;
}

function process_cmd(cmd) {
    if (cmd == "") return;

    let throw_err = () => { log("invalid command '" + cmd + "'"); };
    if (cmd.length < 2) throw_err();

    let args = cmd.split(" ");

    switch (cmd.substr(0,2)) {
        case "CL":
            draw(" ", args[1], args[2], -1);
            break;
        case "EM":
            break;
        case "EG":
            game_state = 2;
            end_game();
            break;
        case "ST":
            break;
        case "NA":
            {
                const that_id = args[1];
                const that_name = args[2];
                log(`@server [recv] NA: id:${that_id} name:${that_name}`);

                let text = `[${that_id}] ${that_name}`;
                if (id == that_id) text += " (YOU)";
                text += ": ";

                document.getElementById(`p${that_id}_name`).innerHTML = text;
            }
            break;
        case "SC":
            {
                const that_id = args[1];
                const that_abs_score = Math.abs(args[2]);
                const is_dead = args[2] < 0;

                let style = "";
                if (is_dead) style += "color: red;"
                if (id == that_id) style += "font-weight: bold;"
                document.getElementById(`p${that_id}_div`).hidden = false;
                document.getElementById(`p${that_id}_div`).style = style;

                let text = `${that_abs_score}`;
                if (is_dead) text += " DEAD";
                document.getElementById(`p${that_id}_score`).innerHTML = text;
            }
            break;
        case "SN":
            const chr = args[2] == "h" ? "@" : "x";
            draw(chr, args[3], args[4], args[1]);
            break;
        case "LO":
            draw("$", args[2], args[3], -1);
            break;
        default:
            throw_err();
    }
}

function process_server_input(message) {
    log(`@server [recv]: ${message}`);
    if (game_state == 0) {
        let cmds = message.split(';');
        // id
        {
            let args = cmds[0].split(" ");
            if (args[0] != "ID") log("@server [recv] broken protocol");
            id = args[1];
        }
        // windows size
        let wx, wy;
        {
            let args = cmds[1].split(" ");
            if (args[0] != "ST") log("@server [recv] broken protocol");
            wx = args[1];
            wy = args[2];
        }
        setScale(wx, wy);
        snakeboard.width = 2 + wx * scale;
        snakeboard.height = 2 + wy * scale;
        const this_name = document.getElementById("conf_name").value;
        send(`NA ${id} ${this_name};EM;`);
        game_state = 1;

        snakeboard_ctx.lineWidth = 1;
        clearBoard();
        redrawBoardBoundaries();
    } else {
        let cmds = message.split(';');
        cmds.forEach(process_cmd);
    }
}

function send(message) {
    log(`@client [send]: ${message}`);
    socket.send(message);
}

function init_connection() {
    if (socket) { end_game(); }

    game_state = 0;
    const server = document.getElementById("conf_addr").value;
    const port = document.getElementById("conf_port").value;

    log(`@client [init] connecting to ${server}:${port}`);
    socket = new WebSocket("ws://" + server + ":" + port);
    socket.onopen = function(e) {
        log(`@server [open]: connection established ${socket}`);
    };
    socket.onmessage = (event) => { process_server_input(event.data) };
    socket.onclose = function(event) {
        if (event.wasClean) {
            log(`@server [close]: connection closed cleanly. code:${event.code} reason:${event.reason}`);
        } else {
            log('@server [close]: connection died');
        }
        game_state = 0;
        socket = null;
    };
}

function redrawBoardBoundaries() {
    snakeboard_ctx.strokeStyle = "black";
    snakeboard_ctx.strokeRect(0, 0, snakeboard.width, snakeboard.height);
}

function clearBoard() {
    snakeboard_ctx.fillStyle = "white";
    snakeboard_ctx.fillRect(1, 1, snakeboard.width - 2, snakeboard.height - 2);
}

function draw(chr, pos_x, pos_y, that_id) {
    let inner_color, outer_color;

    if (chr == " ") {
        inner_color = outer_color = "white";
    } else if (chr == "$") {
        inner_color = "lightgreen";
        outer_color = "darkgreen";
    } else {
        const pcolors = [
            ["blue", "lightblue", "darkblue"],
            ["salmon", "lightsalmon", "indianred"],
            ["seagreen", "lightseagreen", "darkseagreen"],
            ["cyan", "lightcyan", "darkcyan"],
        ][that_id];
        inner_color = chr == "@" ? pcolors[0] : pcolors[1];
        outer_color = pcolors[2];
    }

    const x = 1 + pos_x * scale;
    const y = snakeboard.height - 1 - scale - pos_y * scale;

    snakeboard_ctx.fillStyle = outer_color;
    snakeboard_ctx.fillRect(x, y, scale, scale);
    snakeboard_ctx.fillStyle = inner_color;
    snakeboard_ctx.fillRect(x + 1, y + 1, scale - 2, scale - 2);
}

function input_process(cmd) {
    if (game_state != 1) return;
    switch (cmd) {
        case "Q": end_game(); break;
        case "L":
        case "R":
        case "U":
        case "D":
            send('MO ' + cmd + ';EM;');
            break;
        default:
            log(`@client [input] unknown command: ${cmd}`);
    };
}

function key_listener(event) {
    const cmds = [
        {code:81, cmd:"Q"},
        {code:37, cmd:"L"},
        {code:39, cmd:"R"},
        {code:38, cmd:"U"},
        {code:40, cmd:"D"},
    ];
    cmds.forEach((key) => {
        if (key.code === event.keyCode) input_process(key.cmd);
    });
}
