
#ifndef _BOOT_HOME_SOLVESPACE_SRC_STRIANGLE_H_CPP2
#error This file is part of a '.h2' header compiled to be consumed from another -pure-cpp2 file. To use this file, write '#include "/boot/home/solvespace/src/striangle.h2"' in a '.h2' or '.cpp2' file compiled with -pure-cpp2.
#endif

#ifndef _BOOT_HOME_SOLVESPACE_SRC_STRIANGLE_HPP_CPP2
#define _BOOT_HOME_SOLVESPACE_SRC_STRIANGLE_HPP_CPP2



//=== Cpp2 function definitions =================================================

#line 1 "/boot/home/solvespace/src/striangle.h2"

#line 13 "/boot/home/solvespace/src/striangle.h2"
  STriangle::STriangle(){}
#line 14 "/boot/home/solvespace/src/striangle.h2"
  STriangle::STriangle(STriangle const& that)
 : tag{ that.tag }
 , meta{ that.meta }
 , a{ that.a }
 , b{ that.b }
 , c{ that.c }
 , an{ that.an }
 , bn{ that.bn }
 , cn{ that.cn }{

#line 24 "/boot/home/solvespace/src/striangle.h2"
  }
#line 14 "/boot/home/solvespace/src/striangle.h2"
  auto STriangle::operator=(STriangle const& that) -> STriangle& {
 tag = that.tag;
 meta = that.meta;
 a = that.a;
 b = that.b;
 c = that.c;
 an = that.an;
 bn = that.bn;
 cn = that.cn;
 return *this;

#line 24 "/boot/home/solvespace/src/striangle.h2"
  }
#line 14 "/boot/home/solvespace/src/striangle.h2"
  STriangle::STriangle(STriangle&& that) noexcept
 : tag{ cpp2::move(that).tag }
 , meta{ cpp2::move(that).meta }
 , a{ cpp2::move(that).a }
 , b{ cpp2::move(that).b }
 , c{ cpp2::move(that).c }
 , an{ cpp2::move(that).an }
 , bn{ cpp2::move(that).bn }
 , cn{ cpp2::move(that).cn }{

#line 24 "/boot/home/solvespace/src/striangle.h2"
  }
#line 14 "/boot/home/solvespace/src/striangle.h2"
  auto STriangle::operator=(STriangle&& that) noexcept -> STriangle& {
 tag = cpp2::move(that).tag;
 meta = cpp2::move(that).meta;
 a = cpp2::move(that).a;
 b = cpp2::move(that).b;
 c = cpp2::move(that).c;
 an = cpp2::move(that).an;
 bn = cpp2::move(that).bn;
 cn = cpp2::move(that).cn;
 return *this;

#line 24 "/boot/home/solvespace/src/striangle.h2"
  }
#line 25 "/boot/home/solvespace/src/striangle.h2"
  STriangle::STriangle(cpp2::impl::in<STriMeta> metax, cpp2::impl::in<Vector> ax, cpp2::impl::in<Vector> bx, cpp2::impl::in<Vector> cx)
 : meta{ metax }
 , a{ ax }
 , b{ bx }
 , c{ cx }{

#line 30 "/boot/home/solvespace/src/striangle.h2"
  }
#line 31 "/boot/home/solvespace/src/striangle.h2"
  STriangle::STriangle(cpp2::impl::in<STriMeta> metax, cpp2::impl::in<Vector> ax, cpp2::impl::in<Vector> bx, cpp2::impl::in<Vector> cx, cpp2::impl::in<Vector> anx, cpp2::impl::in<Vector> bnx, cpp2::impl::in<Vector> cnx)
 : meta{ metax }
 , a{ ax }
 , b{ bx }
 , c{ cx }
 , an{ anx }
 , bn{ bnx }
 , cn{ cnx }{

#line 40 "/boot/home/solvespace/src/striangle.h2"
  }
#line 41 "/boot/home/solvespace/src/striangle.h2"
  [[nodiscard]] auto STriangle::From(cpp2::impl::in<STriMeta> metax, cpp2::impl::in<Vector> ax, cpp2::impl::in<Vector> bx, cpp2::impl::in<Vector> cx) -> STriangle{
    return STriangle(metax, ax, bx, cx); 
  }
#line 44 "/boot/home/solvespace/src/striangle.h2"
  [[nodiscard]] auto STriangle::Normal() const& -> Vector{
    Vector ab {CPP2_UFCS(Minus)((*this).b, (*this).a)}; 
 Vector bc {CPP2_UFCS(Minus)((*this).c, (*this).b)}; 
    return CPP2_UFCS(Cross)(cpp2::move(ab), cpp2::move(bc)); 
  }
#line 49 "/boot/home/solvespace/src/striangle.h2"
  auto STriangle::FlipNormal() & -> void{
    std::swap((*this).a, (*this).b);
    std::swap((*this).an, (*this).bn);
  }
#line 53 "/boot/home/solvespace/src/striangle.h2"
  [[nodiscard]] auto STriangle::MinAltitude() const& -> double{
 double altA {CPP2_UFCS(DistanceToLine)((*this).a, (*this).b, CPP2_UFCS(Minus)((*this).c, (*this).b))}; 
 double altB {CPP2_UFCS(DistanceToLine)((*this).b, (*this).c, CPP2_UFCS(Minus)((*this).a, (*this).c))}; 
 double altC {CPP2_UFCS(DistanceToLine)((*this).c, (*this).a, CPP2_UFCS(Minus)((*this).b, (*this).a))}; 

    return min(cpp2::move(altA), min(cpp2::move(altB), cpp2::move(altC))); 
  }
#line 60 "/boot/home/solvespace/src/striangle.h2"
  [[nodiscard]] auto STriangle::ContainsPoint(cpp2::impl::in<Vector> p) const& -> bool{
    Vector n {CPP2_UFCS(Normal)((*this))}; 
    if ((cpp2::impl::cmp_less(CPP2_UFCS(MinAltitude)((*this)),LENGTH_EPS))) {// shouldn't happen; zero-area triangle
        return false; 
    }
    return CPP2_UFCS(ContainsPointProjd)((*this), CPP2_UFCS(WithMagnitude)(cpp2::move(n), 1.0), p); 
  }
#line 67 "/boot/home/solvespace/src/striangle.h2"
  [[nodiscard]] auto STriangle::ContainsPointProjd(cpp2::impl::in<Vector> n, cpp2::impl::in<Vector> p) const& -> bool{
    Vector ab {CPP2_UFCS(Minus)((*this).b, (*this).a)}; 
 Vector bc {CPP2_UFCS(Minus)((*this).c, (*this).b)}; 
 Vector ca {CPP2_UFCS(Minus)((*this).a, (*this).c)}; 

    Vector no_ab {CPP2_UFCS(Cross)(n, cpp2::move(ab))}; 
    if ((cpp2::impl::cmp_less(CPP2_UFCS(Dot)(no_ab, p),CPP2_UFCS(Dot)(no_ab, (*this).a) - LENGTH_EPS))) {return false; }

    Vector no_bc {CPP2_UFCS(Cross)(n, cpp2::move(bc))}; 
    if ((cpp2::impl::cmp_less(CPP2_UFCS(Dot)(no_bc, p),CPP2_UFCS(Dot)(no_bc, (*this).b) - LENGTH_EPS))) {return false; }

    Vector no_ca {CPP2_UFCS(Cross)(n, cpp2::move(ca))}; 
    if ((cpp2::impl::cmp_less(CPP2_UFCS(Dot)(no_ca, p),CPP2_UFCS(Dot)(no_ca, (*this).c) - LENGTH_EPS))) {return false; }

    return true; 
  }
#line 83 "/boot/home/solvespace/src/striangle.h2"
  [[nodiscard]] auto STriangle::Transform(cpp2::impl::in<Vector> u, cpp2::impl::in<Vector> v, cpp2::impl::in<Vector> n) const& -> STriangle{
 return STriangle((*this).meta, 
   CPP2_UFCS(ScaleOutOfCsys)((*this).a, u, v, n), 
   CPP2_UFCS(ScaleOutOfCsys)((*this).b, u, v, n), 
   CPP2_UFCS(ScaleOutOfCsys)((*this).c, u, v, n), 
      CPP2_UFCS(ScaleOutOfCsys)((*this).an, u, v, n), 
      CPP2_UFCS(ScaleOutOfCsys)((*this).bn, u, v, n), 
      CPP2_UFCS(ScaleOutOfCsys)((*this).cn, u, v, n)
 ); 
  }
#line 93 "/boot/home/solvespace/src/striangle.h2"
  [[nodiscard]] auto STriangle::Raytrace(cpp2::impl::in<Vector> rayPoint, cpp2::impl::in<Vector> rayDir, cpp2::impl::in<bool> calcIntersection) const& -> Raytrace_ret{
     cpp2::impl::deferred_init<bool> hit;
     cpp2::impl::deferred_init<double> t;
     cpp2::impl::deferred_init<Vector> inters;
 // algorithm from "Fast, Minimum Storage Ray/Triangle Intersection" by Tomas Moeller & Ben Trumbore
#line 95 "/boot/home/solvespace/src/striangle.h2"
 hit.construct(false);
 t.construct(0);
 inters.construct(Vector(0, 0, 0));

    // find vectors for two edges sharing vertex A
    Vector edge1 {CPP2_UFCS(Minus)((*this).b, (*this).a)}; 
    Vector edge2 {CPP2_UFCS(Minus)((*this).c, (*this).a)}; 

    // begin calculating determinant - also used to calculate U parameter
    Vector pvec {CPP2_UFCS(Cross)(rayDir, edge2)}; 

    // if determinant is near zero, ray lies in plane of triangle. also cull back-facing triangles here
    double det {CPP2_UFCS(Dot)(edge1, pvec)}; 
    if ((cpp2::impl::cmp_less(-det,LENGTH_EPS))) {return  { std::move(hit.value()), std::move(t.value()), std::move(inters.value()) }; }
 double inv_det {1.0f / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(1.0f),cpp2::move(det))}; 

    // calculate distance from vertex A to ray origin
    Vector tvec {CPP2_UFCS(Minus)(rayPoint, (*this).a)}; 

    // calculate U parameter and test bounds
    double u {CPP2_UFCS(Dot)(tvec, cpp2::move(pvec)) * inv_det}; 
    if ((cpp2::impl::cmp_less(u,0.0f) || cpp2::impl::cmp_greater(u,1.0f))) {return  { std::move(hit.value()), std::move(t.value()), std::move(inters.value()) }; }

    // prepare to test V parameter
    Vector qvec {CPP2_UFCS(Cross)(cpp2::move(tvec), cpp2::move(edge1))}; 

    // calculate v parameter and test bounds
    double v {CPP2_UFCS(Dot)(rayDir, qvec) * inv_det}; 
    if ((cpp2::impl::cmp_less(v,0.0f) || cpp2::impl::cmp_greater(cpp2::move(u) + v,1.0f))) {return  { std::move(hit.value()), std::move(t.value()), std::move(inters.value()) }; }

    // calculate t, ray intersects triangle
    t.value() = CPP2_UFCS(Dot)(cpp2::move(edge2), cpp2::move(qvec)) * cpp2::move(inv_det);

    // calculate intersection point
    if ((calcIntersection)) {
  inters.value() = CPP2_UFCS(Plus)(rayPoint, CPP2_UFCS(ScaledBy)(rayDir, t.value()));
 }

    hit.value() = true;
  return  { std::move(hit.value()), std::move(t.value()), std::move(inters.value()) }; }
#line 135 "/boot/home/solvespace/src/striangle.h2"
  [[nodiscard]] auto STriangle::SignedVolume() const& -> double{
    return CPP2_UFCS(Dot)((*this).a, CPP2_UFCS(Cross)((*this).b, (*this).c)) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(CPP2_UFCS(Dot)((*this).a, CPP2_UFCS(Cross)((*this).b, (*this).c))),6.0); 
  }
#line 138 "/boot/home/solvespace/src/striangle.h2"
  [[nodiscard]] auto STriangle::Area() const& -> double{
    Vector ab {CPP2_UFCS(Minus)((*this).a, (*this).b)}; 
    Vector cb {CPP2_UFCS(Minus)((*this).c, (*this).b)}; 
    return CPP2_UFCS(Magnitude)(CPP2_UFCS(Cross)(cpp2::move(ab), cpp2::move(cb))) / CPP2_ASSERT_NOT_ZERO(CPP2_TYPEOF(CPP2_UFCS(Magnitude)(CPP2_UFCS(Cross)(cpp2::move(ab), cpp2::move(cb)))),2.0); 
  }
#line 143 "/boot/home/solvespace/src/striangle.h2"
  [[nodiscard]] auto STriangle::IsDegenerate() const& -> bool{
    return CPP2_UFCS(OnLineSegment)((*this).a, (*this).b, (*this).c) || 
           CPP2_UFCS(OnLineSegment)((*this).b, (*this).a, (*this).c) || 
           CPP2_UFCS(OnLineSegment)((*this).c, (*this).a, (*this).b); 
  }
#line 148 "/boot/home/solvespace/src/striangle.h2"
  [[nodiscard]] auto STriangle::vertices(cpp2::impl::in<int> i) const& -> Vector{
    if ((i == 0)) {
    return (*this).a; 
 }else {if ((i == 1)) {
    return (*this).b; 
 }else {if ((i == 2)) {
    return (*this).c; 
 }else {
  ssassert(false, "invalid array access on vertices");
 }}}
  }
#line 159 "/boot/home/solvespace/src/striangle.h2"
  [[nodiscard]] auto STriangle::normals(cpp2::impl::in<int> i) const& -> Vector{
    if ((i == 0)) {
    return (*this).an; 
 }else {if ((i == 1)) {
    return (*this).bn; 
 }else {if ((i == 2)) {
    return (*this).cn; 
 }else {
  ssassert(false, "invalid array access on normal");
 }}}
  }
#endif

