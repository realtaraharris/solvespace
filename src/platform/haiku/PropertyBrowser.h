/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#ifndef PROPERTYBROWSER_H
#define PROPERTYBROWSER_H

#include "GroupsListView.h"
#include <Window.h>

class PropertyBrowser : public BWindow {
  public:
  PropertyBrowser (void);
  void MessageReceived (BMessage *msg);
  void ShowListOfGroups ();

  private:
  GroupsListView *groupList;
};

#endif
