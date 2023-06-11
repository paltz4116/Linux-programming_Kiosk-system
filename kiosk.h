//상품 정보
typedef struct Product
{
    char name[20];
    int cost;
    int quantity;
} product;

//구매 요청 정보
typedef struct Purchase
{
    int num;
    int quantity;
} purchase;

//키오스크 정보 출력
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

//설정된 사용시간이되면 클라이언트 종료.
void endKiosk(int signo){
    printf("\n");
    printf("키오스크 사용시간 3분 경과로 인해 종료됩니다.\n");

    exit(0);
}
