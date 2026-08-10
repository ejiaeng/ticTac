#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "handle.h"

static char *minimax_json = NULL;
static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_minimax() {
    pthread_mutex_lock(&init_mutex);
    if (minimax_json == NULL) {
        FILE *f = fopen("minimax.json", "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);
            minimax_json = malloc(fsize + 1);
            if (minimax_json) {
                fread(minimax_json, 1, fsize, f);
                minimax_json[fsize] = '\0';
            }
            fclose(f);
        } else {
            perror("Failed to open minimax.json");
        }
    }
    pthread_mutex_unlock(&init_mutex);
}

void get_best_move(int *board) {
    if (!minimax_json) return;

    int best_score = 999999;
    int best_move = -1;

    for (int i = 0; i < 9; i++) {
        if (board[i] == 0) {
            board[i] = 2; // AI attempts move

            char search_str[64];
            int p_board[9];
            for (int j = 0; j < 9; j++) {
                if (board[j] == 1) p_board[j] = 1;
                else if (board[j] == 2) p_board[j] = -1;
                else p_board[j] = 0;
            }

            snprintf(search_str, sizeof(search_str), "\"%d,%d,%d,%d,%d,%d,%d,%d,%d\":",
                     p_board[0], p_board[1], p_board[2], p_board[3],
                     p_board[4], p_board[5], p_board[6], p_board[7], p_board[8]);

            char *ptr = strstr(minimax_json, search_str);
            if (ptr) {
                int score = 0;
                sscanf(ptr + strlen(search_str), " %d", &score);
                
                // AI is player -1 in Python, so it wants to MINIMIZE the score
                if (score < best_score) {
                    best_score = score;
                    best_move = i;
                }
            }
            
            board[i] = 0; // undo move
        }
    }

    if (best_move != -1) {
        board[best_move] = 2;
    }
}


void *handle_client(void *socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc);
    
    int board[3][3];
    ssize_t valread;
    
    // Continuously read exactly 36 bytes (3x3 array of ints) from the client
    while (1) {
        // Read 36 bytes exactly
        valread = read(sock, board, sizeof(board));
        
        if (valread <= 0 || valread != sizeof(board)) {
            send(sock, "wrong size board", sizeof("wrong size board"), 0);
        }

        printf("Received valid 36-byte board from client. Calling Local Minimax...\n");

        init_minimax();
        get_best_move((int*)board);
        
        // Send the exact 36-byte board back to the client
        send(sock, board, sizeof(board), 0);
        printf("Sent updated 36-byte board to client.\n");
    }
    
    close(sock);
    return NULL;
}

