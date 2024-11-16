//-----------------------------------------------------------------------------
// The parametric structure of our sketch, in multiple groups, that generate
// geometric entities and surfaces.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------

#ifndef SOLVESPACE_SKETCH_H
#define SOLVESPACE_SKETCH_H

class hGroup;
class hRequest;
class hEntity;
class hParam;
class hStyle;
class hConstraint;
class hEquation;

class Entity;
class Param;
class Equation;
class Style;

enum class PolyError : uint32_t {
    GOOD              = 0,
    NOT_CLOSED        = 1,
    NOT_COPLANAR      = 2,
    SELF_INTERSECTING = 3,
    ZERO_LEN_EDGE     = 4
};

enum class StipplePattern : uint32_t {
    CONTINUOUS     = 0,
    SHORT_DASH     = 1,
    DASH           = 2,
    LONG_DASH      = 3,
    DASH_DOT       = 4,
    DASH_DOT_DOT   = 5,
    DOT            = 6,
    FREEHAND       = 7,
    ZIGZAG         = 8,

    LAST           = ZIGZAG
};

const std::vector<double> &StipplePatternDashes(StipplePattern pattern);
double StipplePatternLength(StipplePattern pattern);

enum class Command : uint32_t;

// All of the hWhatever handles are a 32-bit ID, that is used to represent
// some data structure in the sketch.
class hGroup {
public:
    // bits 15: 0   -- group index
    uint32_t v;

    inline hEntity entity(int i) const;
    inline hParam param(int i) const;
    inline hEquation equation(int i) const;
};

template<>
struct IsHandleOracle<hGroup> : std::true_type {};

class hRequest {
public:
    // bits 15: 0   -- request index
    uint32_t v;

    inline hEntity entity(int i) const;
    inline hParam param(int i) const;

    inline bool IsFromReferences() const;
};

template<>
struct IsHandleOracle<hRequest> : std::true_type {};

class hEntity {
public:
    // bits 15: 0   -- entity index
    //      31:16   -- request index
    uint32_t v;

    inline bool isFromRequest() const;
    inline hRequest request() const;
    inline hGroup group() const;
    inline hEquation equation(int i) const;
};

template<>
struct IsHandleOracle<hEntity> : std::true_type {};

class hParam {
public:
    // bits 15: 0   -- param index
    //      31:16   -- request index
    uint32_t v;

    inline hRequest request() const;
};

template<>
struct IsHandleOracle<hParam> : std::true_type {};

class hStyle {
public:
    uint32_t v;
};

template<>
struct IsHandleOracle<hStyle> : std::true_type {};

struct EntityId {
    uint32_t v;     // entity ID, starting from 0
};

template<>
struct IsHandleOracle<EntityId> : std::true_type {};

struct EntityKey {
    hEntity     input;
    int         copyNumber;
    // (input, copyNumber) gets mapped to ((Request)xxx).entity(h.v)
};
struct EntityKeyHash {
    size_t operator()(const EntityKey &k) const {
        size_t h1 = std::hash<uint32_t>{}(k.input.v),
               h2 = std::hash<uint32_t>{}(k.copyNumber);
        return h1 ^ (h2 << 1);
    }
};
struct EntityKeyEqual {
    bool operator()(const EntityKey &a, const EntityKey &b) const {
        return std::tie(a.input, a.copyNumber) == std::tie(b.input, b.copyNumber);
    }
};
typedef std::unordered_map<EntityKey, EntityId, EntityKeyHash, EntityKeyEqual> EntityMap;

#include "group.h"

// A user request for some primitive or derived operation; for example a
// line, or a step and repeat.
class Request {
public:
    // Some predefined requests, that are present in every sketch.
    static const hRequest   HREQUEST_REFERENCE_XY;
    static const hRequest   HREQUEST_REFERENCE_YZ;
    static const hRequest   HREQUEST_REFERENCE_ZX;

    int         tag;
    hRequest    h;

    // Types of requests
    enum class Type : uint32_t {
        WORKPLANE              = 100,
        DATUM_POINT            = 101,
        LINE_SEGMENT           = 200,
        CUBIC                  = 300,
        CUBIC_PERIODIC         = 301,
        CIRCLE                 = 400,
        ARC_OF_CIRCLE          = 500,
        TTF_TEXT               = 600,
        IMAGE                  = 700
    };

    Request::Type type;
    int         extraPoints;

    hEntity     workplane; // or Entity::FREE_IN_3D
    hGroup      group;
    hStyle      style;

    bool        construction;

    std::string str;
    std::string font;
    Platform::Path file;
    double      aspectRatio;
    int groupRequestIndex;

    static hParam AddParam(ParamList *param, hParam hp);
    void Generate(EntityList *entity, ParamList *param);

    std::string DescriptionString() const;
    int IndexOfPoint(hEntity he) const;

    void Clear() {}
};

#include "entitybase.h"
#include "entity.h"

class EntReqTable {
public:
    static bool GetRequestInfo(Request::Type req, int extraPoints,
                               EntityBase::Type *ent, int *pts, bool *hasNormal, bool *hasDistance);
    static bool GetEntityInfo(EntityBase::Type ent, int extraPoints,
                              Request::Type *req, int *pts, bool *hasNormal, bool *hasDistance);
    static Request::Type GetRequestForEntity(EntityBase::Type ent);
};

class Param {
public:
    int         tag;
    hParam      h;

    double      val;
    bool        known;
    bool        free;

    // Used only in the solver
    Param       *substd;

    static const hParam NO_PARAM;

    void Clear() {}
};


class hConstraint {
public:
    uint32_t v;

    inline hEquation equation(int i) const;
    inline hParam param(int i) const;
};

template<>
struct IsHandleOracle<hConstraint> : std::true_type {};

#include "constraintbase.h"
#include "constraint.h"

class hEquation {
public:
    uint32_t v;

    inline bool isFromConstraint() const;
    inline hConstraint constraint() const;
};

template<>
struct IsHandleOracle<hEquation> : std::true_type {};

class Equation {
public:
    int         tag;
    hEquation   h;

    Expr        *e;

    void Clear() {}
};


class Style {
public:
    int         tag;
    hStyle      h;

    enum {
        // If an entity has no style, then it will be colored according to
        // whether the group that it's in is active or not, whether it's
        // construction or not, and so on.
        NO_STYLE       = 0,

        ACTIVE_GRP     = 1,
        CONSTRUCTION   = 2,
        INACTIVE_GRP   = 3,
        DATUM          = 4,
        SOLID_EDGE     = 5,
        CONSTRAINT     = 6,
        SELECTED       = 7,
        HOVERED        = 8,
        CONTOUR_FILL   = 9,
        NORMALS        = 10,
        ANALYZE        = 11,
        DRAW_ERROR     = 12,
        DIM_SOLID      = 13,
        HIDDEN_EDGE    = 14,
        OUTLINE        = 15,

        FIRST_CUSTOM   = 0x100
    };

    std::string name;

    enum class UnitsAs : uint32_t {
        PIXELS   = 0,
        MM       = 1
    };
    double      width;
    UnitsAs     widthAs;
    double      textHeight;
    UnitsAs     textHeightAs;
    enum class TextOrigin : uint32_t {
        NONE    = 0x00,
        LEFT    = 0x01,
        RIGHT   = 0x02,
        BOT     = 0x04,
        TOP     = 0x08
    };
    TextOrigin  textOrigin;
    double      textAngle;
    RgbaColor   color;
    bool        filled;
    RgbaColor   fillColor;
    bool        visible;
    bool        exportable;
    StipplePattern stippleType;
    double      stippleScale;
    int         zIndex;

    // The default styles, for entities that don't have a style assigned yet,
    // and for datums and such.
    typedef struct {
        hStyle      h;
        const char *cnfPrefix;
        RgbaColor   color;
        double      width;
        int         zIndex;
        bool        exportable;
        StipplePattern stippleType;
    } Default;
    static const Default Defaults[];

    static std::string CnfColor(const std::string &prefix);
    static std::string CnfWidth(const std::string &prefix);
    static std::string CnfStippleType(const std::string &prefix);
    static std::string CnfStippleScale(const std::string &prefix);
    static std::string CnfTextHeight(const std::string &prefix);
    static std::string CnfPrefixToName(const std::string &prefix);
    static std::string CnfExportable(const std::string &prefix);

    static void CreateAllDefaultStyles();
    static void CreateDefaultStyle(hStyle h);
    static void FillDefaultStyle(Style *s, const Default *d = NULL, bool factory = false);
    static void FreezeDefaultStyles(Platform::SettingsRef settings);
    static void LoadFactoryDefaults();

    static void AssignSelectionToStyle(uint32_t v);
    static uint32_t CreateCustomStyle(bool rememberForUndo = true);

    static RgbaColor RewriteColor(RgbaColor rgb);

    static Style *Get(hStyle hs);
    static RgbaColor Color(hStyle hs, bool forExport=false);
    static RgbaColor Color(int hs, bool forExport=false);
    static RgbaColor FillColor(hStyle hs, bool forExport=false);
    static double Width(hStyle hs);
    static double Width(int hs);
    static double WidthMm(int hs);
    static double TextHeight(hStyle hs);
    static double DefaultTextHeight();
    static Canvas::Stroke Stroke(hStyle hs);
    static Canvas::Stroke Stroke(int hs);
    static bool Exportable(int hs);
    static hStyle ForEntity(hEntity he);
    static StipplePattern PatternType(hStyle hs);
    static double StippleScale(hStyle hs);
    static double StippleScaleMm(hStyle hs);
    static std::string StipplePatternName(hStyle hs);
    static std::string StipplePatternName(StipplePattern stippleType);
    static StipplePattern StipplePatternFromString(std::string name);

    std::string DescriptionString() const;

    void Clear() {}
};


inline hEntity hGroup::entity(int i) const
    { hEntity r; r.v = 0x80000000 | (v << 16) | (uint32_t)i; return r; }
inline hParam hGroup::param(int i) const
    { hParam r; r.v = 0x80000000 | (v << 16) | (uint32_t)i; return r; }
inline hEquation hGroup::equation(int i) const
    { hEquation r; r.v = (v << 16) | 0x80000000 | (uint32_t)i; return r; }

inline bool hRequest::IsFromReferences() const {
    if(*this == Request::HREQUEST_REFERENCE_XY) return true;
    if(*this == Request::HREQUEST_REFERENCE_YZ) return true;
    if(*this == Request::HREQUEST_REFERENCE_ZX) return true;
    return false;
}
inline hEntity hRequest::entity(int i) const
    { hEntity r; r.v = (v << 16) | (uint32_t)i; return r; }
inline hParam hRequest::param(int i) const
    { hParam r; r.v = (v << 16) | (uint32_t)i; return r; }

inline bool hEntity::isFromRequest() const
    { if(v & 0x80000000) return false; else return true; }
inline hRequest hEntity::request() const
    { hRequest r; r.v = (v >> 16); return r; }
inline hGroup hEntity::group() const
    { hGroup r; r.v = (v >> 16) & 0x3fff; return r; }
inline hEquation hEntity::equation(int i) const
    { hEquation r; r.v = v | 0x40000000 | (uint32_t)i; return r; }

inline hRequest hParam::request() const
    { hRequest r; r.v = (v >> 16); return r; }


inline hEquation hConstraint::equation(int i) const
    { hEquation r; r.v = (v << 16) | (uint32_t)i; return r; }
inline hParam hConstraint::param(int i) const
    { hParam r; r.v = v | 0x40000000 | (uint32_t)i; return r; }

inline bool hEquation::isFromConstraint() const
    { if(v & 0xc0000000) return false; else return true; }
inline hConstraint hEquation::constraint() const
    { hConstraint r; r.v = (v >> 16); return r; }

// The format for entities stored on the clipboard.
class ClipboardRequest {
public:
    Request::Type type;
    int         extraPoints;
    hStyle      style;
    std::string str;
    std::string font;
    Platform::Path file;
    bool        construction;

    Vector      point[MAX_POINTS_IN_ENTITY] = {
	    Vector(0, 0, 0),
		Vector(0, 0, 0),
		Vector(0, 0, 0),

		Vector(0, 0, 0),
		Vector(0, 0, 0),
		Vector(0, 0, 0),

		Vector(0, 0, 0),
		Vector(0, 0, 0),
		Vector(0, 0, 0),

		Vector(0, 0, 0),
		Vector(0, 0, 0),
		Vector(0, 0, 0)
	};
    double      distance;

    hEntity     oldEnt;
    hEntity     oldPointEnt[MAX_POINTS_IN_ENTITY];
    hRequest    newReq;
};

#endif
