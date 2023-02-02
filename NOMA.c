#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#define betaF 0.0165

typedef struct {
    int idx;
    int timer;
    int active;
    int preamble;
    int nTxPreamble;
    int rarWindow;
    int msg1ReTx;
    int msg2;
    int msg3Wait;
    int msg3Faile;
    int txTime;
    int nowBackoff;
    int firstTxTime;
    int secondTxTime;
    int RaFailed;
    int RA;

    // ------------------------------------------- //
    int power;       // Power 증가 level
    float xCoordinate;      // UE의 x 좌표
    float yCoordinate;      // UE의 y 좌표
    float xChannel;
    float yChannel;
    int sector;
    float angle;            // BS -- UE의 각도
    float channel;
    float distance;         // BS -- UE의 거리
    float pathLoss;         // BS -- UE의 pathloss
    double channelGain;
    int partner;
}UserInfo;

int nPreamble = 54;        // Number of preambles
int backoffIndicator = 20; // Number of backoff indicator
int nGrantUL = 2;          // Number of UL Grant

int maxRarWindow = 5;
int maxMsg1ReTx = 10;
int accessTime = 5;

float SysBandwidth;
float TxPower_dB;
float NoiseDensity_dB;
float TxPower;
float NoisePower;

float pi = 3.14;
float cellRadius = 500; // 500m
float bandwidth = 5; // 5GHz


void printCHgain(UserInfo* UE, int nUE) {
    for (int i = 0; i < nUE; i++) {
        if ((UE + i)->channelGain != -1) {
            printf("%.8f ", (UE + i)->channelGain);
            if (i > 0 && i % 20 == 19) {
                printf("\n");
            }
        }
    }
}

void UEequal(UserInfo* para, UserInfo* data) {
    para->idx = data->idx;
    para->timer = data->timer;
    para->active = data->active;
    para->preamble = data->preamble;
    para->rarWindow = data->rarWindow;
    para->msg1ReTx = data->msg1ReTx;
    para->msg2 = data->msg2;
    para->msg3Wait = data->msg3Wait;
    para->txTime = data->txTime;
    para->nowBackoff = data->nowBackoff;
    para->firstTxTime = data->firstTxTime;
    para->secondTxTime = data->secondTxTime;
    para->RaFailed = data->RaFailed;
    para->RA = data->RA;

    para->power = data->power;
    para->channelGain = data->channelGain;
}
void sortUE(UserInfo* picked, int activeUE, UserInfo* sorted) {
    UserInfo temp = { 0, };

    // 버블 정렬
    for (int i = 0; i < activeUE; i++) {
        for (int j = 0; j < activeUE - 1; j++) {
            if ((picked + j + 1)->channelGain < (picked + j)->channelGain) {
                UEequal(&temp, (picked + j));
                UEequal((picked + j), (picked + j + 1));
                UEequal((picked + j + 1), &temp);
            }
        }
    }
}
void initUserInfo(UserInfo* user, int idx, int timer, int active, int preamble, int rarWindow, int msg1ReTx, int msg2, int msg3Wait, int txTime, int nowBackoff, int RaFailed, int RA, int power, double channelGain) {
    user->idx = idx;
    user->timer = 0;
    user->active = active;
    user->preamble = preamble;
    user->nTxPreamble = 0;
    user->rarWindow = rarWindow;
    user->msg1ReTx = msg1ReTx;
    user->msg2 = msg2;
    user->msg3Wait = msg3Wait;
    user->msg3Faile = 0;
    user->txTime = txTime;
    user->nowBackoff = nowBackoff;
    user->RaFailed = RaFailed;
    user->RA = RA;

    // ------------------------------------------- //
    user->power = power;       // Power 증가 level
    user->xCoordinate = 0;      // UE의 x 좌표
    user->yCoordinate = 0;      // UE의 y 좌표
    user->sector = -1;
    user->angle = 0;            // BS -- UE의 각도
    user->channel = 0;
    user->distance = 0;         // BS -- UE의 거리
    user->pathLoss = 0;         // BS -- UE의 pathloss
    user->channelGain = channelGain;
}
void activeUE(UserInfo* user, int time) {
    user->active = 1;
    user->preamble = rand() % nPreamble;
    user->nTxPreamble++;
    user->txTime = time + 1;
    user->timer = 0;
    user->rarWindow = 0;
    user->msg1ReTx = 0;
    user->nowBackoff = 0;
    user->firstTxTime = time + 1;

    float angle = (float)rand() / (float)(RAND_MAX) * 2 * pi;

    user->angle = angle;

    if (user->angle >= 0 && user->angle < ((1. / 3.) * pi)) {
        user->sector = 0;
    }
    else if (user->angle >= ((1. / 3.) * pi) && user->angle < ((2. / 3.) * pi)) {
        user->sector = 1;
    }
    else if (user->angle >= ((2. / 3.) * pi) && user->angle < 3.14) {
        user->sector = 2;
    }
    else if (user->angle >= pi && user->angle < ((4. / 3.) * pi)) {
        user->sector = 3;
    }
    else if (user->angle >= ((4. / 3.) * pi) && user->angle < ((5. / 3.) * pi)) {
        user->sector = 4;
    }
    else {
        user->sector = 5;
    }

    float r;

    while (1) {
        r = cellRadius * sqrt((float)rand() / (float)RAND_MAX);
        if (r > 35.0) {
            break;
        }
    }

    float pathloss = 128.1 + 37.6 * log10(r / 1000);

    user->xCoordinate = r * cos(angle);
    user->xChannel = sin(cos(angle));
    user->yCoordinate = r * sin(angle);
    user->yChannel = sin(sin(angle));
    user->distance = r;
    user->pathLoss = pow(10, (-0.1 * pathloss));
    user->channelGain = 20 * log10(4. * pi * r / (bandwidth / 1000));
    double env = sqrt(user->xCoordinate * user->xCoordinate + user->yCoordinate * user->yCoordinate);
    double ch_g=0, rayleigh;
    while (ch_g < 1e-7) { // pairing시 채널 크기가 n배 차이나는 경우를 위해 설정한 조건임 굳이 크기 지정할 필요 없음
       pathloss = sqrt(1 + pow(env, 2));            // free space 가정
       rayleigh = sqrt(-2 * log((double)rand() / (double)RAND_MAX));      // 영문 위키트리 "Rayleigh Distribution" 참조...
       ch_g = pow(rayleigh / pathloss, 2);         // 크기값으로 저장
    }

    user->channelGain = ch_g;
}

void preambleSectorCollisionDetection(UserInfo* user, int activeCheck, int time, int* grantCheck) {
    int sector = 6;
    int* preambleTxCheck = (int*)calloc(sector, sizeof(int));


    int** preambleTxUEs = (int**)malloc(sector * sizeof(int*));
    for (int i = 0; i < sector; i++) {
        preambleTxUEs[i] = (int*)calloc(activeCheck, sizeof(int));
    }
    UserInfo* txUEs = (UserInfo*)malloc(nPreamble * sizeof(UserInfo));


    for (int i = 0; i < activeCheck; i++) {
        if (user[i].RA == 0 && user[i].txTime == time + 1 && user[i].msg2 == 0 && user[i].nowBackoff <= 0 && user[i].RaFailed == 0) {
            int s = (user + i)->sector;
            preambleTxUEs[s][preambleTxCheck[s]] = user[i].idx;
            preambleTxCheck[s]++;
        }
    }

    for (int s = 0; s < sector; s++) {
        if (preambleTxCheck[s] != 0) {
            int* preambles = (int*)calloc(nPreamble, sizeof(int));
            int** userCheck = (int**)malloc(nPreamble * sizeof(int*)); // 해당 프리앰블 전송하는 유저의 아이디
            for (int i = 0; i < nPreamble; i++) userCheck[i] = (int*)calloc(preambleTxCheck[s], sizeof(int));


            for (int i = 0; i < preambleTxCheck[s]; i++) {
                int RAPID = (user + (preambleTxUEs[s][i]))->preamble;

                userCheck[RAPID][preambles[RAPID]] = preambleTxUEs[s][i];
                preambles[RAPID]++;
            }

            // NOMA apply X
            // for (int i = 0; i < nPreamble; i++) {
            //    if (preambles[i] == 1) {
            //        grantCheck[s] = grantCheck[s] + 1;
            //        for (int j = 0; j < preambles[i]; j++) {
            //            if (grantCheck[s] <= nGrantUL) {
            //                (user + userCheck[i][j])->msg2 = 1;
            //            }
            //        }
            //    }
            // }

            // NOMA apply O
            
            // 일단 충돌 안 난 거 빼와 => preamble[0~nPreamble]==1
            int count = 0;
            for (int i = 0; i < nPreamble; i++) {
                if (preambles[i] == 1) {
                    UEequal((txUEs + count), (user + userCheck[i][0]));
                    count++;
                }
            }
            
            if (count > 0) {
                if (count <= nGrantUL) {
                    for (int i = 0; i < count; i++) {
                        if (grantCheck[s] < nGrantUL){   //자원부여
                            grantCheck[s] = grantCheck[s] + 1;  //grant개수제한 수정

                            (user + (txUEs + i)->idx)->msg2 = 1;
                        }
                    }
                }
                else {
                    sortUE(txUEs, count, txUEs);
                    int pair = 0, enNoma=0;
                    double low, high, p1 = 0.9, p2 = 0.1;
                    double txSignal, rxSignal[2], rxNoise[2];
                    int rx[2], SNR = 10;
                    // pairing
                    for (int i = 0; i < count-1; i++) {
                        for (int j = enNoma + 1; j < count; j++) {

                            rx[0] = (txUEs + i)->idx;
                            rx[1] = (txUEs + j)->idx;
                            low = (txUEs + i)->channelGain;
                            high = (txUEs + j)->channelGain;

                            if (rx[0] != -1 && rx[1] != -1 && 10 * log(high) - 10 * log(low)> 15.) {//******************************************************************

                            // if (rx[0] != -1 && rx[1] != -1 && high - low > 10.) {//******************************************************************
                                pair += 2;
                                (txUEs + i)->idx = -1;
                                (txUEs + j)->idx = -1;
                                if (grantCheck[s] < nGrantUL) {  //자원부여
                                    grantCheck[s] = grantCheck[s] + 1;  //grant개수제한 수정
                                    double p = (double)rand() / (double)RAND_MAX;
                                    if (p < 0.3) {
                                        int randomUE = rand() % 2;
                                        (user + rx[randomUE])->msg2 = 1;
                                    }
                                    else {
                                        (user + rx[0])->msg2 = 1;
                                        (user + rx[1])->msg2 = 1;     // UE 입장 msg2 받았다, BS grant 부여
                                    }

                                }
                                break;
                            }
                        }
                    }
                    if (count - pair > 0 ) {
                        for (int i = 0; i < count; i++) {
                            if ((txUEs + i)->idx != -1 && grantCheck[s] < nGrantUL) {

                                grantCheck[s] = grantCheck[s] + 1;
                                (user + (txUEs + i)->idx)->msg2 = 1;
                            }
                        }
                    }
                }
            }
            // ---------------------------------------------------------
            free(preambles);
            for (int i = 0; i < nPreamble; i++) {
                free(userCheck[i]);
            }

            free(userCheck);
        }
    }
    for (int i = 0; i < sector; i++) {
        free(preambleTxUEs[i]);
    }
    free(preambleTxUEs);
    free(preambleTxCheck);
}
void preambleCollisionDetection(UserInfo* user, int activeCheck, int time, int* grantCheck) {
    int preambleTxCheck = 0;
    int* preambleTxUEs = (int*)malloc(activeCheck * sizeof(int));

    // Store idx of UEs attempting to send MSG 1 at the current time
    for (int i = 0; i < activeCheck; i++) {
        if (user[i].RA == 0 && user[i].txTime == time + 1 && user[i].msg2 == 0 && user[i].nowBackoff <= 0 && user[i].RaFailed == 0) {
            if ((user + i)->active == 1 && (user + i)->msg2 == 0) {
                preambleTxUEs[preambleTxCheck] = (user + i)->idx;
                preambleTxCheck++;
            }
        }
    }

    UserInfo* txUEs = (UserInfo*)malloc(nPreamble * sizeof(UserInfo));

    if (preambleTxCheck > 0) {
        int* preambles = (int*)calloc(nPreamble, sizeof(int));
        int** userCheck = (int**)malloc(nPreamble * sizeof(int*));

        for (int i = 0; i < nPreamble; i++) {
            userCheck[i] = (int*)calloc(preambleTxCheck, sizeof(int));
        }

        for (int i = 0; i < preambleTxCheck; i++) {
            int RAPID = (user + (preambleTxUEs[i]))->preamble;

            userCheck[RAPID][preambles[RAPID]] = preambleTxUEs[i];
            preambles[RAPID]++;
        }

        // for (int i = 0; i < nPreamble; i++) {
        //     if (preambles[i] == 1) {
        //         *grantCheck = *grantCheck + 1;
        //         for (int j = 0; j < preambles[i]; j++) {
        //             if (*grantCheck <= nGrantUL) {
        //                 (user + userCheck[i][j])->msg2 = 1;
        //             }
        //         }
        //     }
        // }

        // ------------ NOMA ------------------------------
        int count = 0;
        for (int i = 0; i < nPreamble; i++) {
            if (preambles[i] == 1) {
                UEequal((txUEs + count), (user + userCheck[i][0]));
                count++;
            }
        }
        
        if (count > 0) {
            if (count <= nGrantUL) {
                for (int i = 0; i < count; i++) {
                    if (*grantCheck < nGrantUL)   //자원부여
                        *grantCheck = *grantCheck + 1;  //grant개수제한 수정

                    (user + (txUEs + i)->idx)->msg2 = 1;
                }
            }
            else {
                sortUE(txUEs, count, txUEs);

                int pair = 0, enNoma=0;
                double low, high, p1 = 0.9, p2 = 0.1;
                double txSignal, rxSignal[2], rxNoise[2];
                int rx[2], SNR = 10;
                // pairing
                for (int i = 0; i < count-1; i++) {
                    for (int j = enNoma + 1; j < count; j++) {

                        //printf("%d %d %d\n", count, i, count-1-i);
                        rx[0] = (txUEs + i)->idx;
                        rx[1] = (txUEs + j)->idx;
                        low = (txUEs + i)->channelGain;
                        high = (txUEs + j)->channelGain;

                        //printf("%.8f\n", 10 * log(high) - 10 * log(low));
                        if (rx[0] != -1 && rx[1] != -1 && 10 * log(high) - 10 * log(low)> 15.) {//******************************************************************

                        // if (rx[0] != -1 && rx[1] != -1 && high - low > 10.) {//******************************************************************
                            pair += 2;
                            (txUEs + i)->idx = -1;
                            (txUEs + j)->idx = -1;
                            if (*grantCheck < nGrantUL) {  //자원부여
                                *grantCheck = *grantCheck + 1;  //grant개수제한 수정
                                double p = (double)rand() / (double)RAND_MAX;
                                //printf("%f\n", p);
                                if (p < 0.3) {
                                    (user + rx[0])->msg2 = 1;
                                }
                                else {
                                    (user + rx[0])->msg2 = 1;
                                (user + rx[1])->msg2 = 1;     // UE 입장 msg2 받았다, BS grant 부여
                                }

                            }
                            //printIdx(txUEs, count);
                            break;
                        }
                    }
                }
                if (count - pair > 0 ) {
                        for (int i = 0; i < count; i++) {
                            if ((txUEs + i)->idx != -1 && *grantCheck < nGrantUL) {

                                *grantCheck = *grantCheck + 1;
                            (user + (txUEs + i)->idx)->msg2 = 1;
                        }
                    }
                }
            }
        }

        free(preambles);
        for (int i = 0; i < nPreamble; i++) {
            free(userCheck[i]);
        }
        free(userCheck);
    }
    free(txUEs);
    free(preambleTxUEs);
}

void msg2Results(UserInfo* user, int time) {
    int txLimit = 0;

    if (user->msg2 == 0 && user->active == 1) {
        user->rarWindow = 5;
        user->txTime += 3;
        if (user->rarWindow >= maxRarWindow) {
            user->nTxPreamble++;
            user->rarWindow = 0;
            user->msg1ReTx++;

            int tmp = (rand() % backoffIndicator);

            int subTime = user->txTime + tmp;

            if (subTime % accessTime == 0) {
                // 일정 시간 다음에 전송 시도
                user->txTime = subTime + 1;
            }
            else if (subTime % accessTime == 1) {
                // 일정 시간 다음에 전송 시도
                user->txTime = subTime;
            }
            else {
                // 일정 시간 다음에 전송 시도
                user->txTime = subTime + (accessTime - (subTime % accessTime) + 1);
            }

            // backoff counter 설정
            user->nowBackoff = user->txTime - time - 1;
            user->secondTxTime = user->txTime;

            if (user->msg1ReTx >= maxMsg1ReTx) {
                user->preamble = rand() % nPreamble;
                user->RaFailed++;
                user->nTxPreamble = 0;
                user->rarWindow = 0;
                user->msg1ReTx = 0;
                user->timer = 0;
            }
        }
    }
    else if (user->msg2 == 1) {
        user->active = 2;
        // MSG 1에 대한 MSG 2수신 성공
        user->txTime += 10;
        user->secondTxTime = user->txTime;
        user->msg3Wait = 0;
    }
}
void resourceRequestAllocation(UserInfo* user, int activeCheck, int time) {
    for (int i = 0; i < activeCheck; i++) {
        if ((user + i)->txTime == time && (user + i)->msg2 == 1 && (user + i)->active == 2 && (user + i)->RaFailed == 0) {
            if ((user + i)->msg3Wait <= 48) {
                float p = (float)rand() / (float)RAND_MAX;
                if (p > 0.1) {
                    (user + i)->active = 0;
                    (user + i)->RA = 1;
                    (user + i)->timer = (user + i)->timer + 6;
                }
                else {
                    (user + i)->txTime += 49;
                    (user + i)->msg3Wait = 49;
                }
            }
            else {
                (user + i)->RA = 0;
                (user + i)->msg3Faile++;
                (user + i)->active = 1;
                (user + i)->msg2 = 0;
                (user + i)->preamble = rand() % nPreamble;
                int tmp = (rand() % backoffIndicator);

                int subTime = (user + i)->txTime + tmp;

                if (subTime % accessTime == 0) {
                    // 일정 시간 다음에 전송 시도
                    (user + i)->txTime = subTime + 1;
                }
                else if (subTime % accessTime == 1) {
                    // 일정 시간 다음에 전송 시도
                    (user + i)->txTime = subTime;
                }
                else {
                    // 일정 시간 다음에 전송 시도
                    (user + i)->txTime = subTime + (accessTime - (subTime % accessTime) + 1);
                }
                (user + i)->secondTxTime = (user + i)->txTime;
                // backoff counter 설정
                (user + i)->nowBackoff = (user + i)->txTime - time - 1;
                (user + i)->rarWindow = 0;
                (user + i)->nTxPreamble = 0;
                (user + i)->msg1ReTx = 0;
                (user + i)->timer = 0;
            }
        }
    }
}
void timerIncrease(UserInfo* user) {
    user->timer++;

    if (user->nowBackoff > 0) {
        user->nowBackoff--;
    }
}
int successUEs(UserInfo* user, int nUE) {
    int success = 0;
    for (int i = 0; i < nUE; i++) {
        if ((user + i)->RA == 1) {
            success++;
        }
    }
    return success;
}
float betaDist(float a, float b, float x) {
    float betaValue = (1 / betaF) * (pow(x, (a - 1))) * (pow((1 - x), (b - 1)));
    return betaValue;
}
void saveResultLogs(UserInfo* user, int nUE) {
    FILE* fp_l;
    char logBuff[1000];
    char fileNameResultLog[500];

    sprintf(fileNameResultLog, "./TestResults/%d_logs.txt", nUE);

    fp_l = fopen(fileNameResultLog, "w+");

    for (int i = 0; i < nUE; i++) {
        sprintf(logBuff, "Idx: %d| FirstTxTime: %d| SecondTxTime: %d| Timer: %d| Active: %d| Preamble: %d| Sector: %d| RarWindow: %d| Msg1ReTx: %d| MSG2: %d| MSG3 Falied: %d| TxTime: %d| Backoff: %d| RAFailed: %d| RA: %d\n",
            (user + i)->idx,
            (user + i)->firstTxTime,
            (user + i)->secondTxTime,
            (user + i)->timer,
            (user + i)->active,
            (user + i)->preamble,
            (user + i)->sector,
            (user + i)->rarWindow,
            (user + i)->msg1ReTx,
            (user + i)->msg2,
            (user + i)->msg3Faile,
            (user + i)->txTime,
            (user + i)->nowBackoff,
            (user + i)->RaFailed,
            (user + i)->RA);
        fputs(logBuff, fp_l);
    }

    fclose(fp_l);
}
void saveResult(UserInfo* user, int nUE, int nSuccessUE) {
    FILE* fp_l;
    char logBuff[1000];
    char fileNameResultLog[500];

    sprintf(fileNameResultLog, "./TestResults/Sector_%d_Result.txt", nUE);

    fp_l = fopen(fileNameResultLog, "aw+");
    printf("%d ", nUE);
    sprintf(logBuff, "%d ", nUE);
    fputs(logBuff, fp_l);

    printf("%d ", nSuccessUE);
    sprintf(logBuff, "%d ", nSuccessUE);
    fputs(logBuff, fp_l);

    printf("%lf ", ((float)nSuccessUE / (float)nUE) * 100.0);
    sprintf(logBuff, "%lf ", ((float)nSuccessUE / (float)nUE) * 100.0);
    fputs(logBuff, fp_l);

    int delay = 0;
    int nTxP = 0;
    for (int i = 0; i < nUE; i++) {
        if ((user + i)->RA == 1) {
            delay += (user + i)->timer;
            nTxP += (user + i)->nTxPreamble;
        }
    }
    printf("%lf ", ((float)nTxP / (float)nSuccessUE));
    sprintf(logBuff, "%lf ", ((float)nTxP / (float)nSuccessUE));
    fputs(logBuff, fp_l);

    printf("%lf\n", ((float)delay / (float)nSuccessUE));
    sprintf(logBuff, "%lf\n", ((float)delay / (float)nSuccessUE));
    fputs(logBuff, fp_l);

    fclose(fp_l);
}

int main(int argc, char** argv) {
    SysBandwidth = 4.32 * pow(10.0, 6.0);
    TxPower_dB = 43.0;
    NoiseDensity_dB = -169.0;
    TxPower = pow(10, (TxPower_dB / 10.0));
    NoisePower = SysBandwidth * pow(10.0, (0.1 * NoiseDensity_dB));

    for (int seed = 0; seed < 10; seed++) {

        srand(seed);

        for (int nUE = 10000; nUE <= 100000; nUE += 10000) {

            UserInfo* UEs;
            UEs = (UserInfo*)calloc(nUE, sizeof(UserInfo));

            for (int i = 0; i < nUE; i++) {
                initUserInfo(UEs + i, i, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.);
            }

            int activeCheck = 0;
            int grantCheck = 0;
            int sectorGrants[6];

            int time;
            int maxTime = 10000;
            int nSuccessUE;

            for (time = 0; time < maxTime; time++) {
                nSuccessUE = 0;

                if (time % accessTime == 0) {
                    grantCheck = 0;
                    for (int s = 0; s < 6; s++) {
                        sectorGrants[s] = 0;
                    }

                    // printf("%d\n", time);
                    float numBetaDist = betaDist(3, 4, (float)time / (float)maxTime);
                    int accessUEs = (int)ceil((float)nUE * numBetaDist / ((float)maxTime / (float)accessTime));
                    activeCheck += accessUEs;

                    if (activeCheck >= nUE) {
                        activeCheck = nUE;
                    }
                    for (int i = 0; i < activeCheck; i++) {
                        if ((UEs + i)->RA == 0 && (UEs + i)->active == 0 && (UEs + i)->RaFailed == 0) {
                            activeUE(UEs + i, time);
                        }
                    }
                    // nGrantUL = 12;
                    // preambleCollisionDetection(UEs, activeCheck, time, &grantCheck);
                    preambleSectorCollisionDetection(UEs, activeCheck, time, sectorGrants);

                    for (int i = 0; i < activeCheck; i++) {
                        if ((UEs + i)->nowBackoff <= 0 && (UEs + i)->txTime == time + 1 && (UEs + i)->active == 1 && (UEs + i)->RA == 0 && (UEs + i)->RaFailed == 0) {
                            // printf("Here? ");
                            msg2Results(UEs + i, time + 1);
                        }
                    }
                }

                resourceRequestAllocation(UEs, activeCheck, time);

                // Increase timers of all active UEs by 1
                for (int i = 0; i < activeCheck; i++) {
                    if ((UEs + i)->active > 0 && (UEs + i)->RA == 0 && (UEs + i)->RaFailed == 0) {
                        timerIncrease(UEs + i);
                    }
                }
                nSuccessUE = successUEs(UEs, nUE);
                if (nSuccessUE == nUE) {
                    break;
                }
            }
            saveResult(UEs, nUE, nSuccessUE);

            free(UEs);
        }
        printf("Done\n");
    }
    return 0;
}