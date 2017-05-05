import socketserver
import urllib.request

class MyTCPHandler(socketserver.BaseRequestHandler):
    def handle(self):
        while 1:
            self.data = self.request.recv(1024).strip()
            if self.data == b"":
                print("Connection with {} closed by user".format(self.client_address[0]))
                break;
            words = self.data.split()
            host, req, user = "", "", ""
            for k in range(0, len(words)):
                temp = words[k].decode("utf-8")
                if temp == "GET":
                    req = words[k+1].decode("utf-8")
                if temp == "User-Agent:":
                    user = words[k+1].decode("utf-8")
                if temp == "Host:":
                    host = words[k+1].decode("utf-8")
            print("Host: " + host + "\n" + "Page: " + req)
            
            if req != "":
                print("User-Agent: " + user)

            print("\nEverything:")
            for word in words:
                print("'" + word.decode("utf-8") + "' ")

            webpage = urllib.request.urlopen(host + req)
            self.request.sendall(self.data+b"\n")
            self.request.sendall(bytes(webpage, 'utf-8'))
            

if __name__ == "__main__":
    HOST, PORT = "localhost", 9998
    with socketserver.ThreadingTCPServer((HOST, PORT), MyTCPHandler) as server:
        server.serve_forever()
