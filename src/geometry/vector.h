
#ifndef _BOOT_HOME_SOLVESPACE_SRC_VECTOR_H_CPP2
#define _BOOT_HOME_SOLVESPACE_SRC_VECTOR_H_CPP2

#define CPP2_IMPORT_STD          Yes

//=== Cpp2 type declarations ====================================================


#include "cpp2util.h"

#line 1 "/boot/home/solvespace/src/vector.h2"
namespace SolveSpace {
#line 2 "/boot/home/solvespace/src/vector.h2"
class Vector;
  

#line 447 "/boot/home/solvespace/src/vector.h2"
}


//=== Cpp2 type definitions and function declarations ===========================

#line 1 "/boot/home/solvespace/src/vector.h2"
namespace SolveSpace {
#line 2 "/boot/home/solvespace/src/vector.h2"
class Vector {
  public: double x {}; 
  public: double y {}; 
  public: double z {}; 
  public: explicit Vector(cpp2::impl::in<double> px, cpp2::impl::in<double> py, cpp2::impl::in<double> pz);

#line 11 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] static auto From(cpp2::impl::in<double> px, cpp2::impl::in<double> py, cpp2::impl::in<double> pz) -> Vector;

#line 14 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto Plus(cpp2::impl::in<Vector> b) const& -> Vector;

#line 17 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto ToString() const& -> std::string;

#line 20 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto Element(cpp2::impl::in<int> i) const& -> double;

#line 30 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto Equals(cpp2::impl::in<Vector> v, cpp2::impl::in<double> tol = LENGTH_EPS) const& -> bool;

#line 39 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto EqualsExactly(cpp2::impl::in<Vector> v) const& -> bool;

#line 42 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto Minus(cpp2::impl::in<Vector> v) const& -> Vector;

#line 45 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto Negated() const& -> Vector;

#line 48 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto Cross(cpp2::impl::in<Vector> b) const& -> Vector;

#line 55 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto DirectionCosineWith(cpp2::impl::in<Vector> b) const& -> double;

#line 60 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto Dot(cpp2::impl::in<Vector> b) const& -> double;

#line 63 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto Normal(cpp2::impl::in<int> which) const& -> Vector;

#line 93 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto RotatedAbout(cpp2::impl::in<Vector> orig, cpp2::impl::in<Vector> axis, cpp2::impl::in<double> theta) const& -> Vector;

#line 98 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto RotatedAbout(cpp2::impl::in<Vector> axis, cpp2::impl::in<double> theta) const& -> Vector;

#line 118 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto DotInToCsys(cpp2::impl::in<Vector> u, cpp2::impl::in<Vector> v, cpp2::impl::in<Vector> n) const& -> Vector;

#line 125 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto ScaleOutOfCsys(cpp2::impl::in<Vector> u, cpp2::impl::in<Vector> v, cpp2::impl::in<Vector> n) const& -> Vector;

#line 131 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto DistanceToLine(cpp2::impl::in<Vector> p0, cpp2::impl::in<Vector> dp) const& -> double;

#line 135 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto DistanceToPlane(cpp2::impl::in<Vector> normal, cpp2::impl::in<Vector> origin) const& -> double;

#line 138 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto OnLineSegment(cpp2::impl::in<Vector> a, cpp2::impl::in<Vector> b, cpp2::impl::in<double> tol = LENGTH_EPS) const& -> bool;

#line 153 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto ClosestPointOnLine(cpp2::impl::in<Vector> p0, cpp2::impl::in<Vector> dp) const& -> Vector;

#line 165 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto Magnitude() const& -> double;

#line 168 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto MagSquared() const& -> double;

#line 171 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto WithMagnitude(cpp2::impl::in<double> v) const& -> Vector;

#line 182 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto ScaledBy(cpp2::impl::in<double> v) const& -> Vector;

#line 185 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto ProjectVectorInto(cpp2::impl::in<hEntity> workplane) const& -> Vector;

#line 195 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto ProjectInto(cpp2::impl::in<hEntity> workplane) const& -> Vector;

#line 203 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto DivProjected(cpp2::impl::in<Vector> delta) const& -> double;

#line 207 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto ClosestOrtho() const& -> Vector;

#line 244 "/boot/home/solvespace/src/vector.h2"
  /*
  // the following code is correct cpp2, but there's a bug in cppfront that prevents its use
  MakeMaxMin: (this, maxv_in: Vector, minv_in: Vector) -> (maxv: Vector, minv: Vector) = {
    maxv = VectorFromD(
      std::max(maxv_in.x, this.x),
	  std::max(maxv_in.y, this.y),
	  std::max(maxv_in.z, this.z)
    );
    minv = VectorFromD(
	  std::min(minv_in.x, this.x),
      std::min(minv_in.y, this.y),
      std::min(minv_in.z, this.z)
    );
  }
  */
  public: auto MakeMaxMin(Vector& maxv, Vector& minv) const& -> void;

#line 271 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto ClampWithin(cpp2::impl::in<double> minv, cpp2::impl::in<double> maxv) const& -> Vector;

#line 286 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto OutsideAndNotOn(cpp2::impl::in<Vector> maxv, cpp2::impl::in<Vector> minv) const& -> bool;

#line 291 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto InPerspective(cpp2::impl::in<Vector> u, cpp2::impl::in<Vector> v, cpp2::impl::in<Vector> n, cpp2::impl::in<Vector> origin, cpp2::impl::in<double> cameraTan) const& -> Vector;

#line 300 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto Project2d(cpp2::impl::in<Vector> u, cpp2::impl::in<Vector> v) const& -> SolveSpace::Point2d;

#line 303 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto ProjectXy() const& -> SolveSpace::Point2d;

#line 306 "/boot/home/solvespace/src/vector.h2"
  public: [[nodiscard]] auto Project4d() const& -> Vector4;
  public: [[nodiscard]] auto operator<=>(Vector const& that) const& -> std::strong_ordering = default;
public: Vector(Vector const& that);

public: auto operator=(Vector const& that) -> Vector& ;
public: Vector(Vector&& that) noexcept;
public: auto operator=(Vector&& that) noexcept -> Vector& ;
public: explicit Vector();

#line 309 "/boot/home/solvespace/src/vector.h2"
};

[[nodiscard]] auto VectorFromD(cpp2::impl::in<double> px, cpp2::impl::in<double> py, cpp2::impl::in<double> pz) -> Vector;

#line 315 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorFromH(cpp2::impl::in<hParam> px, cpp2::impl::in<hParam> py, cpp2::impl::in<hParam> pz) -> Vector;

#line 319 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorAtIntersectionOfPlanes(cpp2::impl::in<Vector> n1, cpp2::impl::in<double> d1, cpp2::impl::in<Vector> n2, cpp2::impl::in<double> d2) -> Vector;
struct VectorClosestPointBetweenLines_ret { double ta; double tb; };



#line 327 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorClosestPointBetweenLines(cpp2::impl::in<Vector> a0, cpp2::impl::in<Vector> da, cpp2::impl::in<Vector> b0, cpp2::impl::in<Vector> db) -> VectorClosestPointBetweenLines_ret;
struct VectorAtIntersectionOfLines_ret { double parama; double paramb; Vector intersectionPoint; bool skewed; };



#line 340 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorAtIntersectionOfLines(cpp2::impl::in<Vector> a0, cpp2::impl::in<Vector> a1, cpp2::impl::in<Vector> b0, cpp2::impl::in<Vector> b1, cpp2::impl::in<bool> skew) -> VectorAtIntersectionOfLines_ret;

#line 357 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorAtIntersectionOfPlaneAndLine(cpp2::impl::in<Vector> n, cpp2::impl::in<double> d, cpp2::impl::in<Vector> p0, cpp2::impl::in<Vector> p1, bool* parallel) -> Vector;

#line 373 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorBoundingBoxesDisjoint(cpp2::impl::in<Vector> amax, cpp2::impl::in<Vector> amin, cpp2::impl::in<Vector> bmax, cpp2::impl::in<Vector> bmin) -> bool;

#line 381 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorBoundingBoxIntersectsLine(cpp2::impl::in<Vector> amax, cpp2::impl::in<Vector> amin, cpp2::impl::in<Vector> p0, cpp2::impl::in<Vector> p1, cpp2::impl::in<bool> asSegment) -> bool;

#line 412 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto det2(cpp2::impl::in<double> a1, cpp2::impl::in<double> b1, cpp2::impl::in<double> a2, cpp2::impl::in<double> b2) -> double;

#line 416 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto det3(cpp2::impl::in<double> a1, cpp2::impl::in<double> b1, cpp2::impl::in<double> c1, 
       cpp2::impl::in<double> a2, cpp2::impl::in<double> b2, cpp2::impl::in<double> c2, 
       cpp2::impl::in<double> a3, cpp2::impl::in<double> b3, cpp2::impl::in<double> c3) -> double;

#line 424 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorAtIntersectionOfPlanes(cpp2::impl::in<Vector> na, cpp2::impl::in<double> da, cpp2::impl::in<Vector> nb, cpp2::impl::in<double> db, cpp2::impl::in<Vector> nc, cpp2::impl::in<double> dc, bool* parallel) -> Vector;

#line 447 "/boot/home/solvespace/src/vector.h2"
}

#endif
