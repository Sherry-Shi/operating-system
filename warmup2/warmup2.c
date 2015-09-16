#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mylist.h"
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include "token.h"

int main(int argc, char *argv[])
{
    float lambda=0.5,mu=0.35,r=1.5;
    long B=10,P=3,num=20;
    char * tsfile = NULL;
    int i=1;
    for(i=1;i<argc;i++){
        if(strcmp("-lambda", argv[i])==0){
            if(i!=argc && strstr(argv[i+1],"-")==NULL){
                lambda = atof(argv[i+1]);
            }
            else{
                fprintf(stderr,"Please provide a valid value with -lambda argument, else remove -lambda to use default value\n");
                exit(1);
            }
        }
        if(strcmp("-mu", argv[i])==0){
            if(i!=argc && strstr(argv[i+1],"-")==NULL){
                mu = atof(argv[i+1]);
            }
            else{
                fprintf(stderr,"Please provide a valid value with -mu argument, else remove -mu to use default value\n");
                exit(1);
            }
        }
        if(strcmp("-r", argv[i])==0){
           if(i!=argc && strstr(argv[i+1],"-")==NULL){
               r = atof(argv[i+1]);
           }
           else{
               fprintf(stderr,"Please provide a valid value with -r argument, else remove -r to use default value\n");
               exit(1);
           }
        }
        if(strcmp("-B", argv[i])==0){
            if(i!=argc && strstr(argv[i+1],"-")==NULL){
                B = atol(argv[i+1]);
            }
            else{
                fprintf(stderr,"Please provide a valid value with -B argument, else remove -B to use default value\n");
                exit(1);
            }
        }
        if(strcmp("-P", argv[i])==0){
            if(i!=argc && strstr(argv[i+1],"-")==NULL){
            P = atof(argv[i+1]);
            }
            else{
                fprintf(stderr,"Please provide a valid value with -P argument, else remove -P to use default value\n");
                exit(1);
            }
        }
        if(strcmp("-n", argv[i])==0){
            if(i!=argc && strstr(argv[i+1],"-")==NULL){
                num = atof(argv[i+1]);
            }
            else{
                fprintf(stderr,"Please provide a valid value with -num argument, else remove -num to use default value\n");
                exit(1);
            }
        }
        if(strcmp("-t", argv[i])==0){
           if(i!=argc){
               tsfile = malloc(sizeof(argv[i+1]));
               strcpy(tsfile,argv[i+1]);
           }
           else{
               fprintf(stderr,"Please provide a file name with -t argument");
               exit(1);
           }
        }
    }
    process(lambda,mu,r,B,P,num,tsfile);
    return 0;
}





