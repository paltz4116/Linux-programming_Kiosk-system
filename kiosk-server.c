#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdbool.h>
#include "kiosk.h"
#define DEFAULT_PROTOCOL 0
#define MAXLINE 100

int createFile(int kfd, char *fileName)
{
    if ((kfd = open(fileName, O_RDWR | O_CREAT | O_TRUNC, 0640)) == -1)
    {
        perror("createFile error");
        exit(1);
    }
    return kfd;
}

// 파일에 상품 정보 저장.
int writeFile(int kfd, product *kioskInfo, int kiosknum)
{
    int value;
    lseek(kfd, 0, SEEK_SET);
    value = write(kfd, kioskInfo, (kiosknum * sizeof(product)));
    return value;
}

// 파일 상품 정보 읽기.
int readFile(int kfd, product *kioskInfo, int kiosknum)
{
    int value;
    lseek(kfd, 0, SEEK_SET);
    value = read(kfd, kioskInfo, (kiosknum * sizeof(product)));
    return value;
}

// 서버를 시작할때 키오스크의 정보를 동적으로 받는다.
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

// 클라이언트로부터 구매할 상품의 정보를 받고, 구매처리를 한 후 성공, 실패 여부를 클라이언트에 전달.
int readTaskNum(int cfd, int kfd, product *kioskInfo, int kiosknum)
{
    purchase purInfo[50];
    int index, clientCost, i, totalCost = 0, tmp;
    int num, quantity, tmpCost;
    bool kioskErr = false;
    struct flock lock;

    // 파일 잠금
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;

    if (fcntl(kfd, F_SETLKW, &lock) == -1)
    {
        perror("lock file error");
        exit(1);
    }

    readFile(kfd, kioskInfo, kiosknum);
    write(cfd, kioskInfo, (kiosknum * sizeof(product)));
    printInfo(kioskInfo, kiosknum);

    read(cfd, &index, sizeof(int));

    read(cfd, &purInfo, (index * sizeof(purchase)));

    read(cfd, &clientCost, sizeof(int));

    for (i = 0; i < index; i++)
    {
        num = purInfo[i].num - 1;
        quantity = purInfo[i].quantity;
        tmpCost = kioskInfo[num].cost * quantity;

        if ((kioskInfo[num].quantity - quantity) >= 0 && (totalCost + tmpCost) <= clientCost)
        {
            kioskInfo[num].quantity = kioskInfo[num].quantity - quantity;
            totalCost = totalCost + tmpCost;
        }
        else
        {
            kioskErr = true;
            write(cfd, &kioskErr, sizeof(bool));
            readFile(kfd, kioskInfo, kiosknum);
            lock.l_type = F_UNLCK;
            fcntl(kfd, F_SETLK, &lock);

            return -1;
        }
    }

    if (totalCost > clientCost)
    {
        kioskErr = true;
        write(cfd, &kioskErr, sizeof(bool));
        readFile(kfd, kioskInfo, kiosknum);
        lock.l_type = F_UNLCK;
        fcntl(kfd, F_SETLK, &lock);

        return -1;
    }

    write(cfd, &kioskErr, sizeof(bool));
    write(cfd, kioskInfo, (kiosknum * sizeof(product)));
    writeFile(kfd, kioskInfo, kiosknum);
    lock.l_type = F_UNLCK;
    fcntl(kfd, F_SETLK, &lock);

    return 0;
}

// 전반적인 서버 동작 관리.
void operateServer(product *kioskInfo, int kiosknum, char *fileName)
{
    int listenfd, kfd, connfd, clientlen, task;
    char inmsg[MAXLINE], outmsg[MAXLINE];
    struct sockaddr_un serverAddr, clientAddr;

    printf("서버 시작\n");
    kfd = createFile(kfd, fileName); // 상품 정보를 저장하는 파일 생성.
    writeFile(kfd, kioskInfo, kiosknum);

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
            readFile(kfd, kioskInfo, kiosknum);
            write(connfd, &kiosknum, sizeof(kiosknum));
            write(connfd, kioskInfo, (kiosknum * sizeof(product)));

            read(connfd, &task, sizeof(int));

            while (task == 1)
            {
                readTaskNum(connfd, kfd, kioskInfo, kiosknum);
                read(connfd, &task, sizeof(int));
            }

            close(connfd);
            exit(0);
        }
        else
            close(connfd);
    }
}

int main(int argc, char *argv[])
{
    int num;
    product *kioskInfo;

    kioskInfo = kioskInformation(kioskInfo, &num);
    operateServer(kioskInfo, num, argv[1]);

    free(kioskInfo);

    return 0;
}