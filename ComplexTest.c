#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <complex.h>
#include <time.h>

int main(void)
{
    clock_t start = clock();
    int sum = 0;
    for(int i = 0; i < 10000000; i++){
        sum += i;
    }
    sum = 0;
    for(int i = 0; i < 10000000; i++){
        sum += i;
    }
    sum = 0;
    for(int i = 0; i < 10000000; i++){
        sum += i;
    }
    sum = 0;
    for(int i = 0; i < 10000000; i++){
        sum += i;
    }
    clock_t end = clock();
    printf("Latency: %lf\n", (double)(end - start) / CLOCKS_PER_SEC);
    return 0;
}