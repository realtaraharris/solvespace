/*
 * Copyright 2023, 2024 Tara Harris <3769985+realtaraharris@users.noreply.github.com>
 * All rights reserved. Distributed under the terms of the GPLv3 and MIT licenses.
 */

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <Window.h>

class AppToolbar : public BWindow {
  public:
    AppToolbar(BRect rect, BSize toolbarIconSize);
    void MessageReceived(BMessage *msg);

  private:
};

#endif
