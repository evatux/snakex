import time, sys
import pygame
import argparse
import asyncio
import websockets

#import network.server_log_reader as network
import network.ws_network as network
import message

GREEN = (0, 200, 0)
RED = (200, 0, 0)
BLUE = (0, 0, 200)
YELLOW = (200, 200, 0)

class Graphics:
    def __init__(self, cells):
        self.board_cells = cells
        self.board_cell_size = 25
        self.board_size = [num_cells * self.board_cell_size for num_cells in self.board_cells]
        self.info_size = (200, self.board_size[1])
        self.screen_size = (self.board_size[0] + self.info_size[0], self.board_size[1])
        self.screen = pygame.display.set_mode(self.screen_size, flags=pygame.DOUBLEBUF)
        self.screen.fill((15, 15, 15))
        self.board = pygame.Surface(self.board_size)
        self.board.fill((0, 0, 0))
        self.rect = pygame.Surface((self.board_cell_size, self.board_cell_size))

    def draw(self):
        # FIXME: Add info
        self.screen.blit(self.board, (0, 0))
        pygame.display.flip()

    def pos2pixel(self, pos):
        pixel = [v * self.board_cell_size for v in pos]
        p = pixel[1]
        pixel[1] = self.board_size[1] - pixel[1] - self.board_cell_size
        return pixel

    def snake(self, s):
        if s.id == 0:
            color = GREEN
        elif s.id == 1:
            color = RED
        elif s.id == 2:
            color = BLUE
        elif s.id == 4:
            color = YELLOW
        # FIXME: Dispatch between body and head
        self.rect.fill(color)
        self.board.blit(self.rect, self.pos2pixel(s.position))

    def loot(self, s):
        # FIXME: s.id
        self.rect.fill((200, 200, 200))
        self.board.blit(self.rect, self.pos2pixel(s.position))

    def clear(self, s):
        self.rect.fill((0, 0, 0))
        self.board.blit(self.rect, self.pos2pixel(s))

class Game:
    def __init__(self, net_iface):
        self.players_score = [0, 0, 0, 0]
        self.end_game = False
        pygame.init()
        self.network = net_iface

    def update_clear(self, s):
        print(s.__dict__.keys())
        self.graphics.clear(s.position)

    def update_end_game(self, s):
        self.end_game = True

    def update_loot(self, s):
        self.graphics.loot(s)

    def update_score_change(self, s):
        self.players_score[s.id] = s.score

    def update_snake(self, s):
        self.graphics.snake(s)

    def set_id(self, s):
        self.id = s.id

    def setup_graphics(self, s):
        self.graphics = Graphics(s.cells)

    def update_state(self, msg):
        print(msg)
        for s in msg.statements:
            types = {
                'CL' : self.update_clear,
                'EG' : self.update_end_game,
                'ID' : self.set_id,
                'LO' : self.update_loot,
                # 'MO' : self.update_move,
                'SC' : self.update_score_change,
                'ST' : self.setup_graphics,
                'SN' : self.update_snake,
                # 'EM' : self.update_end_message
            }
            print(s.type)

            updater = types.get(s.type)
            if updater:
                updater(s)

    async def play(self):
        clock = pygame.time.Clock()

        while not self.end_game:
            clock.tick(20)

            recv, msg = await self.network.get_message()
            if recv:
                self.update_state(msg)

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    run = False
            if event.type == pygame.KEYDOWN:
                print(pygame.key.name(event.key))

            keys = pygame.key.get_pressed()
            if keys[pygame.K_RIGHT]: await self.network.send_message("MO R;")
            if keys[pygame.K_LEFT]: await self.network.send_message("MO L;")
            if keys[pygame.K_UP]: await self.network.send_message("MO U;")
            if keys[pygame.K_DOWN]: await self.network.send_message("MO D;")
            if keys[pygame.K_ESCAPE]: self.end_game = True
            self.graphics.draw()

        pygame.quit()
        exit()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
            'Address',
            metavar='address',
            type=str,
            help='address of server. For example, ws://localhost:1234')
    parser.add_argument(
            'Username',
            metavar='username',
            type=str,
            help='username of player. For example, R2D2')

    args = parser.parse_args()

    net_iface = network.Network(args.Address)
    asyncio.get_event_loop().run_until_complete(net_iface.connect(args.Username))

    g = Game(net_iface=net_iface)
    asyncio.get_event_loop().run_until_complete(g.play())

if __name__ == '__main__':
    main()
