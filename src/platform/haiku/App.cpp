/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include "App.h"
#include "MainWindow.h"
#include <StorageKit.h>

#include <iostream>

App::App(void) : BApplication("application/solvespace") {
  MainWindow *mainwin = new MainWindow();
  mainwin->Show();
}

void App::RefsReceived(BMessage *message) {
  int32  count = 0;
  uint32 type  = 0;

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
      }
    }
  }
}

void App::MessageReceived(BMessage *msg) {
  switch (msg->what) {
  case EXPORT_VIEW:      // fall through, intentionally
  case EXPORT_SECTION:   // fall through, intentionally
  case EXPORT_WIREFRAME: // fall through, intentionally
  case EXPORT_MESH:      // fall through, intentionally
  case EXPORT_SURFACES:  // fall through, intentionally
  case IMPORT_FILE:      // fall through, intentionally
  case READ_FILE:        // fall through, intentionally
  case SAVE_FILE:        // fall through, intentionally
  case SAVE_AS_FILE: {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(msg);
    break;
  }
  }
}

void App::ArgvReceived(int32 argc, char **argv) {
  if (argc > 0) {
    BMessage *msg = new BMessage(READ_FILE);
    entry_ref ref;
    get_ref_for_path(argv[1], &ref);

    if (msg->AddRef("refs", &ref) != B_OK) {
      std::cerr << "error parsing args in App::ArgvReceived" << std::endl;
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
