#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mylist.h"
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include "token.h"
#include <signal.h>
#include <math.h>

void * monitor(void * args){
    sigwait(&set);
    pthread_mutex_lock(&mutex);
    struct timeval current_time;
    while(!My402ListEmpty(&Q1)){
        My402ListElem * temp_element = My402ListLast(&Q1);
        My402ListUnlink(&Q1,temp_element);
        struct packet * temp_packet = (struct packet *)(temp_element->obj);
        gettimeofday(&current_time, NULL);
        printf("%012.3fms: packet p%ld removed from Q1\n",calculate_time_difference(current_time,emulation_start_time),temp_packet->id);
        free(temp_packet);
        temp_element = NULL;
        temp_packet = NULL;
    }
    while(!My402ListEmpty(&Q2)){
        My402ListElem * temp_element = My402ListLast(&Q2);
        My402ListUnlink(&Q2,temp_element);
        struct packet * temp_packet = (struct packet *)(temp_element->obj);
        gettimeofday(&current_time, NULL);
        printf("%012.3fms: packet p%ld removed from Q2\n",calculate_time_difference(current_time,emulation_start_time),temp_packet->id);
        free(temp_packet);
        temp_element = NULL;
        temp_packet = NULL;
    }
    cntrl_c_signal = 1;
    pthread_cond_broadcast(&condition_variable);
    pthread_mutex_unlock(&mutex);
    return NULL;
}
