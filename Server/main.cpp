#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

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
    ctx = SSL_CTX_new(TLS_server_method());

    if (!ctx) {
        std::cerr << "SSL context creation failed." << std::endl;
        return 1;
    }

    // Load the server's certificate and private key
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Failed to load server certificate." << std::endl;
        return 1;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        std::cerr << "Failed to load server private key." << std::endl;
        return 1;
    }

    // Create a TCP socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Error creating server socket." << std::endl;
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the specified address and port
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Error binding server socket." << std::endl;
        close(server_socket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, 5) == -1) {
        std::cerr << "Error listening." << std::endl;
        close(server_socket);
        return 1;
    }

    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    // Accept incoming connections
    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == -1) {
            std::cerr << "Error accepting client connection." << std::endl;
            continue;
        }

        // Create a new SSL connection
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_socket);

        // Perform SSL handshake
        if (SSL_accept(ssl) <= 0) {
            std::cerr << "SSL handshake failed." << std::endl;
            SSL_free(ssl);
            close(client_socket);
            continue;
        }

        // Handle SSL-encrypted communication
        char buffer[BUFFER_SIZE];
        int bytes_received = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::cout << "Received: " << buffer << std::endl;

            // Respond to the client (e.g., send a message)
            const char* response = "Hello, client!";
            SSL_write(ssl, response, strlen(response));
        }

        // Close the SSL connection and client socket
        SSL_free(ssl);
        close(client_socket);
    }

    // Clean up and close the server socket
    SSL_CTX_free(ctx);
    close(server_socket);

    return 0;
}
