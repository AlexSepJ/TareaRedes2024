#include <sys/types.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#define _BSD_SOURCE
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 4444

using namespace std;
extern int errno;
char message[100], message2[100];
struct PlayerInfo {
    char playerName[256], playerSign;
    int number;
    int score = 0;
};
PlayerInfo player1, player2;
char board[9][10];
int width = 7, height = 6, chosenColumn, win, full, again;


extern int errno; 

char* convert_address(struct sockaddr_in address)
{
    static char str[25];
    char port[7];

    strcpy(str, inet_ntoa(address.sin_addr));
    bzero(port, 7);
    sprintf(port, ":%d", ntohs(address.sin_port));
    strcat(str, port);
    return (str);
}

int playerTurn(char board[][10], PlayerInfo active)
{
    char buffer[100];
    char errorMessage[200];
    int chosenColumn;
    write(active.number, "r", 100);
    do {
        cout << "Turn of player " << active.number << "\n";
        char received[100];
        if (read(active.number, received, 100) <= 0) {
            perror("[server]Error reading from client.\n");
            close(active.number); 
            continue; 
        }
        chosenColumn = atoi(received);

        while (board[1][chosenColumn] == 'X' || board[1][chosenColumn] == 'O') {
            write(active.number, "e", sizeof(errorMessage));
            if (read(active.number, received, 100) <= 0) {
                perror("[server]Error reading from client.\n");
                close(active.number); 
                continue; 
            }
            chosenColumn = atoi(received);
        }

    } while (chosenColumn < 1 || chosenColumn > 7);

    return chosenColumn;
}

void move(char board[][10], PlayerInfo active, int chosenColumn)
{
    int length, turn;
    length = 6;
    turn = 0;

    do {
        if (board[length][chosenColumn] != 'X' && board[length][chosenColumn] != 'O') {
            board[length][chosenColumn] = active.playerSign;
            turn = 1;
        }
        else
            --length;
    } while (turn != 1);
}

void displayBoard(char board[][10], PlayerInfo active)
{
    int rows = 6, columns = 7, i, j;

    char something[2024];
    for (i = 1; i <= rows; i++) {
        cout << "| ";
        strcat(something, "1");
        for (j = 1; j <= columns; j++) {
            if (board[i][j] != 'X' && board[i][j] != 'O') {
                board[i][j] = '*';
                strcat(something, "2");
            }
            cout << board[i][j] << " ";
            if (board[i][j] == 'X') {
                strcat(something, "3");
            }
            if (board[i][j] == 'O') {
                strcat(something, "4");
            }
        }

        cout << "| " << endl;
        strcat(something, "1");
    }
    printf("Matrix to send %s\n", something);
    write(active.number, "t", 100);
    if (write(active.number, something, sizeof(something)) <= 0) {
        perror("[server]Error writing to client.\n");
    }
    else
        printf("[server]Message sent successfully.\n"), bzero(something, 2000);
}

int checkSolution(char board[][10], PlayerInfo active)
{
    char sign;
    int win;

    sign = active.playerSign;
    win = 0;

    for (int i = 8; i >= 1; --i) {

        for (int j = 9; j >= 1; --j) {

            if (board[i][j] == sign && board[i - 1][j - 1] == sign && board[i - 2][j - 2] == sign && board[i - 3][j - 3] == sign) {
                win = 1;
            }

            if (board[i][j] == sign && board[i][j - 1] == sign && board[i][j - 2] == sign && board[i][j - 3] == sign) {
                win = 1;
            }

            if (board[i][j] == sign && board[i - 1][j] == sign && board[i - 2][j] == sign && board[i - 3][j] == sign) {
                win = 1;
            }

            if (board[i][j] == sign && board[i - 1][j + 1] == sign && board[i - 2][j + 2] == sign && board[i - 3][j + 3] == sign) {
                win = 1;
            }

            if (board[i][j] == sign && board[i][j + 1] == sign && board[i][j + 2] == sign && board[i][j + 3] == sign) {
                win = 1;
            }
        }
    }

    return win;
}

int isBoardFull(char board[][10])
{
    int full;
    full = 0;
    for (int i = 1; i <= 7; ++i) {
        if (board[1][i] != '*')
            ++full;
    }
    return full;
}

void winner(PlayerInfo active, PlayerInfo active1)
{
    cout << endl;
    cout << "Player " << active.number << " connected 4 pieces. You win!" << endl;
    char* x;
    write(active.number, "w", 100);
    read(active.number, x, 100);
    write(active1.number, "l", 100);
    read(active.number, x, 100);
}
void tie(PlayerInfo active, PlayerInfo active1)
{
    cout << endl;
    cout << "It's a tie!\n";
    char* x;
    write(active.number, "i", 100);
    read(active.number, x, 100);
    write(active1.number, "j", 100);
    read(active.number, x, 100);
}

int restartGame(char board[][10], PlayerInfo active)
{
    int restart;
    char* u;
    cout << "Do you want to restart the game? 1 - Yes, 2 - No ";
    read(active.number, u, 10);
    restart = atoi(u);
    if (restart == 1) {
        for (int i = 1; i <= 6; i++) {
            for (int j = 1; j <= 7; j++) {
                board[i][j] = '*';
            }
        }
    }
    else
        cout << "See you next time!" << endl;
    return restart;
}

int main()
{
    struct sockaddr_in server; 
    struct sockaddr_in from;
    char message[100]; 
    int sd; 

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[server]Error creating socket().\n");
        return errno;
    }

    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr*)&server, sizeof(struct sockaddr)) == -1) {
        perror("[server]Error binding socket().\n");
        return errno;
    }

    if (listen(sd, 1) == -1) {
        perror("[server]Error listening.\n");
        return errno;
    }

    while (1) {
        int player1, player2;
        unsigned int length = sizeof(from);

        printf("[server]Waiting on port %d...\n", PORT);
        fflush(stdout);

        player1 = accept(sd, (struct sockaddr*)&from, &length);
        player2 = accept(sd, (struct sockaddr*)&from, &length);
        if (player1 < 0 ) {
            perror("[server]Error accepting player 1.\n");
            continue;
        }
        if (player2 < 0) {
            perror("[server]Error accepting player 2.\n");
            continue;
        }

        int pid;
        if ((pid = fork()) == -1) {
            close(player1);
            close(player2);
            continue;
        }
        else if (pid > 0) {
            close(player1);
            close(player2);
            while (waitpid(-1, NULL, WNOHANG));
            continue;
        }
        else if (pid == 0) {
            close(sd);
            if (write(player1, "P", 100) <= 0) {
                perror("[server]Error writing to client.\n");
                continue; 
            }
            if (write(player2, "D", 100) <= 0) {
                perror("[server]Error writing to client.\n");
                continue;
            }
            printf("[server]Waiting for message...\n");
            fflush(stdout);
            while (1) {
                bzero(message, 100);


                PlayerInfo player1, player2;
                char board[9][10];
                int width = 7;
                int height = 6;
                int chosenColumn, win, full, again;

                cout << "Let's start the game" << endl << endl;
                player1.playerSign = 'X';
                player1.number = 1;
                player2.playerSign = 'O';
                player2.number = 2;

                full = 0;
                win = 0;
                again = 0;
                int tieVariable = 0;
                do {
                    chosenColumn = playerTurn(board, player1);
                    move(board, player1, chosenColumn);
                    displayBoard(board, player2);
                    win = checkSolution(board, player1);
                    tieVariable = 0;
                    tieVariable = isBoardFull(board);
                    if (tieVariable == 7) {
                        tie(player1, player2);
                        again = 2;
                        if (again == 2) {
                            break;
                        }
                    }
                    if (win == 1) {
                        player1.score++;
                        winner(player1, player2);
                        again = 2;
                        if (again == 2) {
                            break;
                        }
                    }

                    chosenColumn = playerTurn(board, player2);
                    move(board, player2, chosenColumn);
                    displayBoard(board, player1);
                    win = checkSolution(board, player2);
                    tieVariable = 0;
                    tieVariable = isBoardFull(board);
                    if (tieVariable == 7) {
                        tie(player2, player1);
                        again = 2;
                        if (again == 2) {
                            break;
                        }
                    }
                    if (win == 1) {
                        player2.score++;
                        winner(player2, player1);
                        again = 2;
                        if (again == 2) {
                            break;
                        }
                    }
                } while (again != 2);
                printf("Final score: Player 1 %d - %d Player 2\n", player1.score, player2.score);
                break;
            }
            close(player1);
            close(player2);
            exit(0);
        }

    } 
}

