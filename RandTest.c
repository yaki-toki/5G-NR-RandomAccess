#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SIZE 60000
void loopTest();
int main(){
    loopTest();
    return 0;
}

void loopTest(){
    for(int i = 0; i < 10; i++){
        printf("%d\n",i);
        if(i == 5){
            return;
        }
    }
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