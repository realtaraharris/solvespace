/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#pragma once

#include <StringView.h>
#include <Window.h>

class ViewParameters : public BWindow {
  public:
  ViewParameters (void);
  void UpdateViewParameters ();
  void MessageReceived (BMessage *msg);

  private:
  BStringView *overallScaleFactor;
  BStringView *origin;
  BStringView *projection;
  BStringView *perspectiveFactor;
  BStringView *explodeDistance;
};
