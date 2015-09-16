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

void * token_depositing_thread_function(void *args){
    pthread_sigmask(SIG_BLOCK, &set, 0);
    long local_token_created_count = 0;
    long local_token_dropped_count = 0;
    float local_total_packet_time_in_Q1 = 0.0;
    int stop_flag = 0;
    struct timeval current_time;
    
    while(1){
        
        
        /* Sleep for needed time */
        usleep(token_arrival_time * 1000000.0);
        
        /* Lock the mutex */
        pthread_mutex_lock(&mutex);
        
        if(cntrl_c_signal){
            stop_flag = 1;
        }
        else{
            local_token_created_count++;
            
            /* Increment token count */
            if(number_of_tokens_in_bucket < bucket_capacity){
                
                number_of_tokens_in_bucket++;
                
                gettimeofday(&current_time, NULL);
                printf("%012.3fms: token t%ld arrives, token bucket now has %ld tokens\n",calculate_time_difference(current_time,emulation_start_time),local_token_created_count,number_of_tokens_in_bucket);
            }
            else {
                
                local_token_dropped_count++;
                
                gettimeofday(&current_time, NULL);
                printf("%012.3fms: token t%ld arrives, dropped\n",calculate_time_difference(current_time,emulation_start_time),local_token_created_count);
            }
            
            if(!My402ListEmpty(&Q1)){
                My402ListElem * temp_arrival_thread_element = My402ListLast(&Q1);
                struct packet * temp_arrival_thread_packet = (struct packet *)(temp_arrival_thread_element->obj);
                
                /* Move the first packet to q2 if there are enough tokens */
                if(number_of_tokens_in_bucket >= temp_arrival_thread_packet->tokens_needed){
                    
                    int is_empty = My402ListEmpty(&Q2);
                    
                    number_of_tokens_in_bucket = number_of_tokens_in_bucket - temp_arrival_thread_packet->tokens_needed;
                    
                    My402ListUnlink(&Q1,temp_arrival_thread_element);
                    
                    gettimeofday(&current_time, NULL);
                    printf("%012.3fms: p%ld leaves Q1, time in Q1 = %.3fms, token bucket now has %ld token\n",calculate_time_difference(current_time,emulation_start_time),temp_arrival_thread_packet->id,calculate_time_difference(current_time, temp_arrival_thread_packet->updation_time),number_of_tokens_in_bucket);
                    
                    local_total_packet_time_in_Q1 = local_total_packet_time_in_Q1 + calculate_time_difference(current_time, temp_arrival_thread_packet->updation_time);
                    
                    gettimeofday(&temp_arrival_thread_packet->updation_time, NULL);
                    My402ListPrepend(&Q2, temp_arrival_thread_packet);
                    
                    gettimeofday(&current_time, NULL);
                    printf("%012.3fms: p%ld enters Q2\n",calculate_time_difference(current_time,emulation_start_time),temp_arrival_thread_packet->id);
                    
                    if(is_empty){
                        pthread_cond_broadcast(&condition_variable);
                    }
                    
                }
                temp_arrival_thread_element = NULL;
                temp_arrival_thread_packet = NULL;
            }
            else {
                if(packet_arrival_thread_stop){
                    stop_flag = 1;
                }
            }
            if(server_thread_stop){
                stop_flag = 1;
            }
        }
        /* Unloack the mutex */
        pthread_mutex_unlock(&mutex);
        if(stop_flag){
            pthread_mutex_lock(&mutex);
            
            if(local_total_packet_time_in_Q1 != 0.0){
                if(average_number_of_packets_in_Q1 == -1.0){
                    average_number_of_packets_in_Q1 = 0.0;
                }
                average_number_of_packets_in_Q1 = average_number_of_packets_in_Q1 + local_total_packet_time_in_Q1;
            }
            average_number_of_packets_in_Q1 = average_number_of_packets_in_Q1 + local_total_packet_time_in_Q1;
            if(local_token_created_count !=0){
                token_drop_probability = (float)local_token_dropped_count / (float)local_token_created_count;
            }
            if(packet_arrival_thread_stop && !server_thread_stop){
                pthread_cond_broadcast(&condition_variable);
            }
            pthread_mutex_unlock(&mutex);
            break;
        }
    }
    pthread_exit(0);
    return NULL;
}
