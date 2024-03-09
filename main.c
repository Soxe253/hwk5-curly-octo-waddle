#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "rtclock.h"
#include "mmm.h"

// shared  globals
unsigned int mode;
unsigned int size, num_threads;
double **A, **B, **SEQ_MATRIX, **PAR_MATRIX;
int NUM_RUNS = 4;

int main(int argc, char *argv[]) {

	// TODO - deal with command line arguments, save the "mode"
	// "size" and "num threads" into globals so threads can see them
	int check = 0;
	int checkThreads = 0;
	if(argc == 3){
		mode = 0; // SEQUENTIAL MODE
		check = strtol(argv[2], NULL, 10);
		
	}
	else if(argc == 4){
		mode = 1; //PARALLEL MODE
		checkThreads = strtol(argv[2], NULL, 10);
		check = strtol(argv[3], NULL, 10);
		
	}
	if(check < 1){
		printf("size cannot be less than 1\n");
		return 0;
	}
	size = check;
	num_threads = checkThreads;
	if(num_threads > size){//make sure threads are good
		printf("the number of threads must be greater than 0 and less than the size of the matricies\n");
		return 0;
		}
	// initialize my matrices
	mmm_init();
	double clockstart, clockend;

	// << stuff I want to clock here >>
	if(mode == 0){
		double seq_time = 0.0;
		for(int i = 0; i < NUM_RUNS; i++){//number of runs
			clockstart = rtclock();
			mmm_seq();
			clockend = rtclock();
			if(i > 0){
			seq_time += (clockend - clockstart);//calc time
			}
		}
		seq_time = (seq_time / (NUM_RUNS - 1));//calc avg
		printf("=======\nmode: Sequential\nthread count: 1\nsize: %d\n=======\n", size);
		printf("Sequential Time (%d run avg): %lf sec\n", (NUM_RUNS -1), seq_time);

	}
	else if(mode == 1){
		double seq_time = 0.0;
		double par_time = 0.0;
		for(int i = 0; i < NUM_RUNS; i++){
		//parallel stuff
		clockstart = rtclock();
		t_args *args = (t_args*) malloc(num_threads * sizeof(t_args));//memory
		int rate = (size / num_threads);
		for(int i = 0; i < num_threads; i++){//set the working rows
			args[i].id = i;
			args[i].startRow = (i * rate);
			args[i].endRow = (((i+1) * rate) - 1);
		}
		if(size % num_threads != 0){//distribute the rest of the work if uneven
			int remainingWork = size % num_threads;
			for(int i = num_threads - 1; remainingWork > 0; i--){
				args[i].startRow += remainingWork - 1;
				args[i].endRow += remainingWork;
				remainingWork--;
			}
		}		 
		pthread_t *threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
		for(int i = 0; i < num_threads; i++){//memory and launch each thread
			pthread_create(&threads[i], NULL, mmm_par, &args[i]);
		}

		for(int i = 0; i < num_threads; i++){//collect threads
			pthread_join(threads[i], NULL);
		}
		clockend = rtclock(); // stop the clock
		if(i > 0){//throw first run
		par_time += (clockend - clockstart);
		}
		//sequential 
		clockstart = rtclock();
		mmm_seq();
		clockend = rtclock();
		if(i > 0){//throw away first run
		seq_time += (clockend - clockstart);
		}


		free(args);
		args = NULL;
		free(threads);//return memory and reset matricies
		threads = NULL;
		mmm_reset(SEQ_MATRIX);
		mmm_reset(PAR_MATRIX);
		}
		seq_time = seq_time / (NUM_RUNS - 1);//avg times
		par_time = par_time / (NUM_RUNS - 1);
		
		printf("=======\nmode: parallel\nthread count: %d\nsize: %d\n=======\n", num_threads, size);
		printf("Sequential Time (%d run avg): %lf sec\n",(NUM_RUNS -1), seq_time);
		printf("Parallel time (%d run avg): %lf sec\n", (NUM_RUNS -1), par_time);
		printf("Speedup: %lf\n", seq_time / par_time);
		printf("Verifying... largest error is %lf\n", mmm_verify());
	}

	mmm_freeup();//give everything back
	return 0;
}
