import socketserver

class MyUDPHandler(socketserver.BaseRequestHandler):
    def handle(self):
        global cams
        global n
        data = self.request[0].strip()
        socket = self.request[1]
        if data == b"Hi":
            cams.append(self.client_address)
            print("Cameleon added")
            
        if len(cams) >= n and len(cams)%2 == 0:
            for k in range(int(n/2)):
                c1 = cams[2*k]
                c2 = cams[2*k+1]

                socket.sendto(bytes(c1[0], "utf-8"), c2)
                socket.sendto(bytes(str(c1[1]), "utf-8"), c2)
                
                socket.sendto(bytes(c2[0], "utf-8"), c1)
                socket.sendto(bytes(str(c2[1]), "utf-8"), c1)
                
                del cams[:] # on vide la liste
            print("Finished")

cams = [] # caméléons
n = 2 # nombre de caméléons nécessaires pour commencer le processus
        
if __name__ == "__main__":
    HOST, PORT = "localhost", 9999
    server = socketserver.ThreadingUDPServer((HOST, PORT), MyUDPHandler)
    server.serve_forever()
