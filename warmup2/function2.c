void *PacketThread(void *ptr){
	struct timeval prev, current;
	prev.tv_sec = systime1.tv_sec;
	prev.tv_usec = systime1.tv_usec;
	while(number_bucket-- > 0){	
	gettimeofday(&current);
	long sec = 0L, microsec = 0L;
	sec = current.tv_sec - prev.tv_sec;
	microsec = (current.tv_usec - prev.tv_usec);
	long diff = (long)(mu*pow10(6) - sec*pow10(6)-microsec);
	if(diff > = 0){
		nanosleep(diff*pow10(3));
	}
	gettimeofday(&prev);
	double time_packet = (double)(prev.tv_sec - current.tv_sec)*pow10(6) + (prev.tv_usec - current.tv_usec);
	printf("%12.3fms", time_packet);
    }
}