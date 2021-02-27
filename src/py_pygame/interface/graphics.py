import pygame

GREEN = (0, 200, 0)
RED = (200, 0, 0)
BLUE = (0, 0, 200)
YELLOW = (200, 200, 0)
BLACK = (0, 0, 0)
DARK_GREY = (35, 35, 35)
LIGHT_GREY = (200, 200, 200)

class Graphics:
    def __init__(self, cells):
        self.board_cells = cells
        self.board_cell_size = 25
        self.board_size = [num_cells * self.board_cell_size for num_cells in self.board_cells]
        self.info_size = (self.board_size[0], 40)
        self.screen_size = (self.board_size[0],  self.info_size[1] + self.board_size[1])

        self.screen = pygame.display.set_mode(self.screen_size, flags=pygame.DOUBLEBUF)
        self.screen.fill(DARK_GREY)

        self.board = pygame.Surface(self.board_size)
        self.board.fill(BLACK)

        self.info = pygame.Surface(self.info_size)
        self.info.fill(GREEN)
        self.font = pygame.font.SysFont(None, 44)


        self.rect = pygame.Surface((self.board_cell_size, self.board_cell_size))

        # TODO: make interface modular
        # info = Window(0, 0, 'info')
        # player_score = Window(self.player_score_size, 'score')
        # self.info.append(player_score, 'b')
        # self.info.append(player_score, 'r')
        # self.info.append(player_score, 'r')
        # self.info.append(player_score, 'r')
        # self.game_window = Window(0, 0, 'game')
        # self.game_window.append(self.info, 'b');
        # board = Window(self.board_size, 'board')
        # self.game_window.append(board, 'b');

    def draw(self):
        # FIXME: Add info
        self.screen.blit(self.board, (0, 0))
        self.screen.blit(self.info, (0, self.board_size[1]))
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

    def draw_info(self, scores, names):
        self.info.fill(DARK_GREY)

        field_length = self.info_size[0] / len(scores)
        for i in range(0, len(scores)):
            name = names[i][0:6]
            text_surface = self.font.render(f"{name}: {scores[i]}", True, LIGHT_GREY)
            clip_area = pygame.Rect((0, 0), (field_length-7-10, self.info_size[1] - 14))
            self.info.blit(text_surface, (7 + field_length * i, 7), clip_area)

    def clear(self, s):
        self.rect.fill((0, 0, 0))
        self.board.blit(self.rect, self.pos2pixel(s))

