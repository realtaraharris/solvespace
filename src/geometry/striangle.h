
#ifndef _BOOT_HOME_SOLVESPACE_SRC_STRIANGLE_H_CPP2
#define _BOOT_HOME_SOLVESPACE_SRC_STRIANGLE_H_CPP2

#define CPP2_IMPORT_STD          Yes

//=== Cpp2 type declarations ====================================================


#include "cpp2util.h"

#line 1 "/boot/home/solvespace/src/striangle.h2"
class STriangle;
#line 2 "/boot/home/solvespace/src/striangle.h2"
  

//=== Cpp2 type definitions and function declarations ===========================

#line 1 "/boot/home/solvespace/src/striangle.h2"
class STriangle {
#line 2 "/boot/home/solvespace/src/striangle.h2"
  public: cpp2::i32 tag {}; 
  public: STriMeta meta {}; 

  public: Vector a {};   // TODO: try taking these private once this replaces the cpp1 Vector class
  public: Vector b {}; 
  public: Vector c {}; 

  public: Vector an {}; 
  public: Vector bn {}; 
  public: Vector cn {}; 

  public: explicit STriangle();
  public: STriangle(STriangle const& that);
#line 14 "/boot/home/solvespace/src/striangle.h2"
  public: auto operator=(STriangle const& that) -> STriangle& ;
#line 14 "/boot/home/solvespace/src/striangle.h2"
  public: STriangle(STriangle&& that) noexcept;
#line 14 "/boot/home/solvespace/src/striangle.h2"
  public: auto operator=(STriangle&& that) noexcept -> STriangle& ;

#line 25 "/boot/home/solvespace/src/striangle.h2"
  public: explicit STriangle(cpp2::impl::in<STriMeta> metax, cpp2::impl::in<Vector> ax, cpp2::impl::in<Vector> bx, cpp2::impl::in<Vector> cx);

#line 31 "/boot/home/solvespace/src/striangle.h2"
  public: explicit STriangle(cpp2::impl::in<STriMeta> metax, cpp2::impl::in<Vector> ax, cpp2::impl::in<Vector> bx, cpp2::impl::in<Vector> cx, cpp2::impl::in<Vector> anx, cpp2::impl::in<Vector> bnx, cpp2::impl::in<Vector> cnx);

#line 41 "/boot/home/solvespace/src/striangle.h2"
  public: [[nodiscard]] static auto From(cpp2::impl::in<STriMeta> metax, cpp2::impl::in<Vector> ax, cpp2::impl::in<Vector> bx, cpp2::impl::in<Vector> cx) -> STriangle;

#line 44 "/boot/home/solvespace/src/striangle.h2"
  public: [[nodiscard]] auto Normal() const& -> Vector;

#line 49 "/boot/home/solvespace/src/striangle.h2"
  public: auto FlipNormal() & -> void;

#line 53 "/boot/home/solvespace/src/striangle.h2"
  public: [[nodiscard]] auto MinAltitude() const& -> double;

#line 60 "/boot/home/solvespace/src/striangle.h2"
  public: [[nodiscard]] auto ContainsPoint(cpp2::impl::in<Vector> p) const& -> bool;

#line 67 "/boot/home/solvespace/src/striangle.h2"
  public: [[nodiscard]] auto ContainsPointProjd(cpp2::impl::in<Vector> n, cpp2::impl::in<Vector> p) const& -> bool;

#line 83 "/boot/home/solvespace/src/striangle.h2"
  public: [[nodiscard]] auto Transform(cpp2::impl::in<Vector> u, cpp2::impl::in<Vector> v, cpp2::impl::in<Vector> n) const& -> STriangle;
struct Raytrace_ret { bool hit; double t; Vector inters; };



#line 93 "/boot/home/solvespace/src/striangle.h2"
  public: [[nodiscard]] auto Raytrace(cpp2::impl::in<Vector> rayPoint, cpp2::impl::in<Vector> rayDir, cpp2::impl::in<bool> calcIntersection) const& -> Raytrace_ret;

#line 135 "/boot/home/solvespace/src/striangle.h2"
  public: [[nodiscard]] auto SignedVolume() const& -> double;

#line 138 "/boot/home/solvespace/src/striangle.h2"
  public: [[nodiscard]] auto Area() const& -> double;

#line 143 "/boot/home/solvespace/src/striangle.h2"
  public: [[nodiscard]] auto IsDegenerate() const& -> bool;

#line 148 "/boot/home/solvespace/src/striangle.h2"
  public: [[nodiscard]] auto vertices(cpp2::impl::in<int> i) const& -> Vector;

#line 159 "/boot/home/solvespace/src/striangle.h2"
  public: [[nodiscard]] auto normals(cpp2::impl::in<int> i) const& -> Vector;

#line 170 "/boot/home/solvespace/src/striangle.h2"
};

#endif
