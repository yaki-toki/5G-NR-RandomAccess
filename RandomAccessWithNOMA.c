#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define betaF 0.0165

struct UEinfo
{
    int idx;               // UE �ĺ���
    int timer;             // UE�� Ÿ�̸� (���� RA �������� �ɸ��� ������)
    int active;            // ���� ���� ���� (Not active: 0, MSG 1-2: 1, MSG 3-4: 2)
    int txTime;            // MSG ���� �ð�
    int preamble;          // �����ϴ� preamble
    int rarWindow;         // RAR window ����� 5�� ����
    int maxRarCounter;     // �ִ� RAR window�� ��� Ƚ�� ����� 10
    int preambleTxCounter; // Preamble�� �����Ͽ� �������� Ƚ��
    int msg2Flag;          // MSG 1-2 ���� ����
    int connectionRequest; // MSG 3 Ÿ�̸� ����� 48�� ����
    int msg4Flag;          // MSG 3-4 ���� ���� (��� RA ����)
    int preambleChange;    // ���� �� �� ���� preamble �� �� �ٲ����
    int raFailed;          // MSG 2 max count �Ǹ� �׳� ����
    int nowBackoff;        // Backoff counter
    int firstTxTime;
    int secondTxTime;
    
    // using for NOMA
    int rampingPower;       // Power ���� level
    float xCoordinate;      // UE�� x ��ǥ
    float yCoordinate;      // UE�� y ��ǥ
    float distance;         // BS -- UE�� �Ÿ�
    float pathLoss;         // BS -- UE�� pathloss
};

void initialUE(struct UEinfo *user, int id);
void selectPreamble(struct UEinfo *user, int nPreamble, int time, int backoff, int accessTime, int maxRarWindow, int maxMsg2TxCount);
void preambleCollision(struct UEinfo *user, struct UEinfo *UEs, int nUE, int checkPreamble, int nPreamble, int time, int backoff, int maxRarWindow, int *grantCheck, int nGrantUL);
void requestResourceAllocation(struct UEinfo *user, int time, int backoff, int nPreamble);
void timerIncrease(struct UEinfo *user);
int successUEs(struct UEinfo *user, int nUE);

void saveSimulationLog(int seed, int time, int nUE, int nSuccessUE, int failedUEs, int preambleTxCount, float totalDelay, int distribution, int nPreamble, double layency);
void saveResult(int seed, int nUE, struct UEinfo *UE, int distribution, int nPreamble);
float beta_dist(float a, float b, float x);

int collisionPreambles = 0;
int totalPreambleTxop = 0;

int main(int argc, char **argv){
    int randomSeed;

    int nPreamble = 54;        // Number of preambles
    int backoffIndicator = 20; // Number of backoff indicator
    int nGrantUL = 12;         // Number of UL Grant
    int grantCheck;            // �� time���� �ο��� UL grant�� ���� Ȯ���ϴ� ����

    int maxRarWindow = 6;
    int maxMsg2TxCount = 9;
    int accessTime = 5; // 1, 6ms���� ����

    float cellRange = 400;

    // distribution: 0(Uniform), 1(Beta)
    int distribution = 1;

    if (distribution == 0){
        printf("Traffic model: Uniform\n\n");
    }
    else{
        printf("Traffic model: Beta\n\n");
    }

    for (randomSeed = 0; randomSeed < 10; randomSeed++){
        clock_t start = clock();
        // randomSeed = 2022;
        srand(randomSeed); // Fix random seed

        for (int n = 10000; n <= 100000; n += 10000){
            collisionPreambles = 0;
            totalPreambleTxop = 0;

            int nUE = n;
            struct UEinfo *UE;
            // UE�� �� ��ŭ ����� �޸� ����
            UE = (struct UEinfo *)calloc(nUE, sizeof(struct UEinfo));

            // UE ���� �ʱ�ȭ
            for (int i = 0; i < nUE; i++){
                initialUE(UE + i, i);
            }

            int time;
            int maxTime = 0;

            int nAccessUE; // Uniform distribution���� ����ϴ� ����

            if (distribution == 0){
                // Beta distribution Max time: 60s
                maxTime = 60000; // second to millisecond

                // 5 ms���� �� ���� UE�� �����ϴ���
                nAccessUE = ceil((float)n * (float)accessTime * 1.0 / (float)maxTime);

                // ����� ���� UE�� ���� 0���� �۰ų� ���� ���
                if (nAccessUE <= 0){
                    nAccessUE = 1;
                }
            }else{
                // Beta distribution Max time: 10s
                maxTime = 10000; // second to millisecond
            }

            int nSuccessUE;

            // ������� ������ UE�� ��
            int activeCheck = 0;
            grantCheck = 0;
            for (time = 0; time < maxTime; time++){
                if (time % 5 == 0){
                    grantCheck = 0;
                    // printf("%d\n", time);
                }

                if (activeCheck >= nUE){
                    activeCheck = nUE;
                }
                // 5 ms���� UE ����
                if (time % accessTime == 0 && activeCheck != nUE){
                    // printf("%d\n", activeCheck);
                    if (distribution == 0){
                        activeCheck += nAccessUE;
                    }else{
                        // Beta(3, 4)�� ������ ����
                        float betaDist = beta_dist(3, 4, (float)time / (float)maxTime);
                        int accessUEs = (int)ceil((float)nUE * betaDist / ((float)maxTime / (float)accessTime));
                        activeCheck += accessUEs;
                    }
                    
                    if (activeCheck >= nUE){
                        activeCheck = nUE;
                    }

                    for (int i = 0; i < activeCheck; i++){
                        if ((UE + i)->active == -1){
                            // MSG 1 ������ �õ��ϴ� ����
                            (UE + i)->active = 1;
                            // ���� ������ �����ϴ� �������� ����
                            (UE + i)->txTime = time + 1;
                            (UE + i)->timer = 0;
                            (UE + i)->msg2Flag = 0;
                            (UE + i)->firstTxTime = time + 1;
                            (UE+i)->xCoordinate = ((float)rand()/(float)(RAND_MAX)) * cellRange;
                            (UE+i)->yCoordinate = ((float)rand()/(float)(RAND_MAX)) * cellRange;
                        }
                    }
                }

                // Time t���������� UE���� ���¸� Ȯ��
                for (int i = 0; i < nUE; i++){
                    if (NULL != UE){
                        // Random access�� �����ϰų� ������ UE���� ���� ��Ŵ
                        if ((UE + i)->msg4Flag == 0 && (UE + i)->raFailed != -1){

                            if ((UE + i)->active == 1 && (UE + i)->msg2Flag == 0){
                                // MSG 1�� �����ϴ� UE���� preamble�� ����
                                selectPreamble(UE + i, nPreamble, time, backoffIndicator, accessTime, maxRarWindow, maxMsg2TxCount);
                            }

                            // MSG 1 -> MSG 2: Preamble collision detection
                            if ((UE + i)->active == 1 && (UE + i)->msg2Flag == 0 && (UE + i)->txTime == time){
                                int checkPreambleNumber = (UE + i)->preamble; // ������ UE�� ������ preamble number
                                preambleCollision(UE + i, UE, nUE, checkPreambleNumber, nPreamble, time, backoffIndicator, maxRarWindow, &grantCheck, nGrantUL);
                            }

                            // MSG 3 -> MSG 4: Resource allocation (contention resolution)
                            if ((UE + i)->txTime == time && (UE + i)->active == 2){
                                // ������ UE�� MSG 1->2�� ������ ���
                                requestResourceAllocation(UE + i, time, backoffIndicator, nPreamble);
                            }

                            // ������ �õ��ϴ� UE���� ���ð� ����
                            if ((UE + i)->active > 0)
                                timerIncrease(UE + i);
                        }
                    }
                }
                nSuccessUE = successUEs(UE, nUE);

                if (nSuccessUE == nUE){
                    break;
                }
            }

            int failedUEs = nUE - nSuccessUE; // ������ UE
            float totalDelay = 0;             // ������ ��� UE�� delay ��
            int preambleTxCount = 0;          // ������ ��� UE�� preamble tx Ƚ��

            for (int i = 0; i < nUE; i++){
                if (NULL != UE){
                    // RA�� ������ UE�鸸 Ȯ��
                    if ((UE + i)->msg4Flag == 1){
                        totalDelay += (float)(UE + i)->timer;
                        preambleTxCount += (UE + i)->preambleTxCounter;
                    }
                }
            }

            // ��� ���
            printf("-------- %05d Result ---------\n", activeCheck);
            if (distribution == 0){
                printf("Number of RA try UEs per Subframe: %d\n", nAccessUE);
            }

            clock_t end = clock();
            printf("Latency: %lf\n", (double)(end - start) / CLOCKS_PER_SEC);

            saveSimulationLog(randomSeed, time, nUE, nSuccessUE, failedUEs, preambleTxCount, totalDelay, distribution, nPreamble, (double)(end - start) / CLOCKS_PER_SEC);
            // saveResult(randomSeed, nUE, UE, distribution, nPreamble);
            free(UE);
        }

        

    }

    return 0;
}

void initialUE(struct UEinfo *user, int id){
    // UE �ʱ�ȭ
    user->idx = id;
    user->timer = -1;
    user->active = -1;
    user->txTime = -1;
    user->preamble = -1;
}

void selectPreamble(struct UEinfo *user, int nPreamble, int time, int backoff, int accessTime, int maxRarWindow, int maxMsg2TxCount){
    // ó�� ������ �õ��ϴ� UE
    if (user->preamble == -1){
        int tmp = rand() % nPreamble;
        user->preamble = tmp;
        user->rarWindow = 0;
        user->maxRarCounter = 0;
        user->preambleChange = 1;
        user->preambleTxCounter = 1;
        user->nowBackoff = 0;
    }
    // �̹� �� �� ������ �õ� �ߴ� UE
    else{
        // �� �� backoff counter�� 0�� UE�鸸
        if (user->nowBackoff <= 0){
            // RAR window 1 ����
            user->rarWindow++;

            // RAR window ���ð��� 5ms�� �Ǵ� ���
            if (user->rarWindow >= maxRarWindow){
                // ���� preamble�� ���� Ƚ���� �ִ��� ���
                if (user->maxRarCounter >= maxMsg2TxCount){
                    // MSG 2 ���ſ� ���������� ������ UE�� ������ RA���� �õ� X
                    // user->raFailed = -1;
                    int preamble = rand() % nPreamble;
                    user->preamble = preamble;
                    user->rarWindow = 0;
                    user->maxRarCounter = 0;
                    user->preambleChange = 1;
                    user->preambleTxCounter = 1;
                    user->nowBackoff = 0;
                    user->timer = 0;
                    user->firstTxTime = time + 1;
                    // user->txTime = time + 1;

                    int tmp = (rand() % backoff);

                    int subTime = user->txTime + tmp;
                    
                    if (subTime % accessTime == 0){
                        // ���� �ð� ������ ���� �õ�
                        user->txTime = subTime + 1;
                    }else if (subTime % accessTime == 1){
                        // ���� �ð� ������ ���� �õ�
                        user->txTime = subTime;
                    }else{
                        // ���� �ð� ������ ���� �õ�
                        user->txTime = subTime + (accessTime - (subTime % accessTime) + 1);
                    }

                    // backoff counter ����
                    user->nowBackoff = user->txTime - time;
                }
                else{
                    // RAR window �ʱ�ȭ
                    user->rarWindow = 0;
                    // �ִ� ���� Ƚ���� 1 ����
                    user->maxRarCounter++;

                    user->preambleTxCounter++;

                    int tmp = (rand() % backoff);

                    int subTime = time + tmp;
                    
                    if (subTime % accessTime == 0){
                        // ���� �ð� ������ ���� �õ�
                        user->txTime = subTime + 1;
                    }else if (subTime % accessTime == 1){
                        // ���� �ð� ������ ���� �õ�
                        user->txTime = subTime;
                    }else{
                        // ���� �ð� ������ ���� �õ�
                        user->txTime = subTime + (accessTime - (subTime % accessTime) + 1);
                    }

                    // backoff counter ����
                    user->nowBackoff = user->txTime - time;
                    user->secondTxTime = user->txTime;
                }
            }
        }
    }
}

// preamble�� �浹 Ȯ��
void preambleCollision(struct UEinfo *user, struct UEinfo *UEs, int nUE, int checkPreamble, int nPreamble, int time, int backoff, int maxRarWindow, int *grantCheck, int nGrantUL){
    int check = 0;
    // MSG 1�� BS�� ������ �� ���� 2 time slot�� ��ٷ��� ��
    // ���� �����ϴ� �������� 2 time slot�� ������ ���� time�� �����ؾ� �浹�� Ȯ���� �� ����
    int *userIdx = (int *)malloc(sizeof(int) * nUE);
    if (NULL != userIdx){
        for (int i = 0; i < nUE; i++){
            // MSG 1�� �����ϴ� �����̸鼭 ���� Ȯ���ϴ� UE�� ���� ������ ���� UE�� Ȯ��
            if ((UEs + i)->active == 1 && (UEs + i)->txTime == time){
                // preamble �浹
                if ((UEs + i)->preamble == checkPreamble){
                    userIdx[check] = (UEs + i)->idx;
                    check++;
                }
            }
        }

        if (check == 1){ // ���� �����ϴ� UE�� ������ preamble�� �浹�� ���� ���
            // ��ü preamble���� Ƚ��
            totalPreambleTxop++;
            // ������ �Ҵ��� �� ���� Ȯ�ο� ������ 1�� ����
            *grantCheck = *grantCheck + 1;
            // �� time�� �ִ� UL grant�� 12�� ������ �ο�
            if (*grantCheck < nGrantUL){
                user->active = 2;
                user->txTime = time + 11;
                user->connectionRequest = 0;
                user->msg2Flag = 1;

                // printf("%d\n", *grantCheck);
            }else{
                user->txTime++;
            }
        }else{ // Preamble�� �浹�� ���
            collisionPreambles++;
            // ��ü preamble���� Ƚ��
            totalPreambleTxop++;

            for (int i = 0; i < check; i++){
                // RAR window�� 5�� ����
                // (UEs+userIdx[i])->rarWindow = maxRarWindow;

                // ���� ������ 3 time slot �ڷ� �̷�
                (UEs + userIdx[i])->txTime++;
                // ����� �浹�� �� UE�� RAR window�� �ִ밡 �ǵ� �� ����
                // �̹� �浹�� �߻��ϱ� ������ RAR window�� �ִ�� ����

                // ���� ������ time + maxRarWindow - 2�� �� ������
                // �̹� 2 Ÿ�� ���� ������ Ȯ���ϴ� �� �̱� ����
            }
            // printf("%d\n ", subTime);
        }
    }
    free(userIdx);
}

void requestResourceAllocation(struct UEinfo *user, int time, int backoff, int nPreamble){
    user->connectionRequest++;
    if (user->connectionRequest < 48){
        float p = (float)rand() / (float)RAND_MAX;
        if (p > 0.1){
            // 90%�� Ȯ���� MSG 3-4 ����
            user->msg4Flag = 1;
            user->timer = user->timer + 6;
            user->active = 0;
        }else{
            user->connectionRequest = 48;
            user->txTime += 48;
        }
    }else{
        // �ִ� ��� �ð����� MSG 4�� ������ ���� ���
        // ��� ���¸� �ʱ�ȭ �Ѵ�.
        int tmp = rand() % backoff;
        int subTime = user->txTime + tmp;
        int accessTime = 5;
        if (subTime % accessTime == 0){
            // ���� �ð� ������ ���� �õ�
            user->txTime = subTime + 1;
        }else if (subTime % accessTime == 1){
            // ���� �ð� ������ ���� �õ�
            user->txTime = subTime;
        }else{
            // ���� �ð� ������ ���� �õ�
            user->txTime = subTime + (accessTime - (subTime % accessTime) + 1);
        }
        user->active = 1;
        // user->txTime = time + tmp;
        user->nowBackoff = user->txTime - time;
        user->preamble = rand() % nPreamble;
        user->timer = 0;
        user->msg2Flag = 0;
        user->rarWindow = 0;
        user->maxRarCounter = 0;
        user->connectionRequest = 0;
        // user->preambleTxCounter++;
    }
}

void timerIncrease(struct UEinfo *user){
    user->timer++;

    if (user->nowBackoff > 0){
        user->nowBackoff--;
    }
}

int successUEs(struct UEinfo *user, int nUE){
    int success = 0;
    for (int i = 0; i < nUE; i++){
        if ((user + i)->msg4Flag == 1){
            success++;
        }
    }
    return success;
}

// Result logging
void saveSimulationLog(int seed, int time, int nUE, int nSuccessUE, int failedUEs, int preambleTxCount, float totalDelay, int distribution, int nPreamble, double layency){

    float ratioSuccess = (float)nSuccessUE / (float)nUE * 100.0;
    float ratioFailed = (float)failedUEs / (float)nUE;
    float nCollisionPreambles = (float)collisionPreambles / ((float)nUE * (float)nPreamble);
    float averagePreambleTx = (float)preambleTxCount / (float)nSuccessUE;
    float averageDelay = totalDelay / (float)nSuccessUE;

    printf("Number of UEs: %d\n", nUE);
    printf("Total simulation time: %dms\n", time);
    printf("Success ratio: %.2lf\n", ratioSuccess);
    printf("Number of succeed UEs: %d\n", nSuccessUE);
    printf("Number of collision preambles: %.6lf\n", nCollisionPreambles);
    printf("Average preamble tx count: %.2lf\n", averagePreambleTx);
    printf("Average delay: %.2lf\n", averageDelay);

    FILE *fp;
    char resultBuff[1000];
    char fileNameResult[500];

    if (distribution == 0){
        sprintf(fileNameResult, "./Uniform_SimulationResults/%d_%d_%d_Results.txt", seed, nPreamble, nUE);
    }else{
        sprintf(fileNameResult, "./Beta_SimulationResults/%d_%d_%d_Results.txt", seed, nPreamble, nUE);
    }

    fp = fopen(fileNameResult, "w+");

    sprintf(resultBuff, "%d\n", nUE);
    fputs(resultBuff, fp);

    // sprintf(resultBuff, "Total simulation time: %dms\n", time);
    // fputs(resultBuff, fp);

    sprintf(resultBuff, "%.2lf\n", ratioSuccess);
    fputs(resultBuff, fp);

    sprintf(resultBuff, "%d\n", nSuccessUE);
    fputs(resultBuff, fp);

    // sprintf(resultBuff, "Number of collision preambles: %.2lf\n", nCollisionPreambles);
    // fputs(resultBuff, fp);

    sprintf(resultBuff, "%.2lf\n", averagePreambleTx);
    fputs(resultBuff, fp);

    sprintf(resultBuff, "%.2lf\n", averageDelay);
    fputs(resultBuff, fp);

    sprintf(resultBuff, "%lf", layency);
    fputs(resultBuff, fp);

    fclose(fp);
}
void saveResult(int seed, int nUE, struct UEinfo *UE, int distribution, int nPreamble){
    FILE *fp_l;
    char logBuff[1000];
    char fileNameResultLog[500];
    if (distribution == 0){
        sprintf(fileNameResultLog, "./Uniform_SimulationResults/%d_Exclude_msg2_failures_UE%05d_Logs.txt", nPreamble, nUE);
    }else{
        sprintf(fileNameResultLog, "./NomaBeta/%d_%d_UE%05d_Logs.txt", seed, nPreamble, nUE);
    }

    fp_l = fopen(fileNameResultLog, "w+");

    for (int i = 0; i < nUE; i++){
        // printf("Idx: %d | Timer: %d | Active: %d | txTime: %d | Preamble: %d | RAR window: %d | Max RAR: %d | Preamble reTx: %d | MSG 2 Flag: %d | ConnectRequest: %d | MSG 4 Flag: %d\n",
        // (UE+i)->idx, (UE+i)->timer, (UE+i)->active, (UE+i)->txTime, (UE+i)->preamble, (UE+i)->rarWindow, (UE+i)->maxRarCounter, (UE+i)->preambleTxCounter, (UE+i)->msg2Flag, (UE+i)->connectionRequest, (UE+i)->msg4Flag);
        sprintf(logBuff, "Idx: %d | Timer: %d | Active: %d | txTime: %d | FirstTxTime: %d | SecondTxTime: %d | NowBackoff: %d | Preamble: %d | Preamble change: %d | RAR window: %d | Max RAR: %d | Preamble reTx: %d | MSG 2 Flag: %d | ConnectRequest: %d | MSG 4 Flag: %d\n",
                (UE + i)->idx, (UE + i)->timer, (UE + i)->active, 
                (UE + i)->txTime, (UE + i)->firstTxTime, 
                (UE + i)->secondTxTime, (UE + i)->nowBackoff, 
                (UE + i)->preamble, (UE + i)->preambleChange, 
                (UE + i)->rarWindow, (UE + i)->maxRarCounter, 
                (UE + i)->preambleTxCounter, (UE + i)->msg2Flag, 
                (UE + i)->connectionRequest, (UE + i)->msg4Flag);

        fputs(logBuff, fp_l);
    }

    fclose(fp_l);
}

float beta_dist(float a, float b, float x){
    float betaValue = (1 / betaF) * (pow(x, (a - 1))) * (pow((1 - x), (b - 1)));
    return betaValue;
}