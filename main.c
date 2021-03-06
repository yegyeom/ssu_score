#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "ssu_score.h"

#define SECOND_TO_MICRO 1000000

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t);
//시작시간, 끝나는 시간
int main(int argc, char *argv[])
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL); //시간 측정 시작

	ssu_score(argc, argv);

	gettimeofday(&end_t, NULL); //시간 측정 끝
 	ssu_runtime(&begin_t, &end_t);

	exit(0);
}

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t)
{
	end_t->tv_sec -= begin_t->tv_sec; 

	if(end_t->tv_usec < begin_t->tv_usec){ //시작 시간의 수가 더 크면
		end_t->tv_sec--; //end_t->tv_sec 감소
		end_t->tv_usec += SECOND_TO_MICRO; 
	}

	end_t->tv_usec -= begin_t->tv_usec; 
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_t->tv_sec, end_t->tv_usec); //microsecond
}
