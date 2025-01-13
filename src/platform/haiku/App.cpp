/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include <StorageKit.h>
#include <ControlLook.h>

#include "Toolbar.h"
#include "App.h"

#include <iostream>

App::App(void) : BApplication("application/solvespace") {
  settings = new BMessage(SETTINGS);
  LoadSettings();

  SS.Init();

  // NB: this call is extremely order-dependent!
  // create this window as early as possible because it sets the camera width
  // and height based on the window size, and we need that information in place
  // in order for ZoomToFit (inside AfterNewFile) to yield correct output
  mainwin = new MainWindow();
  mainwin->Show();

  const BSize toolbarIconSize = be_control_look->ComposeIconSize(B_LARGE_ICON);
  BRect toolbarRect = BRect(BPoint(10, 35), BSize(0, 0));
  toolWindow = new AppToolbar(toolbarRect, toolbarIconSize);
  toolWindow->ResizeToPreferred();
  toolWindow->Show();

  propertyBrowser = new PropertyBrowser();
  propertyBrowser->Show();

  viewParameters = new ViewParameters();
  viewParameters->Show();

  mainwin->Activate();
}

void App::RefsReceived(BMessage *message) {
  int32 count = 0;
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
    mainwin->PostMessage(msg);
    break;
  }
  }
}

void App::ArgvReceived(int32 argc, char **argv) {
  if (argc > 0) {
    entry_ref ref;
    get_ref_for_path(argv[1], &ref);

    BMessage *msg = new BMessage(READ_FILE);
    if (msg->AddRef("refs", &ref) != B_OK) {
      std::cerr << "error parsing args in App::ArgvReceived" << std::endl;
      return;
    }
    mainwin->PostMessage(msg);
  }
}

void App::LoadSettings() {
  if (load_settings(settings, "SolveSpace_settings") != B_OK) {
    return;
  }

  BRect frame;
  if (settings->FindRect("SolveSpace Window", &frame) == B_OK) {
    be_app->WindowAt(MAIN_WINDOW)->MoveTo(frame.LeftTop());
    be_app->WindowAt(MAIN_WINDOW)->ResizeTo(frame.Width(), frame.Height());
  }

  if (settings->FindRect("Toolbar Window", &frame) == B_OK) {
    be_app->WindowAt(TOOLBAR)->MoveTo(frame.LeftTop());
    be_app->WindowAt(TOOLBAR)->ResizeTo(frame.Width(), frame.Height());
  }
  if (settings->FindRect("Property Browser Window", &frame) == B_OK) {
    be_app->WindowAt(PROPERTY_BROWSER)->MoveTo(frame.LeftTop());
    be_app->WindowAt(PROPERTY_BROWSER)->ResizeTo(frame.Width(), frame.Height());
  }
  if (settings->FindRect("View Parameters Window", &frame) == B_OK) {
    be_app->WindowAt(VIEW_PARAMETERS)->MoveTo(frame.LeftTop());
    be_app->WindowAt(VIEW_PARAMETERS)->ResizeTo(frame.Width(), frame.Height());
  }
}

void App::SaveSettings() {
  if (settings->ReplaceRect("SolveSpace Window", be_app->WindowAt(MAIN_WINDOW)->Frame()) != B_OK) {
    settings->AddRect("SolveSpace Window", be_app->WindowAt(MAIN_WINDOW)->Frame());
  }

  if (settings->ReplaceRect("Toolbar Window", be_app->WindowAt(TOOLBAR)->Frame()) != B_OK) {
    settings->AddRect("Toolbar Window", be_app->WindowAt(TOOLBAR)->Frame());
  }
  if (settings->ReplaceRect("Property Browser Window",
                            be_app->WindowAt(PROPERTY_BROWSER)->Frame()) != B_OK) {
    settings->AddRect("Property Browser Window", be_app->WindowAt(PROPERTY_BROWSER)->Frame());
  }
  if (settings->ReplaceRect("View Parameters Window", be_app->WindowAt(VIEW_PARAMETERS)->Frame()) !=
      B_OK) {
    settings->AddRect("View Parameters Window", be_app->WindowAt(VIEW_PARAMETERS)->Frame());
  }

  if (save_settings(settings, "SolveSpace_settings") != B_OK) {
    std::cerr << "error saving SolveSpace settings" << std::endl;
  }
}

status_t load_settings(BMessage *message, const char *filename) {
  status_t status = B_BAD_VALUE;
  if (!message) {
    return status;
  }

  BPath path;
  status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
  if (status != B_OK) {
    return status;
  }

  status = path.Append(filename);
  if (status != B_OK) {
    return status;
  }

  BFile file(path.Path(), B_READ_ONLY);
  status = file.InitCheck();
  if (status != B_OK) {
    return status;
  }

  status = message->Unflatten(&file);
  file.Unset();

  return status;
}

status_t save_settings(BMessage *message, const char *filename) {
  status_t status = B_BAD_VALUE;
  if (!message) {
    return status;
  }

  BPath path;
  status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
  if (status != B_OK) {
    return status;
  }

  status = path.Append(filename);
  if (status != B_OK) {
    return status;
  }

  BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
  status = file.InitCheck();
  if (status != B_OK) {
    return status;
  }

  status = message->Flatten(&file);
  file.Unset();

  return status;
}

bool App::QuitRequested(void) {
  be_app->PostMessage(B_QUIT_REQUESTED);
  return true;
}

int main(void) {
  App *app = new App();
  app->Run();
  delete app;
  return 0;
}
