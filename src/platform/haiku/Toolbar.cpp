/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include "Toolbar.h"

#include <Application.h>
#include <Bitmap.h>
#include <NodeInfo.h>
#include <PictureButton.h>
#include <Roster.h>
#include <View.h>
#include <Window.h>

#include "HVIFUtil.h"

#include "App.h" // contains message enums

#include <Alert.h> // BAlert
#include <Box.h>
#include <Button.h>
#include <LayoutBuilder.h>

#include <iostream>

#include "solvespace.h"
#include "ssg.h"

BButton *MakeButton(const BSize toolbarIconSize, BBitmap *icon, BMessage *message,
                    const char *tooltipText) {
  BButton *toolbarButton =
      new BButton(BRect(BPoint(0.0, 0.0), toolbarIconSize), "toolbarButton", "", message);
  toolbarButton->SetIcon(icon, 0);
  toolbarButton->SetToolTip(tooltipText);
  toolbarButton->SetFlat(true);
  toolbarButton->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
  return toolbarButton;
}

::BBox *MakeDivider() {
  ::BBox *divider = new ::BBox(BRect(0, 0, 1, 1), B_EMPTY_STRING, B_FOLLOW_LEFT_RIGHT,
                               B_WILL_DRAW | B_SUBPIXEL_PRECISE | B_FRAME_EVENTS, B_FANCY_BORDER);
  divider->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, 1));
  return divider;
}

AppToolbar::AppToolbar(BRect rect, BSize toolbarIconSize)
    : BWindow(rect, "Tools", B_FLOATING_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL,
              B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS, B_CURRENT_WORKSPACE) {
  const int iconSize = toolbarIconSize.IntegerWidth() / 1.5f;

  static BBitmap *lineToolIcon = LoadIconFromResource("line-tool", iconSize);
  static BBitmap *rectToolIcon = LoadIconFromResource("rect-tool", iconSize);
  static BBitmap *circleToolIcon = LoadIconFromResource("circle-tool", iconSize);
  static BBitmap *arcToolIcon = LoadIconFromResource("arc-tool", iconSize);
  static BBitmap *tangentArcToolIcon = LoadIconFromResource("tangent-arc-tool", iconSize);
  static BBitmap *cubicSplineToolIcon = LoadIconFromResource("cubic-spline-tool", iconSize);
  static BBitmap *datumPointToolIcon = LoadIconFromResource("datum-point-tool", iconSize);
  static BBitmap *constructionToolIcon = LoadIconFromResource("construction-tool", iconSize);
  static BBitmap *splitCurvesToolIcon = LoadIconFromResource("split-curves-tool", iconSize);
  static BBitmap *textToolIcon = LoadIconFromResource("text-tool", iconSize);

  static BBitmap *distanceToolIcon = LoadIconFromResource("distance-tool", iconSize);
  static BBitmap *angleToolIcon = LoadIconFromResource("angle-tool", iconSize);

  static BBitmap *horizontalToolIcon = LoadIconFromResource("horizontal-tool", iconSize);
  static BBitmap *verticalToolIcon = LoadIconFromResource("vertical-tool", iconSize);
  static BBitmap *parallelToolIcon = LoadIconFromResource("parallel-tool", iconSize);

  static BBitmap *nearestIsoToolIcon = LoadIconFromResource("nearest-iso-tool", iconSize);
  static BBitmap *nearestOrthoToolIcon = LoadIconFromResource("nearest-ortho-tool", iconSize);

  static BBitmap *constrainPerpendicularToolIcon =
      LoadIconFromResource("constrain-perpendicular-tool", iconSize);
  static BBitmap *constrainPointOnLineToolIcon =
      LoadIconFromResource("constrain-point-on-line-tool", iconSize);
  static BBitmap *constrainSymmetricToolIcon =
      LoadIconFromResource("constrain-symmetric-tool", iconSize);
  static BBitmap *constrainEqualToolIcon = LoadIconFromResource("constrain-equal-tool", iconSize);
  static BBitmap *constrainParallelNormalsToolIcon =
      LoadIconFromResource("constrain-parallel-normals-tool", iconSize);
  static BBitmap *otherSupplementaryAngleToolIcon =
      LoadIconFromResource("other-supplementary-angle-tool", iconSize);
  static BBitmap *refToolIcon = LoadIconFromResource("ref-tool", iconSize);

  lineToolButton = MakeButton(toolbarIconSize, lineToolIcon, new BMessage(LINE_TOOL_BTN_CLICKED),
                              "Sketch line segment (S)");
  rectToolButton = MakeButton(toolbarIconSize, rectToolIcon, new BMessage(RECT_TOOL_BTN_CLICKED),
                              "Sketch rectangle (R)");
  circleToolButton = MakeButton(toolbarIconSize, circleToolIcon,
                                new BMessage(CIRCLE_TOOL_BTN_CLICKED), "Sketch circle (C)");
  arcToolButton = MakeButton(toolbarIconSize, arcToolIcon, new BMessage(ARC_TOOL_BTN_CLICKED),
                             "Sketch arc (A)");
  tangentArcToolButton =
      MakeButton(toolbarIconSize, tangentArcToolIcon, new BMessage(TANGENT_ARC_TOOL_BTN_CLICKED),
                 "Create tangent arc at selected point (Shift+A)");
  cubicSplineToolButton =
      MakeButton(toolbarIconSize, cubicSplineToolIcon, new BMessage(CUBIC_SPLINE_TOOL_BTN_CLICKED),
                 "Sketch cubic Bezier spline (B)");
  datumPointToolButton = MakeButton(toolbarIconSize, datumPointToolIcon,
                                    new BMessage(DATUM_POINT_TOOL_BTN_CLICKED), "Sketch point (P)");
  constructionToolButton =
      MakeButton(toolbarIconSize, constructionToolIcon, new BMessage(CONSTRUCTION_TOOL_BTN_CLICKED),
                 "Toggle construction (G)");
  splitCurvesToolButton =
      MakeButton(toolbarIconSize, splitCurvesToolIcon, new BMessage(SPLIT_CURVES_TOOL_BTN_CLICKED),
                 "Split intersection of lines and curves (I)");
  textToolButton = MakeButton(toolbarIconSize, textToolIcon, new BMessage(TEXT_TOOL_BTN_CLICKED),
                              "Add curves from text (T)");

  distanceToolButton =
      MakeButton(toolbarIconSize, distanceToolIcon, new BMessage(DISTANCE_DIA_TOOL_BTN_CLICKED),
                 "Constrain by distance, diameter or length (D)");
  angleToolButton = MakeButton(toolbarIconSize, angleToolIcon, new BMessage(ANGLE_TOOL_BTN_CLICKED),
                               "Constrain by angle (N)");
  horizontalToolButton =
      MakeButton(toolbarIconSize, horizontalToolIcon, new BMessage(HORIZONTAL_TOOL_BTN_CLICKED),
                 "Constrain horizontally (H)");
  verticalToolButton =
      MakeButton(toolbarIconSize, verticalToolIcon, new BMessage(VERTICAL_TOOL_BTN_CLICKED),
                 "Constraint vertically (V)");
  parallelToolButton =
      MakeButton(toolbarIconSize, parallelToolIcon, new BMessage(PARALLEL_TOOL_BTN_CLICKED),
                 "Constrain parallelly (L)");

  constrainPerpendicularToolButton =
      MakeButton(toolbarIconSize, constrainPerpendicularToolIcon,
                 new BMessage(CONSTRAIN_PERP_TOOL_BTN_CLICKED), "Constrain perpendicularly ([)");
  constrainPointOnLineToolButton = MakeButton(toolbarIconSize, constrainPointOnLineToolIcon,
                                              new BMessage(PT_ON_LINE_TOOL_BTN_CLICKED),
                                              "Constrain point on line, curve plane or point (O)");
  constrainSymmetricToolButton =
      MakeButton(toolbarIconSize, constrainSymmetricToolIcon,
                 new BMessage(CONSTRAIN_SYMMETRIC_TOOL_BTN_CLICKED), "Constrain symmetrically (Y)");
  constrainEqualToolButton = MakeButton(toolbarIconSize, constrainEqualToolIcon,
                                        new BMessage(CONSTRAIN_EQUAL_TOOL_BTN_CLICKED),
                                        "Constrain by equal length, radius or angle (Q)");
  constrainParallelNormalsToolButton =
      MakeButton(toolbarIconSize, constrainParallelNormalsToolIcon,
                 new BMessage(CONSTRAIN_ORIENTED_SAME_TOOL_BTN_CLICKED),
                 "Constrain normals in same orientation (X)");
  otherSupplementaryAngleToolButton =
      MakeButton(toolbarIconSize, otherSupplementaryAngleToolIcon,
                 new BMessage(OTHER_ANGLE_TOOL_BTN_CLICKED), "Find supplementary angle (U)");
  refToolButton = MakeButton(toolbarIconSize, refToolIcon, new BMessage(REF_TOOL_BTN_CLICKED),
                             "Toggle reference dimension (E)");

  nearestIsoToolButton =
      MakeButton(toolbarIconSize, nearestIsoToolIcon, new BMessage(NEAREST_ISO_TOOL_BTN_CLICKED),
                 "Align view to nearest isometric view (F3)");
  nearestOrthoToolButton = MakeButton(toolbarIconSize, nearestOrthoToolIcon,
                                      new BMessage(NEAREST_ORTHO_TOOL_BTN_CLICKED),
                                      "Align view to active workplane (W)");

  BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
      .AddGrid(0.0, 0.0)
      .Add(lineToolButton, 0, 0)
      .Add(rectToolButton, 1, 0)
      .Add(circleToolButton, 0, 1)
      .Add(arcToolButton, 1, 1)
      .Add(tangentArcToolButton, 0, 2)
      .Add(cubicSplineToolButton, 1, 2)
      .Add(datumPointToolButton, 0, 3)
      .Add(constructionToolButton, 1, 3)
      .Add(splitCurvesToolButton, 0, 4)
      .Add(textToolButton, 1, 4)
      .End()
      .Add(MakeDivider())
      .AddGrid(0.0, 0.0)
      .Add(distanceToolButton, 0, 0)
      .Add(angleToolButton, 1, 0)
      .Add(horizontalToolButton, 0, 1)
      .Add(verticalToolButton, 1, 1)
      .Add(parallelToolButton, 0, 2)
      .Add(constrainPerpendicularToolButton, 1, 2)
      .Add(constrainPointOnLineToolButton, 0, 3)
      .Add(constrainSymmetricToolButton, 1, 3)
      .Add(constrainEqualToolButton, 0, 4)
      .Add(constrainParallelNormalsToolButton, 1, 4)
      .Add(otherSupplementaryAngleToolButton, 0, 5)
      .Add(refToolButton, 1, 5)
      .End()
      .Add(MakeDivider())
      .AddGrid(0.0, 0.0)
      .Add(nearestIsoToolButton, 0, 0)
      .Add(nearestOrthoToolButton, 1, 0)
      .End();
}

void AppToolbar::MessageReceived(BMessage *message) {
  switch (message->what) {
  case LINE_TOOL_BTN_CLICKED: {
    SS.MenuRequest(SolveSpace::Command::LINE_SEGMENT);
    lineToolButton->SetValue(1);
    break;
  }
  case LINE_TOOL_BTN_UNCLICKED: {
    lineToolButton->SetValue(0);
    break;
  }
  case RECT_TOOL_BTN_CLICKED: {
    SS.MenuRequest(SolveSpace::Command::RECTANGLE);
    rectToolButton->SetValue(1);
    break;
  }
  case RECT_TOOL_BTN_UNCLICKED: {
    rectToolButton->SetValue(0);
    break;
  }
  case CIRCLE_TOOL_BTN_CLICKED: {
    SS.MenuRequest(SolveSpace::Command::CIRCLE);
    circleToolButton->SetValue(1);
    break;
  }
  case CIRCLE_TOOL_BTN_UNCLICKED: {
    circleToolButton->SetValue(0);
    break;
  }
  case ARC_TOOL_BTN_CLICKED: {
    SS.MenuRequest(SolveSpace::Command::ARC);
    arcToolButton->SetValue(1);
    break;
  }
  case ARC_TOOL_BTN_UNCLICKED: {
    arcToolButton->SetValue(0);
    break;
  }
  case TANGENT_ARC_TOOL_BTN_CLICKED: {
    SS.MenuRequest(SolveSpace::Command::TANGENT_ARC);
    tangentArcToolButton->SetValue(1);
    break;
  }
  case TANGENT_ARC_TOOL_BTN_UNCLICKED: {
    tangentArcToolButton->SetValue(0);
    break;
  }
  case CUBIC_SPLINE_TOOL_BTN_CLICKED: {
    SS.MenuRequest(SolveSpace::Command::CUBIC);
    cubicSplineToolButton->SetValue(1);
    break;
  }
  case CUBIC_SPLINE_TOOL_BTN_UNCLICKED: {
    cubicSplineToolButton->SetValue(0);
    break;
  }
  case DATUM_POINT_TOOL_BTN_CLICKED: {
    SS.MenuRequest(SolveSpace::Command::DATUM_POINT);
    datumPointToolButton->SetValue(1);
    break;
  }
  case DATUM_POINT_TOOL_BTN_UNCLICKED: {
    datumPointToolButton->SetValue(0);
    break;
  }
  case CONSTRUCTION_TOOL_BTN_CLICKED: {
    SS.MenuRequest(SolveSpace::Command::CONSTRUCTION);
    constructionToolButton->SetValue(1);
    break;
  }
  case CONSTRUCTION_TOOL_BTN_UNCLICKED: {
    constructionToolButton->SetValue(0);
    break;
  }
  case SPLIT_CURVES_TOOL_BTN_CLICKED: {
    SS.MenuRequest(SolveSpace::Command::SPLIT_CURVES);
    splitCurvesToolButton->SetValue(1);
    break;
  }
  case SPLIT_CURVES_TOOL_BTN_UNCLICKED: {
    splitCurvesToolButton->SetValue(0);
    break;
  }
  case TEXT_TOOL_BTN_CLICKED: {
    SS.MenuRequest(SolveSpace::Command::TTF_TEXT);
    textToolButton->SetValue(1);
    break;
  }
  case TEXT_TOOL_BTN_UNCLICKED: {
    textToolButton->SetValue(0);
    break;
  }
  default: {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(message);
    break;
  }
  }
}
