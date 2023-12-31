#ifndef APP_H
#define APP_H

#include <Application.h>

#define MIN_WIDTH 600.0f
#define MIN_HEIGHT 600.0f
#define MENUBAR_HEIGHT 20.0f

class App : public BApplication {
  public:
    App(void);
};

enum { MAIN_WINDOW = 0, TOOLBAR = 1, PROPERTY_BROWSER = 2, VIEW_PARAMETERS = 3 };

enum {
    LINE_TOOL_BTN_CLICKED = 'ltbc',
    RECT_TOOL_BTN_CLICKED = 'rtbc',
    CIRCLE_TOOL_BTN_CLICKED = 'ctbc',
    ARC_TOOL_BTN_CLICKED = 'atbc',
    TANGENT_ARC_TOOL_BTN_CLICKED = 'tabc',
    CUBIC_SPLINE_TOOL_BTN_CLICKED = 'csbc',
    DATUM_POINT_TOOL_BTN_CLICKED = 'dpbc',
    CONSTRUCTION_TOOL_BTN_CLICKED = 'cnbc',
    SPLIT_CURVES_TOOL_BTN_CLICKED = 'scbc',
    TEXT_TOOL_BTN_CLICKED = 'ttbc',

    NEAREST_ISO_TOOL_BTN_CLICKED = 'nibc',
    NEAREST_ORTHO_TOOL_BTN_CLICKED = 'nobc',

    DISTANCE_DIA_TOOL_BTN_CLICKED = 'ddbc',
    ANGLE_TOOL_BTN_CLICKED = 'anbc',

    HORIZONTAL_TOOL_BTN_CLICKED = 'htbc',
    VERTICAL_TOOL_BTN_CLICKED = 'vtbc',
    PARALLEL_TOOL_BTN_CLICKED = 'ptbc',
    CONSTRAIN_PERP_TOOL_BTN_CLICKED = 'cpti',
    PT_ON_LINE_TOOL_BTN_CLICKED = 'polc',
    CONSTRAIN_SYMMETRIC_TOOL_BTN_CLICKED = 'cstc',
    CONSTRAIN_EQUAL_TOOL_BTN_CLICKED = 'cebc',
    CONSTRAIN_ORIENTED_SAME_TOOL_BTN_CLICKED = 'cosc',

    M_SHOW_EDITOR = 'mshe',

    SHOW_LIST_OF_GROUPS = 'slog',
    UPDATE_VIEW_PARAMETERS = 'upvp',

    READ_FILE = 'rdfl',
    M_OPEN_FILE = 'mof',
    M_SAVE_FILE = 'msf',
    M_QUIT_APP = 'mqa',

    ZOOM_IN = 'zin',
    ZOOM_OUT = 'zout',
    ZOOM_TO_FIT = 'zfit',
    ALIGN_VIEW_TO_WORKPLANE = 'avtw',
    CENTER_VIEW_AT_POINT = 'cvap',

    TOGGLE_SNAP_GRID = 'tsng'
};

#endif
