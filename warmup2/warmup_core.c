#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mylist.h"
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include "warmup2.h"
#include <signal.h>
#include <math.h>

My402List Q1;
My402List Q2;

/* Emulation related data */
long **tsfile_data;
long total_number_of_packets;
long bucket_capacity;
long tokens_needed_for_a_packet;
float token_arrival_time;
float packet_arrival_time;
float packet_service_time;
float token_arrival_time;

int is_trace_driven_mode;
int server_thread_stop;
int packet_arrival_thread_stop;
int cntrl_c_signal;
long number_of_tokens_in_bucket;
long number_of_packets_put_in_Q1;

/* Statistics related data */
float average_packet_inter_arrival_time;
float average_packet_service_time;
float average_number_of_packets_in_Q1;
float average_number_of_packets_in_Q2;
float average_number_of_packets_at_S;
float average_time_a_packet_spent_in_system;
float standard_deviation_for_time_spent_in_system;
float token_drop_probability;
float packet_drop_probability;

sigset_t set;
struct timeval emulation_start_time;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_variable = PTHREAD_COND_INITIALIZER;

pthread_t packet_arrival_thread_id;
pthread_t token_depositing_thread_id;
pthread_t server_thread_id;
pthread_t monitor_thread_id;

void process(float lambda, float mu, float r, long B, long P, long num, char * tsfile){
    
    initialize();
    
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, 0);
    
    if(tsfile != NULL){
        is_trace_driven_mode = 1;
        bucket_capacity = B;
        token_arrival_time = 1/r;
        
        if(token_arrival_time > 10) token_arrival_time = 10.0;
    }
    else {
        packet_arrival_time = 1 / lambda;
        packet_service_time = 1 / mu;
        token_arrival_time = 1 / r;
        total_number_of_packets = num;
        bucket_capacity = B;
        tokens_needed_for_a_packet = P;
        
        if(packet_arrival_time > 10) packet_arrival_time = 10.0;
        if(packet_service_time > 10) packet_service_time = 10.0;
        if(token_arrival_time > 10) token_arrival_time = 10.0;
    }
    
    if(is_trace_driven_mode){
        FILE *fp=NULL;
        fp = fopen(tsfile, "r");
        if (fp == NULL) {
            fprintf(stderr, "\nThe input file %s cannot be opened for reading or it doesn't exist.\n", tsfile);
            exit(1);
        }
        
        digest_file(fp);
    }
    
    printf("Emulation Parameters:\n");
    printf("    number to arrive = %ld\n",total_number_of_packets);
    if(!is_trace_driven_mode){
        printf("    lambda = %f\n",lambda);
    }
    if(!is_trace_driven_mode){
        printf("    mu = %f\n",mu);
    }
    printf("    r = %f\n",r);
    printf("    B = %ld\n",B);
    if(!is_trace_driven_mode){
        printf("    P = %ld\n\n",P);
    }
    if(is_trace_driven_mode){
        printf("    tsfile = %s\n\n",tsfile);
    }
    
    gettimeofday(&emulation_start_time, NULL);
    printf("00000000.000ms: emulation begins\n");
    
    pthread_create(&packet_arrival_thread_id, 0, packet_arrival_thread_function, 0);
    pthread_create(&token_depositing_thread_id, 0, token_depositing_thread_function, 0);
    pthread_create(&server_thread_id, 0, server_thread_function, 0);
    pthread_create(&monitor_thread_id, 0, monitor, 0);
    
    pthread_join(packet_arrival_thread_id, NULL);
    pthread_join(token_depositing_thread_id, NULL);
    pthread_join(server_thread_id, NULL);
    
    display_statistics();
    
}
