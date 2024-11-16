/*
 * Copyright 2023, 2024 Tara Harris <3769985+realtaraharris@users.noreply.github.com>
 * All rights reserved. Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include "App.h"
#include "MainWindow.h"

#include <iostream>

App::App(void) : BApplication("application/x-vnd.dw-solvespace") {
    MainWindow *mainwin = new MainWindow();
    mainwin->Show();
}

void BApplication::ArgvReceived(int32 argc, char **argv) {
	if (argc > 0) {
		BMessage *msg = new BMessage(READ_FILE);
		entry_ref ref;
		get_ref_for_path(argv[1], &ref);

		if (msg->AddRef("refs", &ref) != B_OK) {
			std::cerr << "error parsing args in BApplication::ArgvReceived" << std::endl;
			return;
		}

		be_app->WindowAt(MAIN_WINDOW)->PostMessage(msg);
	}
}

int main(void) {
    App *app = new App();
    app->Run();
    delete app;
    return 0;
}
