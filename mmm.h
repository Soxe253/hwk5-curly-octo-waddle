#ifndef MMM_H_
#define MMM_H_

// shared globals
extern unsigned int mode;
extern unsigned int size, num_threads;
extern double **A, **B, **SEQ_MATRIX, **PAR_MATRIX;

void mmm_init();
void mmm_reset(double **);
void mmm_freeup();
void mmm_seq();
void* mmm_par(void *);
double mmm_verify();
void sigmaMult(int i, int j, double ** matrix);

typedef struct t_args{
    int id;
    int startRow;
    int endRow;
} t_args;

#endif /* MMM_H_ */
