import time, sys
import pygame
import argparse
import asyncio
import websockets

#import network.server_log_reader as network
import network.ws_network as network
import message
import interface.graphics as graphics

class Game:
    def __init__(self, net_iface):
        self.players_scores = [0, 0, 0, 0]
        self.players_names = ['AAAA', 'BBBB', 'CCCC', 'DDDD']
        self.start_game = False
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
        self.players_scores[s.id] = s.score

    def update_snake(self, s):
        self.graphics.snake(s)

    def set_id(self, s):
        self.id = s.id

    def setup_graphics(self, s):
        self.graphics = graphics.Graphics(s.cells)
        self.start_game = True

    def update_player_name(self, s):
        print(f"UPDATE_PLAYER_NAME {s.id} {s.name}")
        self.players_scores[s.id] = 0
        self.players_names[s.id] = s.name

    def update_state(self, msg):
        print(msg)
        for s in msg.statements:
            types = {
                'CL' : self.update_clear,
                'EG' : self.update_end_game,
                'ID' : self.set_id,
                'LO' : self.update_loot,
                # 'MO' : self.update_move,
                'NA' : self.update_player_name,
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

        while not self.start_game:
            recv, msg = await self.network.get_message()
            if recv:
                self.update_state(msg)

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

            self.graphics.draw_info(self.players_scores, self.players_names)
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
