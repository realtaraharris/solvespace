/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#pragma once

#include <Window.h>

class AppToolbar : public BWindow {
  public:
  AppToolbar (BRect rect, BSize toolbarIconSize);
  void MessageReceived (BMessage *msg);

  private:
  BButton *lineToolButton;
  BButton *rectToolButton;
  BButton *circleToolButton;
  BButton *arcToolButton;
  BButton *tangentArcToolButton;
  BButton *cubicSplineToolButton;
  BButton *datumPointToolButton;
  BButton *constructionToolButton;
  BButton *splitCurvesToolButton;
  BButton *textToolButton;

  BButton *distanceToolButton;
  BButton *angleToolButton;
  BButton *horizontalToolButton;
  BButton *verticalToolButton;
  BButton *parallelToolButton;
  BButton *constrainPerpendicularToolButton;
  BButton *constrainPointOnLineToolButton;
  BButton *constrainSymmetricToolButton;
  BButton *constrainEqualToolButton;
  BButton *constrainParallelNormalsToolButton;
  BButton *otherSupplementaryAngleToolButton;
  BButton *refToolButton;

  BButton *nearestIsoToolButton;
  BButton *nearestOrthoToolButton;
};
