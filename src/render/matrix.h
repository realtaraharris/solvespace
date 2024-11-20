#ifndef MATRIX_H
#define MATRIX_H

void MakeMatrix(double *mat, double a11, double a12, double a13, double a14,
                             double a21, double a22, double a23, double a24,
                             double a31, double a32, double a33, double a34,
                             double a41, double a42, double a43, double a44);
void MultMatrix(double *mata, double *matb, double *matr);

#endif // MATRIX_H
