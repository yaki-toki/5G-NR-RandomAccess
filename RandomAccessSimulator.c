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
int preambleCollision(struct UEinfo *user, struct UEinfo *UEs, int nUE, int nPreamble, int time);

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

        for(int i = 0; i < nUE; i++){
            if((UE+i)->active == 1)
                (UE+i)->msg2Flag = preambleCollision(UE+i, UE, nUE, nPreamble, time);
        }

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
int preambleCollision(struct UEinfo *user, struct UEinfo *UEs, int nUE, int nPreamble, int time){
    // MSG 1이 BS에 도달할 때 까지 2 time slot을 기다려야 함
    // 따라서 전송하는 시점에서 2 time slot이 더해진 현재 time이 동일해야 충돌을 확인할 수 있음
    if(user->txTime +2 == time && user->active == 1){
        for(int i = 0; i < nUE; i++){
            if((UEs+i)->active == 1 && (UEs+i)->idx != user->idx){
                // preamble 충돌
                if((UEs+i)->preamble == user->preamble){
                    return 0;
                }
                // preamble이 충돌되지 않은 경우
                else{
                    user->active = 2;
                    return 1;
                }
            }
        }
    }
    // 지금 MSG 1을 전송하는 UE가 아닌경우
    else{
        return 0;
    }
}

void timerIncrease(struct UEinfo *user){
    if(user->active != 0)
        user->timer++;
}