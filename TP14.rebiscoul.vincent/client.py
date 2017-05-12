import socket
import sys

def nouvelle_couleur(couleur1, couleur2):
    if couleur1 != b"rouge" and couleur2 != b"rouge":
        return b"rouge"
    if couleur1 != b"bleu" and couleur2 != b"bleu":
        return b"bleu"
    if couleur1 != b"vert" and couleur2 != b"vert":
        return b"vert"

HOST, PORT = "localhost", 9999
couleur = bytes(sys.argv[1], "utf-8")

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(b"Hi\n", (HOST, PORT))
h = sock.recv(1024)
p = sock.recv(1024)
print(b"Partner: " + h + b" " + p)

host, port = h, int(p)

sock.sendto(couleur, (host, port))

couleur_partenaire = sock.recv(1024)

if couleur_partenaire == couleur:
    print("Nous sommes de la même couleur: " + couleur.decode("utf-8")+ " !")
    sock.sendto(b"Ok", (host, port))
else:
    print("Nous sommes de couleurs différentes !")
    couleur = nouvelle_couleur(couleur, couleur_partenaire)
    print("Nous changeons donc de couleurs pour la couleur :" + couleur.decode("utf-8"))
    sock.sendto(b"C'est bon", (host, port))

msg = sock.recv(1024)
print(msg.decode("utf-8"))
