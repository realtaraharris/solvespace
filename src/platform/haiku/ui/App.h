#ifndef APP_H
#define APP_H

#include <Application.h>

class App : public BApplication
{
public:
	App(void);
};

enum {
	LINE_TOOL_BTN_CLICKED='ltbc',
	RECT_TOOL_BTN_CLICKED='rtbc',

	READ_FILE='rdfl',
	M_OPEN_FILE='mof',
	M_QUIT_APP='mqa',

	ZOOM_IN='zin',
	ZOOM_OUT='zout',
	ZOOM_TO_FIT='zfit',
	TOGGLE_SNAP_GRID='tsng'
};

#endif
