/*
 * Copyright 2023, 2024 Tara Harris <3769985+realtaraharris@users.noreply.github.com>
 * All rights reserved. Distributed under the terms of the GPLv3 and MIT licenses.
 */

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
    OTHER_ANGLE_TOOL_BTN_CLICKED = 'oabc',
    REF_TOOL_BTN_CLICKED = 'rtbx',

    M_SHOW_EDITOR = 'mshe',

    SHOW_LIST_OF_GROUPS = 'slog',
    UPDATE_VIEW_PARAMETERS = 'upvp',

    // File menu
    READ_FILE = 'rdfl',
    SAVE_AS_FILE = 'saaf',
    M_NEW_FILE = 'mnf',
    M_OPEN_FILE = 'mof',
    M_SAVE_FILE = 'msf',
    M_SAVE_AS_FILE = 'msaf',
    M_QUIT_APP = 'mqa',

    // Edit menu
    M_UNDO = 'undo',
    M_REDO = 'redo',

    SET_UNDO_ENABLED = 'suen',
    SET_UNDO_DISABLED = 'sudi',
    SET_REDO_ENABLED = 'sren',
    SET_REDO_DISABLED = 'srdi',

    // View menu
    ZOOM_IN = 'zin',
    ZOOM_OUT = 'zout',
    ZOOM_TO_FIT = 'zfit',
    ALIGN_VIEW_TO_WORKPLANE = 'avtw',
    CENTER_VIEW_AT_POINT = 'cvap',

    TOGGLE_SNAP_GRID = 'tsng',

    M_GROUP_3D = 'gr3d', // Command::GROUP_3D
    M_GROUP_WRKPL = 'grwp', // Command::GROUP_WRKPL
    M_GROUP_TRANS = 'grtr', // Command::GROUP_TRANS
    M_GROUP_ROT = 'grrt', // Command::GROUP_ROT
    M_GROUP_EXTRUDE = 'grex',// Command::GROUP_EXTRUDE
    M_GROUP_HELIX = 'grhe', // Command::GROUP_HELIX
    M_GROUP_LATHE = 'grla', // Command::GROUP_LATHE
    M_GROUP_REVOLVE = 'grre', // Command::GROUP_REVOLVE
    M_GROUP_LINK = 'grlk', // Command::GROUP_LINK
    M_GROUP_RECENT = 'grct', // Command::GROUP_RECENT

    M_NOT_IMPL = 'mnti'
};

#endif
