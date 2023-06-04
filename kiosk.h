typedef struct Product
{
    char name[20];
    int cost;
    int quantity;
} product;

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