import pygame
import time

GREEN = (0, 200, 0)
RED = (200, 0, 0)
BLUE = (0, 0, 200)
YELLOW = (200, 200, 0)
BLACK = (0, 0, 0)
DARK_GREY = (35, 35, 35)
LIGHT_GREY = (200, 200, 200)

class Window:
    def __init__(self, size, display = None, offset = (0, 0), name = '', color = BLACK):
        self.size = size
        self.offset = offset
        self.child_windows = []
        self.display = display
        self.color = color
        if self.display == None:
            self.name = f"Window::{name}"
            self.surface = pygame.display.set_mode(self.size, flags=pygame.DOUBLEBUF)
            self.offset = (0, 0)
            self.surface.fill(color)
        else:
            self.name = f"{display.name}->Window::{name}"
            display.register(self)
            self.display = display
            self.surface = pygame.Surface(self.size)
            self.surface.fill(color)

    def register(self, child):
        self.child_windows.append(child)

    def remove_child(self, child):
        if child in self.child_windows:
            self.child_windows.remove(child)

    def reset(self):
        self.display = None

    def set_color(self, color):
        if color != self.color:
            self.color = color
            self.surface.fill(color)

    def render(self):
        self.surface.fill(self.color)
        for c in self.child_windows:
            c.render()
        if self.display != None:
            self.display.surface.blit(self.surface, self.offset)

    def remove(self):
        for child in self.child_windows:
            child.reset()

        if self.display != None:
            self.display.remove_child(self)

class Label(Window):
    # TODO: Add parameter `align` as an alternative for `offset`
    def __init__(self, text, display, font, offset = (0, 0), name = '', color=BLACK):
        self.offset = offset
        self.display = display
        self.name = f"{display.name}->Label::{name}"
        self.text = text
        self.color = color
        self.surface = font.render(self.text, 1, color)
        self.display = display
        display.register(self)

    def set_color(self, color):
        if color != self.color:
            self.color = color
            self.surface = font.render(self.text, 1, self.color)

    def render(self):
        print(f"render: {self.name}")
        self.display.surface.blit(self.surface, self.offset)


def simple_test():
    pygame.init()

    main = Window(size=(200,200), name="Main", color=BLUE)
    button_a = Window(size=(150, 50), display=main, offset=(25, 25), name='Button A', color=RED)
    button_b = Window(size=(150, 50), display=main, offset=(25, 125), name='Button B', color=YELLOW)
    button_c = Window(size=(100,20), display=button_b, offset=(25,15), name='Button C', color=GREEN)
    font = pygame.font.Font(None, 24)
    label_c = Label(text='hola mucho', display=button_c, font=font, offset = (5, 0), name='Label C')

    main.render()
    time.sleep(2)

    pygame.display.flip()
    time.sleep(2)

    button_b.remove()

    main.render()
    pygame.display.flip()
    time.sleep(2)

simple_test()
