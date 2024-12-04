//-----------------------------------------------------------------------------
// Solve a mostly banded matrix. In a given row, there are LEFT_OF_DIAG
// elements to the left of the diagonal element, and RIGHT_OF_DIAG elements to
// the right (so that the total band width is LEFT_OF_DIAG + RIGHT_OF_DIAG + 1).
// There also may be elements in the last two columns of any row. We solve
// without pivoting.
//-----------------------------------------------------------------------------

#include "bandedmatrix.h"
#include <algorithm> // std::min

void BandedMatrix::Solve () {
  int    i, ip, j, jp;
  double temp;

  // Reduce the matrix to upper triangular form.
  for (i = 0; i < n; i++) {
    for (ip = i + 1; ip < n && ip <= (i + LEFT_OF_DIAG); ip++) {
      temp = A[ip][i] / A[i][i];

      for (jp = i; jp < (n - 2) && jp <= (i + RIGHT_OF_DIAG); jp++) {
        A[ip][jp] -= temp * (A[i][jp]);
      }
      A[ip][n - 2] -= temp * (A[i][n - 2]);
      A[ip][n - 1] -= temp * (A[i][n - 1]);

      B[ip] -= temp * B[i];
    }
  }

  // And back-substitute.
  for (i = n - 1; i >= 0; i--) {
    temp = B[i];

    if (i < n - 1)
      temp -= X[n - 1] * A[i][n - 1];
    if (i < n - 2)
      temp -= X[n - 2] * A[i][n - 2];

    for (j = std::min (n - 3, i + RIGHT_OF_DIAG); j > i; j--) {
      temp -= X[j] * A[i][j];
    }
    X[i] = temp / A[i][i];
  }
}