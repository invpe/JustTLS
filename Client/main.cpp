#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    SSL_CTX* ctx;
    SSL* ssl;

    // Initialize the SSL library
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    
    // Create a new SSL context using the TLS method
    ctx = SSL_CTX_new(TLS_client_method());

    if (!ctx) {
        std::cerr << "SSL context creation failed." << std::endl;
        return 1;
    }

    // Create a TCP socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Error creating client socket." << std::endl;
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Error connecting to the server." << std::endl;
        close(client_socket);
        return 1;
    }

    // Create an SSL connection
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_socket);

    // Perform SSL handshake
    if (SSL_connect(ssl) <= 0) {
        std::cerr << "SSL handshake failed." << std::endl;
        SSL_free(ssl);
        close(client_socket);
        return 1;
    }

    // SSL-encrypted communication
    const char* message = "Hello, server!";
    SSL_write(ssl, message, strlen(message));

    char buffer[BUFFER_SIZE];
    int bytes_received = SSL_read(ssl, buffer, sizeof(buffer));
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::cout << "Received: " << buffer << std::endl;
    }

    // Close the SSL connection and client socket
    SSL_free(ssl);
    close(client_socket);

    // Clean up SSL context
    SSL_CTX_free(ctx);

    return 0;
}
