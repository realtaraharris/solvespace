void MakeMatrix(double *mat, double a11, double a12, double a13, double a14, double a21, double a22,
                double a23, double a24, double a31, double a32, double a33, double a34, double a41,
                double a42, double a43, double a44) {
  mat[0] = a11;
  mat[1] = a21;
  mat[2] = a31;
  mat[3] = a41;
  mat[4] = a12;
  mat[5] = a22;
  mat[6] = a32;
  mat[7] = a42;
  mat[8] = a13;
  mat[9] = a23;
  mat[10] = a33;
  mat[11] = a43;
  mat[12] = a14;
  mat[13] = a24;
  mat[14] = a34;
  mat[15] = a44;
}

void MultMatrix(double *mata, double *matb, double *matr) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      double s = 0.0;
      for (int k = 0; k < 4; k++) {
        s += mata[k * 4 + j] * matb[i * 4 + k];
      }
      matr[i * 4 + j] = s;
    }
  }
}