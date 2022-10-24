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
        // MSG 1�� ������ �õ��ϴ� UE���� ����
        for(int i = 0; i < nUE; i++)
            activeUE(UE+i, nUE, pTx, time);
        
        // MSG 1�� �����ϴ� UE���� preamble�� ����
        for(int i = 0; i < nUE; i++)
            selectPreamble(UE+i, nPreamble, time, backoffIndicator);
        
        // Preamble collision detection



        // ������ �õ��ϴ� UE���� ���ð� ����
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
    // MSG 1 ������ ������ UE��
    if(user->active == 1){
        // ó�� ������ �õ��ϴ� UE
        if(user->preamble == -1){
            user->preamble = rand() % nPreamble;
            user->rarWindow = 0;
            user->maxRarCounter = 0;
            user->preambleTxCounter = 1;
        }
        // ������ ������ �ߴ� UE��
        else{
            // �ϴ� ���� backoff���� ���
            user->txTime = time + (rand() % backoff);
            // RAR window�� Ȯ��
            if(user->rarWindow == 5){
                // RAR window�� ������ ��(5) ���� ū ���
                user->rarWindow = 0; // RAR window �ʱ�ȭ

                // ���� preamble�� ���� Ƚ���� �ִ��� ���
                if(user->maxRarCounter == 10){
                    // preamble�� �����ϰ� backoff��ŭ ���
                    user->preamble = rand() % nPreamble;
                    // preamble ���� Ƚ��
                    user->preambleTxCounter++;
                }else{
                    // �ִ� ���� Ƚ���� �ƴѰ�� maximum rar counter�� ����
                    user->maxRarCounter++;
                }
            }else{
                user->rarWindow++;
            }
        }
    }
}

// preamble�� �浹 Ȯ��
void preambleCollision(struct UEinfo *user, int nUE, int nPreamble){

}

void timerIncrease(struct UEinfo *user){
    if(user->active != 0)
        user->timer++;
}