/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#pragma once

#include <Application.h>

#define MIN_WIDTH 600.0f
#define MIN_HEIGHT 600.0f
#define MENUBAR_HEIGHT 20.0f
#define	MAX_REFS_RECEIVED 32

class App : public BApplication {
  public:
  App (void);
	void RefsReceived(BMessage *message);
	void ArgvReceived(int32 argc, char **argv);
	void MessageReceived(BMessage *msg);
};

enum { MAIN_WINDOW = 0, TOOLBAR = 1, PROPERTY_BROWSER = 2, VIEW_PARAMETERS = 3 };

enum {
  LINKED_FILE_IMAGE = 'limf',
  IMPORT_FILE       = 'imfi',
  GROUP_LINK_FILE   = 'glfi',
  EXPORT_VIEW       = 'exvi',
  EXPORT_WIREFRAME  = 'exwi',
  EXPORT_SECTION    = 'exse',
  EXPORT_MESH       = 'exme',
  EXPORT_SURFACES   = 'exsu',
  STOP_TRACING      = 'sttr',

  LINE_TOOL_BTN_CLICKED           = 'ltbc',
  LINE_TOOL_BTN_UNCLICKED         = 'ltuc',
  RECT_TOOL_BTN_CLICKED           = 'rtbc',
  RECT_TOOL_BTN_UNCLICKED         = 'rtbu',
  CIRCLE_TOOL_BTN_CLICKED         = 'ctbc',
  CIRCLE_TOOL_BTN_UNCLICKED       = 'ctuc',
  ARC_TOOL_BTN_CLICKED            = 'atbc',
  ARC_TOOL_BTN_UNCLICKED          = 'atuc',
  TANGENT_ARC_TOOL_BTN_CLICKED    = 'tabc',
  TANGENT_ARC_TOOL_BTN_UNCLICKED  = 'tauc',
  CUBIC_SPLINE_TOOL_BTN_CLICKED   = 'csbc',
  CUBIC_SPLINE_TOOL_BTN_UNCLICKED = 'csuc',
  DATUM_POINT_TOOL_BTN_CLICKED    = 'dpbc',
  DATUM_POINT_TOOL_BTN_UNCLICKED  = 'dpuc',
  CONSTRUCTION_TOOL_BTN_CLICKED   = 'cnbc',
  CONSTRUCTION_TOOL_BTN_UNCLICKED = 'cnuc',
  SPLIT_CURVES_TOOL_BTN_CLICKED   = 'scbc',
  SPLIT_CURVES_TOOL_BTN_UNCLICKED = 'scuc',
  TEXT_TOOL_BTN_CLICKED           = 'ttbc',
  TEXT_TOOL_BTN_UNCLICKED         = 'ttuc',

  DISTANCE_DIA_TOOL_BTN_CLICKED              = 'ddbc',
  DISTANCE_DIA_TOOL_BTN_UNCLICKED            = 'dduc',
  ANGLE_TOOL_BTN_CLICKED                     = 'anbc',
  ANGLE_TOOL_BTN_UNCLICKED                   = 'anuc',
  HORIZONTAL_TOOL_BTN_CLICKED                = 'htbc',
  HORIZONTAL_TOOL_BTN_UNCLICKED              = 'htbc',
  VERTICAL_TOOL_BTN_CLICKED                  = 'vtbc',
  VERTICAL_TOOL_BTN_UNCLICKED                = 'vtuc',
  PARALLEL_TOOL_BTN_CLICKED                  = 'ptbc',
  PARALLEL_TOOL_BTN_UNCLICKED                = 'ptuc',
  CONSTRAIN_PERP_TOOL_BTN_CLICKED            = 'cpti',
  CONSTRAIN_PERP_TOOL_BTN_UNCLICKED          = 'cpui',
  PT_ON_LINE_TOOL_BTN_CLICKED                = 'polc',
  PT_ON_LINE_TOOL_BTN_UNCLICKED              = 'pouc',
  CONSTRAIN_SYMMETRIC_TOOL_BTN_CLICKED       = 'cstc',
  CONSTRAIN_SYMMETRIC_TOOL_BTN_UNCLICKED     = 'csuc',
  CONSTRAIN_EQUAL_TOOL_BTN_CLICKED           = 'cebc',
  CONSTRAIN_EQUAL_TOOL_BTN_UNCLICKED         = 'ceuc',
  CONSTRAIN_ORIENTED_SAME_TOOL_BTN_CLICKED   = 'cosc',
  CONSTRAIN_ORIENTED_SAME_TOOL_BTN_UNCLICKED = 'couc',
  OTHER_ANGLE_TOOL_BTN_CLICKED               = 'oabc',
  OTHER_ANGLE_TOOL_BTN_UNCLICKED             = 'oauc',
  REF_TOOL_BTN_CLICKED                       = 'rtbx',
  REF_TOOL_BTN_UNCLICKED                     = 'rtux',

  NEAREST_ISO_TOOL_BTN_CLICKED     = 'nibc',
  NEAREST_ISO_TOOL_BTN_UNCLICKED   = 'niuc',
  NEAREST_ORTHO_TOOL_BTN_CLICKED   = 'nobc',
  NEAREST_ORTHO_TOOL_BTN_UNCLICKED = 'nouc',

  M_SHOW_EDITOR = 'mshe',

  SHOW_LIST_OF_GROUPS    = 'slog',
  UPDATE_VIEW_PARAMETERS = 'upvp',

  // File menu
  READ_FILE          = 'rdfl',
  SAVE_AS_FILE       = 'saaf',
  M_NEW_FILE         = 'mnf',
  M_OPEN_FILE        = 'mof',
  M_SAVE_FILE        = 'msf',
  M_SAVE_AS_FILE     = 'msaf',
  M_EXPORT_IMAGE     = 'mexi',
  PNG_EXPORT_IMAGE   = 'pexi',
  M_EXPORT_VIEW      = 'mexv',
  M_EXPORT_SECTION   = 'mese',
  M_EXPORT_WIREFRAME = 'mexw',
  M_EXPORT_MESH      = 'mexm',
  M_EXPORT_SURFACES  = 'mexs',
  M_IMPORT           = 'mimp',
  M_QUIT_APP         = 'mqa',
  SAVE_FILE          = 'safi',

  // Edit menu
  M_UNDO = 'undo',
  M_REDO = 'redo',

  SET_UNDO_ENABLED  = 'suen',
  SET_UNDO_DISABLED = 'sudi',
  SET_REDO_ENABLED  = 'sren',
  SET_REDO_DISABLED = 'srdi',

  // View menu
  ZOOM_IN                 = 'zin',
  ZOOM_OUT                = 'zout',
  ZOOM_TO_FIT             = 'zfit',
  ALIGN_VIEW_TO_WORKPLANE = 'avtw',
  CENTER_VIEW_AT_POINT    = 'cvap',

  TOGGLE_SNAP_GRID = 'tsng',

  M_GROUP_3D      = 'gr3d', // Command::GROUP_3D
  M_GROUP_WRKPL   = 'grwp', // Command::GROUP_WRKPL
  M_GROUP_TRANS   = 'grtr', // Command::GROUP_TRANS
  M_GROUP_ROT     = 'grrt', // Command::GROUP_ROT
  M_GROUP_EXTRUDE = 'grex', // Command::GROUP_EXTRUDE
  M_GROUP_HELIX   = 'grhe', // Command::GROUP_HELIX
  M_GROUP_LATHE   = 'grla', // Command::GROUP_LATHE
  M_GROUP_REVOLVE = 'grre', // Command::GROUP_REVOLVE
  M_GROUP_LINK    = 'grlk', // Command::GROUP_LINK
  M_GROUP_RECENT  = 'grct', // Command::GROUP_RECENT

  M_SEL_WORKPLANE  = 'mswp', // Command::SEL_WORKPLANE
  M_FREE_IN_3D     = 'mf3d', // Command::FREE_IN_3D
  M_DATUM_POINT    = 'mdpt', // Command::DATUM_POINT
  M_WORKPLANE      = 'mwkp', // Command::WORKPLANE
  M_LINE_SEGMENT   = 'mlis', // Command::LINE_SEGMENT
  M_CONSTR_SEGMENT = 'mcrs', // Command::CONSTR_SEGMENT
  M_RECTANGLE      = 'mrec', // Command::RECTANGLE
  M_CIRCLE         = 'mcir', // Command::CIRCLE
  M_ARC            = 'marc', // Command::ARC
  M_CUBIC          = 'mcub', // Command::CUBIC

  M_TTF_TEXT     = 'mttt', // Command::TTF_TEXT
  M_IMAGE        = 'mimg', // Command::IMAGE
  M_CONSTRUCTION = 'mcon', // Command::CONSTRUCTION
  M_TANGENT_ARC  = 'mtar', // Command::TANGENT_ARC
  M_SPLIT_CURVES = 'mspc', // Command::SPLIT_CURVES

  M_DISTANCE_DIA  = 'dida', // Command::DISTANCE_DIA
  M_REF_DISTANCE  = 'rdia', // Command::REF_DISTANCE
  M_ANGLE         = 'angl', // Command::ANGLE
  M_REF_ANGLE     = 'rang', // Command::REF_ANGLE
  M_OTHER_ANGLE   = 'oang', // Command::OTHER_ANGLE
  M_REFERENCE     = 'mref', // Command::REFERENCE
  M_HORIZONTAL    = 'horz', // Command::HORIZONTAL
  M_VERTICAL      = 'vert', // Command::VERTICAL
  M_ON_ENTITY     = 'oent', // Command::ON_ENTITY
  M_EQUAL         = 'equl', // Command::EQUAL
  M_RATIO         = 'rato', // Command::RATIO
  M_DIFFERENCE    = 'diff', // Command::DIFFERENCE
  M_AT_MIDPOINT   = 'atmd', // Command::AT_MIDPOINT
  M_SYMMETRIC     = 'symm', // Command::SYMMETRIC
  M_PARALLEL      = 'para', // Command::PARALLEL
  M_PERPENDICULAR = 'perp', // Command::PERPENDICULAR
  M_ORIENTED_SAME = 'osam', // Command::ORIENTED_SAME
  M_WHERE_DRAGGED = 'wdra', // Command::WHERE_DRAGGED
  M_COMMENT       = 'comm', // Command::COMMENT

  M_VOLUME         = 'mvol', // Command::VOLUME
  M_AREA           = 'area', // Command::AREA
  M_PERIMETER      = 'peri', // Command::PERIMETER
  M_INTERFERENCE   = 'intf', // Command::INTERFERENCE
  M_NAKED_EDGES    = 'nked', // Command::NAKED_EDGES
  M_CENTER_OF_MASS = 'ceom', // Command::CENTER_OF_MASS
  M_SHOW_DOF       = 'shdf', // Command::SHOW_DOF
  M_TRACE_PT       = 'trpt', // Command::TRACE_PT
  M_STOP_TRACING   = 'sttr', // Command::STOP_TRACING
  M_STEP_DIM       = 'stdi', // Command::STEP_DIM

  M_ABOUT = 'mabu', // Command::ABOUT

  M_NOT_IMPL = 'mnti'
};
