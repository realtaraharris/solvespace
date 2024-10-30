#include <iostream>
#include "solvespace.h"
#include "polygon.h"

#include "vectorex.h"
#include "vector.h"

#define assertm(exp, msg) assert(((void)msg, exp))
#define EPSILON (0.0000001f)
#define iseq(x, y) (((x)*(x) - (y)*(y) <= EPSILON) ? 1 : 0)

int main () {
  // Vector::Vector
  {
    Vector v = Vector(1, 2, 3);
    // --
    VectorEx vx = VectorEx(1, 2, 3);
    // --
    assertm(v.x == vx.x, "x component ok");
    assertm(v.y == vx.y, "y component ok");
    assertm(v.z == vx.z, "z component ok");
  }

  // Vector::From
  {
    Vector f = Vector::From(4.0d, 5.0d, 6.0d);
    // --
    VectorEx fx = VectorExFromD(4.0d, 5.0d, 6.0d);
    // --
    assertm(f.x == fx.x, "x component ok");
    assertm(f.y == fx.y, "y component ok");
    assertm(f.z == fx.z, "z component ok");
  }

  // Vector::Cross
  {
    Vector c = Vector(1, 2, 3);
    Vector d = Vector(1, 2, 3);
    Vector e = c.Cross(d);
    // --
    VectorEx cx = VectorEx(1, 2, 3);
    VectorEx dx = VectorEx(1, 2, 3);
    VectorEx ex = cx.Cross(dx);
    // --
    assertm(e.x == ex.x, "x component ok");
    assertm(e.y == ex.y, "y component ok");
    assertm(e.z == ex.z, "z component ok");
  }

  // Vector::AtIntersectionOfPlanes
  {
    Vector g = Vector(0.0d, 0.0d, 1.0d);
    Vector h = Vector(1.0d, 0.0d, 0.0d);
    double gd = 2.0;
    double hd = 2.0;
    Vector i = Vector::AtIntersectionOfPlanes(g, gd, h, hd);
    // --
    VectorEx gx = VectorEx(0.0d, 0.0d, 1.0d);
    VectorEx hx = VectorEx(1.0d, 0.0d, 0.0d);
    double gdx = 2.0;
    double hdx = 2.0;
    VectorEx ix = VectorExAtIntersectionOfPlanes(gx, gdx, hx, hdx);
    // --
    assertm(i.x == ix.x, "x component ok");
    assertm(i.y == ix.y, "y component ok");
    assertm(i.z == ix.z, "z component ok");
  }

  // Vector::AtIntersectionOfLines
  {
     Vector a0 = Vector(0.0d, 1.0d, 0.0d);
     Vector da = Vector(1.0d, 0.0d, 0.0d);
	 Vector b0 = Vector(0.0d, 1.0d, 1.0d);
     Vector db = Vector(1.0d, 0.0d, 1.0d);

     bool skew;
	 Vector pi = Vector::AtIntersectionOfLines(a0, a0.Plus(da), b0, b0.Plus(db), &skew);
     // --
     VectorEx a0x = VectorEx(0.0d, 1.0d, 0.0d);
     VectorEx dax = VectorEx(1.0d, 0.0d, 0.0d);
	 VectorEx b0x = VectorEx(0.0d, 1.0d, 1.0d);
     VectorEx dbx = VectorEx(1.0d, 0.0d, 1.0d);
	 VectorExAtIntersectionOfLines_ret eeep = VectorExAtIntersectionOfLines(a0x, a0x.Plus(dax), b0x, b0x.Plus(dbx), false);
	 // --
	 assertm(pi.x == eeep.intersectionPoint.x, "x component ok");
	 assertm(pi.y == eeep.intersectionPoint.y, "y component ok");
	 assertm(pi.z == eeep.intersectionPoint.z, "z component ok");
  }

  // Vector::AtIntersectionOfPlaneAndLine
  {
    Vector n = Vector(0.0d, 1.0d, 0.0d);
    double dd = 10;
    Vector p0 = Vector(0.0d, 0.0d, 1.0d);
    Vector p1 = Vector(1.0d, 0.0d, 0.0d);
    bool parallel = false;
    Vector k = Vector::AtIntersectionOfPlaneAndLine(n, dd, p0, p1, &parallel);
    // --
    VectorEx nx = VectorEx(0.0d, 1.0d, 0.0d);
    double ddx = 10;
    VectorEx p0x = VectorEx(0.0d, 0.0d, 1.0d);
    VectorEx p1x = VectorEx(1.0d, 0.0d, 0.0d);
    bool parallelx = false;
    VectorEx kx = VectorExAtIntersectionOfPlaneAndLine(nx, ddx, p0x, p1x, &parallelx);
    // --
    assertm(k.x == kx.x, "x component ok");
    assertm(k.y == kx.y, "y component ok");
    assertm(k.z == kx.z, "z component ok");
  }

  // Vector::AtIntersectionOfPlanes
  {
    Vector n1 = Vector(1.0d, 0.0d, 0.0d);
    double d1 = 1.0;
	Vector n2 = Vector(0.0d, 0.0d, 1.0d);
    double d2 = 1.0;
	Vector p0 = Vector::AtIntersectionOfPlanes(n1, d1, n2, d2);
    // --
	VectorEx n1x = VectorEx(1.0d, 0.0d, 0.0d);
    double d1x = 1.0;
	VectorEx n2x = VectorEx(0.0d, 0.0d, 1.0d);
    double d2x = 1.0;
	VectorEx p0x = VectorExAtIntersectionOfPlanes(n1x, d1x, n2x, d2x);
	// --
    assertm(p0.x == p0x.x, "x component ok");
    assertm(p0.y == p0x.y, "y component ok");
    assertm(p0.z == p0x.z, "z component ok");	
  }

  // Vector::ClosestPointBetweenLines
  {
    Vector a0 = Vector(1.0d, 0.0d, 0.0d);
    Vector da = Vector(0.0d, 1.0d, 0.0d);
    Vector b0 = Vector(0.0d, 0.0d, 1.0d);
    Vector db = Vector(0.0d, 1.0d, 1.0d);
    double ta;
    double tb;
	Vector::ClosestPointBetweenLines(a0, da, b0, db, &ta, &tb);
	// --
    VectorEx a0x = VectorEx(1.0d, 0.0d, 0.0d);
    VectorEx dax = VectorEx(0.0d, 1.0d, 0.0d);
    VectorEx b0x = VectorEx(0.0d, 0.0d, 1.0d);
    VectorEx dbx = VectorEx(0.0d, 1.0d, 1.0d);
    VectorExClosestPointBetweenLines_ret resultx = VectorExClosestPointBetweenLines(a0x, dax, b0x, dbx);
	// --
	assertm(ta == resultx.ta, "ta component ok");
	assertm(tb == resultx.tb, "tb component ok");
  }

  // Vector::BoundingBoxesDisjoint
  {
    Vector amax = Vector(0.0d, 0.0d, 1.0d);
    Vector amin = Vector(1.0d, 0.0d, 0.0d);
    Vector bmax = Vector(0.0d, 1.0d, 1.0d);
    Vector bmin = Vector(1.0d, 1.0d, 1.0d);
    bool result = Vector::BoundingBoxesDisjoint(amax, amin, bmax, bmin);
	// --
    VectorEx amaxex = VectorEx(0.0d, 0.0d, 1.0d);
    VectorEx aminex = VectorEx(1.0d, 0.0d, 0.0d);
    VectorEx bmaxex = VectorEx(0.0d, 1.0d, 1.0d);
    VectorEx bminex = VectorEx(1.0d, 1.0d, 1.0d);
    bool resultx = VectorExBoundingBoxesDisjoint(amaxex, aminex, bmaxex, bminex);
	// --
    assertm(result == resultx, "results match");
  }

  // Vector::BoundingBoxIntersectsLine
  {
    Vector amax = Vector(0.0d, 0.0d, 1.0d);
    Vector amin = Vector(1.0d, 0.0d, 0.0d);
    Vector a = Vector(0.0d, 1.0d, 1.0d);
    Vector b = Vector(1.0d, 1.0d, 1.0d);
	bool asSegment = true;
    bool result = Vector::BoundingBoxIntersectsLine(amax, amin, a, b, asSegment);
    // --
    VectorEx amaxx = VectorEx(0.0d, 0.0d, 1.0d);
    VectorEx aminx = VectorEx(1.0d, 0.0d, 0.0d);
    VectorEx ax = VectorEx(0.0d, 1.0d, 1.0d);
    VectorEx bx = VectorEx(1.0d, 1.0d, 1.0d);
	bool asSegmentx = true;
    bool resultx = VectorExBoundingBoxIntersectsLine(amaxx, aminx, ax, bx, asSegmentx);
	// --
	assertm(result == resultx, "results match");
  }

  // Vector::Element
  {
    Vector e = Vector(1.0d, 1.0d, 1.0d);
	double x = e.Element(0);
	double y = e.Element(1);
	double z = e.Element(2);
    // --
    VectorEx ex = VectorEx(1.0d, 1.0d, 1.0d);
	double xx = ex.Element(0);
	double yx = ex.Element(1);
	double zx = ex.Element(2);
	// --
	assertm(x == xx, "results match");
    assertm(y == yx, "results match");
    assertm(z == zx, "results match");
  }

  // Vector::Equals
  {
    Vector e = Vector(1.0d, 1.0d, 1.0d);
	bool result = e.Equals(e, LENGTH_EPS);
    // --
    VectorEx ex = VectorEx(1.0d, 1.0d, 1.0d);
	bool resultx = ex.Equals(ex, LENGTH_EPS);
	// --
	assertm(result == resultx, "results match");
  }

  // Vector::EqualsExactly
  {
    Vector e = Vector(1.0d, 1.0d, 1.0d);
	bool result = e.EqualsExactly(e);
    // --
    VectorEx ex = VectorEx(1.0d, 1.0d, 1.0d);
	bool resultx = ex.EqualsExactly(ex);
	// --
	assertm(result == resultx, "results match");
  }

  // Vector::Plus
  {
    Vector e = Vector(1.0d, 1.0d, 1.0d);
	Vector result = e.Plus(e);
    // --
    VectorEx ex = VectorEx(1.0d, 1.0d, 1.0d);
	VectorEx resultx = ex.Plus(ex);
	// --
	assertm(result.x == resultx.x, "results match");
	assertm(result.y == resultx.y, "results match");
	assertm(result.z == resultx.z, "results match");
  }

  // Vector::Minus
  {
    Vector e = Vector(1.0d, 1.0d, 1.0d);
	Vector result = e.Minus(e);
    // --
    VectorEx ex = VectorEx(1.0d, 1.0d, 1.0d);
	VectorEx resultx = ex.Minus(ex);
	// --
	assertm(result.x == resultx.x, "results match");
	assertm(result.y == resultx.y, "results match");
	assertm(result.z == resultx.z, "results match");
  }

  // Vector::Negated
  {
    Vector e = Vector(1.0d, 1.0d, 1.0d);
	Vector result = e.Negated();
    // --
    VectorEx ex = VectorEx(1.0d, 1.0d, 1.0d);
	VectorEx resultx = ex.Negated();
	// --
	assertm(result.x == resultx.x, "results match");
	assertm(result.y == resultx.y, "results match");
	assertm(result.z == resultx.z, "results match");
  }

  // Vector::Cross
  {
    Vector e = Vector(1.0d, 1.0d, 1.0d);
	Vector result = e.Cross(e);
    // --
    VectorEx ex = VectorEx(1.0d, 1.0d, 1.0d);
	VectorEx resultx = ex.Cross(ex);
	// --
	assertm(result.x == resultx.x, "results match");
	assertm(result.y == resultx.y, "results match");
	assertm(result.z == resultx.z, "results match");
  }

  // Vector::DirectionCosineWith
  {
    Vector e = Vector(1.0d, 1.0d, 1.0d);
	double result = e.DirectionCosineWith(e);
    // --
    VectorEx ex = VectorEx(1.0d, 1.0d, 1.0d);
	double resultx = ex.DirectionCosineWith(ex);
	// --
	assertm(result - resultx < LENGTH_EPS, "results match");
  }

  // Vector::Dot
  {
    Vector e = Vector(1.0d, 1.0d, 1.0d);
	float result = e.Dot(e);
    // --
    VectorEx ex = VectorEx(1.0d, 1.0d, 1.0d);
	float resultx = ex.Dot(ex);
	// --
	assertm(result == resultx, "results match");
  }

  // Vector::Normal
  {
    Vector e = Vector(1.0d, 1.0d, 1.0d);
	Vector result0 = e.Normal(0);
	Vector result1 = e.Normal(1);
    // --
    VectorEx ex = VectorEx(1.0d, 1.0d, 1.0d);
	VectorEx result0x = ex.Normal(0);
	VectorEx result1x = ex.Normal(1);
	// --
	assertm(iseq(result0.x, result0x.x), "results match");
	assertm(iseq(result0.y, result0x.y), "results match");
	assertm(iseq(result0.z, result0x.z), "results match");
	assertm(iseq(result1.x, result1x.x), "results match");
	assertm(iseq(result1.y, result1x.y), "results match");
	assertm(iseq(result1.z, result1x.z), "results match");
  }

  // Vector::RotatedAbout
  {
    Vector v = Vector(1.0d, 0.0d, 0.0d);
	Vector orig = Vector(0.0d, 1.0d, 0.0d);
	Vector axis = Vector(0.0d, 0.0d, 1.0d);
	double theta = 2.0d;
	Vector result = v.RotatedAbout(orig, axis, theta);
    // --
    VectorEx vx = VectorEx(1.0d, 0.0d, 0.0d);
	VectorEx origx = VectorEx(0.0d, 1.0d, 0.0d);
	VectorEx axisx = VectorEx(0.0d, 0.0d, 1.0d);
	double thetax = 2.0d;
	VectorEx resultx = vx.RotatedAbout(origx, axisx, thetax);
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::RotatedAbout
  {
    Vector v = Vector(1.0d, 0.0d, 0.0d);
	Vector axis = Vector(0.0d, 1.0d, 0.0d);
	double theta = 2.0d;
	Vector result = v.RotatedAbout(axis, theta);
    // --
    VectorEx vx = VectorEx(1.0d, 0.0d, 0.0d);
	VectorEx axisx = VectorEx(0.0d, 1.0d, 0.0d);
	double thetax = 2.0d;
	VectorEx resultx = vx.RotatedAbout(axisx, thetax);
	// --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::DotInToCsys(Vector u, Vector v, Vector n) const;
  {
    Vector t = Vector(1.0d, 1.0d, 1.0d);
    Vector u = Vector(1.0d, 0.0d, 0.0d);
	Vector v = Vector(0.0d, 1.0d, 0.0d);
	Vector n = Vector(0.0d, 0.0d, 1.0d);
	Vector result = t.DotInToCsys(u, v, n);
    // --
    VectorEx tx = VectorEx(1.0d, 1.0d, 1.0d);
    VectorEx ux = VectorEx(1.0d, 0.0d, 0.0d);
	VectorEx vx = VectorEx(0.0d, 1.0d, 0.0d);
	VectorEx nx = VectorEx(0.0d, 0.0d, 1.0d);
	VectorEx resultx = tx.DotInToCsys(ux, vx, nx);
	// --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::ScaleOutOfCsys(Vector u, Vector v, Vector n) const;
  {}

  // double DistanceToLine(Vector p0, Vector dp) const;
  {}

  // double DistanceToPlane(Vector normal, Vector origin) const;
  {}

  // bool OnLineSegment(Vector a, Vector b, double tol=LENGTH_EPS) const;
  {}

  // Vector ClosestPointOnLine(Vector p0, Vector deltal) const;
  {}

/*
	std::cout << "result.x: " << result.x << std::endl;
	std::cout << "result.y: " << result.y << std::endl;
	std::cout << "result.z: " << result.z << std::endl;
	std::cout << "resultx.x: " << resultx.x << std::endl;
	std::cout << "resultx.y: " << resultx.y << std::endl;
	std::cout << "resultx.z: " << resultx.z << std::endl;
*/

/*
    double Magnitude() const;
    double MagSquared() const;
    Vector WithMagnitude(double s) const;
    Vector ScaledBy(double s) const;
    Vector ProjectInto(hEntity wrkpl) const;
    Vector ProjectVectorInto(hEntity wrkpl) const;
    double DivProjected(Vector delta) const;
    Vector ClosestOrtho() const;
    void MakeMaxMin(Vector *maxv, Vector *minv) const;
    Vector ClampWithin(double minv, double maxv) const;
    bool OutsideAndNotOn(Vector maxv, Vector minv) const;
    Vector InPerspective(Vector u, Vector v, Vector n,
                         Vector origin, double cameraTan) const;
    Point2d Project2d(Vector u, Vector v) const;
    Point2d ProjectXy() const;
    Vector4 Project4d() const;
*/
  return 0;
}

/*
int main () {
  STriangle testTri = STriangle();
  Vector n = testTri.Normal().WithMagnitude(1);

  std::cout << "testTri.a.x: " << testTri.a.x << std::endl;

  STriangleEx testTriEx = STriangleEx();
  std::cout << "testTriEx.a.x: " << testTriEx.a.x << std::endl;

  return 0;
}
*/

void SolveSpace::AssertFailure(const char *file, unsigned line, const char *function,
                               const char *condition, const char *message) {
    std::string formattedMsg;
    formattedMsg += ssprintf("File %s, line %u, function %s:\n", file, line, function);
    formattedMsg += ssprintf("Assertion failed: %s.\n", condition);
    formattedMsg += ssprintf("Message: %s.\n", message);
	std::cout << formattedMsg << std::endl;
}

std::string SolveSpace::ssprintf(const char *fmt, ...)
{
    va_list va;

    va_start(va, fmt);
    int size = vsnprintf(NULL, 0, fmt, va);
    ssassert(size >= 0, "vsnprintf could not encode string");
    va_end(va);

    std::string result;
    result.resize(size + 1);

    va_start(va, fmt);
    vsnprintf(&result[0], size + 1, fmt, va);
    va_end(va);

    result.resize(size);
    return result;
}
