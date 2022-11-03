#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define betaF 0.0165

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
    int preambleChange;     // ���� �� �� ���� preamble �� �� �ٲ����
    int raFailed;           // MSG 2 max count �Ǹ� �׳� ����
    int rampingPower;
    int nowBackoff;
};

void initialUE(struct UEinfo *user, int id);
void selectPreamble(struct UEinfo *user, int nPreamble, int time, int backoff);
void preambleCollision(struct UEinfo *user, struct UEinfo *UEs, int nUE, int checkPreamble, int nPreamble, int time, int backoff);
void requestResourceAllocation(struct UEinfo *user, int time, int backoff, int nPreamble);
void timerIncrease(struct UEinfo *user);
int successUEs(struct UEinfo *user, int nUE);

void saveSimulationLog(int time, int nUE, int nSuccessUE, int failedUEs, int preambleTxCount, float totalDelay, int distribution);
void saveResult(int nUE, struct UEinfo *UE, int distribution);
float beta_dist(float a, float b, float x);

int collisionPreambles = 0;
int totalPreambleTxop = 0;

int main(int argc, char** argv){
    srand(2022);    // Fix random seed

    int distribution = 0;

    for(int n = 10000; n <= 100000; n+=10000){
 
        int nUE = n;
        struct UEinfo *UE;
        UE = (struct UEinfo *) calloc(nUE, sizeof(struct UEinfo));

        int nPreamble = 64;
        int backoffIndicator = 20;
        int nGrantUL = 12;

        for(int i = 0; i < nUE; i++){
            initialUE(UE+i, i);
        }
        
        int time;
        int maxTime = 0; // 10s to ms
        int nAccessUE;
        int accessTime = 5; // 1, 6ms���� ����
        if(distribution == 0){
            maxTime = 60000;
            nAccessUE= ceil((float)n * (float)accessTime * 1.0/(float)maxTime);
            if(nAccessUE == 0){
                nAccessUE = 1;
            }

        }else{
            maxTime = 10000;
        }

        int nSuccessUE;
        int activeCheck = 0;

        for(time = 0; time < maxTime; time++){
            
            if(time % accessTime == 1){
                if(activeCheck >= nUE){
                    activeCheck = nUE;
                }else{
                    if(distribution == 0){
                        activeCheck += nAccessUE;
                    }else{
                        float betaDist = beta_dist(3, 4, (float)time/(float)maxTime);
                        int accessUEs = (int)ceil((float)nUE * betaDist / ((float)maxTime/(float)accessTime));
                        activeCheck += accessUEs;
                    }
                    
                }
                // printf("%d\n", activeCheck);
                for(int i = 0; i < activeCheck; i++){
                    if((UE+i)->active == -1){
                        // MSG 1 ������ �õ��ϴ� ����
                        (UE+i)->active = 1;
                        // ���� ������ �����ϴ� �������� ����
                        (UE+i)->txTime = time+1;
                        (UE+i)->timer = 0;
                        (UE+i)->msg2Flag = 0;
                    }
                }
            }

            // Time t���������� UE���� ���¸� Ȯ��
            for(int i = 0; i < nUE; i++){
                // Random access�� ������ UE���� ���� ��Ŵ
                if((UE+i)->msg4Flag == 0 && (UE+i)->raFailed != -1){

                    if((UE+i)->active == 1 && (UE+i)->msg2Flag == 0){
                        // MSG 1�� �����ϴ� UE���� preamble�� ����
                        selectPreamble(UE+i, nPreamble, time, backoffIndicator);
                    }

                    // Preamble collision detection
                    if((UE+i)->txTime + 2 == time && (UE+i)->txTime != -1){
                        int checkPreambleNumber = (UE+i)->preamble;
                        preambleCollision(UE+i, UE, nUE, checkPreambleNumber, nPreamble, time, backoffIndicator);
                    }
                    
                    // MSG 3 request
                    if((UE+i)->active == 2 && (UE+i)->txTime+2 == time){
                        requestResourceAllocation(UE+i, time, backoffIndicator, nPreamble);
                    }

                    // ������ �õ��ϴ� UE���� ���ð� ����
                    if((UE+i)->active > 0 && (UE+i)->msg4Flag == 0)
                        timerIncrease(UE+i);
                }
            }
            nSuccessUE = successUEs(UE, nUE);
            if(nSuccessUE == nUE){
                break;
            }
        }


        float totalDelay = 0;
        int failedUEs = 0;
        int preambleTxCount = 0;

        for(int i = 0; i < nUE; i++){
            if((UE+i)->msg4Flag == 0){
                failedUEs++;
            }else{
                totalDelay += (float)(UE+i)->timer;
                preambleTxCount += (UE+i)->preambleTxCounter;
            }
        }
        printf("-------- %05d Result ---------\n", activeCheck);
        saveSimulationLog(time, nUE, nSuccessUE, failedUEs, preambleTxCount, totalDelay, distribution);
        saveResult(nUE, UE, distribution);
    }

    return 0;
}

void initialUE(struct UEinfo *user, int id){
    user->idx = id;
    user->timer = -1;
    user->active = -1;
    user->txTime = -1;
    user->preamble = -1;
}

void selectPreamble(struct UEinfo *user, int nPreamble, int time, int backoff){
    // ó�� ������ �õ��ϴ� UE
    if(user->preamble == -1){
        int tmp = rand() % nPreamble;
        user->preamble = tmp;
        user->rarWindow = 0;
        user->maxRarCounter = 0;
        user->preambleTxCounter = 0;
        user->preambleChange = 1;
        user->nowBackoff = 0;
    }else{
        if(user->nowBackoff == 0){
            user->rarWindow++;
            if(user->rarWindow >= 5){
                int tmp = (rand() % backoff)+ 2;
                user->txTime = time + tmp;
                user->nowBackoff = tmp;

                // RAR window�� 1 ����
                user->rarWindow = 0;

                // �ִ� ���� Ƚ���� 1 ����
                user->maxRarCounter++;
                
                // ���� preamble�� ���� Ƚ���� �ִ��� ���
                if(user->maxRarCounter >= 10){

                    // MSG 2 ���ſ� ���������� ������ UE�� ������ RA���� �õ� X
                    user->raFailed = -1;

                    // preamble�� �����ϰ� backoff��ŭ ���
                    user->preamble = rand() % nPreamble;
                    
                    // �ִ� ���� Ƚ���� 0���� �ʱ�ȭ
                    user->maxRarCounter = 0;

                    user->preambleChange++;
                }
            }
        }
    }
}

// preamble�� �浹 Ȯ��
void preambleCollision(struct UEinfo *user, struct UEinfo *UEs, int nUE, int checkPreamble, int nPreamble, int time, int backoff){
    int check = 0;
    // MSG 1�� BS�� ������ �� ���� 2 time slot�� ��ٷ��� ��
    // ���� �����ϴ� �������� 2 time slot�� ������ ���� time�� �����ؾ� �浹�� Ȯ���� �� ����
    int* userIdx = (int*)malloc(sizeof(int) * nUE);
    for(int i = 0; i < nUE; i++){
        // MSG 1�� �����ϴ� �����̸鼭 ���� Ȯ���ϴ� UE�� ���� ������ ���� UE�� Ȯ��
        if((UEs+i)->active == 1 && (UEs+i)->txTime + 2 == time){
            // preamble �浹
            if((UEs+i)->preamble == checkPreamble){
                
                userIdx[check] = (UEs+i)->idx;
                check++;
            }
        }
    }
    if(check == 1){
        // ��ü preamble���� Ƚ��
        totalPreambleTxop++;
        // preamble ���� Ƚ��
        user->preambleTxCounter++;
        user->active = 2;
        user->txTime = time + 2;
        user->connectionRequest = 0;
        user->msg2Flag = 1;
    }else{
        // totalPreambleTxop += check;
        collisionPreambles += check;
        for(int i = 0; i < check; i++){
            (UEs+userIdx[i])->rarWindow = 5;
            (UEs+userIdx[i])->txTime = time + 3;
            // (UEs+userIdx[i])->preambleTxCounter++;
        }
    }
    return;
}

void requestResourceAllocation(struct UEinfo *user, int time, int backoff, int nPreamble){
    user->connectionRequest++;
    if(user->connectionRequest < 48){
        float p = (float)rand() / (float)RAND_MAX;
        if(p > 0.1){
            // 90%�� Ȯ���� MSG 3-4 ����
            user->msg4Flag = 1;
            user->active = 0;
        }else{
            // printf("MSG3 Fail %d\n", user->idx);
            // 10%�� Ȯ���� MSG 3-4 ����
            user->txTime = time + 1; //(rand() % backoff) + 1;
        }
    }else{
        int tmp = (rand() % backoff) + 2;
        user->active = 1;
        user->txTime = time + tmp;
        user->nowBackoff = tmp;
        user->preamble = rand() % nPreamble;
        user->msg2Flag = 0;
        user->rarWindow = 0;
        user->maxRarCounter = 0;
        user->preambleTxCounter++;
    }
}

void timerIncrease(struct UEinfo *user){
    if(user->active != 0)
        user->timer++;
    
    if(user->nowBackoff != 0){
        user->nowBackoff--;
    }
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

void saveSimulationLog(int time, int nUE, int nSuccessUE, int failedUEs, int preambleTxCount, float totalDelay, int distribution){
    FILE *fp;
    char resultBuff[1000];
    char fileNameResult[500];
    if(distribution == 0){
        sprintf(fileNameResult, "./Uniform_SimulationResults/Exclude_msg2_failures_UE%05d_Results.txt", nUE);
    }else{
        sprintf(fileNameResult, "./Beta_SimulationResults/Exclude_msg2_failures_UE%05d_Results.txt", nUE);
    }
    
    fp = fopen(fileNameResult, "w+");

    printf("Number of UEs: %d\n", nUE);
    sprintf(resultBuff, "Number of UEs: %d\n", nUE);
    fputs(resultBuff, fp);
    
    printf("Total simulation time: %dms\n", time);
    sprintf(resultBuff, "Total simulation time: %dms\n", time);
    fputs(resultBuff, fp);
    
    printf("Number of succeed UEs: %d\n", nSuccessUE);
    sprintf(resultBuff, "Number of succeed UEs: %d\n", nSuccessUE);
    fputs(resultBuff, fp);
    
    float ratioSuccess = (float)nSuccessUE/(float)nUE;
    printf("Success ratio: %lf\n", ratioSuccess);
    sprintf(resultBuff, "Success ratio: %lf\n", ratioSuccess);
    fputs(resultBuff, fp);
    
    printf("Number of failed UEs: %d\n", failedUEs);
    sprintf(resultBuff, "Number of failed UEs: %d\n", failedUEs);
    fputs(resultBuff, fp);
    
    float ratioFailed = (float)failedUEs/(float)nUE;
    printf("Fail probability: %lf\n", ratioFailed);
    sprintf(resultBuff, "Fail probability: %lf\n", ratioFailed);
    fputs(resultBuff, fp);
    
    float nCollisionPreambles = (float)collisionPreambles/(float)preambleTxCount;
    printf("Number of collision preambles: %lf\n", nCollisionPreambles);
    sprintf(resultBuff, "Number of collision preambles: %lf\n", nCollisionPreambles);
    fputs(resultBuff, fp);
    
    float averagePreambleTx = (float)totalPreambleTxop/(float)nSuccessUE;
    printf("Average preamble tx count: %lf\n", averagePreambleTx);
    sprintf(resultBuff, "Average preamble tx count: %lf\n", averagePreambleTx);
    fputs(resultBuff, fp);
    
    float averageDelay = totalDelay/(float)nSuccessUE;
    printf("Average delay: %lfms\n", averageDelay);
    sprintf(resultBuff, "Average delay: %lfms\n", averageDelay);
    fputs(resultBuff, fp);

    fclose(fp);
}
void saveResult(int nUE, struct UEinfo *UE, int distribution){
    FILE *fp_l;
    char logBuff[1000];
    char fileNameResultLog[500];
    if(distribution == 0){
        sprintf(fileNameResultLog, "./Uniform_SimulationResults/Exclude_msg2_failures_UE%05d_Logs.txt", nUE);
    }else{
        sprintf(fileNameResultLog, "./Beta_SimulationResults/Exclude_msg2_failures_UE%05d_Logs.txt", nUE);
    }
    
    fp_l = fopen(fileNameResultLog, "w+");

    for(int i = 0; i < nUE; i++){
        // printf("Idx: %d | Timer: %d | Active: %d | txTime: %d | Preamble: %d | RAR window: %d | Max RAR: %d | Preamble reTx: %d | MSG 2 Flag: %d | ConnectRequest: %d | MSG 4 Flag: %d\n",
        // (UE+i)->idx, (UE+i)->timer, (UE+i)->active, (UE+i)->txTime, (UE+i)->preamble, (UE+i)->rarWindow, (UE+i)->maxRarCounter, (UE+i)->preambleTxCounter, (UE+i)->msg2Flag, (UE+i)->connectionRequest, (UE+i)->msg4Flag);
        sprintf(logBuff, "Idx: %d | Timer: %d | Active: %d | txTime: %d | Preamble: %d | Preamble change: %d | RAR window: %d | Max RAR: %d | Preamble reTx: %d | MSG 2 Flag: %d | ConnectRequest: %d | MSG 4 Flag: %d\n",
        (UE+i)->idx, (UE+i)->timer, (UE+i)->active, (UE+i)->txTime, (UE+i)->preamble, (UE+i)->preambleChange, (UE+i)->rarWindow, (UE+i)->maxRarCounter, (UE+i)->preambleTxCounter, (UE+i)->msg2Flag, (UE+i)->connectionRequest, (UE+i)->msg4Flag);

        fputs(logBuff, fp_l);
    }
    
    fclose(fp_l);
}

float beta_dist(float a, float b, float x){
    float betaValue = (1/betaF) *(pow(x, (a-1))) * (pow((1-x), (b-1)));
    return betaValue;
}
