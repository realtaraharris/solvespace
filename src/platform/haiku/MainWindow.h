/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#pragma once

#include <MenuBar.h>
#include <Window.h>
#include <Message.h>

#include "EditorView.h"
#include "PropertyBrowser.h"
#include "ViewParameters.h"

class MainWindow : public BWindow {
  public:
  MainWindow (void);

  void MessageReceived (BMessage *msg);
  bool QuitRequested (void);
  void LoadSettings ();
  void SaveSettings ();
  void SetupMenubar(BRect rect);

  private:
  BMenuBar *menuBar;

  BMenuItem *undoMenuItem;
  BMenuItem *redoMenuItem;

  BWindow         *toolWindow;
  PropertyBrowser *propertyBrowser;
  ViewParameters  *viewParameters;
  EditorView      *editorView;

  BMessage *settings;
};
