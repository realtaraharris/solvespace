/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <functional> // std::function
#include <string>

#include <Alert.h>
#include <Window.h>
#include "App.h"

extern BApplication* be_app;

void ButtonUnclickedEventHook () {
  be_app->WindowAt(TOOLBAR)->PostMessage(new BMessage(LINE_TOOL_BTN_UNCLICKED));
}

void HaikuDialog(
  std::string title,
  std::string message,
  std::string description,
  alert_type alertType,
  std::function<void()> onDismiss = std::function<void()>()
) {
  BAlert *alert = new BAlert(
    title.c_str(),
    (message + "\n" + description).c_str(),
    NULL, NULL,
    "OK",
    B_WIDTH_AS_USUAL, B_OFFSET_SPACING, alertType
  );
  alert->SetShortcut(0, B_ESCAPE);

  if (onDismiss) {
    int32 result = alert->Go(); // synchronous call
    onDismiss();
  } else {
    status_t result = alert->Go(NULL); // asynchronous call
  }
}

void WarningDialog(std::string message, std::string description,
  std::function<void()> onDismiss = std::function<void()>()
) {
  HaikuDialog("SolveSpace::WarningDialog", message, description, B_WARNING_ALERT, onDismiss);
}

void ErrorDialog(std::string message, std::string description,
  std::function<void()> onDismiss = std::function<void()>()
) {
  HaikuDialog("SolveSpace::ErrorDialog", message, description, B_STOP_ALERT, onDismiss);
}
