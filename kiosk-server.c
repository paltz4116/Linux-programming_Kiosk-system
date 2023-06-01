#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>
#include "kiosk.h"
#define DEFAULT_PROTOCOL 0
#define MAXLINE 100

int readLine(int fd, char *str);
void toUpper(char *in, char *out);

product *kioskInformation(product *kioskInfo, int *num)
{
    int i;

    printf("설정할 상품의 개수를 입력해주세요: ");
    scanf("%d", num);

    kioskInfo = (product *)malloc((*num) * sizeof(product));

    printf("%-7s %-7s %-7s\n", "이름", "가격", "수량");

    for (i = 0; i < *num; i++)
    {
        scanf("%s %d %d", kioskInfo[i].name, &kioskInfo[i].cost, &kioskInfo[i].quantity);
    }

    printf("입력 끝\n");

    return kioskInfo;
}

product *readTaskNum(int cfd, product *kioskInfo, product *origin, int kiosknum)
{
    purchase *purInfo;
    int index, clientCost, i, totalCost = 0;
    bool kioskErr = false;

    read(cfd, &index, sizeof(int));

    read(cfd, purInfo, (index * sizeof(purchase)));

    read(cfd, &clientCost, sizeof(int));

    for (i = 0; i < index; i++)
    {
        int num = purInfo[i].num - 1;
        int quantity = purInfo[i].quantity;
        int tmpCost = kioskInfo[num].cost * kioskInfo[num].quantity;

        if ((kioskInfo[num].quantity - quantity) >= 0 && (totalCost + tmpCost) <= clientCost)
        {
            kioskInfo[num].quantity = kioskInfo[num].quantity - quantity;
            totalCost = totalCost + tmpCost;
        }
        else
        {
            kioskErr = true;
            write(cfd, &kioskErr, sizeof(bool));

            return origin;
        }
    }

    if (totalCost > clientCost)
    {
        kioskErr = true;
        write(cfd, &kioskErr, sizeof(bool));

        return origin;
    }

    write(cfd, &kioskErr, sizeof(bool));
    write(cfd, kioskInfo, ((kiosknum) * sizeof(product)));

    return kioskInfo;
}

void operateServer(product *kioskInfo, int kiosknum)
{
    int listenfd, connfd, clientlen, task;
    char inmsg[MAXLINE], outmsg[MAXLINE];
    struct sockaddr_un serverAddr, clientAddr;

    printf("서버 시작\n");
    signal(SIGCHLD, SIG_IGN);
    clientlen = sizeof(clientAddr);

    listenfd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, "convert");
    unlink("convert");
    bind(listenfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    listen(listenfd, 5);

    while (1)
    { /* 소켓 연결 요청 수락 */
        connfd = accept(listenfd, (struct sockaddr *)&clientAddr, &clientlen);
        if (fork() == 0)
        {
            write(connfd, &kiosknum, sizeof(kiosknum));
            write(connfd, kioskInfo, (kiosknum * sizeof(product)));

            read(connfd, &task, sizeof(int));
            while (task == 1)
            {
                kioskInfo = readTaskNum(connfd, kioskInfo, kioskInfo, kiosknum);
                read(connfd, &task, sizeof(int));
            }

            close(connfd);
            exit(0);
        }
        else
            close(connfd);
    }
}

int main()
{
    int num;
    product *kioskInfo;

    kioskInfo = kioskInformation(kioskInfo, &num);
    operateServer(kioskInfo, num);

    free(kioskInfo);

    return 0;
}