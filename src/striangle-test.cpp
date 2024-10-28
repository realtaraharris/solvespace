#include <iostream>
#include "solvespace.h"
#include "polygon.h"

#include "striangle.h"

int main () {
  STriangle testTri = STriangle();
  Vector n = testTri.Normal().WithMagnitude(1);

  std::cout << "testTri.a.x: " << testTri.a.x << std::endl;

  STriangleEx testTriEx = STriangleEx();
  std::cout << "testTriEx.a.x: " << testTriEx.a.x << std::endl;

  return 0;
}
