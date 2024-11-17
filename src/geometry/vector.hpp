
#ifndef _BOOT_HOME_SOLVESPACE_SRC_VECTOR_H_CPP2
#error This file is part of a '.h2' header compiled to be consumed from another -pure-cpp2 file. To use this file, write '#include "/boot/home/solvespace/src/vector.h2"' in a '.h2' or '.cpp2' file compiled with -pure-cpp2.
#endif

#ifndef _BOOT_HOME_SOLVESPACE_SRC_VECTOR_HPP_CPP2
#define _BOOT_HOME_SOLVESPACE_SRC_VECTOR_HPP_CPP2



//=== Cpp2 function definitions =================================================

#line 1 "/boot/home/solvespace/src/vector.h2"
namespace SolveSpace {

#line 6 "/boot/home/solvespace/src/vector.h2"
  Vector::Vector(cpp2::impl::in<double> px, cpp2::impl::in<double> py, cpp2::impl::in<double> pz)
    : x{ px }
    , y{ py }
    , z{ pz }{

#line 10 "/boot/home/solvespace/src/vector.h2"
  }
#line 11 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::From(cpp2::impl::in<double> px, cpp2::impl::in<double> py, cpp2::impl::in<double> pz) -> Vector{
    return VectorFromD(px, py, pz); 
  }
#line 14 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::Plus(cpp2::impl::in<Vector> b) const& -> Vector{
    return Vector((*this).x + b.x, (*this).y + b.y, (*this).z + b.z); 
  }
#line 17 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::ToString() const& -> std::string{
 return "eeep3 x: " + cpp2::to_string((*this).x) + " y: " + cpp2::to_string((*this).y) + " z: " + cpp2::to_string((*this).z) + ""; 
  }
#line 20 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::Element(cpp2::impl::in<int> i) const& -> double{
    if (i == 0) {
   return { (*this).x }; 
 }else {if (i == 1) {
   return { (*this).y }; 
 }else {if (i == 2) {
   return { (*this).z }; 
    }}}
 ssassert(false, "Unexpected vector element index");
  }
#line 30 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::Equals(cpp2::impl::in<Vector> v, cpp2::impl::in<double> tol) const& -> bool{
    // quick axis-aligned tests before going further
 Vector dv {CPP2_UFCS(Minus)((*this), v)}; 
    if ((cpp2::impl::cmp_greater(fabs(dv.x),tol))) {return false; }
    if ((cpp2::impl::cmp_greater(fabs(dv.y),tol))) {return false; }
    if ((cpp2::impl::cmp_greater(fabs(dv.z),tol))) {return false; }

    return cpp2::impl::cmp_less(CPP2_UFCS(MagSquared)(cpp2::move(dv)),tol * tol); 
  }
#line 39 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::EqualsExactly(cpp2::impl::in<Vector> v) const& -> bool{
    return EXACT((*this).x == v.x && (*this).y == v.y && (*this).z == v.z); 
  }
#line 42 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::Minus(cpp2::impl::in<Vector> v) const& -> Vector{
    return Vector((*this).x - v.x, (*this).y - v.y, (*this).z - v.z); 
  }
#line 45 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::Negated() const& -> Vector{
    return Vector(-(*this).x, -(*this).y, -(*this).z); 
  }
#line 48 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::Cross(cpp2::impl::in<Vector> b) const& -> Vector{
     return Vector(
      -((*this).z * b.y) + ((*this).y * b.z), 
       ((*this).z * b.x) - ((*this).x * b.z), 
      -((*this).y * b.x) + ((*this).x * b.y)
  ); 
  }
#line 55 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::DirectionCosineWith(cpp2::impl::in<Vector> b) const& -> double{
    Vector a {CPP2_UFCS(WithMagnitude)((*this), 1)}; 
    Vector tmp {CPP2_UFCS(WithMagnitude)(b, 1)}; 
    return CPP2_UFCS(Dot)(cpp2::move(a), cpp2::move(tmp)); 
  }
#line 60 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::Dot(cpp2::impl::in<Vector> b) const& -> double{
      return { (*this).x * b.x + (*this).y * b.y + (*this).z * b.z }; 
  }
#line 63 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::Normal(cpp2::impl::in<int> which) const& -> Vector{
    cpp2::impl::deferred_init<Vector> n; 

    // arbitrarily choose one vector that's normal to us, pivoting appropriately
    double xa {fabs((*this).x)}; 
 double ya {fabs((*this).y)}; 
 double za {fabs((*this).z)}; 

    if ((CPP2_UFCS(Equals)((*this), VectorFromD(0, 0, 1)))) {
   // make DXFs exported in the XY plane work nicely...
   n.construct(VectorFromD(1, 0, 0));
    }else {if ((cpp2::impl::cmp_less(xa,ya) && cpp2::impl::cmp_less(xa,za))) {
   n.construct(VectorFromD(0, z, -y));
    }else {if ((cpp2::impl::cmp_less(cpp2::move(ya),cpp2::move(za)))) {
   n.construct(VectorFromD(-z, 0, x));
    }else {
   n.construct(VectorFromD(y, -z, 0));
    }}}

    if ((which == 0)) {// that's the vector we return.
    }else {if ((which == 1)) {
      n.value() = CPP2_UFCS(Cross)((*this), n.value());
    }else {
   ssassert(false, "Unexpected vector normal index");
 }}

    n.value() = CPP2_UFCS(WithMagnitude)(n.value(), 1);

    return cpp2::move(n.value()); 
  }
#line 93 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::RotatedAbout(cpp2::impl::in<Vector> orig, cpp2::impl::in<Vector> axis, cpp2::impl::in<double> theta) const& -> Vector{
    Vector r {CPP2_UFCS(Minus)((*this), orig)}; 
    r = CPP2_UFCS(RotatedAbout)(r, axis, theta);
    return CPP2_UFCS(Plus)(cpp2::move(r), orig); 
  }
#line 98 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::RotatedAbout(cpp2::impl::in<Vector> axis, cpp2::impl::in<double> theta) const& -> Vector{
    double c {cos(theta)}; 
    double s {sin(theta)}; 

    Vector axisn {CPP2_UFCS(WithMagnitude)(axis, 1)}; 

    return VectorFromD(
   ((*this).x) * (c + (1 - c) * (axisn.x) * (axisn.x)) + 
   ((*this).y) * ((1 - c) * (axisn.x) * (axisn.y) - s * (axisn.z)) + 
   ((*this).z) * ((1 - c) * (axisn.x) * (axisn.z) + s * (axisn.y)), 

   ((*this).x) * ((1 - c) * (axisn.y) * (axisn.x) + s * (axisn.z)) + 
   ((*this).y) * (c + (1 - c) * (axisn.y) * (axisn.y)) + 
   ((*this).z) * ((1 - c) * (axisn.y) * (axisn.z) - s * (axisn.x)), 

   ((*this).x) * ((1 - c) * (axisn.z) * (axisn.x) - s * (axisn.y)) + 
   ((*this).y) * ((1 - c) * (axisn.z) * (axisn.y) + s * (axisn.x)) + 
   ((*this).z) * (c + (1 - c) * (axisn.z) * (axisn.z))
 ); 
  }
#line 118 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::DotInToCsys(cpp2::impl::in<Vector> u, cpp2::impl::in<Vector> v, cpp2::impl::in<Vector> n) const& -> Vector{
    return VectorFromD(
   CPP2_UFCS(Dot)((*this), u), 
   CPP2_UFCS(Dot)((*this), v), 
   CPP2_UFCS(Dot)((*this), n)
    ); 
  }
#line 125 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::ScaleOutOfCsys(cpp2::impl::in<Vector> u, cpp2::impl::in<Vector> v, cpp2::impl::in<Vector> n) const& -> Vector{
    Vector r {CPP2_UFCS(Plus)(CPP2_UFCS(ScaledBy)(u, (*this).x), 
   CPP2_UFCS(Plus)(CPP2_UFCS(ScaledBy)(v, (*this).y), 
   CPP2_UFCS(ScaledBy)(n, (*this).z)))}; 
    return r; 
  }
#line 131 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::DistanceToLine(cpp2::impl::in<Vector> p0, cpp2::impl::in<Vector> dp) const& -> double{
    double m {CPP2_UFCS(Magnitude)(dp)}; 
    return CPP2_UFCS(Magnitude)((CPP2_UFCS(Cross)((CPP2_UFCS(Minus)((*this), p0)), dp))) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(CPP2_UFCS(Magnitude)((CPP2_UFCS(Cross)((CPP2_UFCS(Minus)((*this), p0)), dp)))),cpp2::move(m)); 
  }
#line 135 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::DistanceToPlane(cpp2::impl::in<Vector> normal, cpp2::impl::in<Vector> origin) const& -> double{
    return CPP2_UFCS(Dot)((*this), normal) - CPP2_UFCS(Dot)(origin, normal); 
  }
#line 138 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::OnLineSegment(cpp2::impl::in<Vector> a, cpp2::impl::in<Vector> b, cpp2::impl::in<double> tol) const& -> bool{
    if ((CPP2_UFCS(Equals)((*this), a, tol) || CPP2_UFCS(Equals)((*this), b, tol))) {return true; }

    Vector d {CPP2_UFCS(Minus)(b, a)}; 
    double m {CPP2_UFCS(MagSquared)(d)}; 
    double distsq {CPP2_UFCS(MagSquared)((CPP2_UFCS(Cross)((CPP2_UFCS(Minus)((*this), a)), d))) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(CPP2_UFCS(MagSquared)((CPP2_UFCS(Cross)((CPP2_UFCS(Minus)((*this), a)), d)))),cpp2::move(m))}; 

    if ((cpp2::impl::cmp_greater_eq(cpp2::move(distsq),tol * tol))) {return false; }

    double t {CPP2_UFCS(DivProjected)((CPP2_UFCS(Minus)((*this), a)), cpp2::move(d))}; 

    if ((cpp2::impl::cmp_less(t,0) || cpp2::impl::cmp_greater(t,1))) {return false; }// on-endpoint already tested

    return true; 
  }
#line 153 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::ClosestPointOnLine(cpp2::impl::in<Vector> p0, cpp2::impl::in<Vector> dp) const& -> Vector{
    Vector dpp {CPP2_UFCS(WithMagnitude)(dp, 1)}; 
    // this, p0, and (p0 + dpp) define a plane; the min distance is in
    // that plane, so calculate its normal
    Vector pn {CPP2_UFCS(Cross)((CPP2_UFCS(Minus)((*this), p0)), dpp)}; 
    // the minimum distance line is in that plane, perpendicular to the line
    Vector n {CPP2_UFCS(Cross)(cpp2::move(pn), dpp)}; 

    // calculate the actual distance
    double d {CPP2_UFCS(Magnitude)((CPP2_UFCS(Cross)(cpp2::move(dpp), CPP2_UFCS(Minus)(p0, (*this)))))}; 
    return CPP2_UFCS(Plus)((*this), CPP2_UFCS(WithMagnitude)(cpp2::move(n), cpp2::move(d))); 
  }
#line 165 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::Magnitude() const& -> double{
    return sqrt(((*this).x) * ((*this).x) + ((*this).y) * ((*this).y) + ((*this).z) * ((*this).z)); 
  }
#line 168 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::MagSquared() const& -> double{
    return ((*this).x) * ((*this).x) + ((*this).y) * ((*this).y) + ((*this).z) * ((*this).z); 
  }
#line 171 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::WithMagnitude(cpp2::impl::in<double> v) const& -> Vector{
    double m {CPP2_UFCS(Magnitude)((*this))}; 
    if ((EXACT(m == 0))) {// we can do a zero vector with zero magnitude, but not any other cases
   if ((cpp2::impl::cmp_greater(fabs(v),1e-100))) {
        // dbp("Vector::WithMagnitude(%g) of zero vector!", v);
      }
   return CPP2_UFCS(From)((*this), 0, 0, 0); 
    }else {
   return CPP2_UFCS(ScaledBy)((*this), v / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(v),cpp2::move(m))); 
    }
  }
#line 182 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::ScaledBy(cpp2::impl::in<double> v) const& -> Vector{
    return Vector((*this).x * v, (*this).y * v, (*this).z * v); 
  }
#line 185 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::ProjectVectorInto(cpp2::impl::in<hEntity> workplane) const& -> Vector{
    EntityBase* w {CPP2_UFCS(GetEntity)(SK, workplane)}; 
    Vector u {CPP2_UFCS(NormalU)((*cpp2::impl::assert_not_null(CPP2_UFCS(Normal)(*cpp2::impl::assert_not_null(w)))))}; 
    Vector v {CPP2_UFCS(NormalV)((*cpp2::impl::assert_not_null(CPP2_UFCS(Normal)(*cpp2::impl::assert_not_null(cpp2::move(w))))))}; 

    double up {CPP2_UFCS(Dot)((*this), u)}; 
    double vp {CPP2_UFCS(Dot)((*this), v)}; 

    return CPP2_UFCS(Plus)((CPP2_UFCS(ScaledBy)(cpp2::move(u), cpp2::move(up))), CPP2_UFCS(ScaledBy)(cpp2::move(v), cpp2::move(vp))); 
  }
#line 195 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::ProjectInto(cpp2::impl::in<hEntity> workplane) const& -> Vector{
    EntityBase* w {CPP2_UFCS(GetEntity)(SK, workplane)}; 
    Vector p0 {CPP2_UFCS(WorkplaneGetOffset)((*cpp2::impl::assert_not_null(cpp2::move(w))))}; 

    Vector f {CPP2_UFCS(Minus)((*this), p0)}; 

    return CPP2_UFCS(Plus)(cpp2::move(p0), CPP2_UFCS(ProjectVectorInto)(cpp2::move(f), workplane)); 
  }
#line 203 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::DivProjected(cpp2::impl::in<Vector> delta) const& -> double{
    return ((*this).x * delta.x + (*this).y * delta.y + (*this).z * delta.z) 
         / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(((*this).x * delta.x + (*this).y * delta.y + (*this).z * delta.z)),(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z)); 
  }
#line 207 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::ClosestOrtho() const& -> Vector{
    double mx {fabs((*this).x)}; 
 double my {fabs((*this).y)}; 
 double mz {fabs((*this).z)}; 

    // TODO: replace with ternary operator once cpp2 has one
 double xTernTmp {0}; 
 if ((cpp2::impl::cmp_greater((*this).x,0))) {
      xTernTmp = 1;
    }else {
      xTernTmp = -1;
    }

    // TODO: replace with ternary operator once cpp2 has one
 double yTernTmp {0}; 
 if ((cpp2::impl::cmp_greater((*this).y,0))) {
   yTernTmp = 1;
 }else {
   yTernTmp = -1;
 }

 // TODO: replace with ternary operator once cpp2 has one
 double zTernTmp {0}; 
    if ((cpp2::impl::cmp_greater((*this).z,0))) {
   zTernTmp = 1;
 }else {
   zTernTmp = -1;
 }

    if ((cpp2::impl::cmp_greater(mx,my) && cpp2::impl::cmp_greater(mx,mz))) {
      return VectorFromD(cpp2::move(xTernTmp), 0, 0); 
    }else {if ((cpp2::impl::cmp_greater(cpp2::move(my),cpp2::move(mz)))) {
      return VectorFromD(0, cpp2::move(yTernTmp), 0); 
    }else {
      return VectorFromD(0, 0, cpp2::move(zTernTmp)); 
    }}
  }

#line 259 "/boot/home/solvespace/src/vector.h2"
  auto Vector::MakeMaxMin(Vector& maxv, Vector& minv) const& -> void{
    maxv = VectorFromD(
      std::max(maxv.x, (*this).x), 
   std::max(maxv.y, (*this).y), 
   std::max(maxv.z, (*this).z)
    );
    minv = VectorFromD(
   std::min(minv.x, (*this).x), 
      std::min(minv.y, (*this).y), 
      std::min(minv.z, (*this).z)
    );
  }
#line 271 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::ClampWithin(cpp2::impl::in<double> minv, cpp2::impl::in<double> maxv) const& -> Vector{
    double tmpx {0}; 
    double tmpy {0}; 
    double tmpz {0}; 

    if ((cpp2::impl::cmp_less((*this).x,minv))) {tmpx = minv; }
    if ((cpp2::impl::cmp_less((*this).y,minv))) {tmpy = minv; }
    if ((cpp2::impl::cmp_less((*this).z,minv))) {tmpz = minv; }

    if ((cpp2::impl::cmp_greater((*this).x,maxv))) {tmpx = maxv; }
    if ((cpp2::impl::cmp_greater((*this).y,maxv))) {tmpy = maxv; }
    if ((cpp2::impl::cmp_greater((*this).z,maxv))) {tmpz = maxv; }

    return VectorFromD(cpp2::move(tmpx), cpp2::move(tmpy), cpp2::move(tmpz)); 
  }
#line 286 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::OutsideAndNotOn(cpp2::impl::in<Vector> maxv, cpp2::impl::in<Vector> minv) const& -> bool{
    return (cpp2::impl::cmp_greater((*this).x,maxv.x + LENGTH_EPS)) || (cpp2::impl::cmp_less((*this).x,minv.x - LENGTH_EPS)) || 
           (cpp2::impl::cmp_greater((*this).y,maxv.y + LENGTH_EPS)) || (cpp2::impl::cmp_less((*this).y,minv.y - LENGTH_EPS)) || 
           (cpp2::impl::cmp_greater((*this).z,maxv.z + LENGTH_EPS)) || (cpp2::impl::cmp_less((*this).z,minv.z - LENGTH_EPS)); 
  }
#line 291 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::InPerspective(cpp2::impl::in<Vector> u, cpp2::impl::in<Vector> v, cpp2::impl::in<Vector> n, cpp2::impl::in<Vector> origin, cpp2::impl::in<double> cameraTan) const& -> Vector{
    Vector r {CPP2_UFCS(DotInToCsys)(CPP2_UFCS(Minus)((*this), origin), u, v, n)}; 
    // yes, minus; we are assuming a csys where u cross v equals n, backwards
    // from the display stuff
    double w {1 - r.z * cameraTan}; 
    r = CPP2_UFCS(ScaledBy)(r, 1 / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(1),cpp2::move(w)));

    return r; 
  }
#line 300 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::Project2d(cpp2::impl::in<Vector> u, cpp2::impl::in<Vector> v) const& -> SolveSpace::Point2d{
    return SolveSpace::Point2d(CPP2_UFCS(Dot)((*this), u), CPP2_UFCS(Dot)((*this), v)); 
  }
#line 303 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::ProjectXy() const& -> SolveSpace::Point2d{
    return SolveSpace::Point2d((*this).x, (*this).y); 
  }
#line 306 "/boot/home/solvespace/src/vector.h2"
  [[nodiscard]] auto Vector::Project4d() const& -> Vector4{
    return Vector4::From(1, x, y, z); 
  }


  Vector::Vector(Vector const& that)
                                : x{ that.x }
                                , y{ that.y }
                                , z{ that.z }{}

auto Vector::operator=(Vector const& that) -> Vector& {
                                x = that.x;
                                y = that.y;
                                z = that.z;
                                return *this;}
Vector::Vector(Vector&& that) noexcept
                                : x{ std::move(that).x }
                                , y{ std::move(that).y }
                                , z{ std::move(that).z }{}
auto Vector::operator=(Vector&& that) noexcept -> Vector& {
                                x = std::move(that).x;
                                y = std::move(that).y;
                                z = std::move(that).z;
                                return *this;}
Vector::Vector(){}
#line 311 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorFromD(cpp2::impl::in<double> px, cpp2::impl::in<double> py, cpp2::impl::in<double> pz) -> Vector{
  return Vector(px, py, pz); 
}

#line 315 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorFromH(cpp2::impl::in<hParam> px, cpp2::impl::in<hParam> py, cpp2::impl::in<hParam> pz) -> Vector{
  return Vector((*cpp2::impl::assert_not_null(CPP2_UFCS(GetParam)(SK, px))).val, (*cpp2::impl::assert_not_null(CPP2_UFCS(GetParam)(SK, py))).val, (*cpp2::impl::assert_not_null(CPP2_UFCS(GetParam)(SK, pz))).val); 
}

#line 319 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorAtIntersectionOfPlanes(cpp2::impl::in<Vector> n1, cpp2::impl::in<double> d1, cpp2::impl::in<Vector> n2, cpp2::impl::in<double> d2) -> Vector{
  double det {(CPP2_UFCS(Dot)(n1, n1)) * (CPP2_UFCS(Dot)(n2, n2)) - (CPP2_UFCS(Dot)(n1, n2)) * (CPP2_UFCS(Dot)(n1, n2))}; 
  double c1 {(d1 * CPP2_UFCS(Dot)(n2, n2) - d2 * CPP2_UFCS(Dot)(n1, n2)) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF((d1 * CPP2_UFCS(Dot)(n2, n2) - d2 * CPP2_UFCS(Dot)(n1, n2))),det)}; 
  double c2 {(d2 * CPP2_UFCS(Dot)(n1, n1) - d1 * CPP2_UFCS(Dot)(n1, n2)) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF((d2 * CPP2_UFCS(Dot)(n1, n1) - d1 * CPP2_UFCS(Dot)(n1, n2))),cpp2::move(det))}; 

  return CPP2_UFCS(Plus)((CPP2_UFCS(ScaledBy)(n1, cpp2::move(c1))), CPP2_UFCS(ScaledBy)(n2, cpp2::move(c2))); 
}

#line 327 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorClosestPointBetweenLines(cpp2::impl::in<Vector> a0, cpp2::impl::in<Vector> da, cpp2::impl::in<Vector> b0, cpp2::impl::in<Vector> db) -> VectorClosestPointBetweenLines_ret{
      cpp2::impl::deferred_init<double> ta;
      cpp2::impl::deferred_init<double> tb;
  // make a semi-orthogonal coordinate system from those directions;
  // note that dna and dnb need not be perpendicular
#line 330 "/boot/home/solvespace/src/vector.h2"
  Vector dn {CPP2_UFCS(Cross)(da, db)}; // normal to both
  Vector dna {CPP2_UFCS(Cross)(dn, da)}; // normal to da
  Vector dnb {CPP2_UFCS(Cross)(cpp2::move(dn), db)}; // normal to db

  // at the intersection of the lines, a0 + pa*da = b0 + pb*db (where pa, pb are scalar params)
  // so dot this equation against dna and dnb to get two equations to solve for da and db
  ta.construct(-(CPP2_UFCS(Dot)((CPP2_UFCS(Minus)(a0, b0)), dnb)) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(-(CPP2_UFCS(Dot)((CPP2_UFCS(Minus)(a0, b0)), dnb))),(CPP2_UFCS(Dot)(da, dnb))));
  tb.construct((CPP2_UFCS(Dot)((CPP2_UFCS(Minus)(a0, b0)), dna)) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF((CPP2_UFCS(Dot)((CPP2_UFCS(Minus)(a0, b0)), dna))),(CPP2_UFCS(Dot)(db, dna))));
return  { std::move(ta.value()), std::move(tb.value()) }; }

#line 340 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorAtIntersectionOfLines(cpp2::impl::in<Vector> a0, cpp2::impl::in<Vector> a1, cpp2::impl::in<Vector> b0, cpp2::impl::in<Vector> b1, cpp2::impl::in<bool> skew) -> VectorAtIntersectionOfLines_ret{
      cpp2::impl::deferred_init<double> parama;
      cpp2::impl::deferred_init<double> paramb;
      cpp2::impl::deferred_init<Vector> intersectionPoint;
      cpp2::impl::deferred_init<bool> skewed;
#line 341 "/boot/home/solvespace/src/vector.h2"
  Vector da {CPP2_UFCS(Minus)(a1, a0)}; 
  Vector db {CPP2_UFCS(Minus)(b1, b0)}; 

  auto eeep {VectorClosestPointBetweenLines(a0, da, b0, db)}; 
  parama.construct(eeep.ta);
  paramb.construct(cpp2::move(eeep).tb);

  // and from either of those, we get the intersection point
  intersectionPoint.construct(CPP2_UFCS(Plus)(a0, CPP2_UFCS(ScaledBy)(cpp2::move(da), parama.value())));

  skewed.construct(false);
  if ((skew)) {// check if the intersection points on each line are actually coincident
 skewed.value() = !(CPP2_UFCS(Equals)(intersectionPoint.value(), CPP2_UFCS(Plus)(b0, CPP2_UFCS(ScaledBy)(cpp2::move(db), paramb.value()))));
  }return  { std::move(parama.value()), std::move(paramb.value()), std::move(intersectionPoint.value()), std::move(skewed.value()) }; 
}

#line 357 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorAtIntersectionOfPlaneAndLine(cpp2::impl::in<Vector> n, cpp2::impl::in<double> d, cpp2::impl::in<Vector> p0, cpp2::impl::in<Vector> p1, bool* parallel) -> Vector{
  Vector dp {CPP2_UFCS(Minus)(p1, p0)}; 

  if ((cpp2::impl::cmp_less(fabs(CPP2_UFCS(Dot)(n, dp)),LENGTH_EPS))) {
    if ((parallel)) {*cpp2::impl::assert_not_null(parallel) = true; }
 return Vector::From(0, 0, 0); 
  }
  if ((parallel)) {*cpp2::impl::assert_not_null(parallel) = false; }

  // n dot (p0 + t*dp) = d
  // (n dot p0) + t * (n dot dp) = d
  double t {(d - CPP2_UFCS(Dot)(n, p0)) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF((d - CPP2_UFCS(Dot)(n, p0))),(CPP2_UFCS(Dot)(n, dp)))}; 

  return CPP2_UFCS(Plus)(p0, CPP2_UFCS(ScaledBy)(cpp2::move(dp), cpp2::move(t))); 
}

#line 373 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorBoundingBoxesDisjoint(cpp2::impl::in<Vector> amax, cpp2::impl::in<Vector> amin, cpp2::impl::in<Vector> bmax, cpp2::impl::in<Vector> bmin) -> bool{
{
auto i{0};
#line 374 "/boot/home/solvespace/src/vector.h2"
  for( ; cpp2::impl::cmp_less(i,3); ++i ) {// for (i = 0; i < 3; i++) {
 if ((cpp2::impl::cmp_less(CPP2_UFCS(Element)(amax, i),CPP2_UFCS(Element)(bmin, i) - LENGTH_EPS))) {return true; }
 if ((cpp2::impl::cmp_greater(CPP2_UFCS(Element)(amin, i),CPP2_UFCS(Element)(bmax, i) + LENGTH_EPS))) {return true; }
  }
}
#line 378 "/boot/home/solvespace/src/vector.h2"
  return false; 
}

#line 381 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorBoundingBoxIntersectsLine(cpp2::impl::in<Vector> amax, cpp2::impl::in<Vector> amin, cpp2::impl::in<Vector> p0, cpp2::impl::in<Vector> p1, cpp2::impl::in<bool> asSegment) -> bool{
  Vector dp {CPP2_UFCS(Minus)(p1, p0)}; 
  double lp {CPP2_UFCS(Magnitude)(dp)}; 
  dp = CPP2_UFCS(ScaledBy)(dp, 1.0 / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(1.0),lp));
{
auto i{0};

#line 386 "/boot/home/solvespace/src/vector.h2"
  for( ; cpp2::impl::cmp_less(i,3); ++i ) {// for (i = 0; i < 3; i++) {
    int j {WRAP(i + 1, 3)}; 
    int k {WRAP(i + 2, 3)}; 
    if ((cpp2::impl::cmp_less(lp * fabs(CPP2_UFCS(Element)(dp, i)),LENGTH_EPS))) {continue; }
{
auto a{0};// parallel to plane

#line 391 "/boot/home/solvespace/src/vector.h2"
    for( ; cpp2::impl::cmp_less(a,2); ++a ) {// for (a = 0; a < 2; a++) {
      cpp2::impl::deferred_init<double> d; 
      if ((a == 0)) {d.construct(CPP2_UFCS(Element)(amax, i)); }else {d.construct(CPP2_UFCS(Element)(amin, i)); }
      // n dot (p0 + t*dp) = d
      // (n dot p0) + t * (n dot dp) = d
      double t {(cpp2::move(d.value()) - CPP2_UFCS(Element)(p0, i)) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF((cpp2::move(d.value()) - CPP2_UFCS(Element)(p0, i))),CPP2_UFCS(Element)(dp, i))}; 
      Vector p {CPP2_UFCS(Plus)(p0, CPP2_UFCS(ScaledBy)(dp, t))}; 

      if ((asSegment && (cpp2::impl::cmp_less(t,-LENGTH_EPS) || cpp2::impl::cmp_greater(t,(lp + LENGTH_EPS))))) {continue; }

      if ((cpp2::impl::cmp_greater(CPP2_UFCS(Element)(p, j),CPP2_UFCS(Element)(amax, j) + LENGTH_EPS))) {continue; }
      if ((cpp2::impl::cmp_greater(CPP2_UFCS(Element)(p, k),CPP2_UFCS(Element)(amax, k) + LENGTH_EPS))) {continue; }

      if ((cpp2::impl::cmp_less(CPP2_UFCS(Element)(p, j),CPP2_UFCS(Element)(amin, j) - LENGTH_EPS))) {continue; }
      if ((cpp2::impl::cmp_less(CPP2_UFCS(Element)(cpp2::move(p), k),CPP2_UFCS(Element)(amin, k) - LENGTH_EPS))) {continue; }
      return true; 
    }
}
#line 408 "/boot/home/solvespace/src/vector.h2"
  }
}
#line 409 "/boot/home/solvespace/src/vector.h2"
  return false; 
}

#line 412 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto det2(cpp2::impl::in<double> a1, cpp2::impl::in<double> b1, cpp2::impl::in<double> a2, cpp2::impl::in<double> b2) -> double{
  return (a1 * b2) - (b1 * a2); 
}

#line 416 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto det3(cpp2::impl::in<double> a1, cpp2::impl::in<double> b1, cpp2::impl::in<double> c1, 
       cpp2::impl::in<double> a2, cpp2::impl::in<double> b2, cpp2::impl::in<double> c2, 
       cpp2::impl::in<double> a3, cpp2::impl::in<double> b3, cpp2::impl::in<double> c3) -> double{
  return a1 * det2(b2, c2, b3, c3) - 
         b1 * det2(a2, c2, a3, c3) + 
         c1 * det2(a2, b2, a3, b3); 
}

#line 424 "/boot/home/solvespace/src/vector.h2"
[[nodiscard]] auto VectorAtIntersectionOfPlanes(cpp2::impl::in<Vector> na, cpp2::impl::in<double> da, cpp2::impl::in<Vector> nb, cpp2::impl::in<double> db, cpp2::impl::in<Vector> nc, cpp2::impl::in<double> dc, bool* parallel) -> Vector{
  double det {det3(na.x, na.y, na.z, 
                     nb.x, nb.y, nb.z, 
                     nc.x, nc.y, nc.z)}; 
  if ((cpp2::impl::cmp_less(fabs(det),1e-10))) {// arbitrary tolerance, not so good
    *cpp2::impl::assert_not_null(parallel) = true;
 return Vector(0, 0, 0); 
  }

  *cpp2::impl::assert_not_null(parallel) = false;

  double detx {det3(da, na.y, na.z, 
                      db,   nb.y, nb.z, 
                      dc,   nc.y, nc.z)}; 
  double dety {det3(na.x, da, na.z, 
                      nb.x, db,   nb.z, 
                      nc.x, dc,   nc.z)}; 
  double detz {det3(na.x, na.y, da, 
                      nb.x, nb.y, db, 
                      nc.x, nc.y, dc  )}; 

  return Vector(cpp2::move(detx) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(cpp2::move(detx)),det), cpp2::move(dety) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(cpp2::move(dety)),det), cpp2::move(detz) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(cpp2::move(detz)),det)); 
}
}

#endif
