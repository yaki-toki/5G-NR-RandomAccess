#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h> 

#define betaF 0.0165

float beta_dist(float a, float b, float x);
int uniform_distribution(int rangeLow, int rangeHigh);

int main(){
    FILE *fp_l;
    char logBuff[1000];
    char fileNameResultLog[500];
    sprintf(fileNameResultLog, "./distribution.txt");
    fp_l = fopen(fileNameResultLog, "w+");

    for(int i = 0; i < 10000; i++){
        if(i % 5 == 1){
            sprintf(logBuff, "%lf\n", beta_dist(3, 4, (float)i/10000.0));
            fputs(logBuff, fp_l);
            printf("%lf\n", beta_dist(3, 4, (float)i/10000.0));
        }
    }
    fclose(fp_l);
    return 0;
}

float beta_dist(float a, float b, float x){
    float betaValue = (1/betaF) *(pow(x, (a-1))) * (pow((1-x), (b-1)));
    return betaValue;
}

int uniform_distribution(int rangeLow, int rangeHigh){
    int range = rangeHigh - rangeLow + 1; //+1 makes it [rangeLow, rangeHigh], inclusive.
    int copies=RAND_MAX/range; // we can fit n-copies of [0...range-1] into RAND_MAX
    // Use rejection sampling to avoid distribution errors
    int limit=range*copies;    
    int myRand=-1;
    while( myRand<0 || myRand>=limit){
        myRand=rand();   
    }
    return myRand/copies+rangeLow;    // note that this involves the high-bits
}