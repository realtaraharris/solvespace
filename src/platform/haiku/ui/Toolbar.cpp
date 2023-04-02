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
	tempView.DrawBitmapAsync(icon, BPoint(6, 6));
	BPicture *off = tempView.EndPicture();

	// Draw a pressed button
	tempView.BeginPicture(new BPicture());
	DrawButton(&tempView, frame.OffsetToCopy(0, 0), true);
	tempView.SetDrawingMode(B_OP_ALPHA);
    tempView.SetHighColor(0, 0, 0, 200);
    tempView.SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_OVERLAY);
	tempView.DrawBitmap(icon, BPoint(5, 5));
	BPicture *on = tempView.EndPicture();

	this->RemoveChild(&tempView);
	this->AddChild(new BPictureButton(frame, "", off, on, message));
}

AppToolbar::AppToolbar(void)
	: BWindow(
		BRect(BPoint(10, 100), BSize(60, 600)),
		"Tools",
		B_FLOATING_WINDOW_LOOK,
		B_FLOATING_APP_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS,
		B_CURRENT_WORKSPACE
	) {
	static BBitmap *lineToolIcon = LoadIconFromResource("line-tool", 18);
	static BBitmap *rectToolIcon = LoadIconFromResource("rect-tool", 18);
	static BBitmap *circleToolIcon = LoadIconFromResource("circle-tool", 18);
	static BBitmap *arcToolIcon = LoadIconFromResource("arc-tool", 18);
	static BBitmap *tangentArcToolIcon = LoadIconFromResource("tangent-arc-tool", 18);
	static BBitmap *cubicSplineToolIcon = LoadIconFromResource("cubic-spline-tool", 18);
	static BBitmap *datumPointToolIcon = LoadIconFromResource("datum-point-tool", 18);

	const BSize buttonSize = BSize(30.0, 30.0);

	this->AddButton(BRect(BPoint(0.0, 0.0), buttonSize), lineToolIcon, new BMessage(LINE_TOOL_BTN_CLICKED));
	this->AddButton(BRect(BPoint(30.0, 0.0), buttonSize), rectToolIcon, new BMessage(RECT_TOOL_BTN_CLICKED));

	this->AddButton(BRect(BPoint(0.0, 30.0), buttonSize), circleToolIcon, new BMessage(CIRCLE_TOOL_BTN_CLICKED));
	this->AddButton(BRect(BPoint(30.0, 30.0), buttonSize), arcToolIcon, new BMessage(ARC_TOOL_BTN_CLICKED));

	this->AddButton(BRect(BPoint(0.0, 60.0), buttonSize), tangentArcToolIcon, new BMessage(TANGENT_ARC_TOOL_BTN_CLICKED));
	this->AddButton(BRect(BPoint(30.0, 60.0), buttonSize), cubicSplineToolIcon, new BMessage(CUBIC_SPLINE_TOOL_BTN_CLICKED));

    this->AddButton(BRect(BPoint(0.0, 90.0), buttonSize), datumPointToolIcon, new BMessage(DATUM_POINT_TOOL_BTN_CLICKED));

	this->Show();
}

void AppToolbar::MessageReceived(BMessage *msg) {
	be_app->WindowAt(0)->PostMessage(msg);
}
