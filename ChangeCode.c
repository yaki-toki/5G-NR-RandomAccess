#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <complex.h>

#define betaF 0.0165

struct UEinfo
{
    int idx;               // UE id
    int timer;             // Timer per UE (���� RA �������� �ɸ��� ������)
    int active;            // Now tx status (Not active: 0, MSG 1-2: 1, MSG 3-4: 2)
    int txTime;            // Each MSG tx time
    int preamble;          // Select preamble
    int* binaryPreamble;
    int rarWindow;         // RAR window (now set up is 5)
    int maxRarCounter;     // Max RAR retransmission count (now set up is 10)
    int preambleTxCounter; // Preamble retransmission count
    int msg2Flag;          // MSG 1-2 Success flag
    int connectionRequest; // MSG 3 waiting time (now set up is 48)
    int msg4Flag;          // MSG 3-4 Success flag (Finally RA success)
    int preambleChange;    // Number of changed preamber number (Not use)
    int raFailed;          // MSG 2 max count �Ǹ� �׳� ����
    int nowBackoff;        // Backoff counter
    int firstTxTime;
    int secondTxTime;
    int failCount;
    
    // using for NOMA
    int rampingPower;       // Power ���� level
    float xCoordinate;      // UE�� x ��ǥ
    float yCoordinate;      // UE�� y ��ǥ
    int sector;
    int powerMode;           // 0: weak, 1: strong
    float angle;            // BS -- UE�� ����
    float channel;
    float distance;         // BS -- UE�� �Ÿ�
    float pathLoss;         // BS -- UE�� pathloss
};

void initialUE(struct UEinfo *user, int id);
void activateUEs(struct UEinfo *user, int time, float cellRadius, float hBS, float hUT);
int* Dec2Bin(int decimal, int Length);
void itialUeInfo(struct UEinfo *user, int nPreamble);
void selectPreamble(struct UEinfo *user, int nPreamble, int time, int backoff, int accessTime, int maxRarWindow, int maxMsg2TxCount, int *continueFaliedUEs);
void preambleDetection(struct UEinfo *user, int *preambleTxUEs, int preambleTxCheck, int nPreamble, int time, int *grantCheck, int nGrantUL, int nUE);
void preambleCollision(struct UEinfo *user, struct UEinfo *UEs, int nUE, int checkPreamble, int nPreamble, int time, int backoff, int maxRarWindow, int *grantCheck, int nGrantUL);
void requestResourceAllocation(struct UEinfo *user, int time, int backoff, int nPreamble, int *finalSuccessUEs, int *continueFaliedUEs);
void timerIncrease(struct UEinfo *user);
int successUEs(struct UEinfo *user, int nUE);

void saveSimulationLog(int seed, int time, int nUE, int nSuccessUE, int failedUEs, int preambleTxCount, 
                       float totalDelay, int distribution, int nPreamble, double layency, int *continueFaliedUEs, int *finalSuccessUEs);
void saveResult(int seed, int nUE, struct UEinfo *UE, int distribution, int nPreamble);
void pointResults(struct UEinfo *UE, int nUE, int distribution);
float beta_dist(float a, float b, float x);

int collisionPreambles = 0;
int totalPreambleTxop = 0;

int main(int argc, char *argv[]){
    mkdir("NomaBetaResults", 0755);
    mkdir("NomaUniformResults", 0755);
    int randomSeed;
    int randomMax = 1;
    int nPreamble = 54;        // Number of preambles
    int backoffIndicator = 20; // Number of backoff indicator
    int nGrantUL = 12;         // Number of UL Grant
    int grantCheck;            // �� time���� �ο��� UL grant�� ���� Ȯ���ϴ� ����

    int maxRarWindow = 5;
    int maxMsg2TxCount = 9;
    int accessTime = 5; // 1, 6ms���� ����

    float cellRadius = 400;  // 400m
    float hBS = 10.0;       // height of BS from ground
    float hUT = 1.8;        // height of UE from ground

    int continueFaliedUEs;
    int finalSuccessUEs;

    // distribution: 1(Uniform), 2(Beta)
    int distribution = 2;

    if (argc > 1){
        int keywords = 0;
        for(int i = 1; i < argc; i+=2){
            if(strcmp(argv[i], "--times") == 0 || strcmp(argv[i], "-t") == 0){
                if(atoi(argv[i+1]) < 1){
                    printf("Simulation count must be greater than zero.");
                    exit(-1);
                }
                randomMax = atoi(argv[i+1]);
            }else if(strcmp(argv[i], "--distribution") == 0 || strcmp(argv[i], "-d") == 0){
                if(atoi(argv[i+1]) != 1 && atoi(argv[i+1]) != 0){
                    printf("Traffic model just choose 1 or 2");
                    exit(-1);
                }
                distribution = atoi(argv[i+1]);
            }else if(strcmp(argv[i], "--preambles") == 0 || strcmp(argv[i], "-p") == 0){
                if(atoi(argv[i+1]) < 1){
                    printf("Number of preamble must be greater than zero.");
                    exit(-1);
                }
                nPreamble = atoi(argv[i+1]);
            }else if(strcmp(argv[i], "--backoff") == 0 || strcmp(argv[i], "-b") == 0){
                if(atoi(argv[i+1]) < 1){
                    printf("Backoff indicator must be greater than zero.");
                    exit(-1);
                }
                backoffIndicator = atoi(argv[i+1]);
            }else if(strcmp(argv[i], "--grant") == 0 || strcmp(argv[i], "-g") == 0){
                if(atoi(argv[i+1]) < 1){
                    printf("The number of Up Link Grant per RAR must be greater than zero.");
                    exit(-1);
                }
                nGrantUL = atoi(argv[i+1]);
            }else if(strcmp(argv[i], "--rarCount") == 0 || strcmp(argv[i], "-rc") == 0){
                if(atoi(argv[i+1]) < 1){
                    printf("The maximum RAR window size must be greater than zero.");
                    exit(-1);
                }
                maxRarWindow = atoi(argv[i+1]) + 1;
            }else if(strcmp(argv[i], "--maxRar") == 0 || strcmp(argv[i], "-mrc") == 0){
                if(atoi(argv[i+1]) < 1){
                    printf("Maximum retransmissions must be greater than zero.");
                    exit(-1);
                }
                maxMsg2TxCount = atoi(argv[i+1]) - 1;
            }else if(strcmp(argv[i], "--subframe") == 0 || strcmp(argv[i], "-s") == 0){
                if(atoi(argv[i+1]) < 5){
                    printf("The size of the subframe must be at least 5.");
                    exit(-1);
                }
                accessTime = atoi(argv[i+1]);
            }else if(strcmp(argv[i], "--cell") == 0 || strcmp(argv[i], "-c") == 0){
                if(atof(argv[i+1]) < 400.0){
                    printf("The radius of the cell is entered in diameter units and must be greater than 400m.");
                    exit(-1);
                }
                cellRadius = atof(argv[i+1]);
            }else if(strcmp(argv[i], "--hbs") == 0 || strcmp(argv[i], "-bs") == 0){
                if(atof(argv[i+1]) < 10.0 || atof(argv[i+1]) > 20.0){
                    printf("The height of the BS must be between 10m and 20m.");
                    exit(-1);
                }
                hBS = atof(argv[i+1]);
            }else if(strcmp(argv[i], "--hut") == 0 || strcmp(argv[i], "-ut") == 0){
                if(atof(argv[i+1]) < 1.5 || atof(argv[i+1]) > 22.5){
                    printf("The height of the UE must be between 1.5m and 22.5m.");
                    exit(-1);
                }
                hUT = atof(argv[i+1]);
            }else{
                printf("--times         -t : Simulation times (int)\n");
                printf("                     Simulation count must be greater than zero.\n");
                printf("                     Default 1\n\n");

                printf("--distribution  -d : Traffic model (1 or 2)\n");
                printf("                     1: traffic model 1 (Uniform distribution)\n");
                printf("                     2: traffic model 2 (Beta distribution)\n\n");

                printf("--preambles     -p : Number of preambles (int)\n");
                printf("                     Number of preamble must be greater than zero.\n");
                printf("                     Default 54\n\n");
                
                printf("--backoff       -b : Backoff indicator (int)\n");
                printf("                     Backoff indicator must be greater than zero.\n");
                printf("                     Default 20\n\n");

                printf("--grant         -g : The number of Up Link Grant per RAR (int)\n");
                printf("                     The number of Up Link Grant per RAR must be greater than zero.\n");
                printf("                     Default 12\n\n");

                printf("--rarCount      -r : RAR window size (int)\n");
                printf("                     The maximum RAR window size must be greater than zero.\n");
                printf("                     Default 5\n\n");

                printf("--maxRar        -m : Maximum retransmission (int)\n");
                printf("                     Maximum retransmissions must be greater than zero.\n");
                printf("                     Default 10\n\n");

                printf("--subframe      -s : Subframe units (int)\n");
                printf("                     The size of the subframe must be at least 5. (float)\n");
                printf("                     Default 5\n\n");

                printf("--cell          -c : Cell radius Size\n");
                printf("                     The radius of the cell is entered in diameter units and must be greater than 400m.\n");
                printf("                     Default 400.0\n\n");

                printf("--hbs           -b : Height of BS from ground (float)\n");
                printf("                     The height of the BS must be between 10m and 20m.\n");
                printf("                     Default 10.0\n\n");
                
                printf("--hut           -u : Height of UE from ground (float)\n");
                printf("                     The height of the UE must be between 1.5m and 22.5m.\n");
                printf("                     Default 1.8\n\n");
                exit(-1);
            }
        }
    }

    if (distribution == 1){
        printf("Traffic model: Uniform\n\n");
    }
    else{
        printf("Traffic model: Beta\n\n");
    }


    for (randomSeed = 1; randomSeed < 2; randomSeed++){
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

            if (distribution == 1){
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
            int sectorGrants[6];
            grantCheck = 0;
            
            finalSuccessUEs = 0;
            continueFaliedUEs = 0;
            
            int preambleTxCheck;
            int msg3TxCheck;

            for (time = 0; time < maxTime; time++){

                preambleTxCheck = 0;
                msg3TxCheck = 0;

                int *preambleTxUEs = (int *)malloc(sizeof(int) * nUE);
                int *msg3UEs = (int *)malloc(sizeof(int) * nUE);
                if (time % 5 == 0){
                    grantCheck = 0;
                    // printf("%d\n", time);
                    for(int s = 0; s < 6; s++){
                        sectorGrants[s] = 0;
                    }
                }

                if (activeCheck >= nUE){
                    activeCheck = nUE;
                }
                // 5 ms���� UE ����
                if (time % accessTime == 0 && activeCheck != nUE){
                    if (distribution == 1){
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
                            activateUEs((UE + i), time, cellRadius, hBS, hUT);
                        }
                    }
                }
                // Time t���������� UE���� ���¸� Ȯ��
                for (int i = 0; i < activeCheck; i++){
                    if (NULL != UE){
                        // Random access�� �����ϰų� ������ UE���� ���� ��Ŵ
                        if ((UE + i)->msg4Flag == 0 && (UE + i)->msg2Flag == 0 && (UE + i)->active > 0 && (UE + i)->txTime == time){
                            selectPreamble(UE + i, nPreamble, time, backoffIndicator, accessTime, maxRarWindow, maxMsg2TxCount, &continueFaliedUEs);
                        }
                    }
                }

                for(int i = 0; i < activeCheck; i++){
                    if(NULL != UE){
                        if ((UE + i)->active == 2 && (UE + i)->txTime == time){
                            preambleTxUEs[preambleTxCheck] = (UE+i)->idx;
                            preambleTxCheck += 1;
                        }
                    }
                }

                if(preambleTxCheck != 0){
                    // printf("Preamble collision Detection\n");
                    preambleDetection(UE, preambleTxUEs, preambleTxCheck, nPreamble, time, &grantCheck, nGrantUL, nUE);
                }

                // MSG 3 -> MSG 4: Resource allocation (contention resolution)
                for(int i = 0; i < activeCheck; i++){
                    if(UE != NULL){
                        if((UE + i)->msg4Flag == 0 && (UE + i)->active > 0){
                            if ((UE + i)->active == 3 && (UE + i)->txTime == time){
                                // ������ UE�� MSG 1->2�� ������ ���
                                requestResourceAllocation(UE + i, time, backoffIndicator, nPreamble, &finalSuccessUEs, &continueFaliedUEs);
                            }
                            // ������ �õ��ϴ� UE���� ���ð� ����
                            timerIncrease(UE + i);
                        }
                    }
                }

                nSuccessUE = successUEs(UE, nUE);

                if (nSuccessUE == nUE){
                    break;
                }
                
                free(preambleTxUEs);
                free(msg3UEs);
            }

            int failedUEs = nUE - nSuccessUE; // ������ UE
            float totalDelay = 0;             // ������ ��� UE�� delay ��
            int preambleTxCount = 0;          // ������ ��� UE�� preamble tx Ƚ��
            int failCounts = 0;

            for (int i = 0; i < nUE; i++){
                if (NULL != UE){
                    // RA�� ������ UE�鸸 Ȯ��
                    if ((UE + i)->msg4Flag == 1){
                        totalDelay += (float)(UE + i)->timer;
                        preambleTxCount += (UE + i)->preambleTxCounter;
                        failCounts += (UE + i)->failCount;
                    }
                }
            }

            // ��� ���
            printf("-------- %05d Result ---------\n", activeCheck);
            if (distribution == 1){
                printf("Number of RA try UEs per Subframe: %d\n", nAccessUE);
            }

            clock_t end = clock();
            printf("Latency: %lf\n", (double)(end - start) / CLOCKS_PER_SEC);
            printf("Fail Counts: %d\n", failCounts);

            saveSimulationLog(randomSeed, time, nUE, nSuccessUE, failedUEs, preambleTxCount, 
                              totalDelay, distribution, nPreamble, 
                              (double)(end - start) / CLOCKS_PER_SEC, &continueFaliedUEs, &finalSuccessUEs);
            saveResult(randomSeed, nUE, UE, distribution, nPreamble);
            pointResults(UE, nUE, distribution);
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

void activateUEs(struct UEinfo *user, int time, float cellRadius, float hBS, float hUT){
    // MSG 1 ������ �õ��ϴ� ����
    user->active = 1;
    // ���� ������ �����ϴ� �������� ����
    user->txTime = time + 1;
    user->timer = 0;
    user->msg2Flag = 0;
    user->firstTxTime = time + 1;
    user->failCount = 0;
    user->nowBackoff = 0;

    float pi = 3.14;
    float theta = (float)rand()/(float)(RAND_MAX) * 2 * pi;
    float r = cellRadius * sqrt((float)rand()/(float)RAND_MAX);

    user->angle = theta;

    if(user->angle >= 0 && user->angle < ((1./3.)*pi)){
        user->sector = 0;
    }else if(user->angle >= ((1./3.)*pi) && user->angle < ((2./3.)*pi)){
        user->sector = 1;
    }else if(user->angle >= ((2./3.)*pi)&& user->angle < 3.14){
        user->sector = 2;
    }else if(user->angle >= pi && user->angle < ((4./3.)*pi)){
        user->sector = 3;
    }else if(user->angle >= ((4./3.)*pi) && user->angle < ((5./3.)*pi)){
        user->sector = 4;
    }else{
        user->sector = 5;
    }

    user->xCoordinate = r * cos(theta);
    user->yCoordinate = r * sin(theta);
    user->distance = r;
}

int* Dec2Bin(int decimal, int Length){
	int *binary;
	binary = (int *) calloc(Length, sizeof(int)); // Length >= ceil( log2(decimal) )
	if(binary!=NULL){
		int i = Length-1;
		while( decimal ){
			*(binary+i) = decimal % 2;
			decimal = decimal / 2;
			i = i-1;
		}
	}
	return binary;
}

void itialUeInfo(struct UEinfo *user, int nPreamble){
    int tmp = rand() % nPreamble;
    user->preamble = tmp;
    user->binaryPreamble = Dec2Bin(tmp, 8);
    user->rarWindow = 0;
    user->maxRarCounter = 0;
    user->preambleChange = 1;
    user->preambleTxCounter = 1;
}

void selectPreamble(struct UEinfo *user, int nPreamble, int time, int backoff, int accessTime, int maxRarWindow, int maxMsg2TxCount, int *continueFaliedUEs){
    // ó�� ������ �õ��ϴ� UE
    if (user->active == 1){
        itialUeInfo(user, nPreamble);
        user->active = 2;
    }
    // �̹� �� �� ������ �õ� �ߴ� UE
    else if (user->active == 2){
        // user->active = 1;
        // �� �� backoff counter�� 0�� UE�鸸
        if (user->nowBackoff <= 0){
            // RAR window 1 ����
            user->rarWindow++;
            // RAR window ���ð��� 5ms�� �Ǵ� ���
            
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

            if (user->rarWindow >= maxRarWindow){
                // ���� preamble�� ���� Ƚ���� �ִ��� ���
                if (user->maxRarCounter >= maxMsg2TxCount){
                    *continueFaliedUEs = *continueFaliedUEs+1;
                    // MSG 2 ���ſ� ���������� ������ UE�� ������ RA���� �õ� X
                    // user->raFailed = -1;
                    itialUeInfo(user, nPreamble);
                    user->nowBackoff = 0;
                    user->timer = 0;
                    // user->firstTxTime = time + 1;
                    user->failCount += 1;

                    // backoff counter ����
                    user->nowBackoff = user->txTime - time;
                    user->secondTxTime = user->txTime;
                }
                else{
                    // RAR window �ʱ�ȭ
                    user->rarWindow = 0;
                    // �ִ� ���� Ƚ���� 1 ����
                    user->maxRarCounter++;

                    user->preambleTxCounter++;

                    // backoff counter ����
                    user->nowBackoff = user->txTime - time;
                    user->secondTxTime = user->txTime;
                }
            }
        }
    }
}

void preambleDetection(struct UEinfo *user, int *preambleTxUEs, int preambleTxCheck, int nPreamble, int time, int *grantCheck, int nGrantUL, int nUE){

    // user            : ��ü UE ����ü
    // preambleTxUEs   : ���� ������ �õ��ϴ� UE���� id�� ������ �迭
    // preambleTxCheck : ���� ������ �õ��ϴ� UE���� �� (for���� ���� ����)
    // nPreamble       : �������� �迭 ������ ũ�� ��
    // grantCheck      : ���� Ÿ�ӿ� �Ҵ�� grant�� ��
    // nGrantUL        : �Ҵ� ������ grant�� ��

    int *preambles = (int *)calloc(nPreamble, sizeof(int));
    int **userCheck = (int**)malloc(sizeof(int*)*nPreamble);

    for(int i = 0; i < nPreamble; i++){
        userCheck[i] = (int*)calloc(preambleTxCheck, sizeof(int));
    }

    // ���� �����ϴ� UE���� ID�� �̿��Ͽ� 
    for(int i = 0; i < preambleTxCheck; i++){
        // ������ �õ��ϴ� UE�� ����ϴ� preamble ��ȣ
        int RAPID = (user+(preambleTxUEs[i]))->preamble;
        if(RAPID == -1){
            printf("%d\n", preambleTxUEs[i]);
        }
        userCheck[RAPID][preambles[RAPID]] = preambleTxUEs[i];
        // �ش� preamble ��ȣ�� count�� 1 ���� (count���� 2�̻��� ���� �浹�� ������ �� ����)
        preambles[RAPID] += 1;
    }


    for(int i = 0; i < nPreamble; i++){
        if (preambles[i] == 1){
            totalPreambleTxop++;
            *grantCheck = *grantCheck + 1;

            if(*grantCheck > 12){
                // printf("Grant �ο� �Ұ���\n");
                // Grant �ο� �Ұ��� -> MSG1-2 ����
                // userCheck[i][0]
                (user+userCheck[i][0])->txTime += 3;
                (user+userCheck[i][0])->rarWindow += 3;
            }else{
                // printf("Grant �ο� ����\n");
                // Grant �ο� ����
                // userCheck[i][0]
                (user+userCheck[i][0])->active = 3;
                (user+userCheck[i][0])->txTime = time + 11;
                (user+userCheck[i][0])->connectionRequest = 0;
                (user+userCheck[i][0])->msg2Flag = 1;
            }
        }else{
            // printf("Preamble collision\n");
            for(int j = 0; j < preambles[i]; j++){
                collisionPreambles++;
                // ��ü preamble���� Ƚ��
                totalPreambleTxop++;
                // userCheck[i][j] ��� ����
                (user+userCheck[i][j])->txTime += 3;
                (user+userCheck[i][j])->rarWindow += 3;
            }

        }
    }

    free(userCheck);
    free(preambles);
}

void requestResourceAllocation(struct UEinfo *user, int time, int backoff, int nPreamble, int *finalSuccessUEs, int *continueFaliedUEs){
    user->connectionRequest++;
    if (user->connectionRequest < 48){
        float p = (float)rand() / (float)RAND_MAX;
        if (p > 0.1){
            // 90%�� Ȯ���� MSG 3-4 ����
            user->msg4Flag = 1;
            user->timer = user->timer + 6;
            user->active = 0;
            *finalSuccessUEs = *finalSuccessUEs+1;
        }else{
            user->connectionRequest = 48;
            user->txTime += 48;
        }
    }else{
        *continueFaliedUEs = *continueFaliedUEs+1;
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
        user->failCount += 1;
    }
}

/*
    int tmp = rand() % nPreamble;
    user->preamble = tmp;
    user->binaryPreamble = Dec2Bin(tmp, 8);
    user->rarWindow = 0;
    user->maxRarCounter = 0;
    user->preambleChange = 1;
    user->preambleTxCounter = 1;
*/

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
void saveSimulationLog(int seed, int time, int nUE, int nSuccessUE, 
                       int failedUEs, int preambleTxCount, float totalDelay, 
                       int distribution, int nPreamble, double layency, int *continueFaliedUEs, int *finalSuccessUEs){

    float ratioSuccess = (float)nSuccessUE / (float)nUE * 100.0;
    float ratioFailed = (float)failedUEs / (float)nUE;
    float nCollisionPreambles = (float)collisionPreambles / ((float)nUE * (float)nPreamble);
    float averagePreambleTx = (float)preambleTxCount / (float)nSuccessUE;
    float averageDelay = totalDelay / (float)nSuccessUE;

    printf("Number of UEs: %d\n", nUE);
    printf("Total simulation time: %dms\n", time);
    printf("Success ratio: %.2lf\n", ratioSuccess);
    printf("Number of succeed UEs: %d\n", nSuccessUE);
    printf("Number of falied UEs: %d\n", *continueFaliedUEs);
    printf("Number of collision preambles: %.6lf\n", nCollisionPreambles);
    printf("Average preamble tx count: %.2lf\n", averagePreambleTx);
    printf("Average delay: %.2lf\n", averageDelay);

    FILE *fp;
    char resultBuff[1000];
    char fileNameResult[500];

    if (distribution == 1){
        sprintf(fileNameResult, "./TestResults/uniform_%d_%d_%d_Results.txt", seed, nPreamble, nUE);
    }else{
        sprintf(fileNameResult, "./TestResults/beta_%d_%d_%d_Results.txt", seed, nPreamble, nUE);
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

    // sprintf(resultBuff, "%lf\n", layency);
    // fputs(resultBuff, fp);

    sprintf(resultBuff, "Number of total preamble tx: %d\n", preambleTxCount);
    fputs(resultBuff, fp);

    sprintf(resultBuff, "Finally Falied: %d\n", *continueFaliedUEs);
    fputs(resultBuff, fp);

    sprintf(resultBuff, "Finally Success: %lf\n", (float)*finalSuccessUEs/(float)(*continueFaliedUEs+*finalSuccessUEs));
    fputs(resultBuff, fp);

    fclose(fp);
}
void saveResult(int seed, int nUE, struct UEinfo *UE, int distribution, int nPreamble){
    FILE *fp_l;
    char logBuff[1000];
    char fileNameResultLog[500];
    if (distribution == 1){
        sprintf(fileNameResultLog, "./TestResults/uniform_%d_%d_%d_Logs.txt", seed, nPreamble, nUE);
    }else{
        sprintf(fileNameResultLog, "./TestResults/beta_%d_%d_%d_Logs.txt", seed, nPreamble, nUE);
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
void pointResults(struct UEinfo *UE, int nUE, int distribution){
    FILE *fp_l;
    char logBuff[1000];
    char fileNameResultLog[500];
    if (distribution == 1){
        sprintf(fileNameResultLog, "./NomaUniformResults/%d_point_Logs.txt", nUE);
    }else{
        sprintf(fileNameResultLog, "./NomaBetaResults/%d_point_Logs.txt", nUE);
    }
    fp_l = fopen(fileNameResultLog, "w+");

    for(int i = 0; i < nUE; i++){
        sprintf(logBuff, "%lf,%lf,%d\n",(UE+i)->xCoordinate, (UE+i)->yCoordinate, (UE+i)->sector);
        fputs(logBuff, fp_l);
    }
    fclose(fp_l);
}

float beta_dist(float a, float b, float x){
    float betaValue = (1 / betaF) * (pow(x, (a - 1))) * (pow((1 - x), (b - 1)));
    return betaValue;
}