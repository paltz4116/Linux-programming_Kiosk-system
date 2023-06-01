#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>
#include "kiosk.h"
#define DEFAULT_PROTOCOL 0
#define MAXLINE 100

int readLine(int fd, char *str);

product *readKiosk(int cfd, product *kioskInfo, int *kiosknum)
{
    int tmp;
    read(cfd, kiosknum, sizeof(int));

    kioskInfo = (product *)malloc((*kiosknum) * sizeof(product));

    read(cfd, kioskInfo, ((*kiosknum) * sizeof(product)));

    return kioskInfo;
}

int inputNum()
{
    int task;

    printf("\n");
    printf("1: 상품 구매 2: 키오스크 종료\n");
    printf("수행할 작업 번호 입력: ");

    scanf("%d", &task);

    return task;
}

void printInfo(product *kioskInfo, int num)
{
    int i, task;

    printf("\n");
    printf("%-7s %-7s %-7s %-7s\n", "번호", "이름", "가격", "수량");
    for (i = 0; i < num; i++)
    {
        printf("%-4d %-7s %-7d %-7d\n", i + 1, kioskInfo[i].name, kioskInfo[i].cost, kioskInfo[i].quantity);
    }
}

product *purchaseProduct(product *kioskInfo, int cfd, int kiosknum)
{
    purchase purArr[100];
    int cost, index = 0;
    char c = 'Y';
    bool kioskErr;

    while (c != 'N')
    {
        printf("\n");
        printf("구매할 상품의 번호를 입력: ");
        scanf("%d", &purArr[index].num);
        printf("구매할 수량 입력: ");
        scanf("%d", &purArr[index].quantity);
        index++;
        printf("구매를 계속 진행 하시겠습니까?(Y/N): ");
        // fflush(stdin);
        // rewind(stdin);
        scanf(" %c", &c);
    }

    printf("지불할 금액 입력: ");
    scanf("%d", &cost);

    write(cfd, &index, sizeof(int));

    write(cfd, purArr, (index * sizeof(purchase)));

    write(cfd, &cost, sizeof(int));

    read(cfd, &kioskErr, sizeof(bool));

    if (kioskErr == true)
    {
        printf("구매 수량 또는 지불 금액으로 인한 오류 발생\n");
        return kioskInfo;
    }
    else
    {
        printf("구매 성공\n");
        read(cfd, kioskInfo, ((kiosknum) * sizeof(product)));

        return kioskInfo;
    }
}

void operateClient()
{
    int cfd, result, kiosknum, task;
    char inmsg[MAXLINE], outmsg[MAXLINE];
    struct sockaddr_un serverAddr;
    product *kioskInfo;

    cfd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
    serverAddr.sun_family = AF_UNIX;
    strcpy(serverAddr.sun_path, "convert");

    do
    { /* 연결 요청 */
        result = connect(cfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        if (result == -1)
            sleep(1);
    } while (result == -1);
    printf("연결 완료\n");

    kioskInfo = readKiosk(cfd, kioskInfo, &kiosknum);
    printInfo(kioskInfo, kiosknum);

    task = inputNum();
    write(cfd, &task, sizeof(int));

    while (task == 1)
    {
        printInfo(kioskInfo, kiosknum);
        purchaseProduct(kioskInfo, cfd, kiosknum);
        task = inputNum();
        write(cfd, &task, sizeof(int));
    }

    close(cfd);
    free(kioskInfo);
    exit(0);
}

/* 소문자-대문자 변환: 클라이언트 프로그램 */
int main()
{
    operateClient();

    return 0;
}