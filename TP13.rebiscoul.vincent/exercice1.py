import socketserver
import _thread
from threading import Thread, Lock

m = Lock()

class MyTCPHandler(socketserver.BaseRequestHandler):
    def handle(self):
        global all_msg
        while 1:
            # self.request is the TCP socket connected to the client
            self.data = self.request.recv(1024).strip()
            if self.data == b"":
                print("Connection with {} closed by user".format(self.client_address[0]))
                break;
            print("{} wrote:".format(self.client_address[0]))
            print(self.data)
            # just send back the same data, but upper-cased
            self.request.sendall(self.data+b"\n")
            m.acquire()
            all_msg = all_msg + b"'" + self.data + b"' "
            m.release()
            self.request.sendall(b"All messages since beginning: " + all_msg + b"\n")

all_msg = b""            
if __name__ == "__main__":
    HOST, PORT = "localhost", 9999
    with socketserver.ThreadingTCPServer((HOST, PORT), MyTCPHandler) as server:
        server.serve_forever()
