import message
from network.network import Network as BaseClass

class Network(BaseClass):
    def __init__(self, address):
        BaseClass.__init__(self, address) 
        self.file = open(address, 'r')

    def get_message(self):
        msg_str = self.file.readline()
        print(f"INFO: NETWORK: MESSAGE: {msg_str}")
        return message.Message(msg_str)
