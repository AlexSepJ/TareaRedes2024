#include <sys/types.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
using namespace std;
extern int errno;

int port;
int flag = 1;

void displayBoard(char* x)
{
    int k = 0;
    for (int i = 0; i < strlen(x); i++)
    {
        if (x[i] == '1')  cout << "| ", k++;
        if ((x[i - 1] == '2' || x[i - 1] == '3' || x[i - 1] == '4') && x[i] == '1') cout << "\n";
        if (x[i] == '2')  cout << "* ", k++;
        if (x[i] == '3')  cout << "\033[1;32mV\033[0m ", k++;
        if (x[i] == '4')  cout << "\033[1;31mR\033[0m ", k++;
    }
}

int main(int argc, char* argv[])
{
    int sd;
    struct sockaddr_in server;
    char received[100];
    char turn;

    if (argc != 3)
    {
        printf("[-]Syntax: %s <server_address> <port>\n", argv[0]);
        return -1;
    }

    port = atoi(argv[2]);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[-]Error at socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr*)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[+]Error at connect().\n");
        return errno;
    }

    cout << "\033[1;31m Game Rule -> \033[0m \n";
    printf("Fill in with numbers from 1 to 7 trying to connect 4 colors to win. \nIf the board is filled and neither of you has won, it will be a draw!\n");
    printf("[+]Waiting for the game partner to connect.\n");

    if (read(sd, received, 100) < 0)
    {
        perror("[-]Error at read() from server.\n");
        return errno;
    }

    turn = received[0];

    if (turn == 'P') {
        printf("You are player number 1! \n");
    }
    else
        printf("You are player number 2! \n");

    char color;
    color = received[0];
    if (color == 'P') {
        cout << "The server has chosen the color \033[1;32m green \033[0m for you! Good luck!\n";
    }
    else
        cout << "The server has chosen the color \033[1;31m red \033[0m for you! Good luck!\n";

    bzero(received, 100);

    while (flag)
    {
        bzero(received, 100);

        if (read(sd, received, 100) < 0)
        {
            perror("[client]Error at read() from server.\n");
            return errno;
        }

        if (received[0] == 'w') {
            char score1 = received[1];

            bzero(received, 100);
            cout << "You won!\n";
            write(sd, "g", 100);
            flag = 0;
        }
        else if (received[0] == 'l')
        {
            bzero(received, 100);
            cout << "You lost!\n";
            write(sd, "g", 100);
            flag = 0;
        }
        else if (received[0] == 'i') 
        {
            bzero(received, 100);
            cout << "It's a draw!\n";
            write(sd, "g", 100);
            flag = 0;
        }
        else if (received[0] == 'j')
        {
            bzero(received, 100);
            cout << "It's a draw!\n";
            write(sd, "g", 100);
            flag = 0;
        }
        
        if (received[0] == 'r')
        {
            bzero(received, 100);
            read(0, received, 100);
            if (write(sd, received, 100) < 0)
            {
                perror("[client]Error at write() to server.\n");
                return errno;
            }
        }

        if (received[0] == 'e')
        {
            cout << "Invalid column.\n";
            bzero(received, 100);
            read(0, received, 100);
            if (write(sd, received, 100) < 0)
            {
                perror("[client]Error at write() to server.\n");
                return errno;
            }
        }

        if (received[0] == 't')
        {
            bzero(received, 100);
            if (read(sd, received, sizeof(received)) < 0)
            {
                perror("[client]Error at write() to server.\n");
                return errno;
            }
            displayBoard(received);
        }
    }

    close(sd);
}

