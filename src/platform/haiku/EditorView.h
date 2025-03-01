/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#pragma once

#include <cstdio>

#include <Bitmap.h>
#include <View.h>

#include "agg_bspline.h"
#include "agg_conv_curve.h"
#include "agg_conv_transform.h"
#include "agg_ellipse.h"
#include "agg_gsv_text.h"
#include "agg_path_storage.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_rendering_buffer.h"
#include "agg_scanline_p.h"
#include "agg_trans_viewport.h"

#include <Message.h>
#include "solvespace.h"
#include "ssg.h"

class EditorView : public BView {
  public:
  EditorView ();
  ~EditorView ();
  //  void AttachedToWindow();
  //  void DetachedFromWindow();
  void Draw (BRect updateRect);
  void FrameResized (float width, float height);
  void SaveToPng ();
  void Load (std::string path);
  void New(float initialWidth, float initialHeight);

  void MouseDown (BPoint point);
  void MouseMoved (BPoint point, uint32 transit, const BMessage *message);
  void MouseUp (BPoint point);

  private:
  AggPixmapRenderer pixmapCanvas;
  Camera            camera;
  Lighting          lighting;
  BPoint            currentMousePosition;

  BBitmap              *retainedBitmap;
  agg::rendering_buffer buffer;

  // These parameters define the map from 2d screen coordinates to the
  // coordinates of the 3d sketch points. We will use an axonometric
  // projection.
  Vector offset;
  Vector projRight;
  Vector projUp;
  double scale;

  void                                     InitBitmapAndBuffer ();
  SolveSpace::Platform::MouseEvent::Button GetMouseButton ();
};
