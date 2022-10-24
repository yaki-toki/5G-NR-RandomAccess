#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

struct UEinfo{
    int idx;
    int active;
    int txTime;
    int preamble;
    int timer;
    int rarWindow;
    int maxRarCounter;
    int preambleTxCounter;
    int msg2Flag;
};

void initialUE(struct UEinfo *user, int id);
void activeUE(struct UEinfo *user, int nUE, float pTx, int txTime);
void selectPreamble(struct UEinfo *user, int nPreamble, int time, int backoff);

void timerIncrease(struct UEinfo *user);

int main(int argc, char** argv){

    int nUE = 5000;
    struct UEinfo *UE;
    UE = (struct UEinfo *) calloc(nUE, sizeof(struct UEinfo));

    int nPreamble = 64;
    int backoffIndicator = 20;

    for(int i = 0; i < nUE; i++){
        initialUE(UE+i, i);
    }
    

    int time;
    float pTx = 0.1; // 10 %
    int maxTime = 60000; // 60s to ms

    for(time = 0; time < maxTime; time++){
        // MSG 1의 전송을 시도하는 UE들을 결정
        for(int i = 0; i < nUE; i++)
            activeUE(UE+i, nUE, pTx, time);
        
        // MSG 1을 전송하는 UE들이 preamble을 선택
        for(int i = 0; i < nUE; i++)
            selectPreamble(UE+i, nPreamble, time, backoffIndicator);
        
        // Preamble collision detection



        // 전송을 시도하는 UE들의 대기시간 증가
        for(int i = 0; i < nUE; i++)
            timerIncrease(UE+i);
        break;
    }

    return 0;
}

void initialUE(struct UEinfo *user, int id){
    user->idx = id;
    user->active = 0;
    user->txTime = -1;
    user->preamble = -1;
    user->timer = -1;
}

void activeUE(struct UEinfo *user, int nUE, float pTx, int txTime){
    float p = (float)rand() / (float)RAND_MAX;
    if(p < pTx && user->active == 0){
        user->active = 1;
        user->txTime = txTime;
        user->timer = 0;
        user->msg2Flag = 0;
    }
}

void selectPreamble(struct UEinfo *user, int nPreamble, int time, int backoff){
    // MSG 1 전송을 시작한 UE들
    if(user->active == 1){
        // 처음 전송을 시도하는 UE
        if(user->preamble == -1){
            user->preamble = rand() % nPreamble;
            user->rarWindow = 0;
            user->maxRarCounter = 0;
            user->preambleTxCounter = 1;
        }
        // 이전에 전송을 했던 UE들
        else{
            // 일단 다음 backoff까지 대기
            user->txTime = time + (rand() % backoff);
            // RAR window를 확인
            if(user->rarWindow == 5){
                // RAR window가 설정한 값(5) 보다 큰 경우
                user->rarWindow = 0; // RAR window 초기화

                // 지금 preamble의 전송 횟수가 최대인 경우
                if(user->maxRarCounter == 10){
                    // preamble을 변경하고 backoff만큼 대기
                    user->preamble = rand() % nPreamble;
                    // preamble 전송 횟수
                    user->preambleTxCounter++;
                }else{
                    // 최대 전송 횟수가 아닌경우 maximum rar counter를 증가
                    user->maxRarCounter++;
                }
            }else{
                user->rarWindow++;
            }
        }
    }
}

// preamble의 충돌 확인
void preambleCollision(struct UEinfo *user, int nUE, int nPreamble){

}

void timerIncrease(struct UEinfo *user){
    if(user->active != 0)
        user->timer++;
}