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

void * packet_arrival_thread_function(void * args){
    pthread_sigmask(SIG_BLOCK, &set, 0);
    long local_packet_created_count = 0;
    long local_packet_added_count = 0;
    long local_packet_dropped_count = 0;
    float local_packet_arrival_time = 0.0;
    long local_tokens_needed_for_a_packet = 0;
    float local_packet_service_time = 0.0;
    float local_total_packet_arrival_time = 0.0;
    float local_total_packet_time_in_Q1 = 0.0;
    
    local_packet_arrival_time = packet_arrival_time;
    local_tokens_needed_for_a_packet = tokens_needed_for_a_packet;
    local_packet_service_time = packet_service_time;
    
    struct timeval previous_time, current_time;
    struct packet *local_arrival_thread_packet;
    
    int stop_flag = 0;
    
    while(1){
        
        if(is_trace_driven_mode){
            local_packet_arrival_time = **(tsfile_data + local_packet_created_count) / 1000.0;
            local_tokens_needed_for_a_packet = *(*(tsfile_data + local_packet_created_count) + 1);
            local_packet_service_time = *(*(tsfile_data + local_packet_created_count) + 2) / 1000.0;
        }
        
        gettimeofday(&previous_time, NULL);
        usleep(local_packet_arrival_time * 1000000.0);
        gettimeofday(&current_time, NULL);
        
        
        local_total_packet_arrival_time = local_total_packet_arrival_time + calculate_time_difference(current_time,previous_time);
        
        local_packet_created_count++;
        
        /* Create a new packet */
        local_arrival_thread_packet = malloc(sizeof(struct packet));
        local_arrival_thread_packet->id = local_packet_created_count;
        local_arrival_thread_packet->packet_arrival_time = local_packet_arrival_time;
        local_arrival_thread_packet->packet_service_time = local_packet_service_time;
        local_arrival_thread_packet->tokens_needed = local_tokens_needed_for_a_packet;
        gettimeofday(&local_arrival_thread_packet->creation_time, NULL);
        gettimeofday(&local_arrival_thread_packet->updation_time, NULL);
        
        
        /* Lock the mutex */
        pthread_mutex_lock(&mutex);
        
        if(cntrl_c_signal){
            stop_flag = 1;
            
        }
        else{
            
            
            if(local_tokens_needed_for_a_packet > bucket_capacity){
                
                printf("%012.3fms: p%ld arrives, needs %ld tokens, inter-arrival time = %.3fms, dropped\n",calculate_time_difference(current_time,emulation_start_time),local_packet_created_count,local_tokens_needed_for_a_packet,calculate_time_difference(current_time,previous_time));
                
                local_packet_dropped_count++;
                free(local_arrival_thread_packet);
            }
            else {
                
                printf("%012.3fms: p%ld arrives, needs %ld tokens, inter-arrival time = %.3fms\n",calculate_time_difference(current_time,emulation_start_time),local_packet_created_count,local_tokens_needed_for_a_packet,calculate_time_difference(current_time,previous_time));
                
                gettimeofday(&previous_time, NULL);
                
                /* Add the packet to Q1 */
                My402ListPrepend(&Q1,local_arrival_thread_packet);
                
                gettimeofday(&current_time, NULL);
                printf("%012.3fms: p%ld enters Q1\n",calculate_time_difference(current_time,emulation_start_time),local_packet_created_count);
                
                local_packet_added_count++;
            }
            
            local_arrival_thread_packet = NULL;
            
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
        }
        pthread_mutex_unlock(&mutex);
        if(stop_flag || (local_packet_created_count == total_number_of_packets)){
            pthread_mutex_lock(&mutex);
            number_of_packets_put_in_Q1 = local_packet_added_count;
            if(local_total_packet_time_in_Q1 != 0.0){
                if(average_number_of_packets_in_Q1 == -1.0){
                    average_number_of_packets_in_Q1 = 0.0;
                }
                average_number_of_packets_in_Q1 = average_number_of_packets_in_Q1 + local_total_packet_time_in_Q1;
            }
            if(local_packet_created_count !=0){
                average_packet_inter_arrival_time = local_total_packet_arrival_time / (float)local_packet_created_count;
                packet_drop_probability = (float)local_packet_dropped_count / (float)local_packet_created_count;
            }
            packet_arrival_thread_stop = 1;
            pthread_mutex_unlock(&mutex);
            break;
            
        }
        
    }
    pthread_exit(0);
    return NULL;
}
