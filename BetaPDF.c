#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 

#define betaF 0.0165

float beta_dist(float a, float b, float x){
    float betaValue = (1/betaF) *(pow(x, (a-1))) * (pow((1-x), (b-1)));
    return betaValue;
}

int main(){
    srand(time(NULL));
    float nUE = 30000;
    int totalUE = 0;
    float TotalTime = 10000;
    int successUE = 0;
    int failedUE = 0;

    float i;

    for(i = 0; i < TotalTime; i++){
        totalUE += (int)round(beta_dist(3, 4, i/TotalTime)/TotalTime*nUE);
    }
    

    for(i = 0; i < TotalTime; i++){
        int txUE = (int)round(beta_dist(3, 4, i/TotalTime)/TotalTime*nUE);

        // 충돌 감지
        // NowTxUE = txUE
        // CollisionUE = 지금 보낼 애들 중 preamble이 충돌 난 애들
        // if((NowTxUE - CollisionUE) > 0 && (NowTxUE - CollisionUE) <= 3)
            // SucceedUE += (NowTxUE - CollisionUE)
        // else if ((NowTxUE - CollisionUE) > 3)
            // SucceedUE += 3
            // DelayUE += (NowTxUE - 3)
                // 카운터를 빼주든 뭐 그런 식으로
        // else
            // DelayUE += NowTxUE

        if(txUE <= 3){
            successUE += txUE;
        }else{
            successUE += 3;
            int temp = txUE - 3;
            failedUE += temp;
        }

        if(successUE >= totalUE){
            break;
        }
    }

    printf("%d, %d, %d, %lf\n", totalUE, successUE, failedUE, i);
    printf("%lf\n", ((float)successUE/(float)totalUE)*100);

    return 0;
}