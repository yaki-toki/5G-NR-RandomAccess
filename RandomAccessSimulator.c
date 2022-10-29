#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <time.h>

struct UEinfo{
    int idx;                // UE �ĺ���
    int timer;              // UE�� Ÿ�̸� (���� RA �������� �ɸ��� ������)
    int active;             // ���� ���� ���� (Not active: 0, MSG 1-2: 1, MSG 3-4: 2)
    int txTime;             // MSG ���� �ð�
    int preamble;           // �����ϴ� preamble
    int rarWindow;          // RAR window ����� 5�� ����
    int maxRarCounter;      // �ִ� RAR window�� ��� Ƚ�� ����� 10
    int preambleTxCounter;  // Preamble�� �����Ͽ� �������� Ƚ��
    int msg2Flag;           // MSG 1-2 ���� ����
    int connectionRequest;  // MSG 3 Ÿ�̸� ����� 48�� ����
    int msg4Flag;           // MSG 3-4 ���� ���� (��� RA ����)
};

void initialUE(struct UEinfo *user, int id);
void activeUE(struct UEinfo *user, int nUE, float pTx, int txTime);
void selectPreamble(struct UEinfo *user, int nPreamble, int time, int backoff);
void preambleCollision(struct UEinfo *user, struct UEinfo *UEs, int nUE, int nPreamble, int time);
void requestResourceAllocation(struct UEinfo *user, int time, int backoff);
void timerIncrease(struct UEinfo *user);
int successUEs(struct UEinfo *user, int nUE);

int collisionPreambles = 0;

int main(int argc, char** argv){
    srand(2022);    // Fix random seed
    int nUE = 10000;
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

    int nSuccessUE;

    for(time = 0; time < maxTime; time++){
        
        // Time t���������� UE���� ���¸� Ȯ��
        for(int i = 0; i < nUE; i++){
            // Random access�� ������ UE���� ���� ��Ŵ
            if((UE+i)->msg4Flag != 1){
                // ���� MSG 1�� �õ����� ���� ���� active 0
                // ���°� ó�� �ٲ�� 0�� �Ǵ� ���� RA�� �����ϴ� ���ۿ� ����
                if((UE+i)->msg2Flag != 1 && (UE+i)->active == 0){
                    activeUE(UE+i, nUE, pTx, time);
                }

                // MSG 1�� �����ϴ� UE���� preamble�� ����
                if((UE+i)->active == 1){
                    selectPreamble(UE+i, nPreamble, time, backoffIndicator);
                }

                // Preamble collision detection
                if((UE+i)->active == 1 && (UE+i)->txTime + 2 == time){
                    preambleCollision(UE+i, UE, nUE, nPreamble, time);
                }

                // MSG 3 request
                if((UE+i)->active == 2 && (UE+i)->txTime+2 == time){
                    requestResourceAllocation(UE+i, time, backoffIndicator);
                }

                // ������ �õ��ϴ� UE���� ���ð� ����
                if((UE+i)->active != 0)
                    timerIncrease(UE+i);
            }
        }
        nSuccessUE = successUEs(UE, nUE);
        if(nSuccessUE == nUE){
            break;
        }
    }

    printf("-------- Result ---------\n");

    float averageDelay = 0;
    int failedUEs = 0;
    int preambleTxCount = 0;

    for(int i = 0; i < nUE; i++){
        if((UE+i)->msg4Flag == 0){
            failedUEs++;
        }else{
            averageDelay += (float)(UE+i)->timer;
            preambleTxCount += (UE+i)->preambleTxCounter;
        }
    }

    printf("Total success time: %dms\n", time);
    printf("Number of succeed UEs: %d\n", nSuccessUE);
    printf("Number of failed UEs: %d\n", failedUEs);
    printf("Number of collision preambles: %lf\n", (float)collisionPreambles/(float)nSuccessUE);
    printf("Average preamble tx count: %lf\n", (float)preambleTxCount/(float)nSuccessUE);
    printf("Average delay: %lfms\n", averageDelay/(float)nSuccessUE);

    FILE *fp;
    char fileBuff[1000];
    char fileName[500];
    sprintf(fileName, "Results_UE%d.txt", nUE);
    fp = fopen(fileName, "w+");

    for(int i = 0; i < nUE; i++){
        // printf("Idx: %d | Timer: %d | Active: %d | txTime: %d | Preamble: %d | RAR window: %d | Max RAR: %d | Preamble reTx: %d | MSG 2 Flag: %d | ConnectRequest: %d | MSG 4 Flag: %d\n",
        // (UE+i)->idx, (UE+i)->timer, (UE+i)->active, (UE+i)->txTime, (UE+i)->preamble, (UE+i)->rarWindow, (UE+i)->maxRarCounter, (UE+i)->preambleTxCounter, (UE+i)->msg2Flag, (UE+i)->connectionRequest, (UE+i)->msg4Flag);
        sprintf(fileBuff, "Idx: %d | Timer: %d | Active: %d | txTime: %d | Preamble: %d | RAR window: %d | Max RAR: %d | Preamble reTx: %d | MSG 2 Flag: %d | ConnectRequest: %d | MSG 4 Flag: %d\n",
        (UE+i)->idx, (UE+i)->timer, (UE+i)->active, (UE+i)->txTime, (UE+i)->preamble, (UE+i)->rarWindow, (UE+i)->maxRarCounter, (UE+i)->preambleTxCounter, (UE+i)->msg2Flag, (UE+i)->connectionRequest, (UE+i)->msg4Flag);

        fputs(fileBuff, fp);
    }

    sprintf(fileBuff, "Total success time: %dms\n", time);
    fputs(fileBuff, fp);
    sprintf(fileBuff, "Number of succeed UEs: %d\n", nSuccessUE);
    fputs(fileBuff, fp);
    sprintf(fileBuff, "Number of failed UEs: %d\n", failedUEs);
    fputs(fileBuff, fp);
    sprintf(fileBuff, "Number of collision preambles: %lf\n", (float)collisionPreambles/(float)nSuccessUE);
    fputs(fileBuff, fp);
    sprintf(fileBuff, "Average preamble tx count: %lf\n", (float)preambleTxCount/(float)nSuccessUE);
    fputs(fileBuff, fp);
    sprintf(fileBuff, "Average delay: %lfms\n", averageDelay/(float)nSuccessUE);
    fputs(fileBuff, fp);

    fclose(fp);

    /*
    struct UEinfo{
    int idx;                // UE �ĺ���
    int timer;              // UE�� Ÿ�̸� (���� RA �������� �ɸ��� ������)
    int active;             // ���� ���� ���� (Not active: 0, MSG 1-2: 1, MSG 3-4: 2)
    int txTime;             // MSG ���� �ð�
    int preamble;           // �����ϴ� preamble
    int rarWindow;          // RAR window ����� 5�� ����
    int maxRarCounter;      // �ִ� RAR window�� ��� Ƚ�� ����� 10
    int preambleTxCounter;  // Preamble�� �����Ͽ� �������� Ƚ��
    int msg2Flag;           // MSG 1-2 ���� ����
    int connectionRequest;  // MSG 3 Ÿ�̸� ����� 48�� ����
    int msg4Flag;           // MSG 3-4 ���� ���� (��� RA ����)
};
    */


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
    if(p < pTx){
        // MSG 1 ������ �õ��ϴ� ����
        user->active = 1;
        // ���� ������ �����ϴ� �������� ����
        user->txTime = txTime+1;
        user->timer = 0;
        user->msg2Flag = 0;
    }
}

void selectPreamble(struct UEinfo *user, int nPreamble, int time, int backoff){
    // MSG 1 ������ ������ UE��
    if(user->active == 1){
        // ó�� ������ �õ��ϴ� UE
        if(user->preamble == -1){
            int tmp = rand() % nPreamble;
            user->preamble = tmp;
            user->rarWindow = 0;
            user->maxRarCounter = 0;
            user->preambleTxCounter = 1;
        }
        // ������ ������ �ߴ� UE��
        else{
            // RAR window�� Ȯ��
            if(user->rarWindow >= 5){
                // RAR window�� ������ ��(5) ���� ū ���
                user->rarWindow = 0; // RAR window �ʱ�ȭ
                // ���� preamble�� ���� Ƚ���� �ִ��� ���
                if(user->maxRarCounter >= 10){
                    // preamble�� �����ϰ� backoff��ŭ ���
                    user->preamble = rand() % nPreamble;
                    // preamble ���� Ƚ��
                    user->preambleTxCounter++;
                    // �ִ� ���� Ƚ���� 0���� �ʱ�ȭ
                    user->maxRarCounter = 0;
                    // �ϴ� ���� backoff���� ���
                    user->txTime = time + (rand() % backoff) + 1;
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
void preambleCollision(struct UEinfo *user, struct UEinfo *UEs, int nUE, int nPreamble, int time){
    int check = 1;
    // MSG 1�� BS�� ������ �� ���� 2 time slot�� ��ٷ��� ��
    // ���� �����ϴ� �������� 2 time slot�� ������ ���� time�� �����ؾ� �浹�� Ȯ���� �� ����
    for(int i = 0; i < nUE; i++){
        // MSG 1-2�� ���� UE�� ��
        if((UEs+i)->active == 1){
            // ���� �����ϴ� UE�� ���� Ÿ�ӿ� �����ϴ� �ֵ� ��
            if((UEs+i)->txTime == user->txTime){
                // ���� �������� �ϴ� �� ����
                if((UEs+i)->idx != user->idx){
                    // preamble �浹
                    if((UEs+i)->preamble == user->preamble){
                        check = 0;
                        collisionPreambles++;
                        
                    }
                }
                
            }
        }
    }
    if(check == 1){
        user->active = 2;
        user->txTime = time+2;
        user->connectionRequest = 0;
        user->msg2Flag = 1;
    }else{
        user->msg2Flag = 0;
    }
}

void requestResourceAllocation(struct UEinfo *user, int time, int backoff){
    user->connectionRequest++;
    if(user->txTime+2 == time){    
        if(user->connectionRequest < 48){
            float p = (float)rand() / (float)RAND_MAX;
            if(p > 0.01){
                // 90%�� Ȯ���� MSG 3-4 ����
                user->msg4Flag = 1;
                user->active = 0;
            }else{
                // printf("MSG3 Fail %d\n", user->idx);
                // 10%�� Ȯ���� MSG 3-4 ����
                user->txTime = time + 1; //(rand() % backoff) + 1;
            }
        }else{
            initialUE(user, user->idx);
            user->active = 1;
        }
    }
}

void timerIncrease(struct UEinfo *user){
    if(user->active != 0)
        user->timer++;
}

int successUEs(struct UEinfo *user, int nUE){
    int success = 0;
    for(int i = 0; i < nUE; i++){
        if((user+i)->msg4Flag == 1){
            success++;
        }
    }
    return success;
}