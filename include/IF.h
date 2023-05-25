#ifndef IF_H
#define IF_H
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#define ITER 4


//functions
//'public'
double* IFalg(double* values, size_t len, size_t amountSensors);

//'private'
void _multiply_matrices(double *m1,size_t rows1, size_t cols1, 
                      double *m2, double*res);
double* _divide_matrix(double* matrix, size_t rows, size_t cols, double scalar);
double _euclidean_distance(double *x, double *r, size_t sensorID,size_t rows, size_t amountCols);
double* _calculate_r(double *r, double *x, double* weigth);
void _calculate_d(double *ret, double* x, double* r);
void _calculate_w(double *w, double *d);
#endif