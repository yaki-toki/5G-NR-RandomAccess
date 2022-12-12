#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SIZE 60000
void append(int *dst, int num);
int main(){
    int n = 10;
    int *array = (int*)malloc(sizeof(int) * n);

    for(int i = 0; i < n; i++){
        if (NULL != array){
            if(array[i] == '\0'){
                printf("NULL %d\n", i);
                array[i] = i;
            }else{
                printf("NOT NULL %d\n", i);
            }
            if(i == 4){
                break;
            }
        }
    }
    printf("---------------\n");

    for(int i = 0; i < n; i++){
        if(array[i] == '\0'){
            printf("NULL %d\n", i);
        }else{
            printf("NOT NULL %d\n", array[i]);
        }
    }
    printf("---------------\n");
    append(array, 2);

    for(int i = 0; i < n; i++){
        if(array[i] == '\0'){
            printf("NULL %d\n", i);
        }else{
            printf("NOT NULL %d\n", array[i]);
        }
    }

    return 0;
}

void append(int *dst, int num) {
    int *p = dst;
    while (*p != '\0') p++; // 문자열 끝 탐색
    *p = num;
    *(p+1) = num; 
}

// double uniform();
// int accessUE(int nUE, double pTx);

// int main() {
//     int nUE = 10000;
//     double pTx = 0.01;
//     int totalUE = 0;
    
//     while(nUE > 0){
//         int nowUE = accessUE(nUE, pTx);
//         printf("%d\n", nowUE);
//         nUE -= nowUE;
//         totalUE += nowUE;
//         break;
//     }
//     printf("%d\n", totalUE);
    
// 	return 0;
// }

// double uniform(){
//     return (double)rand() / (double)RAND_MAX ;
// }

// int accessUE(int nUE, double pTx){
//     int user = 0;
//     for(int i = 0; i < nUE; i++){
//         double temp = uniform();
//         if(temp <= pTx){
//             user++;
//         }
//     }
//     return user;
// }