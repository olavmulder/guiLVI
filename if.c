
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#define ITER 5
#define SENSORS 8
#define READINGS 3

void multiply_matrices(double *m1,int rows1, int cols1, 
                      double *m2, int cols2, double*res)
{
    int i, j, k;
    for (i = 0; i < rows1; i++) {//readings
        res[i] = 0;
        for (j = 0; j < cols1; j++) {//sensoren
            res[i] += m1[i*cols1+j] * m2[j];
        }
    }
}
double* divide_matrix(double* matrix, int rows, int cols, double scalar) {
    double* result = (double*)malloc(rows * cols* sizeof(double*));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            result[i * cols+ j] = matrix[i*cols+j] / scalar;
        }
    }
    return result;
}

double euclidean_distance(double *x, double *r, int sensorID,int rows, int amountCols) {
    double distance = 0.0;

    for (int i = 0; i < rows; i++) {//rows is readings
        distance += pow(r[i] - x[sensorID+amountCols*i], 2);
    }
    return sqrt(distance);
}
double* calculate_r(double *r, double *x, double* weigth)
{
    double *res = (double*)malloc(sizeof(double) * READINGS);
    multiply_matrices(x, READINGS, SENSORS, weigth, READINGS, res);
  
    double b = 0;
    for(int j = 0;j <SENSORS; j++)
    {   
        b+= *(weigth+j);
    }
    r = divide_matrix(res,READINGS,SENSORS, b);
    free(res);
    return r; 
}
void calculate_d(double *ret, double* x, double* r)
{
    for(int j = 0;j < SENSORS; j++)
    {
        double res = euclidean_distance(x,r ,j, READINGS,SENSORS);
        *(ret+j) = (double)(1.00/(double)READINGS) * res;
    }
}

void calculate_w(double *w, double *d)
{
    for(int i = 0 ;i < SENSORS;i++){
            w[i] = pow(d[i], -3);
    }
}
void PrintInput(double *x, int col, int row)
{
    printf("        Reading 1, reading 2, reading 3\n");
    for(int i = 0 ; i < row; i++)
    {
        printf("sensor %d: ", i+1);
        for(int j = 0; j < col; j++)
        {
            printf("%.4f, ", x[i +row * j]);
        }
        printf("\n");
    }
}
double* IF()
{
    double X[READINGS][SENSORS] = {
        {85.3612,0.0, 18.1, 16.5674, 18.1, 60.153, 18.2, 18.5},
        {20.3612,0.0, 18.5, 15.5478, 18.2, 60.152, 18.8, 18.6},
        {0,      0.0, 18.2, 17.0,    18.3, 15.23,  18.01, 18.8},
    };
    PrintInput(X[0], READINGS, SENSORS);
    int l = 0;//variable for iterations
    double weight[SENSORS] = {1,1,1,1,1,1,1,1};

    double *r;
    double *x;
    double *d;

    x = (double*)malloc(SENSORS*READINGS * sizeof *x);
    d = (double*)malloc(SENSORS*READINGS * sizeof *d);
    r = (double*)malloc(sizeof(double)*READINGS);
    for(int i = 0; i < READINGS; i++)
    {
        for(int j = 0; j < SENSORS; j++)
            x[i*SENSORS+j] = X[i][j];
    }

    do{
        r = calculate_r(r, x, weight);
        calculate_d(d, x, r);
        calculate_w(weight, d);
        l++;
    }while(l < ITER);
    return r;

}


int main(int argc, char** argv)
{
    //if(argc ==4)
    //{
        char* res1 = argv[1];
        char* res2 = argv[2];
        char* res3 = argv[3];
        double *r = IF(); 
        char r1[10];
        char r2[10];
        char r3[10];
        snprintf(r1,10 ,"%.4f", *r);
        snprintf(r2,10 ,"%.4f", *(r+1));
        snprintf(r3,10 ,"%.4f", *(r+2));
    
        printf("results: \n%.4f\n%.4f\n%.4f\n", *r, *(r+1), *(r+2));
        free(r);

   // }
    return 0;
    
}