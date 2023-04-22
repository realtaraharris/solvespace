#include "Toolbar.h"

#include <Application.h>
#include <Window.h>
#include <View.h>
#include <PictureButton.h>
#include <Roster.h>
#include <NodeInfo.h>
#include <Bitmap.h>

#include "DrawButton.h"
#include "HVIFUtil.h"

#include "App.h" // contains message enums

#include <Button.h>
#include <iostream>
#include <Alert.h> // BAlert

void AppToolbar::AddButton(const BRect &frame, BBitmap *icon, BMessage *message) {
	BView tempView(frame, "", 0, 0);
	this->AddChild(&tempView);

	// Draw a normal button
	tempView.BeginPicture(new BPicture());
	DrawButton(&tempView, frame.OffsetToCopy(0, 0) , false);
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
	this->AddChild(new BPictureButton(frame, "", off, on, message));
}

AppToolbar::AppToolbar(void)
	: BWindow(
		BRect(BPoint(10, 100), BSize(60, 600)),
		"Tools",
		B_FLOATING_WINDOW_LOOK,
		B_FLOATING_ALL_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS,
		B_CURRENT_WORKSPACE
	) {
		const int TOOLBAR_ICON_SIZE = 20;
	static BBitmap *lineToolIcon = LoadIconFromResource("line-tool", TOOLBAR_ICON_SIZE);
	static BBitmap *rectToolIcon = LoadIconFromResource("rect-tool", TOOLBAR_ICON_SIZE);
	static BBitmap *circleToolIcon = LoadIconFromResource("circle-tool", TOOLBAR_ICON_SIZE);
	static BBitmap *arcToolIcon = LoadIconFromResource("arc-tool", TOOLBAR_ICON_SIZE);
	static BBitmap *tangentArcToolIcon = LoadIconFromResource("tangent-arc-tool", TOOLBAR_ICON_SIZE);
	static BBitmap *cubicSplineToolIcon = LoadIconFromResource("cubic-spline-tool", TOOLBAR_ICON_SIZE);
	static BBitmap *datumPointToolIcon = LoadIconFromResource("datum-point-tool", TOOLBAR_ICON_SIZE);
	static BBitmap *constructionToolIcon = LoadIconFromResource("construction-tool", TOOLBAR_ICON_SIZE);
	static BBitmap *splitCurvesToolIcon = LoadIconFromResource("split-curves-tool", TOOLBAR_ICON_SIZE);
	static BBitmap *textToolIcon = LoadIconFromResource("text-tool", TOOLBAR_ICON_SIZE);

	static BBitmap *distanceToolIcon = LoadIconFromResource("distance-tool", TOOLBAR_ICON_SIZE);
	static BBitmap *angleToolIcon = LoadIconFromResource("angle-tool", TOOLBAR_ICON_SIZE);

	static BBitmap *horizontalToolIcon = LoadIconFromResource("horizontal-tool", TOOLBAR_ICON_SIZE);
	static BBitmap *verticalToolIcon = LoadIconFromResource("vertical-tool", TOOLBAR_ICON_SIZE);

	const BSize buttonSize = BSize(30.0, 30.0);

	this->AddButton(BRect(BPoint(0.0, 0.0), buttonSize), lineToolIcon, new BMessage(LINE_TOOL_BTN_CLICKED));
	this->AddButton(BRect(BPoint(30.0, 0.0), buttonSize), rectToolIcon, new BMessage(RECT_TOOL_BTN_CLICKED));

	this->AddButton(BRect(BPoint(0.0, 30.0), buttonSize), circleToolIcon, new BMessage(CIRCLE_TOOL_BTN_CLICKED));
	this->AddButton(BRect(BPoint(30.0, 30.0), buttonSize), arcToolIcon, new BMessage(ARC_TOOL_BTN_CLICKED));

	this->AddButton(BRect(BPoint(0.0, 60.0), buttonSize), tangentArcToolIcon, new BMessage(TANGENT_ARC_TOOL_BTN_CLICKED));
	this->AddButton(BRect(BPoint(30.0, 60.0), buttonSize), cubicSplineToolIcon, new BMessage(CUBIC_SPLINE_TOOL_BTN_CLICKED));

    this->AddButton(BRect(BPoint(0.0, 90.0), buttonSize), datumPointToolIcon, new BMessage(DATUM_POINT_TOOL_BTN_CLICKED));
    this->AddButton(BRect(BPoint(30.0, 90.0), buttonSize), constructionToolIcon, new BMessage(CONSTRUCTION_TOOL_BTN_CLICKED));

    this->AddButton(BRect(BPoint(0.0, 120.0), buttonSize), splitCurvesToolIcon, new BMessage(SPLIT_CURVES_TOOL_BTN_CLICKED));
    this->AddButton(BRect(BPoint(30.0, 120.0), buttonSize), textToolIcon, new BMessage(TEXT_TOOL_BTN_CLICKED));

    // TODO: add divider

    this->AddButton(BRect(BPoint(0.0, 153.0), buttonSize), distanceToolIcon, new BMessage(DISTANCE_DIA_TOOL_BTN_CLICKED));
    this->AddButton(BRect(BPoint(30.0, 153.0), buttonSize), angleToolIcon, new BMessage(ANGLE_TOOL_BTN_CLICKED));

    this->AddButton(BRect(BPoint(0.0, 183.0), buttonSize), horizontalToolIcon, new BMessage(HORIZONTAL_TOOL_BTN_CLICKED));
    this->AddButton(BRect(BPoint(30.0, 183.0), buttonSize), verticalToolIcon, new BMessage(VERTICAL_TOOL_BTN_CLICKED));

	this->Show();
}

void AppToolbar::MessageReceived(BMessage *msg) {
	be_app->WindowAt(0)->PostMessage(msg);
}
