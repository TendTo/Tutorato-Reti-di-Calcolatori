"""Esempio di Socket in python. 
Il client può comunque essere realizzato in qualsiasi linguaggio.
Il server si mette in ascolto su una porta
"""
from socket import SOCK_STREAM, socket, AF_INET
import sys
from typing import TYPE_CHECKING
if TYPE_CHECKING:
    from socket import _RetAddress
# Mi aspetto di ricevere una stringa con il formato
# <op>:<dati>
# Esempio:
# register:Tizio
# login:Caio

REGISTER = b'register'
LOGIN = b'login'
SORT = b'sort'
UPLOAD = b'upload'

class Connection:

    def __init__(self, conn_socket: socket, addr: '_RetAddress'):
        self.conn_socket = conn_socket
        self.addr = addr
        self.users: dict[str, str] = {}

    def __enter__(self) -> 'Connection':
        return self

    def __exit__(self, _, __, ___):
        self.conn_socket.close()

    def login(self, user: bytes) -> bool:
        return user.decode() in self.users

    def register(self, user: bytes):
        self.users[user.decode()] = ""
        return True
    
    def sort(self, data: bytes) -> bytes:
        if b':' not in data:
            return b"Formato errato. Atteso sort:<username>:el1 el2 el3 ... eln"
        username, *elements = data.split(b':')
        if not self.login(username):
            return b"Non sei registrato"
        return b"Array ordinato: " + b" ".join(sorted(element for element in elements))
    
    def upload(self, filename: bytes) -> bytes:
        # upload:<filename>
        with open(filename, 'wb') as f:
            while True:
                data = self.conn_socket.recv(1024)
                if not data:
                    break
                f.write(data)
        return b"File ricevuto"

    def handle_client_connection(self):
        request = self.conn_socket.recv(1024)
        if len(request) == 0 or b":" not in request:
            print("Connessione chiusa")
            return False
        print('Received {}'.format(request))
        operation, *data = request.split(b":")
        data = b":".join(data)
        resp = b"Operazione sconosciuta"
        if operation == REGISTER:
            resp = b"Ti sei registrato" if self.register(data) else b"Registrazione fallita"
        elif operation == LOGIN:
            resp = b"Login effettuato" if self.login(data) else b"Credenziali errate"
        elif operation == SORT:
            resp = self.sort(data)
        elif operation == UPLOAD:
            resp = self.upload(data)
        self.conn_socket.sendall(resp)
        print(self.users)
        return True

def main():
    if len(sys.argv) < 2 or not sys.argv[1].isnumeric():
        print(f"usage: {sys.argv[0]} <port>")
        exit(1)

    port = int(sys.argv[1])
    with socket(AF_INET, SOCK_STREAM) as s:
        s.bind(('0.0.0.0', port))
        s.listen(10)
        while True:
            conn, addr = s.accept()
            with Connection(conn, addr) as c:
                while c.handle_client_connection():
                    pass

if __name__ == '__main__':
    main()
