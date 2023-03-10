/*
 * Copyright 2023, Tara Harris <3769985+realtaraharris@users.noreply.github.com>
 * Portions copyright 2002-2006 Maxim Shemanarev
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include "EditorView.h"

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

    InitBitmapAndBuffer();
}

EditorView::~EditorView() {}

void EditorView::InitBitmapAndBuffer() {
    retainedBitmap = new BBitmap(currentRect, 0, B_RGBA32);
    if (retainedBitmap->IsValid()) {
        attachBufferToBBitmap(pixmapCanvas.buffer, retainedBitmap);
    } else {
        delete retainedBitmap;
        retainedBitmap = NULL;
    }
}

void EditorView::Draw(BRect updateRect) {
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
    pixmapCanvas.SetCamera(camera);

    pixmapCanvas.Clear();
    pixmapCanvas.StartFrame();
    SS.GW.Draw(&pixmapCanvas);
    pixmapCanvas.FlushFrame();
    pixmapCanvas.FinishFrame();

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
	SS.GW.offset = {};
	SS.GW.scale  = 10.0;
 
	camera = {};
    camera.pixelRatio = 1;
    camera.gridFit    = true;
    camera.width      = initialRect.Width();
    camera.height     = initialRect.Height();
    camera.projUp     = SS.GW.projUp;
    camera.projRight  = SS.GW.projRight;
    camera.scale      = SS.GW.scale;

    pixmapCanvas.SetLighting(SS.GW.GetLighting());
    pixmapCanvas.SetCamera(camera);
    pixmapCanvas.Init(false);

    pixmapCanvas.StartFrame();
    SS.GW.Draw(&pixmapCanvas);
    pixmapCanvas.FlushFrame();
    pixmapCanvas.FinishFrame();

    Draw(initialRect);
}
