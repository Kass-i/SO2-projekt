import socket
import threading
import json

with open("config.json") as f:
    config = json.load(f)

HOST = config["host"]
PORT = config["port"]
SIZE = config["size"]
DISCONNECTED_MSG = config["disconnected_msg"]
disconnected = False


def receive_messages(sock):
    global disconnected
    while True:
        try:
            message = sock.recv(SIZE)
            if not message:
                print("⚠️ Server closed the connection.")
                disconnected = True
                break
            print("\n" + message.decode())
        except:
            print("⚠️ Server connection error.")
            disconnected = True
            break


def main():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((HOST, PORT))

    username = input("Enter nickname: ")

    thread = threading.Thread(target=receive_messages, args=(client_socket,))
    thread.daemon = True
    thread.start()

    try:
        while True:
            msg = input()

            if disconnected:
                print("❌ Server is disconnected.")
                input("Press any key...")
                break

            if msg == DISCONNECTED_MSG:
                client_socket.send(msg.encode())
                break
            full_msg = f"{username}: {msg}"
            client_socket.send(full_msg.encode())
    finally:
        client_socket.close()

if __name__ == "__main__":
    main()
