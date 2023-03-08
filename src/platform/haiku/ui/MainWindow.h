#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Window.h>
#include <MenuBar.h>

#include "EditorView.h"

class MainWindow : public BWindow {
public:
	MainWindow(void);
	void MessageReceived(BMessage *msg);
	bool QuitRequested(void);
private:
	BMenuBar *menuBar;
	BWindow *toolWindow;
	EditorView *editorView;
};

#endif
