#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h> // Include pthread for threading
#include "handle.h"

#define PORT 8080

// Thread function to handle client communication


int main(int argc, char const* argv[]) {
    // Load .env variables (e.g. GEMINI_API_KEY)
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // 1. Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Optional: forcefully attaching socket to the port 8080 to prevent "Address already in use" errors
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 2. Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // 3. Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d...\n", PORT);

    // 4. Continuously accept incoming connections
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("accept");
            continue; // Continue accepting other connections even if one fails
        }
        
        printf("New connection accepted\n");
        
        // Dynamically allocate memory for the new socket descriptor to prevent race conditions
        // if the main thread changes `new_socket` before the new thread reads it.
        int *new_sock_ptr = malloc(sizeof(int));
        *new_sock_ptr = new_socket;
        
        pthread_t client_thread;
        
        // Create a new thread for each client
        if (pthread_create(&client_thread, NULL, handle_client, (void*)new_sock_ptr) < 0) {
            perror("could not create thread");
            free(new_sock_ptr);
            close(new_socket);
            continue;
        }
        
        // Detach the thread so its resources are automatically freed when it finishes
        pthread_detach(client_thread);
    }
    
    // Close the listening socket (unreachable in this infinite loop, but good practice)
    close(server_fd);
    
    return 0;
}