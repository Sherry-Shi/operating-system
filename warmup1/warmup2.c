/*
 * Author:      William Chia-Wei Cheng (bill.cheng@acm.org)
 *
 * @(#)$Id: listtest.c,v 1.1 2014/05/20 17:40:03 william Exp $
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "cs402.h"
#include <unistd.h>
#include "stdbool.h"
#include "my402list.h"
#include "time.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

static char gszProgName[MAXPATHLENGTH];
int gnDebug=0;


#define length 100
#define maxamount 10000000
// #define maxdate 4294967295L

bool file_input = true;
char number_max[] = "?,???,???.??";

typedef struct Transaction
{
	char *data;
	char *desc;
	bool withdraw;
	char *balance;
	double amount;
	/* data */
}Trans;

/* ----------------------- Utility Functions ----------------------- */

char *DottedNumber(char*);


int InsertTrans(My402List *head, char *line){
	if(head == NULL || line == NULL) return FALSE;
	//char *flag, *balance, *when, *desc;
	char *ptr[3];
	Trans *trans = (Trans*)malloc(sizeof(Trans));
	if((*line) == '-') 
		trans->withdraw = TRUE;
	else 
		trans->withdraw = FALSE; 
	
	for (int i = 0; i < 3; ++i)
	{
		ptr[i] = strchr(line, '\t');
		if(ptr[i] != NULL){
			*(ptr[i]) = '\0';
			ptr[i] = ptr[i] + 1;
			line = ptr[i];
		}else{
			printf("Malformed Transaction");
			return FALSE;
		}
	}
	trans->data = ptr[0];
	trans->desc = ptr[2];
	trans->balance = ptr[1];
	long max =(long) atol(trans->data);
	time_t current = time(NULL);
	//long maxdate = 4294967295L;
	if(max > (long)current){
		printf("date is out of range");
		exit(1);
	}
	char *test = NULL;
	if((test = strchr(ptr[2],'\n')) != NULL)
		*test = '\0';
	if(My402ListAppend(head, trans)) return TRUE;
	else return FALSE;
}

My402ListElem	*MaxElem(My402List *head){
	if(My402ListEmpty(head)) return NULL;
	My402ListElem *max = NULL, *ptr = (head->anchor).next, *anchor = &(head->anchor);
	Trans *obj = NULL;
long  maxn =  0;	
	while(ptr != anchor){
		obj = (Trans*)ptr->obj;
		long date = atol(obj->data);
		if(date > maxn){
			max = ptr;
			maxn = date;
		}else if(date == maxn){
			printf("there exists the same time stamp");
			exit(1);
		}
		ptr = ptr->next;
	}
	if(max){
		max->prev->next = max->next;
		max->next->prev = max->prev;
		(head->num_members)--;
		return max;
	}else return NULL;

}

void AddElem(My402List *newhead,My402ListElem *elem){
	if(!newhead){
		printf("the newhead is null");
		exit(1);
	}else{
		My402ListElem *anchor = &(newhead->anchor);
		anchor->prev->next = elem;
		elem->prev = anchor->prev;
		elem->next = anchor;
		anchor->prev = elem;
		(newhead->num_members)++;
		return;
	}

}


My402List *SortList(My402List *head){
	My402ListElem *max = NULL;
	My402List *newhead = (My402List*)malloc(sizeof(My402List));
	while(!newhead){
		newhead =(My402List*)malloc(sizeof(My402List)); 
	}
	memset(newhead,0,sizeof(My402List));
	My402ListInit(newhead);

	if(head == NULL){
		printf("the input list iNULL!");
		exit(1);
	}else{
		if(!My402ListEmpty(head)){
			//My402ListElem *cur = (head->anchor).next, *anchor = &(head->anchor);
			while(!My402ListEmpty(head)){
				max = MaxElem(head);
				AddElem(newhead,max);
			}
		}else{
			printf("the list is empty. No need to sort list");
			exit(1);
		}
	}
	free(head);
	return newhead;
}

char * ConvertTime(char *time){
	char *cur = time;
	while(!(*cur >= '0' && *cur <= '9')){
	cur++;
	}
	while(*cur != ' '){
	cur++;
	}
	*(cur + 1) = '\0';
	char *year = cur + 2;
	char *ptr = strchr(year, ' ');
	year = ptr + 1;
	ptr = strchr(year, '\n');
	*ptr = '\0';
	year = strcat(time,year);
	return year;
}

void PrintDateDesc(My402ListElem *elem){
	Trans *ptr = (Trans*)elem->obj;
	long i = atol(ptr->data);
	time_t stamp = (time_t)i;
	char *time_1 = ctime(&stamp);
	time_1 = ConvertTime(time_1);
	printf("| %15s |",time_1);
	char *desc = (char*)malloc(sizeof(char)*100);
	if(!desc){
		printf("error allocating memory!");
		exit(1);
	}
	memset(desc,0,100);
	if(strlen(ptr->desc) <= 24){
		strcpy(desc,ptr->desc);
	}else{
		strncpy(desc,ptr->desc,24);
	}	
	printf(" %-24s | ",desc);
}

void PrintAmount(My402ListElem *elem){
	char *number = ptr->balance;
	double amount = atof(elem->balance);
	if(fabs(amount) <= maxamount){
	char *fnumber = DottedNumber(number);
		if(ptr->withdraw){
			printf("(%12s) | ",fnumber);
		}else{
			printf(" %12s  | ",fnumber);
		}
	}else{
		if(ptr->withdraw){
			printf("(%12s) | ",number_max);
		}else{
			printf(" %12s  | ",number_max);
		}
	}
}

void PrintBalance(My402List *head, My402ListElem *elem){
	Trans *ptr = elem->obj;
	if(My402ListLast(head) == elem){
		if(fabs(atof(ptr->balance)) <= maxamount){
			if(ptr->withdraw){
				printf("(%12s) |", DottedNumber(ptr->balance));
				ptr->amount = -atof(ptr->balance);
			}else{
				printf(" %12s  |",fnumber);	
				ptr->amount = atof(ptr->balance);
			}
		}else{
			if(ptr->withdraw){
				printf("(%12s) |", number_max);
				ptr->amount = -atof(ptr->balance);
			}else{
				printf(" %12s  |", number_max);	
				ptr->amount = atof(ptr->balance);
			}
		}
	}else{
		My402ListElem *prev = My402ListNext(head,elem);
		double balance;
		if(ptr->withdraw){
			balance = ((Trans*)prev->obj)->amount - atof(ptr->balance);
		}else{
			balance = ((Trans*)prev->obj)->amount + atof(ptr->balance);
		}
		ptr->amount = balance;
		char *fnumber = (char*)malloc(sizeof(char)*100);
		if(!fnumber){
			printf("error allocation memory!");
			exit(1);
		}
		sprintf(fnumber,"%.2f",balance);
		if(fabs(balance) > maxamount){
			if(balance > 0){
				printf(" %12s  | ",number_max);
			}else{
				printf("(%12s) | ",number_max);
			}
		}else{
			if(ptr->amount >= 0){
				printf(" %12s  |",DottedNumber(fnumber));
			}else{
				printf("(%12s) |",DottedNumber(fnumber));
			}
		}
		printf("\n");
		free(fnumber);
	}
}

void OutputTrans(My402ListElem *elem,My402List *head){
	PrintDate(elem);
	PrintAmount(elem);
	PrintBalance(head,elem);
	PrintBalance(head, elem);
}

char *DottedNumber(char *number){
	char *dnumber = (char*)malloc(sizeof(char)*100);
	if(!dnumber){
		printf("error allocation memory!");
		return NULL;
	}
	memset(dnumber, 0, 100);

	char *dptr = dnumber;
	char *ptr = strchr(number,'\0');
	if(ptr == NULL){
		printf("error");
		return NULL;
	}else{
		ptr--;
		while(*ptr != '.'){
			*dnumber++ = *ptr--;
		}
		*dnumber++ = '.';
		ptr--;
		int i = 0;
		while(i < 7 && ptr != number - 1 ){
			if(i++ != 0 && i%3 == 0 && ptr != number){
				*dnumber++ = *ptr--;
				*dnumber++ = ',';
			}else{
				*dnumber++ = *ptr--;
			}
		}
		//printf("%s",dptr);

		char *fnumber = (char*)malloc(sizeof(char)*100);
		if(!fnumber){
			printf("error allocation memory!");
			return NULL;
		}
		memset(fnumber, 0, 100);
		char *ptr2 = fnumber;

		char *ptr1 = strchr(dptr,'\0');
		if(!ptr1){
			printf("error");
			return NULL;
		}
		ptr1--;

		while(ptr1 != dptr - 1){
			*fnumber++ = *ptr1--;
		}
		free(dptr);
		dnumber = NULL;
		//printf("%s",ptr2);
		return ptr2;
	}
}

void Traverse(My402List *head){
	if(My402ListEmpty(head)){
		printf("the list is empty!");
		exit(1);
	}else{
		My402ListElem *cur = NULL, *anchor = &(head->anchor);
		cur = (head->anchor).prev;
		char line[] = "+-----------------+--------------------------+----------------+----------------+\n|       Date      | Description              |         Amount |        Balance |\n+-----------------+--------------------------+----------------+----------------+\n";
		printf("%s",line);
		while(cur != anchor){
			//			Trans *obj = (Trans *)cur->obj;
			OutputTrans(cur,head);
			cur = cur->prev;
		}
		printf("+-----------------+--------------------------+----------------+----------------+\n");
	}
}

void ValidateTransaction(char *trans){
	char *ptr1 = trans;
	int sum = 0;
	while(*ptr1++ != '\n')
	sum++;
	if(sum > 1024){
		printf("malformed transaction");
		exit(1);
	}


	if(*trans != '+' && *trans != '-'){
		printf("malformed transaction");
		exit(1);
	}
	char *ptr = trans;
		int count = 0;
	while((ptr = strchr(ptr,'\t')) != NULL){
		count++;
		ptr++;
	}
	if(count != 3){
		printf("malformed transaction");
		exit(1);
	}
	ptr = strchr(trans,'.');
	if(ptr == NULL){
		printf("malformed transaction");
		exit(1);
	}
	ptr++;
	count = 0;
	while(*ptr >= '0' && *ptr <= '9')
	{
		count ++;
		ptr ++;
	}
	if(count != 2){
		printf("malformed transaction");
		exit(1);
	}
}


void OutputTransaction(char *path){
	FILE *file = NULL;

	if(file_input){                              // check the source of input file: stdin or file
		file = fopen(path,"r");
		if(file ==  NULL){
			printf("error opening file!");
			return;
		}
	}else{
		file = stdin;
	}

	My402List *head = NULL;
	while(!head){
		head = (My402List*)malloc(sizeof(My402List));
	}
	memset(head,0,sizeof(My402List));
	My402ListInit(head);
	char *line = (char*)malloc(sizeof(char)*200);
	memset(line,0,200);
	char val = fgetc(file);
	while(val != EOF){
		char *cur = line;   // pointer to transaction line
		while(val != '\n' && val != EOF){
			*line = val;
			line = line + 1;
			val = fgetc(file);
		}
		*line = '\n';
		line = (char*)malloc(sizeof(char)*200);
		while(!line){
			line =  (char*)malloc(sizeof(char)*200);       
		}
		memset(line,0,200); // check the validlity of transaction line
ValidateTransaction(cur);
InsertTrans(head, cur);
		val = fgetc(file);
	}
	fclose(file);
	My402List *newhead = SortList(head);
	Traverse(newhead);
} 

void CheckParameter(int num, char *argv[]){
	if(num == 2){
		if(strncmp(argv[1], "sort", 4) != 0 || strlen(argv[1]) != 4){
			printf("malformed command");
			exit(1);
		}
		file_input = false;
	}else if(num == 3){
		file_input = true;
		if(strncmp(argv[1], "sort", 4) != 0 || strlen(argv[1]) != 4){
			printf("malformed command");
			exit(1);
		}
		if(*(argv[2]) == '-'){
			printf("malformed command or input file \"%s\" does not exist", argv[2]);
			exit(1);
		}else{
			struct stat filestat;
			if(stat(argv[2], &filestat) < 0){
				int errsv = errno;
				if(errsv == EACCES){
					printf("Permission denied");
					exit(1);
				}
				if(errsv == EADDRNOTAVAIL){
					printf("The input path is not available");
					exit(1);
				}
				if(errsv == ENOENT){
					printf("File or Directory doesn't exist");
					exit(1);
				}else{
					printf("Error opening the input path");
					exit(1);
				}
			}else{
				if(S_ISDIR(filestat.st_mode)){
					printf("the input path is a directory");
					exit(1);
				}
			}
		}
	}
}

/* ----------------------- main() ----------------------- */

int main(int argc, char *argv[]){
//	SetProgramName(*argv);
//	ProcessOptions(argc, argv);
//	Process();
	if(argc != 3 && argc != 2){
		printf("malformed command");
		exit(1);
	}
	CheckParameter(argc,argv);
	if(file_input){
		OutputTransaction(argv[2]);
	}
	else{
		OutputTransaction((char*)0);
	}
	return(0);
}
