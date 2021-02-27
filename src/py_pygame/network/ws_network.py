import message
import asyncio, websockets
from network.network import Network as BaseClass

class Network(BaseClass):
    def __init__(self, address):
        BaseClass.__init__(self, address)
        self.id = -1
        self.ws = None
        self.task = None

    async def connect(self, username):
        self.ws = await websockets.connect(self.address)

        setup_str = await self.ws.recv()
        print("@@@ server: %s" % setup_str)

        self.id = int(setup_str.split(';')[0].split(' ')[1])
        send_ack = "NA %d %s;" % (self.id, username)
        print("@@@ client: %s" % send_ack)
        await self.ws.send(send_ack)

    async def get_message(self):
        if self.task is None:
            self.task = asyncio.create_task(self.ws.recv())
        done, pending = await asyncio.wait({self.task}, timeout=1e-1)
        if self.task in done:
            if self.task.done():
                data = self.task.result()
                self.task = None
                print(data)
                return True, message.Message(data)
        return False, None

    async def send_message(self, msg):
        print(f"send msg: {msg}")
        await self.ws.send(msg)
