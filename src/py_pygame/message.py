class Statement:
    def parse_clear(self, args):
        self.position = int(args[0]), int(args[1])

    def parse_end_game(self, args):
        self.end_game = True

    def parse_id(self, args):
        self.id = int(args[0])

    def parse_loot(self, args):
        self.id = int(args[0])
        self.position = int(args[1]), int(args[2])

    def parse_move(self, args):
        self.dir = args[0]

    def parse_name(self, args):
        self.id = int(args[0])
        self.name = args[1]

    def parse_score_change(self, args):
        self.id = int(args[0])
        self.score = int(args[1])

    def parse_setup(self, args):
        self.cells = int(args[0]), int(args[1])

    def parse_snake(self, args):
        self.id = int(args[0])
        self.part = args[1]
        self.position = int(args[2]), int(args[3])
        if self.part == 'h':
            self.dir = args[4]

    def parse_end_message(self, args):
        self.end_message = True

    def __init__(self, string):
        types = {
            'CL' : self.parse_clear,
            'EG' : self.parse_end_game,
            'ID' : self.parse_id,
            'LO' : self.parse_loot,
            'MO' : self.parse_move,
            'SC' : self.parse_score_change,
            'ST' : self.parse_setup,
            'SN' : self.parse_snake,
            'EM' : self.parse_end_message
        }
        self.type = string[0:2]
        parser = types.get(self.type)
        if parser:
            parser(string[3:].split(' '))

class Message:
    def __init__(self):
        self.is_empty = True

    def __init__(self, string):
        self.is_empty = True
        self.statements = []
        if string:
            statements = string[0:].split(';')
            if len(statements):
                self.is_empty = False
                for s in statements:
                    ss = Statement(s)
                    self.statements.append(ss)
                    print(ss.type)
