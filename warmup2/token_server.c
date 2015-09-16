
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

void * server_thread_function(void *args){
    pthread_sigmask(SIG_BLOCK, &set, 0);
    float local_total_packet_time_in_Q2 = 0.0;
    float local_packet_service_time = packet_service_time;
    float local_total_packet_service_time = 0.0;
    float local_total_time_spent_in_system = 0.0;
    float local_total_time_spent_in_system_squared = 0.0;
    float local_total_packet_time_in_S = 0.0;
    long local_total_packet_counter = 0;
    struct timeval previous_time, current_time;
    while(1){
        
        /* Lock the mutex */
        pthread_mutex_lock(&mutex);
        /* wait for the queue-not-empty condition to be signaled */
        while(My402ListEmpty(&Q2) && !cntrl_c_signal){
            pthread_cond_wait(&condition_variable, &mutex);
            if(packet_arrival_thread_stop){
                break;
            }
            
        }
        
        if(!My402ListEmpty(&Q2)){
            My402ListElem * temp_server_thread_element = My402ListLast(&Q2);
            My402ListUnlink(&Q2, temp_server_thread_element);
            struct packet * temp_server_thread_packet = (struct packet *)(temp_server_thread_element->obj);
            
            if(is_trace_driven_mode){
                local_packet_service_time = temp_server_thread_packet->packet_service_time;
            }
            gettimeofday(&current_time, NULL);
            printf("%012.3fms: p%ld leaves Q2, time in Q2 = %.3fms, begin service at S\n",calculate_time_difference(current_time,emulation_start_time),temp_server_thread_packet->id,calculate_time_difference(current_time, temp_server_thread_packet->updation_time));
            
            local_total_packet_time_in_Q2 = local_total_packet_time_in_Q2 + calculate_time_difference(current_time, temp_server_thread_packet->updation_time);
            
            gettimeofday(&temp_server_thread_packet->updation_time, NULL);
            
            /* Unlock the mutex */
            pthread_mutex_unlock(&mutex);
            
            gettimeofday(&previous_time, NULL);
            
            /* Sleep for needed service time */
            usleep(local_packet_service_time * 1000000.0);
            
            gettimeofday(&current_time, NULL);
            
            printf("%012.3fms: p%ld departs from S, service time = %.3fms, time in system = %.3fms\n",calculate_time_difference(current_time,emulation_start_time),temp_server_thread_packet->id,calculate_time_difference(current_time, previous_time),calculate_time_difference(current_time, temp_server_thread_packet->creation_time));
            
            local_total_packet_service_time = local_total_packet_service_time + calculate_time_difference(current_time, previous_time);
            
            local_total_packet_time_in_S = local_total_packet_time_in_S + calculate_time_difference(current_time, previous_time);
            
            float temp = calculate_time_difference(current_time, temp_server_thread_packet->creation_time);
            
            local_total_time_spent_in_system = local_total_time_spent_in_system + temp;
            
            local_total_time_spent_in_system_squared = local_total_time_spent_in_system_squared + (temp * temp);
            
            free(temp_server_thread_packet);
            local_total_packet_counter++;
        }
        else{
            pthread_mutex_unlock(&mutex);
        }
        //Check this condition
        if(cntrl_c_signal || (local_total_packet_counter == number_of_packets_put_in_Q1 && packet_arrival_thread_stop)){
            pthread_mutex_lock(&mutex);
            if(local_total_packet_counter !=0){
                average_packet_service_time = local_total_packet_service_time / (float)local_total_packet_counter;
                average_number_of_packets_in_Q2 = local_total_packet_time_in_Q2;
                average_number_of_packets_at_S = local_total_packet_time_in_S;
                average_time_a_packet_spent_in_system = local_total_time_spent_in_system / (float)local_total_packet_counter;
                local_total_time_spent_in_system_squared = local_total_time_spent_in_system_squared / (float)local_total_packet_counter;
                standard_deviation_for_time_spent_in_system = sqrtf(local_total_time_spent_in_system_squared - ( average_time_a_packet_spent_in_system * average_time_a_packet_spent_in_system));
            }
            server_thread_stop = 1;
            pthread_mutex_unlock(&mutex);
            break;
        }
    }
    pthread_exit(0);
    return NULL;
}
