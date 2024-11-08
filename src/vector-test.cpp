#include <iostream>
#include "solvespace.h"
#include "polygon.h"

#include "vector.h"
#include "vectorold.h"

#define assertm(exp, msg) assert(((void)msg, exp))
#define EPSILON (0.0000001f)
#define iseq(x, y) (((x)*(x) - (y)*(y) <= EPSILON) ? 1 : 0)

int main () {
  // Vector::Vector
  {
    VectorOld v = VectorOld(1, 2, 3);
    // --
    Vector vx = Vector(1, 2, 3);
    // --
    assertm(v.x == vx.x, "x component ok");
    assertm(v.y == vx.y, "y component ok");
    assertm(v.z == vx.z, "z component ok");
  }

  // Vector::From
  {
    VectorOld f = VectorOld::From(4.0d, 5.0d, 6.0d);
    // --
    Vector fx = VectorFromD(4.0d, 5.0d, 6.0d);
    // --
    assertm(f.x == fx.x, "x component ok");
    assertm(f.y == fx.y, "y component ok");
    assertm(f.z == fx.z, "z component ok");
  }

  // Vector::Cross
  {
    VectorOld c = VectorOld(2.0d, 0.0d, -1.0d);
    VectorOld d = VectorOld(1.0d, 1.0d, 0.0d);
    VectorOld e = c.Cross(d);
    // --
    Vector cx = Vector(2.0d, 0.0d, -1.0d);
    Vector dx = Vector(1.0d, 1.0d, 0.0d);
    Vector ex = cx.Cross(dx);
    // --
    assertm(e.x == ex.x, "x component ok");
    assertm(e.y == ex.y, "y component ok");
    assertm(e.z == ex.z, "z component ok");
  }

  // Vector::AtIntersectionOfPlanes
  {
    VectorOld g = VectorOld(0.0d, 0.0d, 1.0d);
    VectorOld h = VectorOld(1.0d, 0.0d, 0.0d);
    double gd = 2.0;
    double hd = 2.0;
    VectorOld i = VectorOld::AtIntersectionOfPlanes(g, gd, h, hd);
    // --
    Vector gx = Vector(0.0d, 0.0d, 1.0d);
    Vector hx = Vector(1.0d, 0.0d, 0.0d);
    double gdx = 2.0;
    double hdx = 2.0;
    Vector ix = VectorAtIntersectionOfPlanes(gx, gdx, hx, hdx);
    // --
    assertm(i.x == ix.x, "x component ok");
    assertm(i.y == ix.y, "y component ok");
    assertm(i.z == ix.z, "z component ok");
  }

  // Vector::AtIntersectionOfLines
  {
     VectorOld a0 = VectorOld(0.0d, 1.0d, 0.0d);
     VectorOld da = VectorOld(1.0d, 0.0d, 0.0d);
	 VectorOld b0 = VectorOld(0.0d, 1.0d, 1.0d);
     VectorOld db = VectorOld(1.0d, 0.0d, 1.0d);

     bool skew;
	 VectorOld pi = VectorOld::AtIntersectionOfLines(a0, a0.Plus(da), b0, b0.Plus(db), &skew);
     // --
     Vector a0x = Vector(0.0d, 1.0d, 0.0d);
     Vector dax = Vector(1.0d, 0.0d, 0.0d);
	 Vector b0x = Vector(0.0d, 1.0d, 1.0d);
     Vector dbx = Vector(1.0d, 0.0d, 1.0d);
	 VectorAtIntersectionOfLines_ret eeep = VectorAtIntersectionOfLines(a0x, a0x.Plus(dax), b0x, b0x.Plus(dbx), false);
	 // --
	 assertm(pi.x == eeep.intersectionPoint.x, "x component ok");
	 assertm(pi.y == eeep.intersectionPoint.y, "y component ok");
	 assertm(pi.z == eeep.intersectionPoint.z, "z component ok");
  }

  // Vector::AtIntersectionOfPlaneAndLine
  {
    VectorOld n = VectorOld(0.0d, 1.0d, 0.0d);
    double dd = 10;
    VectorOld p0 = VectorOld(0.0d, 0.0d, 1.0d);
    VectorOld p1 = VectorOld(1.0d, 0.0d, 0.0d);
    bool parallel = false;
    VectorOld k = VectorOld::AtIntersectionOfPlaneAndLine(n, dd, p0, p1, &parallel);
    // --
    Vector nx = Vector(0.0d, 1.0d, 0.0d);
    double ddx = 10;
    Vector p0x = Vector(0.0d, 0.0d, 1.0d);
    Vector p1x = Vector(1.0d, 0.0d, 0.0d);
    bool parallelx = false;
    Vector kx = VectorAtIntersectionOfPlaneAndLine(nx, ddx, p0x, p1x, &parallelx);
    // --
    assertm(k.x == kx.x, "x component ok");
    assertm(k.y == kx.y, "y component ok");
    assertm(k.z == kx.z, "z component ok");
  }

  // Vector::AtIntersectionOfPlanes
  {
    VectorOld n1 = VectorOld(1.0d, 0.0d, 0.0d);
    double d1 = 1.0;
	VectorOld n2 = VectorOld(0.0d, 0.0d, 1.0d);
    double d2 = 1.0;
	VectorOld p0 = VectorOld::AtIntersectionOfPlanes(n1, d1, n2, d2);
    // --
	Vector n1x = Vector(1.0d, 0.0d, 0.0d);
    double d1x = 1.0;
	Vector n2x = Vector(0.0d, 0.0d, 1.0d);
    double d2x = 1.0;
	Vector p0x = VectorAtIntersectionOfPlanes(n1x, d1x, n2x, d2x);
	// --
    assertm(p0.x == p0x.x, "x component ok");
    assertm(p0.y == p0x.y, "y component ok");
    assertm(p0.z == p0x.z, "z component ok");	
  }

  // Vector::ClosestPointBetweenLines
  {
    VectorOld a0 = VectorOld(1.0d, 0.0d, 0.0d);
    VectorOld da = VectorOld(0.0d, 1.0d, 0.0d);
    VectorOld b0 = VectorOld(0.0d, 0.0d, 1.0d);
    VectorOld db = VectorOld(0.0d, 1.0d, 1.0d);
    double ta;
    double tb;
	VectorOld::ClosestPointBetweenLines(a0, da, b0, db, &ta, &tb);
	// --
    Vector a0x = Vector(1.0d, 0.0d, 0.0d);
    Vector dax = Vector(0.0d, 1.0d, 0.0d);
    Vector b0x = Vector(0.0d, 0.0d, 1.0d);
    Vector dbx = Vector(0.0d, 1.0d, 1.0d);
    VectorClosestPointBetweenLines_ret resultx = VectorClosestPointBetweenLines(a0x, dax, b0x, dbx);
	// --
	assertm(ta == resultx.ta, "ta component ok");
	assertm(tb == resultx.tb, "tb component ok");
  }

  // Vector::BoundingBoxesDisjoint
  {
    VectorOld amax = VectorOld(0.0d, 0.0d, 1.0d);
    VectorOld amin = VectorOld(1.0d, 0.0d, 0.0d);
    VectorOld bmax = VectorOld(0.0d, 1.0d, 1.0d);
    VectorOld bmin = VectorOld(1.0d, 1.0d, 1.0d);
    bool result = VectorOld::BoundingBoxesDisjoint(amax, amin, bmax, bmin);
	// --
    Vector amaxex = Vector(0.0d, 0.0d, 1.0d);
    Vector aminex = Vector(1.0d, 0.0d, 0.0d);
    Vector bmaxex = Vector(0.0d, 1.0d, 1.0d);
    Vector bminex = Vector(1.0d, 1.0d, 1.0d);
    bool resultx = VectorBoundingBoxesDisjoint(amaxex, aminex, bmaxex, bminex);
	// --
    assertm(result == resultx, "results match");
  }

  // Vector::BoundingBoxIntersectsLine
  {
    VectorOld amax = VectorOld(0.0d, 0.0d, 1.0d);
    VectorOld amin = VectorOld(1.0d, 0.0d, 0.0d);
    VectorOld a = VectorOld(0.0d, 1.0d, 1.0d);
    VectorOld b = VectorOld(1.0d, 1.0d, 1.0d);
	bool asSegment = true;
    bool result = VectorOld::BoundingBoxIntersectsLine(amax, amin, a, b, asSegment);
    // --
    Vector amaxx = Vector(0.0d, 0.0d, 1.0d);
    Vector aminx = Vector(1.0d, 0.0d, 0.0d);
    Vector ax = Vector(0.0d, 1.0d, 1.0d);
    Vector bx = Vector(1.0d, 1.0d, 1.0d);
	bool asSegmentx = true;
    bool resultx = VectorBoundingBoxIntersectsLine(amaxx, aminx, ax, bx, asSegmentx);
	// --
	assertm(result == resultx, "results match");
  }

  // Vector::Element
  {
    VectorOld e = VectorOld(1.0d, 1.0d, 1.0d);
	double x = e.Element(0);
	double y = e.Element(1);
	double z = e.Element(2);
    // --
    Vector ex = Vector(1.0d, 1.0d, 1.0d);
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
    VectorOld e = VectorOld(1.0d, 1.0d, 1.0d);
	bool result = e.Equals(e, LENGTH_EPS);
    // --
    Vector ex = Vector(1.0d, 1.0d, 1.0d);
	bool resultx = ex.Equals(ex, LENGTH_EPS);
	// --
	assertm(result == resultx, "results match");
  }

  // Vector::EqualsExactly
  {
    VectorOld e = VectorOld(1.0d, 1.0d, 1.0d);
	bool result = e.EqualsExactly(e);
    // --
    Vector ex = Vector(1.0d, 1.0d, 1.0d);
	bool resultx = ex.EqualsExactly(ex);
	// --
	assertm(result == resultx, "results match");
  }

  // Vector::Plus
  {
    VectorOld e = VectorOld(1.0d, 1.0d, 1.0d);
	VectorOld result = e.Plus(e);
    // --
    Vector ex = Vector(1.0d, 1.0d, 1.0d);
	Vector resultx = ex.Plus(ex);
	// --
	assertm(result.x == resultx.x, "results match");
	assertm(result.y == resultx.y, "results match");
	assertm(result.z == resultx.z, "results match");
  }

  // Vector::Minus
  {
    VectorOld e = VectorOld(1.0d, 1.0d, 1.0d);
    VectorOld f = VectorOld(2.0d, 1.0d, 1.0d);
	VectorOld result = e.Minus(f);
    // --
    Vector ex = Vector(1.0d, 1.0d, 1.0d);
    Vector fx = Vector(2.0d, 1.0d, 1.0d);
	Vector resultx = ex.Minus(fx);
	// --
	assertm(result.x == resultx.x, "results match");
	assertm(result.y == resultx.y, "results match");
	assertm(result.z == resultx.z, "results match");
  }

  // Vector::Negated
  {
    VectorOld e = VectorOld(1.0d, 1.0d, 1.0d);
	VectorOld result = e.Negated();
    // --
    Vector ex = Vector(1.0d, 1.0d, 1.0d);
	Vector resultx = ex.Negated();
	// --
	assertm(result.x == resultx.x, "results match");
	assertm(result.y == resultx.y, "results match");
	assertm(result.z == resultx.z, "results match");
  }

  // Vector::Cross
  {
    VectorOld e = VectorOld(1.0d, 1.0d, 1.0d);
	VectorOld result = e.Cross(e);
    // --
    Vector ex = Vector(1.0d, 1.0d, 1.0d);
	Vector resultx = ex.Cross(ex);
	// --
	assertm(result.x == resultx.x, "results match");
	assertm(result.y == resultx.y, "results match");
	assertm(result.z == resultx.z, "results match");
  }

  // Vector::DirectionCosineWith
  {
    VectorOld e = VectorOld(1.0d, 1.0d, 1.0d);
	double result = e.DirectionCosineWith(e);
    // --
    Vector ex = Vector(1.0d, 1.0d, 1.0d);
	double resultx = ex.DirectionCosineWith(ex);
	// --
	assertm(result - resultx < LENGTH_EPS, "results match");
  }

  // Vector::Dot
  {
    VectorOld e = VectorOld(1.0d, 1.0d, 1.0d);
	float result = e.Dot(e);
    // --
    Vector ex = Vector(1.0d, 1.0d, 1.0d);
	float resultx = ex.Dot(ex);
	// --
	assertm(result == resultx, "results match");
  }

  // Vector::Normal
  {
    VectorOld e = VectorOld(1.0d, 1.0d, 1.0d);
	VectorOld result0 = e.Normal(0);
	VectorOld result1 = e.Normal(1);
    // --
    Vector ex = Vector(1.0d, 1.0d, 1.0d);
	Vector result0x = ex.Normal(0);
	Vector result1x = ex.Normal(1);
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
    VectorOld v = VectorOld(1.0d, 0.0d, 0.0d);
	VectorOld orig = VectorOld(0.0d, 1.0d, 0.0d);
	VectorOld axis = VectorOld(0.0d, 0.0d, 1.0d);
	double theta = 2.0d;
	VectorOld result = v.RotatedAbout(orig, axis, theta);
    // --
    Vector vx = Vector(1.0d, 0.0d, 0.0d);
	Vector origx = Vector(0.0d, 1.0d, 0.0d);
	Vector axisx = Vector(0.0d, 0.0d, 1.0d);
	double thetax = 2.0d;
	Vector resultx = vx.RotatedAbout(origx, axisx, thetax);
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::RotatedAbout
  {
    VectorOld v = VectorOld(1.0d, 0.0d, 0.0d);
	VectorOld axis = VectorOld(0.0d, 1.0d, 0.0d);
	double theta = 2.0d;
	VectorOld result = v.RotatedAbout(axis, theta);
    // --
    Vector vx = Vector(1.0d, 0.0d, 0.0d);
	Vector axisx = Vector(0.0d, 1.0d, 0.0d);
	double thetax = 2.0d;
	Vector resultx = vx.RotatedAbout(axisx, thetax);
	// --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::DotInToCsys
  {
    VectorOld t = VectorOld(1.0d, 1.0d, 1.0d);
    VectorOld u = VectorOld(1.0d, 0.0d, 0.0d);
	VectorOld v = VectorOld(0.0d, 1.0d, 0.0d);
	VectorOld n = VectorOld(0.0d, 0.0d, 1.0d);
	VectorOld result = t.DotInToCsys(u, v, n);
    // --
    Vector tx = Vector(1.0d, 1.0d, 1.0d);
    Vector ux = Vector(1.0d, 0.0d, 0.0d);
	Vector vx = Vector(0.0d, 1.0d, 0.0d);
	Vector nx = Vector(0.0d, 0.0d, 1.0d);
	Vector resultx = tx.DotInToCsys(ux, vx, nx);
	// --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::ScaleOutOfCsys
  {
    VectorOld t = VectorOld(1.0d, 1.0d, 1.0d);
    VectorOld u = VectorOld(1.0d, 0.0d, 0.0d);
	VectorOld v = VectorOld(0.0d, 1.0d, 0.0d);
	VectorOld n = VectorOld(0.0d, 0.0d, 1.0d);
	VectorOld result = t.ScaleOutOfCsys(u, v, n);
    // --
    Vector tx = Vector(1.0d, 1.0d, 1.0d);
    Vector ux = Vector(1.0d, 0.0d, 0.0d);
	Vector vx = Vector(0.0d, 1.0d, 0.0d);
	Vector nx = Vector(0.0d, 0.0d, 1.0d);
	Vector resultx = tx.ScaleOutOfCsys(ux, vx, nx);
	// --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::DistanceToLine
  {
    VectorOld t = VectorOld(1.0d, 1.0d, 1.0d);
    VectorOld p0 = VectorOld(1.0d, 0.0d, 0.0d);
	VectorOld dp = VectorOld(0.0d, 1.0d, 0.0d);
	double result = t.DistanceToLine(p0, dp);
    // --
    Vector tx = Vector(1.0d, 1.0d, 1.0d);
    Vector p0x = Vector(1.0d, 0.0d, 0.0d);
	Vector dpx = Vector(0.0d, 1.0d, 0.0d);
	double resultx = tx.DistanceToLine(p0x, dpx);
	// --
	assertm(iseq(result, resultx), "results match");
  }

  // Vector::DistanceToPlane
  {
    VectorOld t = VectorOld(1.0d, 1.0d, 1.0d);
    VectorOld normal = VectorOld(1.0d, 0.0d, 0.0d);
	VectorOld origin = VectorOld(0.0d, 1.0d, 0.0d);
	double result = t.DistanceToPlane(normal, origin);
    // --
    Vector tx = Vector(1.0d, 1.0d, 1.0d);
    Vector normalx = Vector(1.0d, 0.0d, 0.0d);
	Vector originx = Vector(0.0d, 1.0d, 0.0d);
	double resultx = tx.DistanceToPlane(normalx, originx);
	// --
	assertm(iseq(result, resultx), "results match");
  }

  // Vector::OnLineSegment
  {
	VectorOld a = VectorOld(0.0d, 0.0d, 0.0d);
    VectorOld b = VectorOld(1.0d, 1.0d, 1.0d);
    VectorOld t = VectorOld(0.5d, 0.5d, 0.5d);
	bool result = t.OnLineSegment(a, b);
    // --
	Vector ax = Vector(0.0d, 0.0d, 0.0d);
    Vector bx = Vector(1.0d, 1.0d, 1.0d);
    Vector tx = Vector(0.5d, 0.5d, 0.5d);
	bool resultx = tx.OnLineSegment(ax, bx);
	// --
	assertm(iseq(result, resultx), "results match");
  }

  // Vector::ClosestPointOnLine
  {
    VectorOld p0 = VectorOld(0.0d, 0.0d, 0.0d);
    VectorOld deltal = VectorOld(1.0d, 1.0d, 1.0d);
    VectorOld t = VectorOld(0.5d, 0.5d, 0.5d);
	VectorOld result = t.ClosestPointOnLine(p0, deltal);
    // --
	Vector p0x = Vector(0.0d, 0.0d, 0.0d);
    Vector deltalx = Vector(1.0d, 1.0d, 1.0d);
    Vector tx = Vector(0.5d, 0.5d, 0.5d);
	Vector resultx = tx.ClosestPointOnLine(p0x, deltalx);
	// --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::Magnitude
  {
    VectorOld a = VectorOld(30.0d, 30.0d, 30.0d);
	double result = a.Magnitude();
    // --
    Vector ax = Vector(30.0d, 30.0d, 30.0d);
	double resultx = ax.Magnitude();
	// --
	assertm(iseq(result, resultx), "results match");
  }

  // Vector::MagSquared
  {
    VectorOld a = VectorOld(30.0d, 30.0d, 30.0d);
	double result = a.MagSquared();
    // --
    Vector ax = Vector(30.0d, 30.0d, 30.0d);
	double resultx = ax.MagSquared();
	// --
	assertm(iseq(result, resultx), "results match");
  }

  // Vector::WithMagnitude
  {
    VectorOld a = VectorOld(1.0d, 1.0d, 1.0d);
	double s = 2.0d;
	VectorOld result = a.WithMagnitude(s);
    // --
    Vector ax = Vector(1.0d, 1.0d, 1.0d);
	double sx = 2.0d;
	Vector resultx = ax.WithMagnitude(sx);
	// --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::ScaledBy
  {
    VectorOld a = VectorOld(1.0d, 1.0d, 1.0d);
	double s = 2.0d;
	VectorOld result = a.ScaledBy(s);
    // --
    Vector ax = Vector(1.0d, 1.0d, 1.0d);
	double sx = 2.0d;
	Vector resultx = ax.ScaledBy(sx);
	// --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::ProjectInto(hEntity wrkpl) -- ugh we don't want hEntity coupled here
  {}

  // Vector::ProjectVectorOldInto(hEntity wrkpl) -- ugh we don't want hEntity coupled here
  {}

  // Vector::DivProjected
  {
    VectorOld a = VectorOld(1.0d, 1.0d, 1.0d);
    VectorOld s = VectorOld(1.0d, 0.0d, 0.0d);
	double result = a.DivProjected(s);
    // --
    Vector ax = Vector(1.0d, 1.0d, 1.0d);
    Vector sx = Vector(1.0d, 0.0d, 0.0d);
	double resultx = ax.DivProjected(sx);
	// --
	assertm(iseq(result, resultx), "results match");
  }

  // Vector::ClosestOrtho
  {
    VectorOld a = VectorOld(1.0d, 1.0d, 1.0d);
	VectorOld result = a.ClosestOrtho();
    // --
    Vector ax = Vector(1.0d, 1.0d, 1.0d);
	Vector resultx = ax.ClosestOrtho();
	// --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::MakeMaxMin
  {
    VectorOld t = VectorOld(1.0d, 0.0d, 0.0d);
    VectorOld a = VectorOld(1.0d, 1.0d, 1.0d);
    VectorOld b = VectorOld(0.0d, 0.0d, 0.0d);
	t.MakeMaxMin(&a, &b);
    // --
    Vector tx = Vector(1.0d, 0.0d, 0.0d);
    Vector ax = Vector(1.0d, 1.0d, 1.0d);
    Vector bx = Vector(0.0d, 0.0d, 0.0d);
	tx.MakeMaxMin(ax, bx);
	// --
	assertm(iseq(a.x, ax.x), "results match");
	assertm(iseq(a.y, ax.y), "results match");
	assertm(iseq(a.z, ax.z), "results match");
	assertm(iseq(b.x, bx.x), "results match");
	assertm(iseq(b.y, bx.y), "results match");
	assertm(iseq(b.z, bx.z), "results match");
  }

  // Vector::ClampWithin
  {
    VectorOld t = VectorOld(1.0d, 0.0d, 0.0d);
    double minv = 0.1;
	double maxv = 0.9;
	VectorOld result = t.ClampWithin(minv, maxv);
    // --
    Vector tx = Vector(1.0d, 0.0d, 0.0d);
    double minvx = 0.1;
	double maxvx = 0.9;
	Vector resultx = tx.ClampWithin(minvx, maxvx);
	// --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::OutsideAndNotOn
  {
    VectorOld t = VectorOld(1.0d, 0.0d, 0.0d);
    VectorOld maxv = VectorOld(1.0d, 1.0d, 1.0d);
    VectorOld minv = VectorOld(0.0d, 0.0d, 0.0d);
	bool result = t.OutsideAndNotOn(maxv, minv);
    // --
    Vector tx = Vector(1.0d, 0.0d, 0.0d);
    Vector maxvx = Vector(1.0d, 1.0d, 1.0d);
    Vector minvx = Vector(0.0d, 0.0d, 0.0d);
	bool resultx = tx.OutsideAndNotOn(maxvx, minvx);
	// --
	assertm(result == resultx, "results match");
  }

  // Vector::InPerspective
  {
    VectorOld t = VectorOld(0.0d, 0.0d, 0.0d);
    VectorOld u = VectorOld(1.0d, 0.0d, 0.0d);
    VectorOld v = VectorOld(1.0d, 0.0d, 0.0d);
    VectorOld n = VectorOld(1.0d, 1.0d, 1.0d);
    VectorOld origin = VectorOld(0.0d, 0.0d, 0.0d);
	double cameraTan = 1.0d;
	VectorOld result = t.InPerspective(u, v, n, origin, cameraTan);
    // --
    Vector tx = Vector(0.0d, 0.0d, 0.0d);
    Vector ux = Vector(1.0d, 0.0d, 0.0d);
    Vector vx = Vector(1.0d, 0.0d, 0.0d);
    Vector nx = Vector(1.0d, 1.0d, 1.0d);
    Vector originx = Vector(0.0d, 0.0d, 0.0d);
	double cameraTanx = 1.0d;
	Vector resultx = tx.InPerspective(ux, vx, nx, originx, cameraTanx);
	// --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
	assertm(iseq(result.z, resultx.z), "results match");
  }

  // Vector::Project2d
  {
    VectorOld t = VectorOld(1.0d, 1.0d, 1.0d);
    VectorOld u = VectorOld(1.0d, 0.0d, 0.0d);
    VectorOld v = VectorOld(0.0d, 0.0d, 1.0d);
    Point2d result = t.Project2d(u, v);
    // --
    Vector tx = Vector(1.0d, 1.0d, 1.0d);
    Vector ux = Vector(1.0d, 0.0d, 0.0d);
    Vector vx = Vector(0.0d, 0.0d, 1.0d);
    Point2d resultx = tx.Project2d(ux, vx);
    // --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
  }

  // Vector::ProjectXy
  {
    VectorOld t = VectorOld(1.0d, 1.0d, 1.0d);
    Point2d result = t.ProjectXy();
    // --
    Vector tx = Vector(1.0d, 1.0d, 1.0d);
    Point2d resultx = tx.ProjectXy();
    // --
	assertm(iseq(result.x, resultx.x), "results match");
	assertm(iseq(result.y, resultx.y), "results match");
  }

  // Vector4::Project4d() const;
  {}

/*
	std::cout << "result.x: " << result.x << std::endl;
	std::cout << "result.y: " << result.y << std::endl;
	std::cout << "result.z: " << result.z << std::endl;
	std::cout << "resultx.x: " << resultx.x << std::endl;
	std::cout << "resultx.y: " << resultx.y << std::endl;
	std::cout << "resultx.z: " << resultx.z << std::endl;
*/
  return 0;
}

/*
int main () {
  STriangle testTri = STriangle();
  VectorOld n = testTri.Normal().WithMagnitude(1);

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
