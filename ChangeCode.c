#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#define betaF 0.0165

typedef struct  {
    int idx;
    int timer;
    int active;
    int preamble;
    int rarWindow;
    int msg1ReTx;
    int msg2;
    int msg3Wait;
    int txTime;
    int nowBackoff;
    int firstTxTime;
    int secondTxTime;
    int RA;

    int power;
    double channelGain;
}UserInfo;

int nPreamble = 54;        // Number of preambles
int backoffIndicator = 20; // Number of backoff indicator
int nGrantUL = 12;         // Number of UL Grant

int maxRarWindow = 5;
int maxMsg1ReTx = 10;
int accessTime = 5;

void initUserInfo(UserInfo *user, int idx, int timer, int active, int preamble, int rarWindow, int msg1ReTx, int msg2, int msg3Wait, int txTime, int nowBackoff, int RA, int power, double channelGain){
    user->idx = idx;
    user->timer = timer;
    user->active = active;
    user->preamble = preamble;
    user->rarWindow = rarWindow;
    user->msg1ReTx = msg1ReTx;
    user->msg2 = msg2;
    user->msg3Wait = msg3Wait;
    user->txTime = txTime;
    user->nowBackoff = nowBackoff;
    user->RA = RA;

    user->power = power;
    user->channelGain = channelGain;
}

void preambleCollisionDetection(UserInfo *user, int activeCheck, int time, int *grantCheck){
    int preambleTxCheck = 0;
    int *preambleTxUEs = (int*)malloc(activeCheck * sizeof(int));

    // Store idx of UEs attempting to send MSG 1 at the current time
    for(int i = 0; i < activeCheck; i++){
        if((user + i)->RA == 0 && (user+i)->active == 1 && (user+i)->txTime == time && (user+i)->nowBackoff <= 0 && (user+i)->msg2 == 0){
            preambleTxUEs[preambleTxCheck] = (user+i)->idx;
            preambleTxCheck++;
        }
    }

    if(preambleTxCheck != 0){
        int *preambles = (int*)calloc(nPreamble, sizeof(int));
        int **userCheck = (int**)malloc(nPreamble * sizeof(int*));
        
        for(int i = 0; i < nPreamble; i++){
            userCheck[i] = (int*)calloc(preambleTxCheck, sizeof(int));
        }

        for(int i = 0; i < preambleTxCheck; i++){
            int RAPID = (user+(preambleTxUEs[i]))->preamble;

            userCheck[RAPID][preambles[RAPID]] = preambleTxUEs[i];
            preambles[RAPID]++;
        }

        for(int i = 0; i < nPreamble; i++){
            if(preambles[i] == 1){
                *grantCheck = *grantCheck + 1;
                for(int j = 0; j < preambles[i]; j++){
                    if(*grantCheck <= nGrantUL){
                        (user+userCheck[i][j])->msg2 = 1;
                        // (user+userCheck[i][j])->txTime += 11;
                    }else{
                        (user+userCheck[i][j])->txTime += 3;
                    }
                }
                
            }else if(preambles[i] > 1){
                // printf("---------\n");
                for(int j = 0; j < preambles[i]; j++){
                    // printf("%d %d %d\n", i, preambles[i], userCheck[i][j]);
                    (user+userCheck[i][j])->txTime += 3;
                }
            }
        }
        free(preambles);
        for(int i = 0; i < nPreamble; i++){
            free(userCheck[i]);
        }
        free(userCheck);
    }
    free(preambleTxUEs);
}

void msg2Results(UserInfo *user, int time){
    int txLimit = 0;
    if(user->active == 1 && user->nowBackoff <= 0){
        user->rarWindow++;
        if(user->msg2 == 0){
            int randomBackoff = rand() % 4;

            if(randomBackoff == 0){
                user->txTime += (5-txLimit);
                user->nowBackoff = user->txTime - time;
            }else if(randomBackoff == 1){
                user->txTime += (10-txLimit);
                user->nowBackoff = user->txTime - time;
            }else if(randomBackoff == 2){
                user->txTime += (15-txLimit);
                user->nowBackoff = user->txTime - time;
            }else{
                user->txTime += (20-txLimit);
                user->nowBackoff = user->txTime - time;
            }
            user->secondTxTime = user->txTime;

            // printf("%d\n", user->txTime);
            if(user->rarWindow > maxRarWindow){
                user->msg1ReTx++;
                if(user->msg1ReTx == maxMsg1ReTx){
                    initUserInfo(user, user->idx, 0, 1, rand()%nPreamble, 0, 0, 0, 0, user->txTime, user->nowBackoff, 0, 0, 0.);
                }
            }
            
        }else if(user->msg2 == 1){
            // MSG 1에 대한 MSG 2수신 성공
            user->txTime += 11;
        }
    }
    
}

/*
UserInfo *user, int idx, int timer, int active, int preamble, int rarWindow, int msg1ReTx, int msg2, int msg3Wait, int txTime, int nowBackoff, int RA, int power, double channelGain
*/

void resourceRequestAllocation(UserInfo *user, int activeCheck){
    for(int i = 0; i < activeCheck; i++){
        if((user+i)->msg2 == 1 && (user+i)->txTime && (user+i)->active == 1){
            if((user->msg3Wait) < 48){
                float p = (float)rand() / (float)RAND_MAX;
                if(p > 0.1){
                    (user+i)->active = 0;
                    (user+i)->RA = 1;
                    (user+i)->timer = (user+i)->timer + 6;
                }else{
                    (user+i)->msg2 = 0;
                    (user+i)->txTime += 48;
                    (user->msg3Wait) = 48;
                }
            }else{
                initUserInfo(user, user->idx, 0, 1, rand()%nPreamble, 0, 0, 0, 0, user->txTime, user->nowBackoff, 0, 0, 0.);
            }
        }
    }
}
/*
    user->idx = idx;
    user->timer = timer;
    user->active = active;
    user->preamble = preamble;
    user->rarWindow = rarWindow;
    user->msg1ReTx = msg1ReTx;
    user->msg2 = msg2;
    user->txTime = txTime;
    user->nowBackoff = nowBackoff;
    user->RA = RA;
*/
void activeUE(UserInfo *user, int time){
    user->active = 1;
    user->preamble = rand() % nPreamble;
    user->txTime = time + 1;
    user->firstTxTime = time + 1;
}

void timerIncrease(UserInfo *user){
    user->timer++;

    if(user->nowBackoff > 0){
        user->nowBackoff--;
    }
}

int successUEs(UserInfo *user, int nUE){
    int success = 0;
    for (int i = 0; i < nUE; i++){
        if ((user + i)->RA == 1){
            success++;
        }
    }
    return success;
}

float betaDist(float a, float b, float x){
    float betaValue = (1 / betaF) * (pow(x, (a - 1))) * (pow((1 - x), (b - 1)));
    return betaValue;
}

void saveResult(UserInfo *user, int nUE){
    FILE * fp_l;
    char logBuff[1000];
    char fileNameResultLog[500];

    sprintf(fileNameResultLog, "./TestResults/%d_logs.txt", nUE);

    fp_l = fopen(fileNameResultLog, "w+");

    for(int i = 0; i < nUE; i++){
        sprintf(logBuff, "Idx: %d| FirstTxTime: %d| SecondTxTime: %d| Timer: %d| Active: %d| Preamble: %d| RarWindow: %d| Msg1ReTx: %d| MSG2: %d| TxTime: %d| Backoff: %d| RA: %d\n",
                        (user + i)->idx, 
                        (user + i)->firstTxTime, 
                        (user + i)->secondTxTime, 
                        (user + i)->timer, 
                        (user + i)->active, 
                        (user + i)->preamble, 
                        (user + i)->rarWindow, 
                        (user + i)->msg1ReTx, 
                        (user + i)->msg2, 
                        (user + i)->txTime, 
                        (user + i)->nowBackoff, 
                        (user + i)->RA);
        fputs(logBuff, fp_l);
    }

    fclose(fp_l);
}

int main(int argc, char **argv){
    int nUE;
    for(nUE = 10000; nUE <= 100000; nUE += 10000){
        UserInfo *UEs;
        UEs = (UserInfo *)calloc(nUE, sizeof(UserInfo));

        for(int i = 0; i < nUE; i++){
            initUserInfo(UEs+i, i, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
        }

        int activeCheck = 0;
        int grantCheck = 0;
        int time;
        int maxTime = 10000;
        int nSuccessUE;

        for(time = 0; time < maxTime; time++){
            nSuccessUE = 0;
            if(time % 5 == 1){
                grantCheck = 0;
            }

            if(time % accessTime == 0){
                float numBetaDist = betaDist(3, 4, (float)time/(float)maxTime);
                int accessUEs = (int)ceil((float)nUE * numBetaDist / ((float)maxTime / (float)accessTime));
                activeCheck += accessUEs;

                if(activeCheck >= nUE){
                    activeCheck = nUE;
                }

                for(int i = 0; i < activeCheck; i++){
                    if((UEs + i)->RA == 0 && (UEs + i)->active == 0){
                        activeUE(UEs+i, time);
                    }
                }
            }

            // int check = 0;
            // while(1){
            //     int randUE = rand() % nUE;
            //     activeUE(UEs+randUE, time);
            //     for(int i = 0; i < nUE; i++){
            //         if((UEs + i)->active == 1){
            //             check++;
            //         }
            //     }

            //     if(check >= activeCheck){
            //         break;
            //     }
            // }

            preambleCollisionDetection(UEs, activeCheck, time, &grantCheck);

            for(int i = 0; i < activeCheck; i++){
                if((UEs+i)->nowBackoff == 0 && (UEs+i)->active == 1)
                    msg2Results(UEs+i, time);
            }

            resourceRequestAllocation(UEs, activeCheck);

            // Increase timers of all active UEs by 1
            for(int i = 0; i <= activeCheck; i++){
                if((UEs+i)->active == 1){
                    timerIncrease(UEs+i);
                }
            }
            nSuccessUE = successUEs(UEs, nUE);
            if(nSuccessUE == nUE){
                break;
            }
        }
        saveResult(UEs, nUE);

        printf("UE: %d\n", nUE);

        printf("Success ratio: %lf\n", ((float)nSuccessUE/(float)nUE)*100.0);
        int delay = 0;
        for(int i = 0; i < nUE; i++){
            if((UEs+i)->RA == 1){
                delay += (UEs+i)->timer;
            }
        }

        printf("Average delay: %lf\n",((float)delay/(float)nSuccessUE));
        free(UEs);
    }
    printf("Done\n");

    return 0;
}