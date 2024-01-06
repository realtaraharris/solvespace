#include "Toolbar.h"

#include <Application.h>
#include <Bitmap.h>
#include <NodeInfo.h>
#include <PictureButton.h>
#include <Roster.h>
#include <View.h>
#include <Window.h>

#include "DrawButton.h"
#include "HVIFUtil.h"

#include "App.h" // contains message enums

#include <Alert.h> // BAlert
#include <Button.h>
#include <iostream>

void AppToolbar::AddButton(const BRect &frame, BBitmap *icon,
                           BMessage *message, const char* tooltipText) {
    BView tempView(frame, "", 0, 0);
    this->AddChild(&tempView);

    // Draw a normal button
    tempView.BeginPicture(new BPicture());
    DrawButton(&tempView, frame.OffsetToCopy(0, 0), false);
    tempView.SetDrawingMode(B_OP_OVER);
    tempView.DrawBitmapAsync(icon, BPoint(5, 5));
    BPicture *off = tempView.EndPicture();

    // Draw a pressed button
    tempView.BeginPicture(new BPicture());
    DrawButton(&tempView, frame.OffsetToCopy(0, 0), true);
    tempView.SetDrawingMode(B_OP_ALPHA);
    tempView.SetHighColor(0, 0, 0, 200);
    tempView.SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_OVERLAY);
    tempView.DrawBitmap(icon, BPoint(4, 4));
    BPicture *on = tempView.EndPicture();

    this->RemoveChild(&tempView);
    BPictureButton *pictureButton = new BPictureButton(frame, "", off, on, message);
    pictureButton->SetToolTip(tooltipText);
    this->AddChild(pictureButton);
}

AppToolbar::AppToolbar(void)
    : BWindow(BRect(BPoint(10, 100), BSize(60, 366)), "Tools",
              B_FLOATING_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL,
              B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS,
              B_CURRENT_WORKSPACE) {
    const int TOOLBAR_ICON_SIZE = 20;
    static BBitmap *lineToolIcon =
        LoadIconFromResource("line-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *rectToolIcon =
        LoadIconFromResource("rect-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *circleToolIcon =
        LoadIconFromResource("circle-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *arcToolIcon =
        LoadIconFromResource("arc-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *tangentArcToolIcon =
        LoadIconFromResource("tangent-arc-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *cubicSplineToolIcon =
        LoadIconFromResource("cubic-spline-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *datumPointToolIcon =
        LoadIconFromResource("datum-point-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *constructionToolIcon =
        LoadIconFromResource("construction-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *splitCurvesToolIcon =
        LoadIconFromResource("split-curves-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *textToolIcon =
        LoadIconFromResource("text-tool", TOOLBAR_ICON_SIZE);

    static BBitmap *distanceToolIcon =
        LoadIconFromResource("distance-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *angleToolIcon =
        LoadIconFromResource("angle-tool", TOOLBAR_ICON_SIZE);

    static BBitmap *horizontalToolIcon =
        LoadIconFromResource("horizontal-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *verticalToolIcon =
        LoadIconFromResource("vertical-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *parallelToolIcon =
        LoadIconFromResource("parallel-tool", TOOLBAR_ICON_SIZE);

    static BBitmap *nearestIsoToolIcon =
        LoadIconFromResource("nearest-iso-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *nearestOrthoToolIcon =
        LoadIconFromResource("nearest-ortho-tool", TOOLBAR_ICON_SIZE);

    static BBitmap *constrainPerpendicularToolIcon =
        LoadIconFromResource("constrain-perpendicular-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *constrainPointOnLineToolIcon =
        LoadIconFromResource("constrain-point-on-line-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *constrainSymmetricToolIcon =
        LoadIconFromResource("constrain-symmetric-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *constrainEqualToolIcon =
        LoadIconFromResource("constrain-equal-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *constrainParallelNormalsToolIcon =
        LoadIconFromResource("constrain-parallel-normals-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *otherSupplementaryAngleToolIcon =
        LoadIconFromResource("other-supplementary-angle-tool", TOOLBAR_ICON_SIZE);
    static BBitmap *refToolIcon =
        LoadIconFromResource("ref-tool", TOOLBAR_ICON_SIZE);

    const BSize buttonSize = BSize(30.0, 30.0);

    this->AddButton(BRect(BPoint(0.0, 0.0), buttonSize), lineToolIcon,
                    new BMessage(LINE_TOOL_BTN_CLICKED), "Sketch line segment (S)");
    this->AddButton(BRect(BPoint(30.0, 0.0), buttonSize), rectToolIcon,
                    new BMessage(RECT_TOOL_BTN_CLICKED), "Sketch rectangle (R)");

    this->AddButton(BRect(BPoint(0.0, 30.0), buttonSize), circleToolIcon,
                    new BMessage(CIRCLE_TOOL_BTN_CLICKED), "Sketch circle (C)");
    this->AddButton(BRect(BPoint(30.0, 30.0), buttonSize), arcToolIcon,
                    new BMessage(ARC_TOOL_BTN_CLICKED), "Sketch arc (A)");

    this->AddButton(BRect(BPoint(0.0, 60.0), buttonSize), tangentArcToolIcon,
                    new BMessage(TANGENT_ARC_TOOL_BTN_CLICKED), "Create tangent arc at selected point (Shift+A)");
    this->AddButton(BRect(BPoint(30.0, 60.0), buttonSize), cubicSplineToolIcon,
                    new BMessage(CUBIC_SPLINE_TOOL_BTN_CLICKED), "Sketch cubic Bezier spline (B)");

    this->AddButton(BRect(BPoint(0.0, 90.0), buttonSize), datumPointToolIcon,
                    new BMessage(DATUM_POINT_TOOL_BTN_CLICKED), "Sketch point (P)");
    this->AddButton(BRect(BPoint(30.0, 90.0), buttonSize), constructionToolIcon,
                    new BMessage(CONSTRUCTION_TOOL_BTN_CLICKED), "Toggle construction (G)");

    this->AddButton(BRect(BPoint(0.0, 120.0), buttonSize), splitCurvesToolIcon,
                    new BMessage(SPLIT_CURVES_TOOL_BTN_CLICKED), "Split intersection of lines and curves (I)");
    this->AddButton(BRect(BPoint(30.0, 120.0), buttonSize), textToolIcon,
                    new BMessage(TEXT_TOOL_BTN_CLICKED), "Add curves from text (T)");

    // TODO: add divider

    this->AddButton(BRect(BPoint(0.0, 153.0), buttonSize), distanceToolIcon,
                    new BMessage(DISTANCE_DIA_TOOL_BTN_CLICKED), "Constrain by distance, diameter or length (D)");
    this->AddButton(BRect(BPoint(30.0, 153.0), buttonSize), angleToolIcon,
                    new BMessage(ANGLE_TOOL_BTN_CLICKED), "Constrain by angle (N)");

    this->AddButton(BRect(BPoint(0.0, 183.0), buttonSize), horizontalToolIcon,
                    new BMessage(HORIZONTAL_TOOL_BTN_CLICKED), "Constrain horizontally (H)");
    this->AddButton(BRect(BPoint(30.0, 183.0), buttonSize), verticalToolIcon,
                    new BMessage(VERTICAL_TOOL_BTN_CLICKED), "Constraint vertically (V)");

    this->AddButton(BRect(BPoint(0.0, 213.0), buttonSize), parallelToolIcon,
                    new BMessage(PARALLEL_TOOL_BTN_CLICKED), "Constrain parallelly (L)");
    this->AddButton(BRect(BPoint(30.0, 213.0), buttonSize), constrainPerpendicularToolIcon,
                    new BMessage(CONSTRAIN_PERP_TOOL_BTN_CLICKED), "Constrain perpendicularly ([)");

    this->AddButton(BRect(BPoint(0.0, 243.0), buttonSize), constrainPointOnLineToolIcon,
                    new BMessage(PT_ON_LINE_TOOL_BTN_CLICKED), "Constrain point on line, curve plane or point (O)");
    this->AddButton(BRect(BPoint(30.0, 243.0), buttonSize), constrainSymmetricToolIcon,
                    new BMessage(CONSTRAIN_SYMMETRIC_TOOL_BTN_CLICKED), "Constrain symmetrically (Y)");

    this->AddButton(BRect(BPoint(0.0, 273.0), buttonSize), constrainEqualToolIcon,
                    new BMessage(CONSTRAIN_EQUAL_TOOL_BTN_CLICKED), "Constrain by equal length, radius or angle (Q)");
    this->AddButton(BRect(BPoint(30.0, 273.0), buttonSize), constrainParallelNormalsToolIcon,
                    new BMessage(CONSTRAIN_ORIENTED_SAME_TOOL_BTN_CLICKED), "Constrain normals in same orientation (X)");

    this->AddButton(BRect(BPoint(0.0, 303.0), buttonSize), otherSupplementaryAngleToolIcon,
                    new BMessage(OTHER_ANGLE_TOOL_BTN_CLICKED), "Find supplementary angle (U)");
    this->AddButton(BRect(BPoint(30.0, 303.0), buttonSize), refToolIcon,
                    new BMessage(REF_TOOL_BTN_CLICKED), "Toggle reference dimension (E)");

    this->AddButton(BRect(BPoint(0.0, 336.0), buttonSize), nearestIsoToolIcon,
                    new BMessage(NEAREST_ISO_TOOL_BTN_CLICKED), "Align view to nearest isometric view (F3)");
    this->AddButton(BRect(BPoint(30.0, 336.0), buttonSize),
                    nearestOrthoToolIcon,
                    new BMessage(NEAREST_ORTHO_TOOL_BTN_CLICKED), "Align view to active workplane (W)");
}

void AppToolbar::MessageReceived(BMessage *msg) {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(msg);
}
