	#include <stdio.h>
	#include <string.h>
	#include <pthread.h>
	#include <errno.h>
	#include <stdbool.h>
	#include <math.h>
	#include <stdlib.h>
	#include <time.h>
	#include <sys/time.h>
	#include <unistd.h>
	#include "warmup2.h"
	#include <signal.h>

	#define LineLength 100

	int bucket_size = 10, number_packet = 20, token_need = 3, current_bucket = 0, fail_packet = 0, fail_token = 0, total_token = 0, total_packet = 0;
	double  lambda = 2, mu = 0.35, token = 4;
	bool tfile= false, isbucket = true;
	char *filename = NULL;
	pthread_t server, packet, bucket, cancel;
	pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond_server;
	struct timeval systime1, current;
	sigset_t set;

	/* return the microsec between two timeval*/
	double CalculateTime(struct timeval *prev, struct timeval *current){
		int sec = ((current->tv_sec) - (prev->tv_sec)) * pow(10, 6);
		int micro = current->tv_usec - prev->tv_usec;
		return (double)(sec + micro);
	}

	void TransferPacket(void *ptr, int i){
		warmup2list *list = (warmup2list*)ptr;
		My402List *q1 , *q2;

		switch(i){
			case 0: q1 = list->bucket; q2 = list->server; break;
			case 1: q1 = list->server; q2 = list->service; break;
			default: printf("input error");
			exit(1);	
		}

		My402ListElem *elem = My402ListFirst(q1), *prev = elem->prev;
		prev->next = elem->next;
		elem->next->prev = prev;
		(q1->num_members)--;
		if(My402ListEmpty(q2)){
			My402ListElem  *anchor = &(q2->anchor);
			anchor->next = elem;
			elem->prev = anchor;
			elem->next = anchor;
			anchor->prev = elem;
			q2->num_members += 1;
		}else{
			My402ListElem *last = My402ListLast(q2);
			last->next = elem;
			elem->prev = last;
			elem->next = &(q2->anchor);
			(q2->anchor).prev = elem;
			q2->num_members += 1;
		}
	}

	double SleepThread(struct timeval *current, struct timeval *prev, int i){
		struct timespec sleep, remain;
		gettimeofday(current, 0);
		int sec = 0, microsec = 0;
		sec = current->tv_sec - prev->tv_sec, microsec = (current->tv_usec - prev->tv_usec);
		double orgsec = 0;
		switch(i){
			case 1: orgsec = 1/lambda; break;
			case 2: orgsec = 1/token; break;
			case 3: orgsec = 1/mu; break;
			default: break;
		}
		if((sec*pow(10, 6) + microsec) < orgsec*pow(10, 6)){			
			sleep.tv_sec = (int)(orgsec - sec);
			sleep.tv_nsec = microsec * pow(10, 3) + (orgsec - (int)orgsec) * pow(10, 9);
			nanosleep(&sleep, &remain);
		}
		gettimeofday(prev, 0 );
			double time_packet = (double)(prev->tv_sec - systime1.tv_sec)*pow(10, 6) + (prev->tv_usec - systime1.tv_usec); // calculate the time of the packet into system
			return time_packet;
		}

		void *ServerThread(void *ptr){
			struct timespec sleep_time;
			warmup2list *head = (warmup2list*) ptr;
			My402List *server_queue = head->server;

			while(1){
				pthread_mutex_lock(&mutex1);
				// pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
				// pthread_cleanup_push(UnlockMutex, (void*)2);
				pthread_cleanup_push(pthread_mutex_unlock, &mutex1); 
				while(My402ListEmpty(server_queue)){
					pthread_cond_wait(&cond_server, &mutex1);
				}
				pthread_cleanup_pop(0);
				pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
				My402ListElem *elem = My402ListFirst(server_queue);
				Packet *pack = (Packet*)(elem->obj);
				gettimeofday(&(pack->leave_server), 0);
				double time_sys = CalculateTime(&systime1, &(pack->leave_server));
				double time_server = CalculateTime(&(pack->into_server), &(pack->leave_server));
				printf("%012.3fms: p%d leaves Q2, time in Q2 = %.3fms, begin service at S\n", time_sys/1000, pack->pack_num, time_server/1000);
				sleep_time.tv_sec = (int)((pack->service_time)/1000), sleep_time.tv_nsec = (pack->service_time/1000 - (int)(pack->service_time/1000)) * pow(10, 9);
				// pthread_cleanup_pop(0);
				pthread_mutex_unlock(&mutex1);	

				nanosleep(&sleep_time, 0);

				pthread_mutex_lock(&mutex1);
				// pthread_cleanup_push(UnlockMutex, (void*)2);
				gettimeofday(&(pack->end_service), 0);
				pack->actual_servicetime = CalculateTime(&(pack->leave_server), &(pack->end_service));
				printf("%012.3fms: p%d departs from S, service time = %.3fms, time in system = %.3fms\n", CalculateTime(&systime1, &(pack->end_service))/1000, pack->pack_num, pack->actual_servicetime/1000, CalculateTime(&(pack->arrival), &(pack->end_service))/1000);
				TransferPacket(ptr, 1);
				// pthread_cleanup_pop(0);
				pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
				pthread_mutex_unlock(&mutex1);
				pthread_testcancel();
			}
			return NULL;
		}

		void *BucketThread(void *ptr){
			// printf("bucket %lx", bucket);
			struct timeval prev, current;
			prev.tv_sec = systime1.tv_sec, prev.tv_usec = systime1.tv_usec;
			int count = 0;
			My402List *packet_queue = ((warmup2list*)ptr)->bucket;
			while(1){
				double time_bucket = SleepThread(&current, &prev, 2);
				count++;
				total_token = count;
				pthread_mutex_lock(&mutex1);
				// pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
				pthread_cleanup_push(pthread_mutex_unlock,&mutex1);
				if(current_bucket < bucket_size){
					current_bucket++;
					printf("%012.3fms: token t%d arrives, token bucket now has %d token\n", time_bucket/1000, count, current_bucket);
				}else{
					printf("%012.3fms: token t%d arrives, dropped\n", time_bucket/1000, count);
					fail_token++;
				}
				if(!My402ListEmpty(packet_queue)){
					My402ListElem *elem = My402ListFirst(packet_queue);
					Packet *pack =(Packet*) elem->obj;
					if(pack->num_token <= current_bucket){
						current_bucket -= pack->num_token;
						gettimeofday(&(pack->into_server), 0);
						double time_leave = ((pack->into_server).tv_sec - (pack->arrival).tv_sec) * pow(10, 6) + (pack->into_server).tv_usec - (pack->arrival).tv_usec;
						printf("%012.3fms: p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token\n", time_bucket/1000, pack->pack_num, time_leave/1000, current_bucket);
						pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
						TransferPacket(ptr, 0);
						pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
						struct timeval current;
						gettimeofday(&current, 0);
						printf("%012.3fms: p%d enters Q2\n", CalculateTime(&systime1, &current)/1000, pack->pack_num);
						pthread_cond_signal(&cond_server);
					}
				}	
				// pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
			 	pthread_cleanup_pop(0);
				pthread_mutex_unlock(&mutex1);			
				pthread_testcancel();
			}
			return NULL;
		}

		void *PacketThread(void *ptr){
			// printf("packet thread : %lx", packet);
			struct timeval prev, current;
			prev.tv_sec = systime1.tv_sec, prev.tv_usec = systime1.tv_usec; //obtain the system initiate time
			int count = 0;
			while(number_packet -- > 0){
				struct timeval prev_prev;
				prev_prev.tv_sec = prev.tv_sec, prev_prev.tv_usec = prev.tv_usec;
				double time_packet = SleepThread(&current, &prev, 1);	
				My402List *packet_queue = ((warmup2list*)ptr)->bucket;
				pthread_mutex_lock(&mutex1);
				// pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0 );
				pthread_cleanup_push(pthread_mutex_unlock, &mutex1);
				double interval = CalculateTime(&prev_prev, &prev);
				count ++;
				total_packet = count;
				if(token_need > bucket_size){
					printf("%012.3fms: p%d arrives, need %d tokens, inter-arrival time = %.3fms, dropped\n", time_packet/1000, count, token_need, interval/1000);
					fail_packet++;
				}else{
					printf("%012.3fms: p%d arrives, need %d tokens, inter-arrival time = %.3fms\n", time_packet/1000, count, token_need, interval/1000);
					Packet *pack = (Packet*)malloc(sizeof(Packet));
					(pack->arrival).tv_sec = prev.tv_sec, (pack->arrival).tv_usec = prev.tv_usec;
					pack->service_time = pow(10, 3)/mu;
					pack->pack_num = count;
					pack->num_token = token_need;
					pack->actual_interval = interval; //microsec
					bool isempty = My402ListEmpty(packet_queue);
					pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0 );
					My402ListAppend(packet_queue, pack);
					pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0 );
					gettimeofday(&(pack->enter_q1), 0);
					double enter_time = CalculateTime(&systime1, &(pack->enter_q1));
					printf("%012.3fms: p%d enters Q1\n", enter_time/1000, count);
					if(isempty){
						if(pack->num_token <= current_bucket){
							pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0 );
							TransferPacket(ptr, 0);
							pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0 );
							current_bucket -= pack->num_token;
							struct timeval leave_q1;
							gettimeofday(&leave_q1, 0);
							printf("%012.3fms: p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token\n", CalculateTime(&systime1, &leave_q1)/1000, pack->pack_num, CalculateTime(&(pack->enter_q1), &leave_q1)/1000, current_bucket);
							gettimeofday(&(pack->into_server), 0);
							double enter_q2 = CalculateTime(&systime1, &(pack->into_server));
							printf("%012.3fms: p%d enters Q2\n", enter_q2/1000, pack->pack_num);
							pthread_cond_signal(&cond_server);
						}
					}
				}	
				
			// pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0 );
				pthread_cleanup_pop(0);
				pthread_mutex_unlock(&mutex1);
				pthread_testcancel();
			}
			pause();
			return NULL;
		}

		Packet *CreatePacket(My402List *packet_queue, char *line, int m, bool *empty){
			bool isempty = My402ListEmpty(packet_queue);
			int count = 0;
			Packet *pack = (Packet*)malloc(sizeof(Packet));
			char *ptr = line, *end = line;
			if(!pack){
				printf("error opening file");
				exit(1);
			}
			memset(pack, 0, sizeof(Packet));
			pack->pack_num = m;
			while(*end != '\n'){
				if(count == 2){
					pack->service_time = atof(ptr);
					count ++;
					break;
				}
				if(*end == ' '){
					*end = '\0';
					switch(count){
						case 0:	pack->arrival_interval = atof(ptr);
						ptr = end + 1;
						break;
						case 1: pack->num_token = atoi(ptr);
						ptr = end + 1;
						break;
						case 2:	pack->service_time = atof(ptr);
						ptr = end + 1;
						break;
						default: printf("the format of this file is wrong");
						exit(1);
					}
					count++;
					while(!(*(end) >= '0' && *(end) <= '9'))
						end++;
				}
				end += 1;
			}
			if(count != 3){
				printf("the format of this file is wrong");
				exit(1);
			}	
			*empty = isempty;
			return pack;
		}
		
	void SleepThread1(struct timeval *prev, double interval){
			struct timeval current;
			struct timespec sleep_time;
			gettimeofday(&current, 0);
			double diff = CalculateTime(prev, &current);
		interval = interval * pow(10, 3); // microsec
		if(diff < interval){
			sleep_time.tv_sec = (int)((interval - diff)/pow(10, 6));
			sleep_time.tv_nsec = (((interval - diff)/pow(10, 6)) - (int)((interval - diff)/pow(10, 6))) * pow(10, 9);
			nanosleep(&sleep_time, 0);
		}
			gettimeofday(prev, 0);
	}


	void *PacketFileThread(void *ptr){
		int count, number;
		bool isempty = false;
		Packet *pack = NULL;
		struct timeval prev, prev_prev;
		prev.tv_sec = systime1.tv_sec, prev.tv_usec = systime1.tv_usec, prev_prev.tv_sec = systime1.tv_sec, prev_prev.tv_usec = systime1.tv_usec;
		double interval;

		FILE *file = fopen(filename, "r");
		My402List *packet_queue = ((warmup2list*)ptr)->bucket;
		pthread_cleanup_push(fclose, file);

		if(!file){
			printf("error opening file");
			exit(1);
		}
		
		char *packet_number = (char*)malloc(sizeof(char) * LineLength);

		if(!packet_number){
			printf("error allocating memory");
			exit(1);
		}

		memset(packet_number, 0, LineLength);
		packet_number = fgets(packet_number, LineLength, file);

		if(!packet_number || (number = atoi(packet_number)) <= 0){
			printf("the format of this file is wrong");
			exit(1);
		}

		free(packet_number);
		pthread_testcancel();
		pthread_cleanup_pop(0);
		while(1){
			while(number-- > 0){
				count++;
				total_packet = count;
				pthread_cleanup_push(fclose, file);
				pthread_mutex_lock(&mutex1);
				pthread_cleanup_push(pthread_mutex_unlock, &mutex1);
				char *line = (char*)malloc(sizeof(char) * LineLength);

				if(!line){
					printf("error allocating memory");
					exit(1);
				}

				memset(line, 0, LineLength);
				line = fgets(line, LineLength, file); // read related data
				pthread_cleanup_push(free, pack);
				pack = CreatePacket(packet_queue, line, count, &isempty); // create the packet
				pthread_testcancel();
				pthread_cleanup_pop(0); 
				interval = pack->arrival_interval;
				pthread_cleanup_pop(0);	
				pthread_mutex_unlock(&mutex1);

				SleepThread1(&prev, interval); // sleep thread

				pthread_mutex_lock(&mutex1);
				pthread_cleanup_push(pthread_mutex_unlock, &mutex1);
				gettimeofday(&(pack->arrival), 0);
				double time_packet = CalculateTime(&systime1, &(pack->arrival));
				double interval_arri = CalculateTime(&prev_prev, &prev);
				pack->actual_interval = interval_arri;
				if(pack->num_token > bucket_size){		
					printf("%012.3fms: p%d arrives, need %d tokens, inter-arrival time = %.3fms, dropped\n", time_packet/1000, pack->pack_num, pack->num_token, interval_arri/1000);
					fail_packet++;
				}else{
					printf("%012.3fms: p%d arrives, need %d tokens, inter-arrival time = %.3fms\n", time_packet/1000, pack->pack_num, pack->num_token, interval_arri/1000);
					pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
					My402ListAppend(packet_queue, pack);
					pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
					pthread_testcancel();
					gettimeofday(&(pack->enter_q1), 0);
					if(isempty && current_bucket >= pack->num_token){
						current_bucket -= pack->num_token;
						pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);
						TransferPacket(ptr, 0);
						pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
						struct timeval leave_q1;
						gettimeofday(&(leave_q1), 0);
						time_packet = CalculateTime(&systime1, &(leave_q1));
						gettimeofday(&(pack->into_server), 0);
						double time_leave = CalculateTime(&(pack->enter_q1), &(pack->into_server));

						printf("%012.3fms: p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token\n", time_packet/1000, pack->pack_num, time_leave/1000, current_bucket);
						pthread_cond_signal(&cond_server);
					}
				}		
				pthread_cleanup_pop(0);		
				pthread_mutex_unlock(&mutex1);
				pthread_cleanup_pop(0);	
				if(number == 0){
					fclose(file);
					break;
				}
			}
			pause();
		}
	}




		void InitiateParemeter(int n, char *array[]){ 	
			char *parm[] = {"-lambda","-mu","-r","-B","-P","-n","-t"};
			// bool flag = false;
			// for(int k = 0; k < 7; k++){
			// 	for(int m = 1; m < n; m++){
			// 		if(!strcmp(parm[k], array[m]) ){
			// 			flag = true;
			// 			break;
			// 		}
			// 		if(!flag){
			// 		printf("please input correct parameter");
			// 		exit(1);
			// 	}
			// 	}

			// }
			for(int j = 0; j < 7;  j++){
				for (int i = 0; i < n; ++i){
					if(!(strcmp(array[i], parm[j]))){
						if(j != 6 && atof(array[i+1]) <= 0.0){
							printf("parameter must be positive integer");
							exit(1);
						}
						switch(j){
							case 0: lambda = atof(array[i+1]);
							break;
							case 1: mu = atof(array[i+1]);
							break;
							case 2: token = atof(array[i+1]);
							break;
							case 3: bucket_size = atoi(array[i+1]);
							break;
							case 4: token_need = atoi(array[i+1]);
							break;
							case 5: number_packet = atoi(array[i+1]);
							break;
							case 6: tfile = true;
							filename = (char*)malloc(sizeof(char)*100);
							filename = strncpy(filename, array[i+1], 100);
							if(!filename){
								printf("error");
								exit(1);
							}
							break;
							default: break;
						}
					}
				}
			}
			printf("Emulation Parameters:\n");
			printf("\tnumber to arrive = %d\n", number_packet);
			if(!tfile){
				printf("\tlambda = %.2f\n", lambda);
				printf("\tmu = %.2f\n", mu);
			}
			printf("\tr = %.2f\n", token);
			printf("\tB = %d\n", bucket_size);
			if(!tfile)
				printf("\tP = %d\n", token_need);
			if(tfile){
				printf("\ttsfile = %s\n", filename);
			}
		}

	void InitiateList(warmup2list *list){
		list->bucket = (My402List*)malloc(sizeof(My402List));
		list->server = (My402List*)malloc(sizeof(My402List));
		list->service = (My402List*)malloc(sizeof(My402List));
		My402ListInit(list->bucket);
		My402ListInit(list->server);
		My402ListInit(list->service);
	}

	void *CancelThread(void *ptr){
		int signum;
		// printf("cancel %lx", cancel);
		while(1){
			sigwait(&set, &signum);
			if(signum != SIGINT){
				printf("wrong interuppt");
				exit(1);
			}else{
				//pthread_mutex_lock(&mutex1);
				pthread_cancel(bucket);
				pthread_cancel(packet);
				pthread_cancel(server);
				pthread_join(bucket, 0);
				pthread_join(packet, 0);
				pthread_join(server, 0);
				gettimeofday(&current, 0);
				printf("%012.3fms: emulation ends\n", CalculateTime(&systime1, &current)/pow(10,3));
				printf("\n");
				printf("Statistics:\n");
				break;
			}
		}	
		return 0;
	}

	double gettime(My402List *list, int i){
		if(My402ListEmpty(list)) return 0;
		My402ListElem *anchor = &(list->anchor), *next = My402ListFirst(list);
		Packet *pack = NULL;
		double sum = 0;
		while(next != anchor){
			pack = (Packet*)next->obj;
			if(i == 0)
				sum += pack->actual_interval;
			else sum += pack->actual_servicetime;
			next = next->next; 
		}
		return sum;
	}

	void printinternal(warmup2list *list){
		int packet = 0;
		double total_interval = 0;
		My402List *q1 = list->bucket, *q2 = list->server, *q3 = list->service;
		packet = My402ListLength(q1) + My402ListLength(q2) + My402ListLength(q3);
		total_interval = gettime(q1, 0) + gettime(q2, 0) + gettime(q3, 0);
		printf("\taverage packet inter-arrival time = %.6g\n", total_interval/(pow(10, 6)*packet));
	}

	void printservicetime(warmup2list *list){
		if(My402ListEmpty(list->service)){
			printf("\taverage packet service time = %.6g\n\n",(double)0);
			return;
		}
		int num = My402ListLength(list->service);
		double time_service = gettime(list->service, 1);
		printf("\taverage packet service time = %.6g\n\n", time_service/(pow(10, 6) * num));
	}

	void arvnuminq1(warmup2list *list, struct timeval end){
		double totaltime = CalculateTime(&systime1, &end), numtime = 0;
		My402List *q1 = list->bucket;
		Packet *pack = NULL;
		if(!My402ListEmpty(q1)){
			My402ListElem *next = My402ListFirst(q1), *anchor = &(q1->anchor);
			while(next != anchor){
				pack = (Packet*) next->obj;
				numtime += CalculateTime(&(pack->enter_q1), &end);
				next = next->next;
			}
		}
		if(!My402ListEmpty(list->server)){
			My402ListElem *elem = My402ListFirst(list->server), *anchor = &(list->server->anchor);
			while(elem != anchor){
				pack = (Packet*) elem->obj;
				numtime += CalculateTime(&(pack->enter_q1), &(pack->into_server));
				elem = elem->next;
			}
		}
		if(!My402ListEmpty(list->service)){
			My402ListElem *elem = My402ListFirst(list->service), *anchor = &(list->service->anchor);
			while(elem != anchor){
				pack = (Packet*) elem->obj;
				numtime += CalculateTime(&(pack->enter_q1), &(pack->into_server));
				elem = elem->next;
			}
		}
		printf("\taverage number of packets in Q1 = %.6g\n", numtime/totaltime);
	}

	void arvnuminq2(warmup2list *list, struct timeval end){
		double totaltime = CalculateTime(&systime1, &end), numtime = 0;
		My402List *q2 = list->server;
		Packet *pack = NULL;
		if(!My402ListEmpty(q2)){
			My402ListElem *next = My402ListFirst(q2), *anchor = &(q2->anchor);
			while(next != anchor){
				pack = (Packet*) next->obj;
				numtime += CalculateTime(&(pack->into_server), &end);
				next = next->next;
			}
		}
		if(!My402ListEmpty(list->service)){
			My402ListElem *elem = My402ListFirst(list->service), *anchor = &(list->service->anchor);
			while(elem != anchor){
				pack = (Packet*) elem->obj;
				numtime += CalculateTime(&(pack->into_server), &(pack->leave_server));
				elem = elem->next;
			}
		}
		printf("\taverage number of packets in Q2 = %.6g\n", numtime/totaltime);
	}

	void arvnuminservice(warmup2list *list, struct timeval end){
		double totaltime = CalculateTime(&systime1, &end), numtime = 0;
		Packet *pack = NULL;
		if(!My402ListEmpty(list->service)){
			My402ListElem *elem = My402ListFirst(list->service), *anchor = &(list->service->anchor);
			while(elem != anchor){
				pack = (Packet*) elem->obj;
				numtime += pack->actual_servicetime;
				elem = elem->next;
			}
		}
		printf("\taverage number of packets at S = %.6g\n", numtime/totaltime);
	}

	void arvtimeinsystem(warmup2list *list, struct timeval end){
		int count = 0;
		// double totaltime = CalculateTime(&systime1, &current);
		double sys_time = 0;
		double sys_square = 0;
		My402List *array[] = {list->bucket, list->server};
		Packet *pack = NULL;
		// int length = sizeof(array);
		for(int i = 0; i < 2; i++){
			My402ListElem *anchor = &((array[i])->anchor);
			if(!My402ListEmpty(array[i])){
				My402ListElem *next = My402ListFirst(array[i]);
				while(next != anchor){
					count++;
					pack = (Packet*)next->obj;
					double diff = CalculateTime(&(pack->arrival), &end);
					sys_time += diff;
					sys_square += pow(diff, 2);
					next = next->next;
				}
			}
		}

		My402List *service = list->service;
		if(!My402ListEmpty(service)){
			My402ListElem *anchor = &(service->anchor), *next = My402ListFirst(service);
			while(next != anchor){
				count++;
				pack = (Packet*)next->obj;
				double diff = CalculateTime(&(pack->arrival), &(pack->end_service));
				sys_time += diff;
				sys_square += pow(diff, 2);
				next = next->next;
			}
		}
		printf("\n\taverage time a packet spent in system = %.6g\n", (double) sys_time/(count * pow(10, 6)));
		printf("\tstandard deviation for time spent in system = %.6g", (double)sys_square/pow(sys_time, 2));
	}

	int main(int argc, char *argv[]){
		warmup2list list;
		

		InitiateParemeter(argc, argv);
		sigemptyset(&set);
		sigaddset(&set, SIGINT);
		sigprocmask(SIG_BLOCK, &set, 0);
		InitiateList(&list);
		gettimeofday(&systime1,0);
		printf("\n00000000.000ms: emulation begins\n");
		pthread_create(&server, 0, ServerThread, &list);
		pthread_create(&bucket, 0, BucketThread, &list);
		pthread_create(&cancel, 0, CancelThread, &list);
		if(tfile){
			pthread_create(&packet, 0, PacketFileThread, &list);
		}else{
			pthread_create(&packet, 0, PacketThread, &list);	
		}
		// pthread_join(packet,0);
		// pthread_join(server,0);
		// pthread_join(server,0);
		pthread_join(cancel, 0);
		
		printinternal(&list);
		printservicetime(&list);
		arvnuminq1(&list, current);
		arvnuminq2(&list, current);
		arvnuminservice(&list, current);
		arvtimeinsystem(&list, current);
		printf("\n\n\ttoken drop probability  = %.6g\n", (double)fail_token/ total_token);
		printf("\tpacket drop probability = %.6g\n", (double)fail_packet/total_packet);

		My402ListUnlinkAll(list.bucket);
		My402ListUnlinkAll(list.server);
		My402ListUnlinkAll(list.service);
		// free(&list);
		return 0;
	}




