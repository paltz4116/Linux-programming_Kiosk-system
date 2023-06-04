#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdbool.h>
#include "kiosk.h"
#define DEFAULT_PROTOCOL 0
#define MAXLINE 100

//서버로부터 상품의 수와 키오스크 정보를 받아옴.
product *readKiosk(int cfd, product *kioskInfo, int *kiosknum)
{
    read(cfd, kiosknum, sizeof(int));

    kioskInfo = (product *)malloc((*kiosknum) * sizeof(product));

    read(cfd, kioskInfo, ((*kiosknum) * sizeof(product)));

    return kioskInfo;
}

//수행할 작업번호 입력 받은 후 반환.
int inputNum()
{
    int task;

    printf("\n");
    printf("1: 상품 구매 2: 키오스크 종료\n");
    printf("수행할 작업 번호 입력: ");

    scanf("%d", &task);

    return task;
}

//구매할 상품의 정보를 입력 받고 서버에 전달 후 서버로부터 성공, 실패 여부와 키오스크 정보를 전달받음.
void purchaseProduct(product *kioskInfo, int cfd, int kiosknum)
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
    }
    else
    {
        printf("구매 성공\n");
        read(cfd, kioskInfo, (kiosknum * sizeof(product)));
    }
}

//전반적인 클라이언트 동작 관리.
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
        read(cfd, kioskInfo, (kiosknum * sizeof(product)));
        printInfo(kioskInfo, kiosknum);
        purchaseProduct(kioskInfo, cfd, kiosknum);
        task = inputNum();
        write(cfd, &task, sizeof(int));
    }

    close(cfd);
    free(kioskInfo);
    exit(0);
}

int main()
{
    operateClient();

    return 0;
}