#pragma once

#include "solvespace.h"

class VectorFileWriter {
  protected:
  Vector u, v, n, origin;
  double cameraTan, scale;

  public:
  FILE          *f;
  Platform::Path filename;
  Vector         ptMin, ptMax;

  static double MmToPts (double mm);

  static VectorFileWriter *ForFile (const Platform::Path &filename);

  void   SetModelviewProjection (const Vector &u, const Vector &v, const Vector &n,
                                 const Vector &origin, double cameraTan, double scale);
  Vector Transform (Vector &pos) const;

  void OutputLinesAndMesh (SBezierLoopSetSet *sblss, SMesh *sm);

  void BezierAsPwl (SBezier *sb);
  void BezierAsNonrationalCubic (SBezier *sb, int depth = 0);

  virtual void StartPath (RgbaColor strokeRgb, double lineWidth, bool filled, RgbaColor fillRgb,
                          hStyle hs)    = 0;
  virtual void FinishPath (RgbaColor strokeRgb, double lineWidth, bool filled, RgbaColor fillRgb,
                           hStyle hs)   = 0;
  virtual void Bezier (SBezier *sb)     = 0;
  virtual void Triangle (STriangle *tr) = 0;
  virtual bool OutputConstraints (IdList<Constraint, hConstraint> *) { return false; }
  virtual void Background (RgbaColor color) = 0;
  virtual void StartFile ()                 = 0;
  virtual void FinishAndCloseFile ()        = 0;
  virtual bool HasCanvasSize () const       = 0;
  virtual bool CanOutputMesh () const       = 0;
};