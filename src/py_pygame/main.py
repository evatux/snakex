import time
import sys, pygame
import network.server_log_reader as network
import message
# import websockets

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
        return [v * self.board_cell_size for v in pos]

    def snake(self, s):
        # FIXME: Add color based on user id
        # FIXME: Dispatch between body and head
        self.rect.fill((0, 255, 0))
        self.board.blit(self.rect, self.pos2pixel(s.position))

    def loot(self, s):
        # FIXME: s.id
        self.rect.fill((200, 200, 200))
        self.board.blit(self.rect, self.pos2pixel(s.position))

    def clear(self, s):
        self.rect.fill((0, 0, 0))
        self.board.blit(self.rect, self.pos2pixel(s))

class Game:
    def __init__(self):
        self.players_score = [0, 0, 0, 0]
        print("init")
        pygame.init()
#        self.network = Network('localhost:1234')
        self.network = network.Network('server.log')
        msg = self.network.get_message()
        while not msg.has_setup:
            msg = self.network.get_message()
        # XXX: st[0] in old protocol, but st[1] in new protocol
        self.graphics = Graphics(msg.statements[0].cells)
        self.update_state(msg)

    def update_clear(self, s):
        print(s.__dict__.keys())
        self.graphics.clear(s.position)

    def update_end_game(self, s):
        self.end_game = True

    def update_loot(self, s):
        print('LOOT: ', s)
        self.graphics.loot(s)

    def update_score_change(self, s):
        self.players_score[s.id] = s.score

    def update_snake(self, s):
        self.graphics.snake(s)

    def update_state(self, msg):
        for s in msg.statements:
            types = {
                'CL' : self.update_clear,
                'EG' : self.update_end_game,
                'LO' : self.update_loot,
                # 'MO' : self.update_move,
                'SC' : self.update_score_change,
                # 'ST' : self.update_setup,
                'SN' : self.update_snake,
                # 'EM' : self.update_end_message
            }
            print(s.type)

            updater = types.get(s.type)
            print(s.type, updater)
            if updater:
                updater(s)

    def play(self):
        print("play")
        while 1:
            for event in pygame.event.get():
                if event.type == pygame.QUIT: sys.exit()
            msg = self.network.get_message()
            self.update_state(msg)
            time.sleep(1)
            self.graphics.draw()

g = Game()
g.play()
