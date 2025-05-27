import socket


def get_ip_address():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    ip_address = s.getsockname()[0]
    s.close()
    return ip_address

print(get_ip_address())



# Configuration
HOST = get_ip_address()  # Listen on all interfaces
PORT = 5000        # Must match the port your STM32 client connects to

def start_tcp_server():
    client = 0
    clients = []
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.bind((HOST, PORT))
        server_socket.listen()

        print(f"[+] TCP server listening on {HOST}:{PORT}")
        
        while True:
            client_socket, client_address = server_socket.accept()
            clients.append(client_socket)
            print(f"[+] New connection from {client_address}")

            client += 1
            # Send a message to the client
            
            data = [b'\x01\x00\x00d\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00', b'\x02\x00\x00d\xa3\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00']

            if (client == 2):
                while True:  
                    data[0] = clients[0].recv(1024) 
                    clients[0].sendall(data[1])  
                
                    data[1] = clients[1].recv(1024)
                    clients[1].sendall(data[0])

                    

if __name__ == "__main__":
    start_tcp_server()
