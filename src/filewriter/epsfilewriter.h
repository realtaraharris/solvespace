#pragma once

#include "vectorfilewriter.h"

class EpsFileWriter : public VectorFileWriter {
  public:
  Vector prevPt;
  void   MaybeMoveTo (Vector s, Vector f);

  void StartPath (RgbaColor strokeRgb, double lineWidth, bool filled, RgbaColor fillRgb,
                  hStyle hs) override;
  void FinishPath (RgbaColor strokeRgb, double lineWidth, bool filled, RgbaColor fillRgb,
                   hStyle hs) override;
  void Triangle (STriangle *tr) override;
  void Bezier (SBezier *sb) override;
  void Background (RgbaColor color) override;
  void StartFile () override;
  void FinishAndCloseFile () override;
  bool HasCanvasSize () const override { return true; }
  bool CanOutputMesh () const override { return true; }
};