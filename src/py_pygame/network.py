import message

class Network:
    def __init__(self, filename):
        self.file = open(filename, 'r')

    def get_message(self, config=False):
        msg_str = self.file.readline()
        print(f"INFO: NETWORK: MESSAGE: {msg_str}")
        return message.Message(msg_str)

