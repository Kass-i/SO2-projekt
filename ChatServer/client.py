import socket
import threading
import json

with open("config.json") as f:
    config = json.load(f)

HOST = config["host"]
PORT = config["port"]
SIZE = config["size"]
DISCONNECTED_MSG = config["disconnected_msg"]


def receive_messages(sock):
    while True:
        try:
            message = sock.recv(SIZE)
            if not message:
                break
            print("\n" + message.decode())
        except:
            print("Server connection error.")
            break


def main():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((HOST, PORT))

    username = input("Enter nickname: ")

    thread = threading.Thread(target=receive_messages, args=(client_socket,))
    thread.start()

    try:
        while True:
            msg = input()
            if msg == DISCONNECTED_MSG:
                client_socket.send(msg.encode())
                break
            full_msg = f"{username}: {msg}"
            client_socket.send(full_msg.encode())
    finally:
        client_socket.close()

if __name__ == "__main__":
    main()
