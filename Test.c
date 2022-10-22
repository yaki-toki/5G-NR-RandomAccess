#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

struct UE{
    int idx;
    int state; // 0-Non active, 1-phase 1(msg 1->msg 2), 2-phase 2(msg 3->msg 4)
    int timer;
    int preamble; // preamble number
    int firstTimer;
    int msg1TimeSlot;  // MSG 1 send timing
    int maxPreambleTx; // 메시지 1 재전송 횟수 10번
    int backoff;       // 20 ms
    int msg1Flag;      // Success msg 1
};

void initialUE(struct UE *user, int nUE, int maxTime);
void selectPreamble(struct UE *user, int nUE, int nPreamble, int nSlot);
void detectCollision(struct UE *user, int nUE, int nPreamble, int nSlot);
void activeUETimer(struct UE *user, int nUE);
void printUserInfor(struct UE *user, int nUE);
float averagePhase1Delay(struct UE *user, int nUE);
int successUE(struct UE *user, int nUE);

int main(int argc, char **argv)
{
    srand(2022);
    int nUE = 10000;
    int timer;
    int maxTime = 60000;
    int nPreamble = 64;

    struct UE *user;
    user = (struct UE *)calloc(nUE, sizeof(struct UE));
    initialUE(user, nUE, maxTime);
    int *nowUE;
    for (timer = 0; timer < maxTime; timer++)
    {
        selectPreamble(user, nUE, nPreamble, timer);
        detectCollision(user, nUE, nPreamble, timer);
        activeUETimer(user, nUE);
        if (successUE(user, nUE) == nUE)
        {
            break;
        }
    }
    printf("Timer: %d\n", timer);
    // printUserInfor(user, nUE);
    printf("Success UE: %d\n", successUE(user, nUE));
    printf("Total Phase 1 delay: %lf\n", averagePhase1Delay(user, nUE));
}

void printUserInfor(struct UE *user, int nUE)
{
    for (int i = 0; i < nUE; i++)
    {
        printf("User idx: %d, Preamble %d, Timer: %d, Msg 1 TimeSlot: %d, Msg 1 Max Preamble: %d, Msg 1 Flag: %d\n", (user + i)->idx, (user + i)->preamble, (user + i)->timer, (user + i)->firstTimer, (user + i)->maxPreambleTx, (user + i)->msg1Flag);
    }
}

void initialUE(struct UE *user, int nUE, int maxTime)
{
    for (int i = 0; i < nUE; i++)
    {
        (user + i)->idx = i;
        (user + i)->state = 0;
        (user + i)->timer = 0;
        (user + i)->preamble = -1;
        int tmp = rand() % maxTime;
        (user + i)->firstTimer = tmp;
        (user + i)->msg1TimeSlot = tmp;
        (user + i)->maxPreambleTx = 0;
        (user + i)->backoff = 0;
        (user + i)->msg1Flag = 0;
    }
}

void selectPreamble(struct UE *user, int nUE, int nPreamble, int nSlot)
{
    for (int i = 0; i < nUE; i++)
    {
        if ((user + i)->msg1TimeSlot == nSlot)
        {
            // 처음 보내는 경우 preamble은 -1
            if ((user + i)->preamble == -1 && (user + i)->state == 0 && (user + i)->msg1Flag == 0){
                (user + i)->state = 1;
                (user + i)->preamble = rand() % nPreamble;
            }
            // 그 외의 경우 즉, 한 번 전송을 시도했던 경우
            else{
                if ((user + i)->maxPreambleTx % 10 == 9)
                {
                    (user + i)->preamble = rand() % nPreamble;
                }
            }
        }
    }
}

void detectCollision(struct UE *user, int nUE, int nPreamble, int nSlot)
{
    int *check = (int *)malloc(sizeof(int) * nPreamble);
    for (int i = 0; i < nPreamble; i++){
        check[i] = 0;
    }

    int idx;

    for (int i = 0; i < nUE; i++){
        if ((user + i)->msg1TimeSlot + 2 == nSlot && (user + i)->state == 1 && (user + i)->msg1Flag == 0){

            idx = (user + i)->preamble;
            check[idx]++;
        }
    }

    for (int i = 0; i < nUE; i++)
    {
        for (int p = 0; p < nPreamble; p++)
        {
            if ((user + i)->msg1TimeSlot + 2 == nSlot && (user + i)->state == 1)
            {
                if (check[p] >= 2 && (user + i)->preamble == p)
                {
                    (user + i)->msg1TimeSlot += 5;
                    (user + i)->maxPreambleTx += 1;
                }
                else if (check[p] == 1 && (user + i)->preamble == p)
                {
                    (user + i)->msg1Flag = 1;
                    // 임시 파라미터 설정
                    (user + i)->state = 0;
                    (user + i)->timer += 4;
                }
            }
        }
    }
    free(check);
}

float averagePhase1Delay(struct UE *user, int nUE)
{
    float delay = 0;
    for (int i = 0; i < nUE; i++)
    {
        delay += (float)(user + i)->timer;
    }
    return delay / (float)nUE;
}

void activeUETimer(struct UE *user, int nUE)
{
    for (int i = 0; i < nUE; i++)
    {
        if ((user + i)->state != 0)
        {
            (user + i)->timer += 1;
        }
    }
}

int successUE(struct UE *user, int nUE)
{
    int ues = 0;
    for (int i = 0; i < nUE; i++)
    {
        if ((user + i)->msg1Flag == 1)
        {
            ues += 1;
        }
    }

    return ues;
}