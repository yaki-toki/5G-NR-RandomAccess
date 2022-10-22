#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <math.h>
#include <string.h>
#include <complex.h>

const int ContentionBased = 1;

struct UEinfo{
	int activityFlag; // activityFlag = 1 for users that participate in the RA procedure
	int RAPID; // A value between 0 and nPreambles-1. 
	int reTxCount;
	int TCRNTI[16]; // 16-bit in length. 
	int *CCCH; // *(CCCH) = LCID
	int raFlag; // raFlag = 1 for successful RA procedure; otherwise, raFlag = 0 (these values are assigned if activityFlag = 1)
	int CRNTI[16]; // Upon successful RA procedure, TCRNTI becomes CRNTI
	int successUE;
	int failedUE;
	int nowTx;
};

int Check_Memory_Space(void *ptr);
void UEinfo_Initialize(struct UEinfo *user);
void Print_UEinfo(struct UEinfo *user, int index);
int RA_Result(struct UEinfo *user);
void Distribute(float *d, int n, float R);
int* Active(int n, float p, struct UEinfo *user);
int* Preamble_Selection(int n, int setSize, struct UEinfo *user);
void Print_MSG1_Result(struct UEinfo *Info, float *d, int n);
int cmpfunc(const void *a, const void*b);
int* Collision_Detection(int Length, int *selected, int *activeIndex, float *d, float d_threshold);
void Print_Collision_Detection_Result(int *result, int Length);
int* Dec2Bin(int decimal, int Length);
char* RAR_Generator(int *collision_result, int Length);
double complex** MSG3_Resource_Alocation(int n);
void MSG3_Transmission(struct UEinfo *user, char *str, double complex **resource);
void CCCH_Message(struct UEinfo *user, int LCID);
void QPSK(int *bit_stream, double complex *channel);
void AWGN(float sigma2, double complex **resource, int n);
int** MSG4_Structure(int n);
void MSG4_Transmission(int **msg, double complex **resource, int *collisionResult, int n);
void Decode(int *bitstream, double complex *received);
void Print_User_Info(struct UEinfo *user);
void Print_MSG4(int **msg, int n);
void Contention_Resolution(struct UEinfo *user, int **msg);
void RA_Result_void(struct UEinfo *user);
int TxEnd(struct UEinfo *user, int numUE);

int successFul;

int main(){
	srand( time(NULL) );
	
	int UL_grant = 3;
	int CCE_allocated = 16;
	int CCE_PDCCH = 4;

	int nPreambles = 54;// * UL_grant * CCE_allocated * CCE_PDCCH;
	float delta = 90;
	float cellRange = 400;
	float noisePower = 0.1;
	int totalUE = 500;
	int nUE = 500;

	int txTimes = 10000;
	int num;
	float totalSuccess = 0;
	struct UEinfo *UE;

	// Tx Times
	for(num=0; num < txTimes; num++){
		UE = (struct UEinfo *) calloc(nUE, sizeof(struct UEinfo));
		if( Check_Memory_Space((void *)UE) )
			return 1;
		for(int k=0; k<nUE; ++k){
			UEinfo_Initialize(UE+k);
			// Print_UEinfo(UE+k, k);
		}
		
		float *distanceUE;
		distanceUE = (float *) calloc(nUE, sizeof(float));
		if( Check_Memory_Space((void *)distanceUE) )
			return 1;	
		Distribute(distanceUE, nUE, cellRange);
		
		float prTX = 0.6;
		int *activeUEIndex;
		
		activeUEIndex = Active(nUE, prTX, UE);
		if( Check_Memory_Space((void *)activeUEIndex) )
			return 1;
		// *(activeUEIndex) = n = number of active users
		// *(activeUEIndex+1), ..., *(activeUEIndex+n) = indices of active users
		int nActiveUE;
		nActiveUE = *(activeUEIndex);

		successFul = 0;
		if(nActiveUE){
			// printf(">>> Random Access Preamble (MSG1)\n");
			int *RAPID;
			RAPID = Preamble_Selection(nActiveUE, nPreambles, UE); 
			if( Check_Memory_Space((void *)RAPID) )
				return 1;	
			for(int k=0; k<nActiveUE; ++k){
				(UE+*(activeUEIndex+k+1))->activityFlag = 1;
				(UE+*(activeUEIndex+k+1))->RAPID = *(RAPID+k);
			}
			// Print_MSG1_Result(UE, distanceUE, nUE);
			
			
			// printf(">>> Preamble Detection\n");
			int *collision;
			collision = Collision_Detection(nPreambles, RAPID, activeUEIndex, distanceUE, delta);
			if( Check_Memory_Space((void *)collision) )
				return 1;
			// For preamble i:
			// *(collision+i) = 0, when no UE selects preamble i or BS detects collision on preamble i
			// *(collision+i) = 1, otherwise
			// Print_Collision_Detection_Result(collision, nPreambles);
			
			
			// printf(">>> Random Access Response (MSG2)\n");
			char *rar; 
			rar = RAR_Generator(collision, nPreambles);
			if( Check_Memory_Space((void *)rar) )
				return 1;
			// printf("MAC RAR:\n");
			// printf("%s\n", rar);
			
			
			// printf(">>> Scheduled Transmission (MSG3)\n");
			double complex **allocatedResource = MSG3_Resource_Alocation(nPreambles);
			if( Check_Memory_Space((void *)allocatedResource) )
				return 1;
			for(int i=0; i<nUE; ++i){
				MSG3_Transmission(UE+i, rar, allocatedResource);
				// Print_UEinfo(UE+i, i);
			}
			

			AWGN(noisePower, allocatedResource, nPreambles);
			
			
			// printf(">>> Contention Resolution (MSG4)\n");
			int **MSG4 = MSG4_Structure(nPreambles);
			if( Check_Memory_Space((void *)MSG4) )
				return 1;
			MSG4_Transmission(MSG4, allocatedResource, collision, nPreambles);
			// Print_MSG4(MSG4, nPreambles);
			for(int i=0; i<nUE; ++i){
				Contention_Resolution(UE+i, MSG4);
				// RA_Result(UE+i);
				RA_Result_void(UE+i);
			}
		}
		printf("Time: %d | Succeed UE: %d\n", num, TxEnd(UE, nUE));
	}


	printf("Succed UE: %d\n", TxEnd(UE, nUE));
	printf("%d\n", num);
	
	int faildUEs = 0;
	for(int i = 0; i < nUE; ++i){
		if((UE+i)->failedUE == 1){
			faildUEs+= 1;
		}
	}

	printf("Failed UE: %d\n", faildUEs);
	return 0;
}




















int Check_Memory_Space(void *ptr){
	int flag = 0;
	if(ptr==NULL){
		printf("not enough memory");
		flag = 1;
	}
	return flag;
}

void UEinfo_Initialize(struct UEinfo *user){
	user->activityFlag = 0;
	user->RAPID = 0; // 0 is a valid value but with activityFlag=0 we can understand that this preamble is not chosen by the user
	user->reTxCount = 0;
	for(int i=0; i<16; i++){ 
		user->TCRNTI[i] = 0; // Valid value of TC-RNTI is within the range 0001-FFFF
		user->CRNTI[i] = 0; // Valid value of C-RNTI is within the range 0001-FFFF
	}
	user->CCCH = NULL;
	user->raFlag = 0;
	if(user->successUE == NULL){
		user->successUE = 0;
	}else{
		user->successUE = 1;
	}

	if(user->failedUE == NULL){
		user->failedUE = 0;
	}else{
		user->failedUE = 1;
	}
	
	user->nowTx = 0;
}

void Print_UEinfo(struct UEinfo *user, int index){
	
	printf("User: %3d | Activity Flag: %d | ", index, user->activityFlag);
	printf("RAPID: %2d | ", user->RAPID);
	printf("TC-RNTI: ");
	for(int i=0; i<16; ++i)
		printf("%d", user->TCRNTI[i]);
	printf(" | ");
	printf("CCCH: ");
	if(user->CCCH!=NULL){
		int messageSize;
		if(*(user->CCCH)==52)
			messageSize = 48;
		else
			printf("Error: CCCH message size is not defined for LCID=%d\n", *(user->CCCH));
		for(int i=1; i<=messageSize; ++i)
			printf("%d", *(user->CCCH+i));
	}
	else
		printf("NULL");
	printf("\n");
	RA_Result(user);
}

int RA_Result(struct UEinfo *user){
	if(user->reTxCount == 10){
		printf("This UE is failed\n");
		return 4;
	}

	if(user->successUE == 1){
					printf("Already succeed UE\n");
					return 0;
	}

	if( user->activityFlag==0 ){
		printf("Uer is not active\n");
		return 1;
	}
	else{
		if(user->CCCH==NULL){
			printf("Collision detected in MSG2\n");
			return 2;
		}
		else{
			if(user->raFlag==0){
				printf("Contention detected in MSG4\n");
				return 3;
			}
			else{
				printf("Successful random access procedure\n");	
				return 0;	
			}
		}
	}
}

// 단순히 성공한 UE의 수를 카운팅 하는 method
void RA_Result_void(struct UEinfo *user){
	if(user->reTxCount == 10){
		user->failedUE = 1;
	}
	if(user->activityFlag == 0){
	}else{
		if(user->CCCH == NULL){
			user->reTxCount += 1;
		}
		else{
			if(user->raFlag == 0){
				user->reTxCount += 1;
			}
			else{
				user->successUE = 1;
			}
		}
	}
}

void Distribute(float *d, int n, float R){
	int i;
	float temp;
	for(i=0; i<n; ++i){
		temp = (float)rand()/(float)RAND_MAX;
		*(d+i) = R * sqrt(temp);
	}
}

int* Active(int n, float p, struct UEinfo *user){
	int L = 0;
	int *selected;
	selected = (int *) calloc(L+1, sizeof(int));
	
	int i;
	float temp;
	for(i=0; i<n; ++i){
		if((user+i)->successUE != 1){
			temp = (float)rand()/(float)RAND_MAX;
			
			*(selected) = ++L;
			selected = (int *) realloc(selected, (L+1)*sizeof(int));
				
			if(selected==NULL){
				break;
			}
			*(selected+L) = i;
		}
		
	}
	return selected;
}

int* Preamble_Selection(int n, int setSize, struct UEinfo *user){
	int *selectedPreambles;
	selectedPreambles = (int *) calloc(n, sizeof(int));
	if(selectedPreambles!=NULL){
		int i;
		for(i=0; i<n; ++i){
			if((user+i)->successUE != 1 && (user+i)->failedUE != 1){
				*(selectedPreambles+i) = rand()%setSize;
				(user+i)->reTxCount += 1;
			}
		}
	}
	return selectedPreambles;
}

void Print_MSG1_Result(struct UEinfo *Info, float *d, int n){
	for(int i=0; i<n; ++i){
		if((Info+i)->activityFlag==1)
			printf("User %3d is active | distance : %6.2f | selects preamble : %2d\n", i, *(d+i), (Info+i)->RAPID);	
	}
}

int cmpfunc(const void *a, const void *b){
	int cmp;
	cmp = (int) (*(float *) a - *(float *) b);
	return cmp;
}

int* Collision_Detection(int Length, int *selected, int *activeIndex, float *d, float d_threshold){
	int *result;
	result = (int *) calloc(Length, sizeof(int));
	if(result!=NULL){
		int i; // preamble index; i \in \{0,1,...,Length-1\}
		int j; // active user index; j \in \{0,1,...,n-1\}
		int n=*activeIndex; // number of (active) users
		int k; // number of users that select preamble i
		int index; // index of user that selects preamble i
		for(i=0; i<Length; ++i){
			k = 0;
			float *vec_d;
			vec_d = (float *) calloc(1, sizeof(float)); // stores distances of users that select preamble i
			for(j=0; j<n; ++j){
				if(*(selected+j)==i){
					k += 1;
					vec_d = (float *) realloc(vec_d, k*sizeof(float));
					index = *(activeIndex+j+1);
					*(vec_d+k-1) = *(d+index);
				}
			}
			if(k==1){
				*(result+i) = 1;
			}
			if(k>1){
				qsort(vec_d, k, sizeof(float), cmpfunc);
				if(*(vec_d+k-1)-*(vec_d)<d_threshold){
					*(result+i) = 1;
				}
			}
			free(vec_d);
		}
	}
	return result;
}

void Print_Collision_Detection_Result(int *result, int Length){
	for(int i=0; i<Length; ++i)
		if(*(result+i)==0)
			printf("RAR does NOT include a RAPID filed for preamble %2d\n", i);
		else
			printf("RAR does     include a RAPID filed for preamble %2d\n", i);
		
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

char* RAR_Generator(int *collision_result, int Length){
	// For adding extenstion fields ('E' and 'T'), we need to convert int to str (by sprintf) or char (by +'0') 
	char *str;
	str = (char *) calloc(1, sizeof(char));
	int l_str = 0; // length of string
	int i; // preamble index
	for(i=0; i<Length; ++i){
		if(*(collision_result+i)==1){
			l_str += 25;
			str = (char *) realloc(str, (l_str+1)*sizeof(char)); // +1 for the null character
			if(str==NULL)
				break;
				
			strcat(str, "ET");
			int *RAPID_binary;
			RAPID_binary = Dec2Bin(i, 6);
			char *appendix;
			appendix = (char *) calloc(7, sizeof(char));
			if(RAPID_binary==NULL || appendix==NULL)
				break;
			for(int k=0; k<6; ++k)
				*(appendix+k) = *(RAPID_binary+k) + '0';
			strcat(str, appendix);
			free(RAPID_binary);
			free(appendix);
			
			strcat(str, "R");
			int *TCRNTI_binary;
			TCRNTI_binary = Dec2Bin(i, 16);
			*(TCRNTI_binary+0) = 1; // We set MSB=1 since 0000 is not a valid TC-RNTI
			appendix = (char *) calloc(17, sizeof(char));
			if(TCRNTI_binary==NULL || appendix==NULL)
				break;
			for(int k=0; k<16; ++k)
				*(appendix+k) = *(TCRNTI_binary+k) + '0';
			strcat(str, appendix);
			free(TCRNTI_binary);
			free(appendix);
		}
	}
	return str;
}

double complex** MSG3_Resource_Alocation(int n){
	double complex** resource;
	resource = (double complex **) calloc(n, sizeof(double complex *));
	if( resource!=NULL ){
		for(int i=0; i<n; i++){
			*(resource+i) = (double complex *) calloc(24, sizeof(double complex));
			// We can allocate more resources. 24 is enough for transmitting 6 bytes CCCH message with QPSK
			// Also, MSG4 only uses the first 48 bits belonging to the uplink CCCH SDU within MSG3
			if( *(resource+i)==NULL){
				resource = NULL;	
				break;
			}
		}
	}
	return resource;
}

void MSG3_Transmission(struct UEinfo *user, char *str, double complex **resource){
	if( user->activityFlag==1 ){
		char *rapidSubHeader;
		rapidSubHeader = (char *) calloc(9, sizeof(char));
		strcpy(rapidSubHeader, "ET000000");
		
		int number = user->RAPID;
		int i = 7;
		while(number){
			*(rapidSubHeader+i) = (number%2) + '0';
			number /= 2;
			i -= 1;
		}
		
		char *find;
		find = strstr(str, rapidSubHeader);
		if(find!=NULL){
			for(int i=0; i<16; ++i)
				user->TCRNTI[i] = *(find+i+9) - '0'; // 9 is ude to ET??????R
			if(ContentionBased){
				CCCH_Message(user, 52);		
				QPSK(user->CCCH, *(resource+user->RAPID));
			}
		}	
		free(rapidSubHeader);
	}
}

void CCCH_Message(struct UEinfo *user, int LCID){
	if(LCID==52){
		int messageSize = 48;
		user->CCCH = (int *) calloc(messageSize+1, sizeof(int));
		*(user->CCCH) = 52;
		long int temp = (long int)rand()%(long int)pow(2,39);
		int i=messageSize;
		while(temp>0){
			*(user->CCCH+i) = (int)temp%2;
			temp = temp/2;
			i--;
		}
	}
	else
		printf("Error: CCCH message is not defined for LCID=%d\n", LCID);
}

void QPSK(int *bitstream, double complex *channel){
	int L = 0;
	if(*(bitstream)==52)
		L = 48;
	else
		printf("Error: CCCH message size is not defined for LCID=%d\n", *(bitstream));
		
	int l=0;
	for(int i=1; i<=L;){
		switch(2* (*(bitstream+i)) + (*(bitstream+i+1)) ){
			case 0:
				*(channel+l) +=  1 + 1 *I;
				break;
			case 1:
				*(channel+l) += -1 + 1 *I;
				break;
			case 2:
				*(channel+l) +=  1 - 1 *I;
				break;
			case 3:
				*(channel+l) += -1 - 1 *I;	
		}
		i += 2;
		l += 1;
	}
}

void AWGN(float sigma2, double complex **resource, int n){
	for(int i=0; i<n ;i++){
		for(int j=0; j<24; j++){
			float real = sigma2/2 * ( sqrt(3)*(float)rand()/(float)RAND_MAX - 2*sqrt(3) ); 
			float imag = sigma2/2 * ( sqrt(3)*(float)rand()/(float)RAND_MAX - 2*sqrt(3) );
			// Power of noise is sigma2. A random variable with distribution Unif[-sqrt(3),sqrt(3)] has a unit power
			double complex noise = real + imag * I;
			*(*(resource+i)+j) +=  noise;
		}
	}
}

int** MSG4_Structure(int n){
	int **msg;
	msg = (int **) calloc(n, sizeof(int *));
	if( msg!=NULL ){
		for(int i=0; i<n; i++){
			*(msg+i) = (int *) calloc(48, sizeof(int));
			if( *(msg+i)==NULL ){
				msg = NULL;
				break;
			}
		}
	}	
	return msg;
}

void MSG4_Transmission(int **msg, double complex **resource, int *collisionResult, int n){
	for(int i=0; i<n; i++){
		if(*(collisionResult+i)==1)
			Decode(*(msg+i), *(resource+i));		
	}
}

void Decode(int *bitstream, double complex *received){
	double complex z;
	for(int i=0; i<24; ++i){
		z = *(received+i);	
		if(creal(z)>=0 && cimag(z)>=0){
			*(bitstream+2*i) = 0;
			*(bitstream+2*i+1) = 0;
		}
		else if(creal(z)>0 && cimag(z)<0){
			*(bitstream+2*i) = 1;
			*(bitstream+2*i+1) = 0;
		}
		else if(creal(z)<0 && cimag(z)>0){
			*(bitstream+2*i) = 0;
			*(bitstream+2*i+1) = 1;
		}
		else if(creal(z)<=0 && cimag(z)<=0){
			*(bitstream+2*i) = 1;
			*(bitstream+2*i+1) = 1;
		}		
	}
}


void Print_MSG4(int **msg, int n){
	for(int i=0; i<n; i++){
		printf("Preamble %2d:    ", i);
		for(int j=0; j<48; j++)
			printf("%d", *(*(msg+i)+j));
		printf("\n");
	}
}

void Contention_Resolution(struct UEinfo *user, int **msg){
	if( user->activityFlag==1 && user->CCCH!=NULL ){
		int flag = 1;
		for(int i=0; i<48; i++){
			if( *(user->CCCH+i+1)!=*(*(msg+user->RAPID)+i) ){
				flag = 0;
				break;
			}
		}
		user->raFlag = flag;	
		if(flag==1){
			for(int i=0; i<16; i++)
				user->CRNTI[i] = user->TCRNTI[i];
		}
	}
}

int TxEnd(struct UEinfo *user, int numUE){
	int users = 0;
	for(int i = 0; i < numUE; ++i){
		if((user+i)->successUE == 1){
			users += 1;
		}
	}
	return users;
}