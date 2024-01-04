/*
 * Copyright 2023, Tara Harris <3769985+realtaraharris@users.noreply.github.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "EditorView.h"
#include "../src/render/render.h"
#include "Window.h"

#include <iostream>

void attachBufferToBBitmap(agg::rendering_buffer &buffer, BBitmap *bitmap) {
    uint8 *bits = (uint8 *)bitmap->Bits();
    uint32 width = bitmap->Bounds().IntegerWidth() + 1;
    uint32 height = bitmap->Bounds().IntegerHeight() + 1;
    int32 bpr = bitmap->BytesPerRow();
    buffer.attach(bits, width, height, -bpr);
}

EditorView::EditorView()
    : BView(Bounds(), "SolveSpace Editor View", B_FOLLOW_ALL_SIDES,
            B_FRAME_EVENTS | B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE) {
    // choices: SS.GW.canvas.get(),
    // std::static_pointer_cast<AggPixmapRenderer>(SS.GW.canvas)
    SS.GW.canvas = std::make_shared<AggPixmapRenderer>();

    InitBitmapAndBuffer();
    Invalidate();
}

EditorView::~EditorView() {}

void EditorView::InitBitmapAndBuffer() {
    retainedBitmap = new BBitmap(Bounds(), 0, B_RGBA32);
    if (retainedBitmap->IsValid()) {
        attachBufferToBBitmap(
            std::static_pointer_cast<AggPixmapRenderer>(SS.GW.canvas)->buffer,
            retainedBitmap);
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

void EditorView::FrameResized(float width, float height) {
    InitBitmapAndBuffer(); // do this before drawing
    camera.width = width;
    camera.height = height;
    std::static_pointer_cast<AggPixmapRenderer>(SS.GW.canvas)
        ->SetCamera(camera);

    Draw(Bounds());
}

void EditorView::Load(std::string path) {
    const SolveSpace::Platform::Path fixturePath =
        SolveSpace::Platform::Path::From(path);

    if (!SS.LoadFromFile(fixturePath)) {
        return;
    }

    SS.AfterNewFile();

    SS.GW.Init();
    SS.GW.offset = {};
    SS.GW.scale = 5.0;
    SS.GW.projRight = Vector::From(1, 0, 0);
    SS.GW.projUp = Vector::From(0, 1, 0);

    camera = SS.GW.GetCamera();
    camera.pixelRatio = 1;
    camera.gridFit = true;
    camera.width = Bounds().Width();
    camera.height = Bounds().Height();
    camera.projUp = SS.GW.projUp;
    camera.projRight = SS.GW.projRight;

    SS.gridSpacing = 5.0; // TODO: get these from settings file?

    camera.scale = SS.GW.scale;
    camera.offset = SS.GW.offset;

    SS.GW.canvas.get()->SetLighting(SS.GW.GetLighting());
    SS.GW.canvas.get()->SetCamera(camera);
    std::static_pointer_cast<AggPixmapRenderer>(SS.GW.canvas)->Init(false);

    FrameResized(camera.width, camera.height);
}

SolveSpace::Platform::MouseEvent::Button EditorView::GetMouseButton() {
    int32 buttons;
    if (Window()->CurrentMessage()->FindInt32("Buttons", (int32 *)&buttons)) {
        if (buttons & B_PRIMARY_MOUSE_BUTTON) {
            return SolveSpace::Platform::MouseEvent::Button::LEFT;
        }
        if (buttons & B_SECONDARY_MOUSE_BUTTON) {
            return SolveSpace::Platform::MouseEvent::Button::MIDDLE;
        }
        if (buttons & B_TERTIARY_MOUSE_BUTTON) {
            return SolveSpace::Platform::MouseEvent::Button::RIGHT;
        }
    }

    return SolveSpace::Platform::MouseEvent::Button::LEFT;
}

void EditorView::MouseDown(BPoint point) {
    SolveSpace::Platform::MouseEvent event = {};
    event.type = SolveSpace::Platform::MouseEvent::Type::PRESS;
    event.button = GetMouseButton();
    event.x = point.x;
    event.y = point.y;

    SS.GW.MouseEvent(event);
    SS.GenerateAll(SolveSpaceUI::Generate::UNTIL_ACTIVE);
    Draw(Bounds());
}

void EditorView::MouseMoved(BPoint point, uint32 transit,
                            const BMessage *message) {
    currentMousePosition = point;

    SolveSpace::Platform::MouseEvent event = {};
    event.type = SolveSpace::Platform::MouseEvent::Type::MOTION;
    event.button = GetMouseButton();
    event.x = point.x;
    event.y = point.y;

    SS.GW.MouseEvent(event);
    SS.GenerateAll(SolveSpaceUI::Generate::UNTIL_ACTIVE);
    Draw(Bounds());
}

void EditorView::MouseUp(BPoint point) {
    SolveSpace::Platform::MouseEvent event = {};
    event.type = SolveSpace::Platform::MouseEvent::Type::RELEASE;
    event.button = GetMouseButton();
    event.x = point.x;
    event.y = point.y;

    SS.GW.MouseEvent(event);
    SS.GenerateAll(SolveSpaceUI::Generate::UNTIL_ACTIVE);
    Draw(Bounds());
}

void EditorView::ZoomToMouse(double zoomMultiplyer) {
    double offsetRight = camera.offset.Dot(camera.projRight);
    double offsetUp = camera.offset.Dot(camera.projUp);

    double righti = currentMousePosition.x / camera.scale - offsetRight;
    double upi = currentMousePosition.y / camera.scale - offsetUp;

    // zoomMultiplyer of 1 gives a default zoom factor of 1.2x: zoomMultiplyer
    // * 1.2 zoom = adjusted zoom negative zoomMultiplyer will zoom out,
    // positive will zoom in
    camera.scale *= exp(0.1823216 * zoomMultiplyer); // ln(1.2) = 0.1823216

    double rightf = currentMousePosition.x / camera.scale - offsetRight;
    double upf = currentMousePosition.y / camera.scale - offsetUp;

    camera.offset = camera.offset.Plus(projRight.ScaledBy(rightf - righti));
    camera.offset = camera.offset.Plus(projUp.ScaledBy(upf - upi));

    SS.GW.canvas.get()->SetCamera(camera);
}

void EditorView::ZoomToFit(bool includingInvisibles, bool useSelection) {
    SS.GW.ZoomToFit(camera, false,
                    true); // includingInvisibles = false, useSelection = true

    camera.offset = SS.GW.offset;
    camera.scale = SS.GW.scale;

    SS.GW.canvas.get()->SetCamera(camera);
}

void EditorView::SyncCamera() {
    camera = SS.GW.GetCamera();
    SS.GW.canvas.get()->SetCamera(camera);
}