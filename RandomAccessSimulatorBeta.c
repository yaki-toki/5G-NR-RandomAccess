#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define betaF 0.0165

struct UEinfo{
    int idx;                // UE 식별자
    int timer;              // UE별 타이머 (최종 RA 성공까지 걸리는 딜레이)
    int active;             // 현재 전송 상태 (Not active: 0, MSG 1-2: 1, MSG 3-4: 2)
    int txTime;             // MSG 전송 시간
    int preamble;           // 선택하는 preamble
    int rarWindow;          // RAR window 현재는 5로 설정
    int maxRarCounter;      // 최대 RAR window의 대기 횟수 현재는 10
    int preambleTxCounter;  // Preamble을 변경하여 재전송한 횟수
    int msg2Flag;           // MSG 1-2 성공 여부
    int connectionRequest;  // MSG 3 타이머 현재는 48로 설정
    int msg4Flag;           // MSG 3-4 성공 여부 (결론 RA 성공)
    int preambleChange;     // 성공 할 때 까지 preamble 몇 번 바꿨는지
    int raFailed;           // MSG 2 max count 되면 그냥 실패
    int nowBackoff;         // Backoff counter
    int rampingPower;       // using for NOMA
};

void initialUE(struct UEinfo *user, int id);
void selectPreamble(struct UEinfo *user, int nPreamble, int time, int backoff, int maxRarWindow, int maxMsg2TxCount);
void preambleCollision(struct UEinfo *user, struct UEinfo *UEs, int nUE, int checkPreamble, int nPreamble, int time, int backoff, int maxRarWindow);
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

    // distribution: 0(Uniform), 1(Beta)
    int distribution = 1;

    for(int n = 10000; n <= 100000; n+=10000){
 
        int nUE = n;
        struct UEinfo *UE;
        // UE의 수 만큼 사용할 메모리 선언
        UE = (struct UEinfo *) calloc(nUE, sizeof(struct UEinfo));

        int nPreamble = 64;         // Number of preambles
        int backoffIndicator = 20;  // Number of backoff indicator
        int nGrantUL = 12;          // Number of UL Grant

        int maxRarWindow = 5;
        int maxMsg2TxCount = 10;

        // UE 정보 초기화
        for(int i = 0; i < nUE; i++){
            initialUE(UE+i, i);
        }
        
        int time;
        int maxTime = 0;

        int nAccessUE;      // Uniform distribution에서 사용하는 변수

        int accessTime = 5; // 1, 6ms마다 접근
        if(distribution == 0){
            // Beta distribution Max time: 60s
            maxTime = 60000;    // second to millisecond

            // 5 ms마다 몇 대의 UE가 접근하는지
            nAccessUE = ceil((float)n * (float)accessTime * 1.0/(float)maxTime);
            
            // 계산한 접근 UE의 수가 0보다 작거나 같은 경우
            if(nAccessUE <= 0){
                nAccessUE = 1;
            }

        }else{
            // Beta distribution Max time: 10s
            maxTime = 10000;    // second to millisecond
        }

        int nSuccessUE;

        // 현재까지 접근한 UE의 수
        int activeCheck = 0;

        for(time = 0; time < maxTime; time++){
            
            // 5 ms마다 UE 접근
            if(time % accessTime == 1){
                // 최대 UE수를 넘는 경우
                if(activeCheck >= nUE){
                    activeCheck = nUE;
                }else{
                    if(distribution == 0){
                        activeCheck += nAccessUE;
                    }else{
                        // Beta(3, 4)를 따르는 분포
                        float betaDist = beta_dist(3, 4, (float)time/(float)maxTime);
                        int accessUEs = (int)ceil((float)nUE * betaDist / ((float)maxTime/(float)accessTime));
                        activeCheck += accessUEs;
                    }
                    // printf("%d\n", activeCheck);
                    for(int i = 0; i < activeCheck; i++){
                        if((UE+i)->active == -1){
                            // MSG 1 전송을 시도하는 상태
                            (UE+i)->active = 1;
                            // 지금 시점을 전송하는 시점으로 기준
                            (UE+i)->txTime = time+1;
                            (UE+i)->timer = 0;
                            (UE+i)->msg2Flag = 0;
                        }
                    }
                }
            }

            // Time t시점에서의 UE들의 상태를 확인
            for(int i = 0; i < nUE; i++){
                // Random access에 성공하거나 실패한 UE들은 제외 시킴
                if((UE+i)->msg4Flag == 0 && (UE+i)->raFailed != -1){

                    if((UE+i)->active == 1 && (UE+i)->msg2Flag == 0){
                        // MSG 1을 전송하는 UE들이 preamble을 선택
                        selectPreamble(UE+i, nPreamble, time, backoffIndicator, maxRarWindow, maxMsg2TxCount);
                    }

                    // MSG 1 -> MSG 2: Preamble collision detection
                    if((UE+i)->txTime + 2 == time && (UE+i)->txTime != -1){
                        
                        int checkPreambleNumber = (UE+i)->preamble; // 현재의 UE가 선택한 preamble number
                        preambleCollision(UE+i, UE, nUE, checkPreambleNumber, nPreamble, time, backoffIndicator, maxRarWindow);
                    }
                    
                    // MSG 3 -> MSG 4: Resource allocation (contention resolution)
                    if((UE+i)->active == 2 && (UE+i)->txTime+2 == time){
                        // 현재의 UE가 MSG 1->2를 성공한 경우
                        requestResourceAllocation(UE+i, time, backoffIndicator, nPreamble);
                    }

                    // 전송을 시도하는 UE들의 대기시간 증가
                    if((UE+i)->active > 0 && (UE+i)->msg4Flag == 0)
                        timerIncrease(UE+i);
                }
            }
            nSuccessUE = successUEs(UE, nUE);
            
            if(nSuccessUE == nUE){
                break;
            }
        }

        int failedUEs = nUE - nSuccessUE;   // 실패한 UE
        float totalDelay = 0;               // 성공한 모든 UE의 delay 값
        int preambleTxCount = 0;            // 성공한 모든 UE의 preamble tx 횟수

        for(int i = 0; i < nUE; i++){
            // RA에 성공한 UE들만 확인
            if((UE+i)->msg4Flag != 0){
                totalDelay += (float)(UE+i)->timer;
                preambleTxCount += (UE+i)->preambleTxCounter;
            }
        }

        // 결과 출력
        printf("-------- %05d Result ---------\n", activeCheck);
        saveSimulationLog(time, nUE, nSuccessUE, failedUEs, preambleTxCount, totalDelay, distribution);
        saveResult(nUE, UE, distribution);
    }

    return 0;
}

void initialUE(struct UEinfo *user, int id){
    // UE 초기화
    user->idx = id;
    user->timer = -1;
    user->active = -1;
    user->txTime = -1;
    user->preamble = -1;
}

void selectPreamble(struct UEinfo *user, int nPreamble, int time, int backoff, int maxRarWindow, int maxMsg2TxCount){
    // 처음 전송을 시도하는 UE
    if(user->preamble == -1){
        int tmp = rand() % nPreamble;
        user->preamble = tmp;
        user->rarWindow = 0;
        user->maxRarCounter = 0;
        user->preambleTxCounter = 0;
        user->preambleChange = 1;
        user->nowBackoff = 0;
    }
    // 이미 한 번 전송을 시도 했던 UE
    else{
        // 그 중 backoff counter가 0인 UE들만
        if(user->nowBackoff == 0){

            // RAR window 1 증가
            user->rarWindow++;

            // RAR window 대기시간이 5ms가 되는 경우
            if(user->rarWindow >= maxRarWindow){
                int tmp = (rand() % backoff) + 2;
                // 일정 시간 다음에 전송 시도
                user->txTime = time + tmp;
                // backoff counter 설정
                user->nowBackoff = tmp;

                // RAR window 초기화
                user->rarWindow = 0;

                // 최대 전송 횟수를 1 증가
                user->maxRarCounter++;
                
                // 지금 preamble의 전송 횟수가 최대인 경우
                if(user->maxRarCounter >= maxMsg2TxCount){
                    // MSG 2 수신에 최종적으로 실패한 UE는 앞으로 RA접근 시도 X
                    user->raFailed = -1;

                    // user->preamble = rand() % nPreamble;
                    // 최대 전송 횟수를 0으로 초기화
                    // user->maxRarCounter = 0;
                    // user->preambleChange++;

                }
            }
        }
    }
}

// preamble의 충돌 확인
void preambleCollision(struct UEinfo *user, struct UEinfo *UEs, int nUE, int checkPreamble, int nPreamble, int time, int backoff, int maxRarWindow){
    int check = 0;
    // MSG 1이 BS에 도달할 때 까지 2 time slot을 기다려야 함
    // 따라서 전송하는 시점에서 2 time slot이 더해진 현재 time이 동일해야 충돌을 확인할 수 있음
    int* userIdx = (int*)malloc(sizeof(int) * nUE);
    for(int i = 0; i < nUE; i++){
        // MSG 1을 전송하는 상태이면서 현재 확인하는 UE와 전송 시점이 같은 UE를 확인
        if((UEs+i)->active == 1 && (UEs+i)->txTime + 2 == time){
            // preamble 충돌
            if((UEs+i)->preamble == checkPreamble){
                
                userIdx[check] = (UEs+i)->idx;
                check++;
            }
        }
    }
    if(check == 1){ // 현재 전송하는 UE가 선택한 preamble의 충돌이 없는 경우
        // 전체 preamble전송 횟수
        totalPreambleTxop++;
        // preamble 전송 횟수
        user->preambleTxCounter++;
        user->active = 2;
        user->txTime = time + 2;
        user->connectionRequest = 0;
        user->msg2Flag = 1;
    }else{  // Preamble이 충돌한 경우
        collisionPreambles += check;
        for(int i = 0; i < check; i++){
            // RAR window를 5로 설정
            (UEs+userIdx[i])->rarWindow = maxRarWindow;
            // 전송 시점을 3 time slot 뒤로 미룸
            (UEs+userIdx[i])->txTime = time + maxRarWindow - 2;

            // 실재로 충돌이 된 UE는 RAR window가 최대가 되될 때 까지 
            // 이미 충돌이 발생하기 때문에 RAR window를 최대로 설정

            // 전송 시점을 time + maxRarWindow - 2를 한 이유는 
            // 이미 2 타임 지난 다음에 확인하는 것 이기 때문
        }
    }
    return;
}

void requestResourceAllocation(struct UEinfo *user, int time, int backoff, int nPreamble){
    user->connectionRequest++;
    if(user->connectionRequest < 48){
        float p = (float)rand() / (float)RAND_MAX;
        if(p > 0.1){
            // 90%의 확률로 MSG 3-4 성공
            user->msg4Flag = 1;
            user->active = 0;
        }else{
            // printf("MSG3 Fail %d\n", user->idx);
            // 10%의 확률로 MSG 3-4 실패
            user->txTime = time + 1; //(rand() % backoff) + 1;
        }
    }else{
        // 최대 대기 시간동안 MSG 4의 응답이 없는 경우
        // 모든 상태를 초기화 한다.
        int tmp = (rand() % backoff) + 2;
        user->active = 1;
        user->txTime = time + tmp;
        user->nowBackoff = tmp;
        user->preamble = rand() % nPreamble;
        user->msg2Flag = 0;
        user->rarWindow = 0;
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

// Result logging
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
