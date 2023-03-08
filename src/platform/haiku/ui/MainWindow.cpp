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

MainWindow::MainWindow(void)
	:	BWindow(BRect(100, 100, 700, 720), "SolveSpace", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS, B_CURRENT_WORKSPACE) {
	BRect rect(Bounds());

	menuBar = new BMenuBar(rect, "menubar");	
	BMenu *menu = new BMenu("File");
	menu->AddItem(new BMenuItem("Open", new BMessage(M_OPEN_FILE), 'O'));
	menu->AddItem(new BMenuItem("Quit", new BMessage(M_QUIT_APP), 'Q'));
	menuBar->AddItem(menu);
	
	rect.Set(0, 0, 600, 600);
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
			FreeAllTemporary(); // vital to call this before posting B_QUIT_REQUESTED to allow the allocator to work
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
			std::cout << "Message received: " << msg->what << std::endl;

			BWindow::MessageReceived(msg);
			break;
		}
	}
}


bool
MainWindow::QuitRequested(void) {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}
