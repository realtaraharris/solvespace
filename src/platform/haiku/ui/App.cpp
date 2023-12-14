#include "App.h"
#include "MainWindow.h"

#include <iostream>

App::App(void) : BApplication("application/x-vnd.dw-TestApp") {
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
