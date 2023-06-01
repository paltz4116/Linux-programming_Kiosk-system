#include <stdio.h>

void swap(int* a, int* b){
    int* tmp;

    tmp = a;
    a = b;
    b = tmp;
    printf("%d %d\n", a[0], a[1]);
}

int main(){
    int a[2] = {0, 1};
    int b[2] = {2, 3};

    swap(a, b);

    printf("%d %d\n", a[0], a[1]);

    return 0;
}