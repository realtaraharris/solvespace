//-----------------------------------------------------------------------------
// All declarations not grouped specially elsewhere.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------

#ifndef SOLVESPACE_H
#define SOLVESPACE_H

#include "resource.h"
#include "platform/platform.h"
#include "platform/gui.h"

#include <cctype>
#include <climits>
#include <cmath>
#include <csetjmp>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <functional>
#include <locale>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define EIGEN_NO_DEBUG
#undef Success
#include <Eigen/SparseCore>

// We declare these in advance instead of simply using FT_Library
// (defined as typedef FT_LibraryRec_* FT_Library) because including
// freetype.h invokes indescribable horrors and we would like to avoid
// doing that every time we include solvespace.h.
struct FT_LibraryRec_;
struct FT_FaceRec_;

typedef struct _cairo cairo_t;
typedef struct _cairo_surface cairo_surface_t;

// The few floating-point equality comparisons in SolveSpace have been
// carefully considered, so we disable the -Wfloat-equal warning for them
#ifdef __clang__
#   define EXACT(expr) \
        (_Pragma("clang diagnostic push") \
         _Pragma("clang diagnostic ignored \"-Wfloat-equal\"") \
         (expr) \
         _Pragma("clang diagnostic pop"))
#else
#   define EXACT(expr) (expr)
#endif

// Debugging functions
#if defined(__GNUC__)
#define ssassert(condition, message) \
    do { \
        if(__builtin_expect((condition), true) == false) { \
            SolveSpace::AssertFailure(__FILE__, __LINE__, __func__, #condition, message); \
            __builtin_unreachable(); \
        } \
    } while(0)
#else
#define ssassert(condition, message) \
    do { \
        if((condition) == false) { \
            SolveSpace::AssertFailure(__FILE__, __LINE__, __func__, #condition, message); \
            abort(); \
        } \
    } while(0)
#endif

#define dbp SolveSpace::Platform::DebugPrint
#define DBPTRI(tri) \
    dbp("tri: (%.3f %.3f %.3f) (%.3f %.3f %.3f) (%.3f %.3f %.3f)", \
        CO((tri).a), CO((tri).b), CO((tri).c))

namespace SolveSpace {

using std::min;
using std::max;
using std::swap;
using std::fabs;

[[noreturn]]
void AssertFailure(const char *file, unsigned line, const char *function,
                   const char *condition, const char *message);

#if defined(__GNUC__)
__attribute__((__format__ (__printf__, 1, 2)))
#endif
std::string ssprintf(const char *fmt, ...);

inline bool IsReasonable(double x) {
    return std::isnan(x) || x > 1e11 || x < -1e11;
}

inline int WRAP(int v, int n) {
    // Clamp it to the range [0, n)
    while(v >= n) v -= n;
    while(v < 0) v += n;
    return v;
}
inline double WRAP_NOT_0(double v, double n) {
    // Clamp it to the range (0, n]
    while(v > n) v -= n;
    while(v <= 0) v += n;
    return v;
}
inline double WRAP_SYMMETRIC(double v, double n) {
    // Clamp it to the range (-n/2, n/2]
    while(v >   n/2) v -= n;
    while(v <= -n/2) v += n;
    return v;
}

#define CO(v) (v).x, (v).y, (v).z

static constexpr double ANGLE_COS_EPS =  1e-6;
static constexpr double LENGTH_EPS    =  1e-6;
static constexpr double VERY_POSITIVE =  1e10;
static constexpr double VERY_NEGATIVE = -1e10;


using Platform::AllocTemporary;
using Platform::FreeAllTemporary;

class Expr;
class ExprVector;
class ExprQuaternion;
class RgbaColor;
enum class Command : uint32_t;

enum class Unit : uint32_t {
    MM = 0,
    INCHES,
    METERS,
    FEET_INCHES
};

template<class Key, class T>
using handle_map = std::map<Key, T>;

class Group;
class SSurface;
class hEntity;
class hParam;
class Vector;
class Vector4;
}

#include "vector.h"

namespace SolveSpace {
#include "dsc.h"
#include "polygon.h"
#include "srf/surface.h"
#include "render/render.h"

class Entity;
class hEntity;
class Param;
typedef IdList<Entity,hEntity> EntityList;
typedef IdList<Param,hParam> ParamList;

enum class SolveResult : uint32_t {
    OKAY                     = 0,
    DIDNT_CONVERGE           = 10,
    REDUNDANT_OKAY           = 11,
    REDUNDANT_DIDNT_CONVERGE = 12,
    TOO_MANY_UNKNOWNS        = 20
};

#include "entitymain.h"
#include "ui.h"
#include "expr.h"

// Utility functions that are provided in the platform-independent code.
class utf8_iterator : std::iterator<std::forward_iterator_tag, char32_t> {
    const char *p, *n;
public:
    utf8_iterator(const char *p) : p(p), n(NULL) {}
    bool           operator==(const utf8_iterator &i) const { return p==i.p; }
    bool           operator!=(const utf8_iterator &i) const { return p!=i.p; }
    ptrdiff_t      operator- (const utf8_iterator &i) const { return p -i.p; }
    utf8_iterator& operator++()    { **this; p=n; n=NULL; return *this; }
    utf8_iterator  operator++(int) { utf8_iterator t(*this); operator++(); return t; }
    char32_t       operator*();
    const char*    ptr() const { return p; }
};
class ReadUTF8 {
    const std::string &str;
public:
    ReadUTF8(const std::string &str) : str(str) {}
    utf8_iterator begin() const { return utf8_iterator(&str[0]); }
    utf8_iterator end()   const { return utf8_iterator(&str[0] + str.length()); }
};


#define arraylen(x) (sizeof((x))/sizeof((x)[0]))
#define PI (3.1415926535897931)
void MakeMatrix(double *mat, double a11, double a12, double a13, double a14,
                             double a21, double a22, double a23, double a24,
                             double a31, double a32, double a33, double a34,
                             double a41, double a42, double a43, double a44);
void MultMatrix(double *mata, double *matb, double *matr);

int64_t GetMilliseconds();
void Message(const char *fmt, ...);
void MessageAndRun(std::function<void()> onDismiss, const char *fmt, ...);
void Error(const char *fmt, ...);

#include "system.h"
#include "ttf.h"
#include "filewriter.h"

#ifdef LIBRARY
#   define ENTITY EntityBase
#   define CONSTRAINT ConstraintBase
#else
#   define ENTITY Entity
#   define CONSTRAINT Constraint
#endif
class Sketch {
public:
    // These are user-editable, and define the sketch.
    IdList<Group,hGroup>            group;
    List<hGroup>                    groupOrder;
    IdList<CONSTRAINT,hConstraint>  constraint;
    IdList<Request,hRequest>        request;
    IdList<Style,hStyle>            style;

    // These are generated from the above.
    IdList<ENTITY,hEntity>          entity;
    IdList<Param,hParam>            param;

    inline CONSTRAINT *GetConstraint(hConstraint h)
        { return constraint.FindById(h); }
    inline ENTITY  *GetEntity (hEntity  h) { return entity. FindById(h); }
    inline Param   *GetParam  (hParam   h) { return param.  FindById(h); }
    inline Request *GetRequest(hRequest h) { return request.FindById(h); }
    inline Group   *GetGroup  (hGroup   h) { return group.  FindById(h); }
    // Styles are handled a bit differently.

    void Clear();

    BBox CalculateEntityBBox(bool includingInvisible);
    Group *GetRunningMeshGroupFor(hGroup h);
};
#undef ENTITY
#undef CONSTRAINT

#include "solvespaceui.h"

void ImportDxf(const Platform::Path &file);
void ImportDwg(const Platform::Path &file);
bool LinkIDF(const Platform::Path &filename, EntityList *le, SMesh *m, SShell *sh);
bool LinkStl(const Platform::Path &filename, EntityList *le, SMesh *m, SShell *sh);

#if defined(HAIKU_GUI)
	#include "platform/haiku/ui/HaikuSpaceUI.h"
    extern HaikuSpaceUI SS;
#else
    extern SolveSpaceUI SS;
#endif

extern Sketch SK;

}

#ifndef __OBJC__
using namespace SolveSpace;
#endif

#endif
