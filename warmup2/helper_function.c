
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mylist.h"
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include "warmup2.h"

float calculate_time_difference(struct timeval time1, struct timeval time2){
    long seconds = time1.tv_sec - time2.tv_sec;
    long useconds = time1.tv_usec - time2.tv_usec;
    if(useconds < 0){
        seconds = seconds - 1;
        useconds = useconds + 1000000;
    }
    long total = (seconds * 1000000) + useconds;
    float f = total / 1000.00;
    //printf("\n%ld", total);
    //printf("\n%.3fms",f);
    return f;
}

void digest_file(FILE * fp){
    char buf[1026];
    long num_of_packets = 0;
    if(fgets(buf, sizeof(buf), fp) != NULL){
        num_of_packets = atol((buf));
    }
    total_number_of_packets = num_of_packets;
    tsfile_data = (long **)malloc(num_of_packets * sizeof(long *));
    long packet_count = 0;
    while(fgets(buf, sizeof(buf), fp) != NULL) {

        if(buf[0] == ' ' || buf[0] == '\t' || buf[strlen(buf)-2] == ' ' || buf[strlen(buf)-2] == '\t'){
            fprintf(stderr,"Trailing space or tab found in a line of the file\n");
            exit(1);
        }
        long * record = NULL;
        record = (long *) malloc(3 * sizeof(long));
        int j=0;
        char *start_ptr = buf,*tab_ptr = NULL;
        int i=0;
        for(i=0;i<2;i++){
            tab_ptr = strchr(start_ptr, '\t');
            if(tab_ptr == NULL){
                tab_ptr = strchr(start_ptr, ' ');
            }
            if (tab_ptr != NULL) {
                *tab_ptr++ = '\0';
                while(*tab_ptr == ' ' || *tab_ptr == '\t')tab_ptr++;
                *(record + j) = atol(start_ptr);
                j++;
            }
            start_ptr = tab_ptr;
        }
        *(record + j) = atol(tab_ptr);
        *(tsfile_data + packet_count)  = record;
        packet_count++;
    }
}
