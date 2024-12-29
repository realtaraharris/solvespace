/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include <Window.h>
#include "App.h" // contains message enums
#include "solvespace.h"

void HandleKeyboardShortcut(int32 key, char raw, uint32_t mods) {
  switch (key) {
  case B_F3_KEY: {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(NEAREST_ISO_TOOL_BTN_CLICKED));
    break;
  }
  }
  switch (raw) {
  case B_DELETE: {
    SS.MenuClipboard(SolveSpace::Command::DELETE);
    break;
  }
  case 's': {
    be_app->WindowAt(TOOLBAR)->PostMessage(new BMessage(LINE_TOOL_BTN_CLICKED));
    break;
  }
  case 'r': {
    be_app->WindowAt(TOOLBAR)->PostMessage(new BMessage(RECT_TOOL_BTN_CLICKED));
    break;
  }
  case 'c': {
    be_app->WindowAt(TOOLBAR)->PostMessage(new BMessage(CIRCLE_TOOL_BTN_CLICKED));
    break;
  }
  case 'a': {
    if (mods & B_SHIFT_KEY) {
      be_app->WindowAt(TOOLBAR)->PostMessage(new BMessage(TANGENT_ARC_TOOL_BTN_CLICKED));
    } else {
      be_app->WindowAt(TOOLBAR)->PostMessage(new BMessage(ARC_TOOL_BTN_CLICKED));
    }
    break;
  }
  case 'b': {
    be_app->WindowAt(TOOLBAR)->PostMessage(new BMessage(CUBIC_SPLINE_TOOL_BTN_CLICKED));
    break;
  }
  case 'p': {
    be_app->WindowAt(TOOLBAR)->PostMessage(new BMessage(DATUM_POINT_TOOL_BTN_CLICKED));
    break;
  }
  case 'g': {
    be_app->WindowAt(TOOLBAR)->PostMessage(new BMessage(CONSTRUCTION_TOOL_BTN_CLICKED));
    break;
  }
  case 'i': {
    be_app->WindowAt(TOOLBAR)->PostMessage(new BMessage(SPLIT_CURVES_TOOL_BTN_CLICKED));
    break;
  }
  case 't': {
    be_app->WindowAt(TOOLBAR)->PostMessage(new BMessage(TEXT_TOOL_BTN_CLICKED));
    break;
  }

  case 'd': {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(DISTANCE_DIA_TOOL_BTN_CLICKED));
    break;
  }
  case 'n': {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(ANGLE_TOOL_BTN_CLICKED));
    break;
  }
  case 'h': {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(HORIZONTAL_TOOL_BTN_CLICKED));
    break;
  }
  case 'v': {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(VERTICAL_TOOL_BTN_CLICKED));
    break;
  }
  case 'l': {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(PARALLEL_TOOL_BTN_CLICKED));
    break;
  }
  case '[': {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(CONSTRAIN_PERP_TOOL_BTN_CLICKED));
    break;
  }
  case 'o': {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(PT_ON_LINE_TOOL_BTN_CLICKED));
    break;
  }
  case 'y': {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(CONSTRAIN_SYMMETRIC_TOOL_BTN_CLICKED));
    break;
  }
  case 'q': {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(CONSTRAIN_EQUAL_TOOL_BTN_CLICKED));
    break;
  }
  case 'x': {
    be_app->WindowAt(MAIN_WINDOW)
        ->PostMessage(new BMessage(CONSTRAIN_ORIENTED_SAME_TOOL_BTN_CLICKED));
    break;
  }
  case 'u': {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(OTHER_ANGLE_TOOL_BTN_CLICKED));
    break;
  }
  case 'e': {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(REF_TOOL_BTN_CLICKED));
    break;
  }
  case 'w': {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(NEAREST_ORTHO_TOOL_BTN_CLICKED));
    break;
  }
  }
}
