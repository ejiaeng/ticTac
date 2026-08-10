#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080


void print_board(int board[9]) {
    printf("\n");
    for (int i = 0; i < 9; i++) {
        char c = '.';
        if (board[i] == 1) c = 'X';      
        else if (board[i] == 2) c = 'O'; 
        
        printf(" %c ", c);
        if (i % 3 != 2) printf("|");
        else {
            printf("\n");
            if (i < 8) printf("---+---+---\n");
        }
    }
    printf("\n");
}


int check_win(int board[9]) {
    int win_lines[8][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, 
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, 
        {0, 4, 8}, {2, 4, 6}             
    };

    for (int i = 0; i < 8; i++) {
        int a = win_lines[i][0];
        int b = win_lines[i][1];
        int c = win_lines[i][2];
        if (board[a] != 0 && board[a] == board[b] && board[a] == board[c]) {
            return board[a]; 
        }
    }

    for (int i = 0; i < 9; i++) {
        if (board[i] == 0) return -1; 
    }
    
    return 0;
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    int board[9] = {0}; 

  
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }


    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed. Make sure the server is running on port 8080!\n");
        return -1;
    }

    printf("Connected to Tic-Tac-Toe AI Server!\n");
    printf("You are 'X' and you go first.\n");

    while (1) {
        print_board(board);
        
        int status = check_win(board);
        if (status != -1) {
            if (status == 1) printf("Win\n");
            else if (status == 2) printf("Loss\n");
            else printf("Draw\n");
            break;
        }

        int row, col, index;
        while (1) {
            printf("Enter row (0-2) and column (0-2) separated by space: ");
            if (scanf("%d %d", &row, &col) != 2) {
                printf("Invalid input\n");
                while(getchar() != '\n'); 
                continue;
            }
            if (row < 0 || row > 2 || col < 0 || col > 2) {
                printf("Coordinates out of bounds\n");
                continue;
            }
            index = row * 3 + col;
            if (board[index] != 0) {
                printf("Taken spot\n");
                continue;
            }
            break;
        }

        // Apply user move
        board[index] = 1;

        // Check win again in case user's move ended the game
        status = check_win(board);
        if (status != -1) {
            print_board(board);
            if (status == 1) printf("Win\n");
            else if (status == 2) printf("Loss\n");
            else printf("Draw\n");
            break;
        }

        printf("Thinking...\n");
        
        // Send board to server
        if (send(sock, board, sizeof(board), 0) != sizeof(board)) {
            printf("Failed to send board to server.\n");
            break;
        }

        // Receive AI move from server
        if (read(sock, board, sizeof(board)) != sizeof(board)) {
            printf("Failed to read board from server or server disconnected.\n");
            break;
        }
    }

    close(sock);
    return 0;
}