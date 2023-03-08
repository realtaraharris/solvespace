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

	std::cout << "attached buffer to bbitmap" << std::endl;
}

EditorView::EditorView(BRect rect) :
	BView(rect, "SolveSpace Editor View", B_FOLLOW_ALL_SIDES, B_FRAME_EVENTS | B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE) {
    initialRect = rect;
    currentRect = rect;
    
    std::cout << "currentRect.Width(): " << currentRect.Width() << " currentRect.Height(): " << currentRect.Height() << std::endl;

//    SetTransAffineResizingMatrix(rect.Width(), rect.Height(), true);

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

//void EditorView::AttachedToWindow() {}

//void EditorView::DetachedFromWindow() {}

void EditorView::Draw(BRect updateRect) {
    SS.GW.Draw(&pixmapCanvas);
    DrawBitmap(retainedBitmap, updateRect, updateRect);
}

// this is never actually called in this example because nothing moves the view frame
void EditorView::FrameMoved(BPoint newLocation) {
    currentRect.SetLeftTop(newLocation);
}

void EditorView::FrameResized(float width, float height) {
    currentRect.SetRightBottom(initialRect.LeftTop() + BPoint(width, height));
//    SetTransAffineResizingMatrix(width + 1, height + 1, true);
    InitBitmapAndBuffer(); // do this before drawing

    camera.width = width;
    camera.height = height + 20;
    pixmapCanvas.SetCamera(camera);

    pixmapCanvas.StartFrame();
    SS.GW.Draw(&pixmapCanvas);
    pixmapCanvas.FlushFrame();
    pixmapCanvas.FinishFrame();

    Draw(currentRect);
}
/*
void EditorView::SetTransAffineResizingMatrix(unsigned width, unsigned height, bool keepAspectRatio) {
    if (keepAspectRatio) {
        agg::trans_viewport vp;
        vp.preserve_aspect_ratio(0.5, 0.5, agg::aspect_ratio_meet);
        vp.device_viewport(0, 0, width, height);
        vp.world_viewport(0, 0, initialRect.Width(), initialRect.Height());
        resizeMatrix = vp.to_affine();
    } else {
        resizeMatrix = agg::trans_affine_scaling(
            width / initialRect.Width(),
            height / initialRect.Height());
    }
}

const agg::trans_affine& EditorView::GetTransAffineResizingMatrix() const {
    return resizeMatrix;
}
*/

bool EditorView::Load(std::string path) {
//    Platform::Path fixturePath = GetAssetPath(file, fixture);
	const SolveSpace::Platform::Path fixturePath = SolveSpace::Platform::Path::From(
		path);
	std::cout << "loaded: " << path << std::endl;

    FILE *f = SolveSpace::Platform::OpenFile(fixturePath, "rb");

    if (f) { fclose(f); }

    bool result = SS.LoadFromFile(fixturePath);
//    if (!RecordCheck(result)) {
//        return false;
//    } else {
        SS.AfterNewFile();
        SS.GW.offset = {};
        SS.GW.scale  = 10.0;
 //   }
 
	camera = {};
    camera.pixelRatio = 1;
    camera.gridFit    = true;
    camera.width      = currentRect.Width();
    camera.height     = currentRect.Height() + 20; // how?!
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

    Draw(currentRect);
}
