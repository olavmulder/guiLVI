#include "../include/IF.h"

static size_t SENSORS = 0;
static size_t READINGS = 0;
double* IFalg(double* values, size_t len, size_t amountSensors)
{

    size_t l = 0;
    READINGS = len;
    SENSORS = amountSensors;
    double *r;//array of reputation
    double *x;
    double *d;
    double weight[SENSORS];
    //init all weight as 1
    for(size_t i = 0;i<SENSORS;i++)
    {
        weight[i] = 1;
    }

    x = (double*)malloc(SENSORS*READINGS * sizeof *x);
    d = (double*)malloc(SENSORS*READINGS * sizeof *d);
    r = (double*)malloc(READINGS         * sizeof (double));
    //init all values in x array
    for(size_t i = 0; i < READINGS; i++)
    {
        for(size_t j = 0; j < SENSORS; j++)
            x[i*SENSORS+j] = values[i*SENSORS +j];
    }
    do{
        r = _calculate_r(r, x, weight);
        for(size_t i = 0; i< READINGS; i++){
           // printf("aggregate value l:%d  t%d= %.4f \n",l, i, r[i]);

        }
        _calculate_d(d, x, r);
        _calculate_w(weight, d);
        l++;
    }while(l < ITER);
    free(x);
    free(d);
    return r;
}
void _multiply_matrices(double *m1,size_t rows1, size_t cols1, 
                      double *m2, double*res)
{
    for (size_t i = 0; i < rows1; i++) {//readings
        res[i] = 0;
        for (size_t j = 0; j < cols1; j++) {//sensoren
        //printf("  weigth m2[%d] = %.2f\n", j , m2[j]);
            res[i] += m1[i*cols1+j] * m2[j];
        }
    }
}

double* _divide_matrix(double* matrix, size_t rows, size_t cols, double scalar) {
    double* result = (double*)malloc(rows * cols* sizeof(double*));
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            result[i * cols+ j] = matrix[i*cols+j] / scalar;
        }
    }
    return result;
}


double _euclidean_distance(double *x, double *r, size_t sensorID,size_t rows, size_t amountCols) {
    double distance = 0.0;

    for (size_t i = 0; i < rows; i++) {//rows is readings
        distance += pow(r[i] - x[sensorID+amountCols*i], 2);
    }
    return sqrt(distance);
}

//return double array
double* _calculate_r(double *r, double *x, double* weigth)
{
    double *res = (double*)malloc(sizeof(double) * READINGS);
   _multiply_matrices(x, READINGS, SENSORS, weigth, res);
    
    // matrix of 'readings' long

    /*for(int i = 0;i < READINGS; i++){
            printf("%.4f ", res[i]);
    }*/
    double b = 0;
    //for(int i = 0;i < READINGS; i++){
    for(size_t j = 0;j <SENSORS; j++)
    {   
        b+= *(weigth+j);
    }
    r = _divide_matrix(res,READINGS,SENSORS, b);
    free(res);
    return r;
    
}
void _calculate_d(double *ret, double* x, double* r)
{

    for(size_t j = 0;j < SENSORS; j++)
    {
        double res = _euclidean_distance(x,r ,j, READINGS,SENSORS);
       
        *(ret+j) = (double)(1.00/(double)READINGS) * res;
         //printf("  d[%d]: %.3f\n", j, *(ret+j));
    }
    
}

void _calculate_w(double *w, double *d)
{

    for(size_t i = 0 ;i < SENSORS;i++){
            w[i] = pow(d[i], -3);
    }
}