/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include "EditorView.h"
#include "../src/render/render.h"
#include "Window.h"

#include <iostream>

void attachBufferToBBitmap(agg::rendering_buffer &buffer, BBitmap *bitmap) {
  uint8 *bits   = (uint8 *)bitmap->Bits();
  uint32 width  = bitmap->Bounds().IntegerWidth() + 1;
  uint32 height = bitmap->Bounds().IntegerHeight() + 1;
  int32  bpr    = bitmap->BytesPerRow();
  buffer.attach(bits, width, height, -bpr);
}

EditorView::EditorView()
    : BView(Bounds(), "SolveSpace Editor View", B_FOLLOW_ALL_SIDES,
            B_FRAME_EVENTS | B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE) {
  SS.GW.canvas         = std::make_shared<AggPixmapRenderer>();
  SS.GW.overrideCamera = false;

  InitBitmapAndBuffer();
  Invalidate();
}

EditorView::~EditorView() {}

void EditorView::InitBitmapAndBuffer() {
  retainedBitmap = new BBitmap(Bounds(), 0, B_RGBA32);
  if (retainedBitmap->IsValid()) {
    attachBufferToBBitmap(std::static_pointer_cast<AggPixmapRenderer>(SS.GW.canvas)->buffer,
                          retainedBitmap);
  } else {
    delete retainedBitmap;
    retainedBitmap = NULL;
  }
}

void EditorView::Draw(BRect updateRect) {
  SS.GW.canvas.get()->Clear();
  SS.GW.Paint();

  DrawBitmap(retainedBitmap, updateRect, updateRect);
}

void EditorView::FrameResized(float width, float height) {
  InitBitmapAndBuffer(); // do this before drawing
  camera.width  = width;
  camera.height = height;
  SS.GW.width   = width;
  SS.GW.height  = height;

  std::static_pointer_cast<AggPixmapRenderer>(SS.GW.canvas)
      ->SetCamera(camera); // do this before Init so it has the new values
  std::static_pointer_cast<AggPixmapRenderer>(SS.GW.canvas)
      ->Init(false); // this picks up the width and height set on SS.GW
                     // TODO: consider giving Init width and height params

  Draw(Bounds());
}

void EditorView::Load(std::string path) {
  const SolveSpace::Platform::Path fixturePath = SolveSpace::Platform::Path::From(path);

  if (!SS.LoadFromFile(fixturePath)) {
    return;
  }
}

void EditorView::New() {
  SS.GW.Init(Bounds().Width(), Bounds().Height(),
             1.0); // width, height, pixelDeviceRatio
  SS.GW.offset    = Vector(0, 0, 0);
  SS.GW.scale     = 5.0;
  SS.GW.projRight = Vector::From(1, 0, 0);
  SS.GW.projUp    = Vector::From(0, 1, 0);
  SS.gridSpacing  = 5.0; // TODO: get these from settings file?

  std::static_pointer_cast<AggPixmapRenderer>(SS.GW.canvas)->Init(false);

  FrameResized(camera.width, camera.height);
}

SolveSpace::Platform::MouseEvent::Button EditorView::GetMouseButton() {
  uint32 buttons;

  Window()->CurrentMessage()->FindInt32("buttons", (int32 *)&buttons);
  if (buttons & B_PRIMARY_MOUSE_BUTTON) {
    return SolveSpace::Platform::MouseEvent::Button::LEFT;
  }
  if (buttons & B_SECONDARY_MOUSE_BUTTON) {
    return SolveSpace::Platform::MouseEvent::Button::MIDDLE;
  }
  if (buttons & B_TERTIARY_MOUSE_BUTTON) {
    return SolveSpace::Platform::MouseEvent::Button::RIGHT;
  }

  return SolveSpace::Platform::MouseEvent::Button::LEFT;
}

void EditorView::MouseDown(BPoint point) {
  SolveSpace::Platform::MouseEvent event = {};
  event.type                             = SolveSpace::Platform::MouseEvent::Type::PRESS;
  event.button                           = GetMouseButton();
  event.x                                = point.x;
  event.y                                = point.y;

  SS.GW.MouseEvent(event);
  SS.GenerateAll(SolveSpaceUI::Generate::UNTIL_ACTIVE);
  Draw(Bounds());
}

void EditorView::MouseMoved(BPoint point, uint32 transit, const BMessage *message) {
  currentMousePosition = point;

  SolveSpace::Platform::MouseEvent event = {};
  event.type                             = SolveSpace::Platform::MouseEvent::Type::MOTION;
  event.button                           = GetMouseButton();
  event.x                                = point.x;
  event.y                                = point.y;

  SS.GW.MouseEvent(event);
  SS.GenerateAll(SolveSpaceUI::Generate::UNTIL_ACTIVE);
  Draw(Bounds());
}

void EditorView::MouseUp(BPoint point) {
  SolveSpace::Platform::MouseEvent event = {};
  event.type                             = SolveSpace::Platform::MouseEvent::Type::RELEASE;
  event.button                           = GetMouseButton();
  event.x                                = point.x;
  event.y                                = point.y;

  SS.GW.MouseEvent(event);
  SS.GenerateAll(SolveSpaceUI::Generate::UNTIL_ACTIVE);
  Draw(Bounds());
}
