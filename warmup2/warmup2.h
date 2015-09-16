#ifndef _WARMUP2_H_
#define _WARMUP2_H_

struct packet{
    long id;
    struct timeval updation_time;
    struct timeval creation_time;
    float packet_arrival_time;
    long tokens_needed;
    float packet_service_time;
};

extern My402List Q1;
extern My402List Q2;
extern long **tsfile_data;
extern long total_number_of_packets;
extern long bucket_capacity;
extern long tokens_needed_for_a_packet;
extern float token_arrival_time;
extern float packet_arrival_time;
extern float packet_service_time;
extern float token_arrival_time;
extern int is_trace_driven_mode;
extern int server_thread_stop;
extern int packet_arrival_thread_stop;
extern int cntrl_c_signal;
extern long number_of_tokens_in_bucket;
extern long number_of_packets_put_in_Q1;
extern float average_packet_inter_arrival_time;
extern float average_packet_service_time;
extern float average_number_of_packets_in_Q1;
extern float average_number_of_packets_in_Q2;
extern float average_number_of_packets_at_S;
extern float average_time_a_packet_spent_in_system;
extern float standard_deviation_for_time_spent_in_system;
extern float token_drop_probability;
extern float packet_drop_probability;
extern sigset_t set;
extern struct timeval emulation_start_time;
extern pthread_mutex_t mutex;
extern pthread_cond_t condition_variable;
extern pthread_t packet_arrival_thread_id;
extern pthread_t token_depositing_thread_id;
extern pthread_t server_thread_id;
extern pthread_t monitor_thread_id;

extern void initialize();
extern void digest_file(FILE *);
extern float calculate_time_difference(struct timeval, struct timeval);
extern void process(float, float, float, long, long, long, char *);
extern void * packet_arrival_thread_function(void *);
extern void * token_depositing_thread_function(void *);
extern void * server_thread_function(void *);
extern void * monitor(void * args);
extern void display_statistics();

#endif
