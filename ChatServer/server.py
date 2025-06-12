import socket
import threading
import json

with open("config.json") as f:
    config = json.load(f)

HOST = config["host"]
PORT = config["port"]
SIZE = config["size"]
DISCONNECT_MSG = config["disconnected_msg"]

clients = []
clients_lock = threading.Lock()


def broadcast(message, sender_socket):
    with clients_lock:
        for client in clients:
            if client == sender_socket:
                continue
            
            try:
                client.send(message)
            except:
                clients.remove(client)


def handle_client(client_socket, address):
    print(f"➕ New connection: {address}")
    with clients_lock:
        clients.append(client_socket)

    try:
        while True:
            message = client_socket.recv(SIZE)
            if not message:
                break

            decoded_message = message.decode()
            if decoded_message == DISCONNECT_MSG:
                print(f"🔌 Client [{address}] ended the session")
                break

            print(f"📩 {address} {decoded_message}")
            broadcast(message, client_socket)
    except ConnectionResetError:
        print(f"🚨 {address} was suddenly disconnected")
    finally:
        with clients_lock:
            clients.remove(client_socket)
        client_socket.close()
        print(f"🔌 Disconnected: {address}")


def start_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((HOST, PORT))
    server.listen()
    server.settimeout(0.5)

    print(f"Server is listening on {HOST}:{PORT}")

    try:
        while True:
            try:
                client_socket, address = server.accept()
                thread = threading.Thread(target=handle_client, args=(client_socket, address))
                thread.daemon = True
                thread.start()
            except socket.timeout:
                continue
    except KeyboardInterrupt:
        print("⛔ Stopped the server with Ctrl+C")
    finally:
        server.close()
        print("👋 Server closed.")
        input("Press any key...")



if __name__ == "__main__":
    try:
        start_server()
    except KeyboardInterrupt:
        print("⛔ Stopped the server with Ctrl+C")
        print("👋 Server closed.")
        input("Press any key...")
