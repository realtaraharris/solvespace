//-----------------------------------------------------------------------------
// Routine for STEP output; just a wrapper around the general STEP stuff that
// can also be used for surfaces or 3d curves.
//-----------------------------------------------------------------------------

#include "solvespace.h"
#include "step2dfilewriter.h"

void Step2dFileWriter::StartFile () {
  sfw   = {};
  sfw.f = f;
  sfw.WriteHeader ();
}

void Step2dFileWriter::Background (RgbaColor color) {}

void Step2dFileWriter::Triangle (STriangle *tr) {}

void Step2dFileWriter::StartPath (RgbaColor strokeRgb, double lineWidth, bool filled,
                                  RgbaColor fillRgb, hStyle hs) {}
void Step2dFileWriter::FinishPath (RgbaColor strokeRgb, double lineWidth, bool filled,
                                   RgbaColor fillRgb, hStyle hs) {}

void Step2dFileWriter::Bezier (SBezier *sb) {
  int c = sfw.ExportCurve (sb);
  sfw.curves.Add (&c);
}

void Step2dFileWriter::FinishAndCloseFile () {
  sfw.WriteWireframe ();
  sfw.WriteFooter ();
  fclose (f);
}
