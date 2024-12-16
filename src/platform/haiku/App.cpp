/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include "App.h"
#include "MainWindow.h"
#include <StorageKit.h>

#include <iostream>

App::App (void) : BApplication ("application/x-vnd.dw-solvespace") {
  MainWindow *mainwin = new MainWindow ();
  mainwin->Show ();
}

std::string fetchFilePath (BMessage *message) {
  if (!message->HasRef("refs")) { return std::string(); }

  entry_ref ref;

  if (message->FindRef("refs", 0, &ref) != B_OK) { return std::string(); }

  BEntry entry(&ref, true);
//  BPath *filePath;
	BPath filePath(&ref);
	
  entry.GetPath(&filePath);

  return std::string(filePath.Path());
}

void App::RefsReceived(BMessage *message) {
  std::cout << "in App::RefsReceived" << std::endl;
message->PrintToStream();
	int32	count = 0;
	uint32 type = 0;

	message->GetInfo("refs", &type, &count);
	if (count > MAX_REFS_RECEIVED) {
	  count = MAX_REFS_RECEIVED;
	}
	for (int32 i = 0; i < count; i++) {
		entry_ref ref;
		if (message->FindRef("refs", i, &ref) == B_NO_ERROR) {
			BEntry entry(&ref);
			if (entry.InitCheck() == B_NO_ERROR) {
				BPath path;
				entry.GetPath(&path);
				printf("App::RefsReceived(%s)\n", path.Path());
/*
				fWindow->LockLooper();
				if (strstr(path.Path(), ".medo"))
					fWindow->LoadProject(path.Path());
				else
					fWindow->AddMediaSource(path.Path());
				fWindow->UnlockLooper();
*/
			}
		}
	}
}

void App::MessageReceived(BMessage *msg) {
	msg->PrintToStream();
  switch (msg->what) {
    case READ_FILE: {
      std::cout << "READ_FILE HIT, App" << std::endl;
			  be_app->WindowAt(MAIN_WINDOW)->PostMessage(msg);
	    break;
    }
    case SAVE_FILE: {
      std::cout << "SAVE_FILE HIT, App" << std::endl;
			  be_app->WindowAt(MAIN_WINDOW)->PostMessage(msg);
	    break;
    }
  }
}

/*
void App::RefsReceived(BMessage *message) {
std::cout << "in App::RefsReceived" << std::endl;
  std::string ffp = fetchFilePath(message);
  if (ffp.empty()) { return; }
	
	BMessage *mess = new BMessage(SAVE_FILE);
	mess->AddString("openFilename", ffp.c_str());
	be_app->WindowAt(MAIN_WINDOW)->PostMessage(mess);
//  editorView->Load(ffp); // TODO: convert this to a message to post to the MAIN_WINDOW, like we do in the line immediately below
  be_app->WindowAt(PROPERTY_BROWSER)->PostMessage(new BMessage(SHOW_LIST_OF_GROUPS));
}
*/

void App::ArgvReceived (int32 argc, char **argv) {
  if (argc > 0) {
    BMessage *msg = new BMessage (READ_FILE);
    entry_ref ref;
    get_ref_for_path (argv[1], &ref);

    if (msg->AddRef ("refs", &ref) != B_OK) {
      std::cerr << "error parsing args in App::ArgvReceived" << std::endl;
      return;
    }

    be_app->WindowAt (MAIN_WINDOW)->PostMessage (msg);
  }
}

int main (void) {
  App *app = new App ();
  app->Run ();
  delete app;
  return 0;
}
