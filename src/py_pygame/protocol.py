import message

class Network:
    def __init__(self, filename):
        self.file = open(file=filename, 'r')

    def __del__(self):
        close(self.file)

    def get_message(self):
        msg = self.file.readline()
        print(f"INFO: NETWORK: MESSAGE: {msg}")
        return msg

