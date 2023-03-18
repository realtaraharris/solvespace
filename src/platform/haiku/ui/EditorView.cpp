/*
 * Copyright 2023, Tara Harris <3769985+realtaraharris@users.noreply.github.com>
 * Portions copyright 2002-2006 Maxim Shemanarev
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "EditorView.h"
#include "../src/render/render.h"

#include <iostream>

void attachBufferToBBitmap(agg::rendering_buffer& buffer, BBitmap* bitmap) {
    uint8* bits = (uint8*)bitmap->Bits();
    uint32 width = bitmap->Bounds().IntegerWidth() + 1;
    uint32 height = bitmap->Bounds().IntegerHeight() + 1;
    int32 bpr = bitmap->BytesPerRow();
    buffer.attach(bits, width, height, -bpr);
}

EditorView::EditorView(BRect rect) :
	BView(rect, "SolveSpace Editor View", B_FOLLOW_ALL_SIDES, B_FRAME_EVENTS | B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE) {
    initialRect = rect;
    currentRect = rect;

    // choices: SS.GW.canvas.get(), std::static_pointer_cast<AggPixmapRenderer>(SS.GW.canvas)
    SS.GW.canvas = std::make_shared<AggPixmapRenderer>();

    InitBitmapAndBuffer();
}

EditorView::~EditorView() {}

void EditorView::InitBitmapAndBuffer() {
    retainedBitmap = new BBitmap(currentRect, 0, B_RGBA32);
    if (retainedBitmap->IsValid()) {
        attachBufferToBBitmap(std::static_pointer_cast<AggPixmapRenderer>(SS.GW.canvas)->buffer, retainedBitmap);
    } else {
        delete retainedBitmap;
        retainedBitmap = NULL;
    }
}

void EditorView::Draw(BRect updateRect) {
    SS.GW.canvas.get()->Clear();
    SS.GW.canvas.get()->StartFrame();
    SS.GW.Draw(SS.GW.canvas.get());
    SS.GW.canvas.get()->FlushFrame();
    SS.GW.canvas.get()->FinishFrame();

    DrawBitmap(retainedBitmap, updateRect, updateRect);
}

// this is never actually called in this example because nothing moves the view frame
void EditorView::FrameMoved(BPoint newLocation) {
    currentRect.SetLeftTop(newLocation);
}

void EditorView::FrameResized(float width, float height) {
    currentRect.SetRightBottom(currentRect.LeftTop() + BPoint(width, height));
    initialRect = currentRect;

    InitBitmapAndBuffer(); // do this before drawing

    camera.width = width;
    camera.height = height;
    std::static_pointer_cast<AggPixmapRenderer>(SS.GW.canvas)->SetCamera(camera);

    Draw(currentRect);
}

bool EditorView::Load(std::string path) {
	const SolveSpace::Platform::Path fixturePath = SolveSpace::Platform::Path::From(
		path);

    if (!SS.LoadFromFile(fixturePath)) {
	    return;
	}
	std::cout << "loaded: " << path << std::endl;

	SS.AfterNewFile();

	SS.GW.Init();
	SS.GW.offset = {};
	SS.GW.scale  = 10.0;

	camera = SS.GW.GetCamera();
    camera.pixelRatio = 1;
    camera.gridFit    = true;
    camera.width      = initialRect.Width();
    camera.height     = initialRect.Height();
    camera.projUp     = SS.GW.projUp;
    camera.projRight  = SS.GW.projRight;

    SS.gridSpacing = 5.0; // TODO: get these from settings file?

    camera.scale      = SS.GW.scale;
    camera.offset     = SS.GW.offset;

    SS.GW.canvas.get()->SetLighting(SS.GW.GetLighting());
    SS.GW.canvas.get()->SetCamera(camera);
    std::static_pointer_cast<AggPixmapRenderer>(SS.GW.canvas)->Init(false);

    Draw(initialRect);
}

void EditorView::MouseMoved(BPoint point, uint32 transit, const BMessage* message) {
	currentMousePosition = point;
}

void EditorView::ZoomToMouse(double zoomMultiplyer) {
    double offsetRight = camera.offset.Dot(camera.projRight);
    double offsetUp    = camera.offset.Dot(camera.projUp);

    double righti = currentMousePosition.x / camera.scale - offsetRight;
    double upi    = currentMousePosition.y / camera.scale - offsetUp;

    // zoomMultiplyer of 1 gives a default zoom factor of 1.2x: zoomMultiplyer * 1.2
    // zoom = adjusted zoom negative zoomMultiplyer will zoom out, positive will zoom in
    camera.scale *= exp(0.1823216 * zoomMultiplyer); // ln(1.2) = 0.1823216

    double rightf = currentMousePosition.x / camera.scale - offsetRight;
    double upf    = currentMousePosition.y / camera.scale - offsetUp;

    camera.offset = camera.offset.Plus(projRight.ScaledBy(rightf - righti));
    camera.offset = camera.offset.Plus(projUp.ScaledBy(upf - upi));

    SS.GW.canvas.get()->SetCamera(camera);
}

void EditorView::ZoomToFit(bool includingInvisibles, bool useSelection) {
	 SS.GW.ZoomToFit(camera, false, true); // includingInvisibles = false, useSelection = true

	 camera.offset = SS.GW.offset;
	 camera.scale = SS.GW.scale;

	 SS.GW.canvas.get()->SetCamera(camera);
}