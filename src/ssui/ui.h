//-----------------------------------------------------------------------------
// Declarations relating to our user interface, in both the graphics and
// text browser window.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------

#ifndef SOLVESPACE_UI_H
#define SOLVESPACE_UI_H

class Locale {
public:
    std::string language;
    std::string region;
    uint16_t    lcid;
    std::string displayName;

    std::string Culture() const {
        return language + "-" + region;
    }
};

struct LocaleLess {
    bool operator()(const Locale &a, const Locale &b) const {
        return a.language < b.language ||
            (a.language == b.language && a.region < b.region);
    }
};

const std::set<Locale, LocaleLess> &Locales();
bool SetLocale(const std::string &name);
bool SetLocale(uint16_t lcid);

const std::string &Translate(const char *msgid);
const std::string &Translate(const char *msgctxt, const char *msgid);
const std::string &TranslatePlural(const char *msgid, unsigned n);
const std::string &TranslatePlural(const char *msgctxt, const char *msgid, unsigned n);

inline const char *N_(const char *msgid) {
    return msgid;
}
inline const char *CN_(const char *msgctxt, const char *msgid) {
    return msgid;
}

inline const char *_(const char *msgid) {
    return Translate(msgid).c_str();
}
inline const char *C_(const char *msgctxt, const char *msgid) {
    return Translate(msgctxt, msgid).c_str();
}

// This table describes the top-level menus in the graphics window.
enum class Command : uint32_t {
    NONE = 0,
    // File
    NEW = 100,
    OPEN,
    OPEN_RECENT,
    SAVE,
    SAVE_AS,
    EXPORT_IMAGE,
    EXPORT_MESH,
    EXPORT_SURFACES,
    EXPORT_VIEW,
    EXPORT_SECTION,
    EXPORT_WIREFRAME,
    IMPORT,
    EXIT,
    // View
    ZOOM_IN,
    ZOOM_OUT,
    ZOOM_TO_FIT,
    SHOW_GRID,
    DIM_SOLID_MODEL,
    PERSPECTIVE_PROJ,
    EXPLODE_SKETCH,
    ONTO_WORKPLANE,
    NEAREST_ORTHO,
    NEAREST_ISO,
    CENTER_VIEW,
    SHOW_TOOLBAR,
    SHOW_TEXT_WND,
    UNITS_INCHES,
    UNITS_FEET_INCHES,
    UNITS_MM,
    UNITS_METERS,
    FULL_SCREEN,
    // Edit
    UNDO,
    REDO,
    CUT,
    COPY,
    PASTE,
    PASTE_TRANSFORM,
    DELETE,
    SELECT_CHAIN,
    SELECT_ALL,
    SNAP_TO_GRID,
    ROTATE_90,
    UNSELECT_ALL,
    REGEN_ALL,
    EDIT_LINE_STYLES,
    VIEW_PROJECTION,
    CONFIGURATION,
    // Request
    SEL_WORKPLANE,
    FREE_IN_3D,
    DATUM_POINT,
    WORKPLANE,
    LINE_SEGMENT,
    CONSTR_SEGMENT,
    CIRCLE,
    ARC,
    RECTANGLE,
    CUBIC,
    TTF_TEXT,
    IMAGE,
    SPLIT_CURVES,
    TANGENT_ARC,
    CONSTRUCTION,
    // Group
    GROUP_3D,
    GROUP_WRKPL,
    GROUP_EXTRUDE,
    GROUP_HELIX,
    GROUP_LATHE,
    GROUP_REVOLVE,
    GROUP_ROT,
    GROUP_TRANS,
    GROUP_LINK,
    GROUP_RECENT,
    // Constrain
    DISTANCE_DIA,
    REF_DISTANCE,
    ANGLE,
    REF_ANGLE,
    OTHER_ANGLE,
    REFERENCE,
    EQUAL,
    RATIO,
    DIFFERENCE,
    ON_ENTITY,
    SYMMETRIC,
    AT_MIDPOINT,
    HORIZONTAL,
    VERTICAL,
    PARALLEL,
    PERPENDICULAR,
    ORIENTED_SAME,
    WHERE_DRAGGED,
    COMMENT,
    // Analyze
    VOLUME,
    AREA,
    PERIMETER,
    INTERFERENCE,
    NAKED_EDGES,
    SHOW_DOF,
    CENTER_OF_MASS,
    TRACE_PT,
    STOP_TRACING,
    STEP_DIM,
    // Help
    LOCALE,
    WEBSITE,
    GITHUB,
    ABOUT,
};

class Button;

#include "tw/textwindow.h"
#include "gw/graphicswindow.h"

#endif
