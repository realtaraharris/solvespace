#include "MainWindow.h"

#include <Application.h>
#include <Menu.h>
#include <MenuItem.h>
#include <View.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <StorageKit.h>
#include <Alert.h> // BAlert

#include "Solver.h"
#include "Toolbar.h"
#include "App.h" // contains message enums

#include <iostream>

#define INIT_X 100
#define INIT_Y 100
#define MIN_WIDTH 600
#define MIN_HEIGHT 600
#define MENUBAR_HEIGHT 20

MainWindow::MainWindow(void)
	:	BWindow(BRect(INIT_X, INIT_Y, INIT_X + MIN_WIDTH, INIT_Y + MIN_HEIGHT + MENUBAR_HEIGHT), "SolveSpace", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS, B_CURRENT_WORKSPACE) {
	BRect rect(Bounds());

	SetSizeLimits(MIN_WIDTH, 100000, MIN_HEIGHT + MENUBAR_HEIGHT, 100000); // the Haiku API needs a way to not set any upper bound

	menuBar = new BMenuBar(rect, "menubar");	
	BMenu *menu = new BMenu("File");
	menu->AddItem(new BMenuItem("Open", new BMessage(M_OPEN_FILE), 'O'));
	menu->AddItem(new BMenuItem("Quit", new BMessage(M_QUIT_APP), 'Q'));
	menuBar->AddItem(menu);
	
	rect.Set(0, 0, MIN_WIDTH, MIN_HEIGHT);
    editorView = new EditorView(rect);

    SetLayout(new BGroupLayout(B_VERTICAL));

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar)
		.Add(editorView)
		.End();

	toolWindow = new AppToolbar();
	toolWindow->Show();
}

void MainWindow::MessageReceived(BMessage *msg) {
	switch (msg->what) {
		case M_QUIT_APP: {
			FreeAllTemporary();
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case M_OPEN_FILE: {
			BFilePanel *fp = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL, B_FILE_NODE, false, new BMessage(READ_FILE));
			fp->Show();
			break;
		}
		case READ_FILE: {
			if (msg->HasRef("refs")) {
				entry_ref ref;
				if (msg->FindRef("refs", 0, &ref) == B_OK) {
					BEntry entry(&ref, true);
					BPath path;
					entry.GetPath(&path);
					editorView->Load(std::string(path.Path()));
				}
			}
			break;
		}
		default: {
			BWindow::MessageReceived(msg);
			break;
		}
	}
}


bool MainWindow::QuitRequested(void) {
	FreeAllTemporary();
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
