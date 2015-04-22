#ifndef _WARMUP2_H_
#define _WARMUP2_H_

#include "my402list.h"

typedef struct {
		int num_token;
		double arrival_interval;
			struct timeval arrival;
			struct timeval enter_q1;
				struct timeval into_server;
					struct timeval leave_server;
					struct timeval end_service;
					int pack_num;
					double service_time; 
		double actual_interval;
		double actual_servicetime;

}Packet;


typedef struct{
	My402List *bucket;
	My402List *server;
	My402List *service;
}warmup2list;


#endif
