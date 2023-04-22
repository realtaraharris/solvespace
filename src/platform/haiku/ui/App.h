#ifndef APP_H
#define APP_H

#include <Application.h>

#define MIN_WIDTH 600
#define MIN_HEIGHT 600
#define MENUBAR_HEIGHT 20

class App : public BApplication
{
public:
	App(void);
};

enum {
	LINE_TOOL_BTN_CLICKED='ltbc',
	RECT_TOOL_BTN_CLICKED='rtbc',
	CIRCLE_TOOL_BTN_CLICKED='ctbc',
	ARC_TOOL_BTN_CLICKED='atbc',
	TANGENT_ARC_TOOL_BTN_CLICKED='tabc',
	CUBIC_SPLINE_TOOL_BTN_CLICKED='csbc',
	DATUM_POINT_TOOL_BTN_CLICKED='dpbc',
	CONSTRUCTION_TOOL_BTN_CLICKED='cnbc',
	SPLIT_CURVES_TOOL_BTN_CLICKED='scbc',
	TEXT_TOOL_BTN_CLICKED='ttbc',

	DISTANCE_DIA_TOOL_BTN_CLICKED='ddbc',
	ANGLE_TOOL_BTN_CLICKED='anbc',

	HORIZONTAL_TOOL_BTN_CLICKED='htbc',
	VERTICAL_TOOL_BTN_CLICKED='vtbc',

	M_SHOW_EDITOR='mshe',

	READ_FILE='rdfl',
	M_OPEN_FILE='mof',
	M_QUIT_APP='mqa',

	ZOOM_IN='zin',
	ZOOM_OUT='zout',
	ZOOM_TO_FIT='zfit',
	TOGGLE_SNAP_GRID='tsng'
};

#endif
