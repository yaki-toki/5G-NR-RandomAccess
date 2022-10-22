#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

struct UE{
    int idx;
    int activate;
    int preamble;
    int rarCounter;
    int msg1Success;
    int resolutionCounter;
    int msg3Success;
    int raSuccess;
    int backoff;
    int timer;
    int sendTimer;
};

struct gNB{
    int preamble;
    int idxUE;
    int pCounter;
};

double uniform();
void initializationUE(struct UE *user, int nUE);
void initializationNB(struct gNB *gNB, int nPreamble);
void activateUE(struct UE *user, int nUE, float pTx);
void selectPreambleNumber(struct UE *user, int nPreamble, int nUE);
void collisionPreamble(struct UE *user, int nUE, struct gNB *gNB, int nPreamble);
void Msg1Success(struct UE *user, int nUE, struct gNB *gNB, int nPreamble);
void SendMsg3(struct UE *user, int nUE, struct gNB *gNB);
void timerUE(struct UE *user, int nUE);
int countingSuccessUE(struct UE *user, int nUE);
void printUserInfo(struct UE *user);
void printNbInfo(struct gNB *nb);

int main(int argc, char** argv){

    int nUE = 100;
    int nPreamble = 64;
    struct UE *UE;
    UE = (struct UE *) calloc(nUE, sizeof(struct UE));
    initializationUE(UE, nUE);

    struct gNB *gNB;
    gNB = (struct gNB *)calloc(nPreamble, sizeof(struct gNB));
    initializationNB(gNB, nPreamble);

    int maxTimer = 5000; // 60s -> 60000ms
    int timer;

    for(timer = 0; timer < maxTimer; timer++){
        float pTx = uniform();
        activateUE(UE, nUE, pTx);
        selectPreambleNumber(UE, nPreamble, nUE);
        collisionPreamble(UE, nUE, gNB, nPreamble);
        Msg1Success(UE, nUE, gNB, nPreamble);
        SendMsg3(UE, nUE, gNB);
        timerUE(UE, nUE);
        if(timer == 9999){
            break;
        }
    }

    printf("%d\n", countingSuccessUE(UE, nUE));

    return 0;
}

void printUserInfo(struct UE *user){
    printf("User idx: %d | Activate: %d | Preamble: %d | RarCounter: %d | Msg1Success: %d\n", user->idx, user->activate, user->preamble, user->rarCounter, user->msg1Success);
}

void printNbInfo(struct gNB *nb){
    printf("User's idx: %d| User's preamble: %d\n", nb->idxUE, nb->preamble);
}

void initializationUE(struct UE *user, int nUE){
    for(int i = 0; i < nUE; i++){
        (user+i)->idx = i;
        (user+i)->activate = 0;
        (user+i)->preamble = -1;
        (user+i)->rarCounter = 0;
        (user+i)->msg1Success = 0;
        (user+i)->msg3Success = 0;
        (user+i)->backoff = 0;
        (user+i)->timer = 0;
        (user+i)->sendTimer = 0;
    }
}

void initializationNB(struct gNB *gNB, int nPreamble){
    for(int p = 0; p < nPreamble; p++){
        (gNB+p)->preamble = -1;
        (gNB+p)->idxUE = NULL;
        (gNB+p)->pCounter = 0;
    }
}

void activateUE(struct UE *user, int nUE, float pTx){
    for(int i = 0; i < nUE; i++){
        if((user+i)->activate == 0){
            float temp = uniform();
            if(temp <= pTx){
                (user+i)->activate = 1;
            }
        }
    }
}

void timerUE(struct UE *user, int nUE){
    for(int i = 0; i < nUE; i++){
        if((user+i)->activate == 1){
            (user+i)->timer += 1;
            (user+i)->sendTimer += 1;
        }
    }
}

void selectPreambleNumber(struct UE *user, int nPreamble, int nUE){
    for(int i = 0; i < nUE; i++){
        if((user+i)->activate == 1 && (user+i)->rarCounter == 0 && (user+i)->raSuccess == 0 && (user+i)->backoff == 0 && (user+i)->msg1Success == 0){
            // Select New Preamble Number
            (user+i)->preamble = rand()%nPreamble;
        }
        else if((user+i)->activate == -1 && (user+i)->backoff != 0){
            // if Now Backoff
            (user+i)->backoff -= 1;
            (user+i)->timer += 1;
        }else if((user+i)->activate == 1 && (user+i)->rarCounter != 0){
            // if Now not received msg 2
            if((user+i)->rarCounter == 10){
                (user+i)->backoff = 20;
                (user+i)->rarCounter = 0;
                (user+i)->activate = -1;
            }
        }else if((user+i)->activate == -1 && (user+i)->raSuccess == 0 && (user+i)->backoff == 0){
            (user+i)->activate = 1;
        }
    }
}

void collisionPreamble(struct UE *user, int nUE, struct gNB *gNB, int nPreamble){

    int *check = (int *)malloc(sizeof(int) * nPreamble);
    for(int i = 0; i < nPreamble; i++){
        check[i] = 0;
    }

    int preamble;
    for(int i = 0; i < nUE; i++){
        if((user+i)->activate == 1 && (user+i)->sendTimer == 2){
            preamble = (user+i)->preamble;
            check[preamble]++;
        }
    }
    
    for(int p = 0; p < nPreamble; p++){
        if(check[p] >= 2 || (gNB+p)->pCounter == 1){
            for(int i = 0; i < nUE; i++){
                if((user+i)->preamble == p){
                    (user+i)->rarCounter += 1;
                }
            }
        }else{
            for(int i = 0; i < nUE; i++){
                if((user+i)->preamble == p){
                    (gNB+p)->idxUE = (user+i)->idx;
                    (gNB+p)->preamble = p;
                    (gNB+p)->pCounter = 1;
                }
            }
        }
    }

    free(check);
}

void Msg1Success(struct UE *user, int nUE, struct gNB *gNB, int nPreamble){
    for(int i = 0; i < nUE; i++){
        if((user+i)->activate == 1){
            for(int p = 0; p < nPreamble; p++){
                if((user+i)->idx == (gNB+p)->idxUE){
                    (user+i)->msg1Success = 1;
                }
            }
        }
    }
}

void SendMsg3(struct UE *user, int nUE, struct gNB *gNB){
    for(int i = 0; i < nUE; i++){
        if((user+i)->activate && (user+i)->msg1Success == 1){
            (user+i)->resolutionCounter += 1;
            float tmp = uniform();
            if(tmp >= 0.1){
                (user+i)->msg3Success = 1;
                (user+i)->raSuccess = 1;
                (user+i)->activate = -1;
                (gNB+((user+i)->preamble))->idxUE = NULL;
                (gNB+((user+i)->preamble))->preamble = -1;
                (gNB+((user+i)->preamble))->pCounter = 0;
            }
        }
    }
}

int countingSuccessUE(struct UE *user, int nUE){
    int total = 0;
    for(int i = 0; i < nUE; i++){
        if((user+i)->raSuccess == 1){
            total += 1;
        }
    }
    return total;
}

double uniform(){
    return (double)rand() / (double)RAND_MAX ;
}