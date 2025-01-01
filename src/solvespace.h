//-----------------------------------------------------------------------------
// all declarations not grouped specially elsewhere
//
// copyright 2008-2013 Jonathan Westhues
//-----------------------------------------------------------------------------

#pragma once

#include "debugging.h"
#include "resource.h"
#include "platform/platform.h"
#include "platform/gui.h"

#define EIGEN_NO_DEBUG
#undef Success
#include <Eigen/SparseCore>

// We declare these in advance instead of simply using FT_Library
// (defined as typedef FT_LibraryRec_* FT_Library) because including
// freetype.h invokes indescribable horrors and we would like to avoid
// doing that every time we include solvespace.h.
struct FT_LibraryRec_;
struct FT_FaceRec_;

// The few floating-point equality comparisons in SolveSpace have been
// carefully considered, so we disable the -Wfloat-equal warning for them
#ifdef __clang__
#  define EXACT(expr) \
    (_Pragma ("clang diagnostic push") _Pragma ("clang diagnostic ignored \"-Wfloat-equal\"") ( \
        expr) _Pragma ("clang diagnostic pop"))
#else
#  define EXACT(expr) (expr)
#endif

namespace SolveSpace {
  [[noreturn]] void AssertFailure (const char *file, unsigned line, const char *function,
                                   const char *condition, const char *message);

#if defined(__GNUC__)
  __attribute__ ((__format__ (__printf__, 1, 2)))
#endif
  std::string
  ssprintf (const char *fmt, ...);

  inline bool IsReasonable (double x) {
    return std::isnan (x) || x > 1e11 || x < -1e11;
  }

  inline int WRAP (int v, int n) {
    // Clamp it to the range [0, n)
    while (v >= n)
      v -= n;
    while (v < 0)
      v += n;
    return v;
  }

  inline double WRAP_NOT_0 (double v, double n) {
    // Clamp it to the range (0, n]
    while (v > n)
      v -= n;
    while (v <= 0)
      v += n;
    return v;
  }

  inline double WRAP_SYMMETRIC (double v, double n) {
    // Clamp it to the range (-n/2, n/2]
    while (v > n / 2)
      v -= n;
    while (v <= -n / 2)
      v += n;
    return v;
  }

#define CO(v) (v).x, (v).y, (v).z

  static constexpr double ANGLE_COS_EPS = 1e-6;
  static constexpr double LENGTH_EPS    = 1e-6;
  static constexpr double VERY_POSITIVE = 1e10;
  static constexpr double VERY_NEGATIVE = -1e10;

  using Platform::AllocTemporary;
  using Platform::FreeAllTemporary;

  class Expr;
  class ExprVector;
  class ExprQuaternion;
  class RgbaColor;
  enum class Command : uint32_t;

  enum class Unit : uint32_t { MM = 0, INCHES, METERS, FEET_INCHES };

  template<class Key, class T>
  using handle_map = std::map<Key, T>;

  class Group;
  class SSurface;
  class hEntity;
  class hParam;
  class Vector;
  class Vector4;
} // namespace SolveSpace

#include "geometry/vector.h"

namespace SolveSpace {
#include "datastructures.h"
#include "geometry/polygon.h"
#include "srf/surface.h"
#include "render/render.h"

  class Entity;
  class hEntity;
  class Param;
  typedef IdList<Entity, hEntity> EntityList;
  typedef IdList<Param, hParam>   ParamList;

  enum class SolveResult : uint32_t {
    OKAY                     = 0,
    DIDNT_CONVERGE           = 10,
    REDUNDANT_OKAY           = 11,
    REDUNDANT_DIDNT_CONVERGE = 12,
    TOO_MANY_UNKNOWNS        = 20
  };

#include "entitymain.h"
#include "expr.h"
#include "system.h"
#include "util.h"

#include "sketch.h"
  extern Sketch SK;

#include "ssui/ui.h"
} // namespace SolveSpace

using namespace SolveSpace;