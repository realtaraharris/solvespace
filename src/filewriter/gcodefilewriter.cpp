//-----------------------------------------------------------------------------
// Routines for G Code output. Slightly complicated by our ability to generate
// multiple passes, and to specify the feeds and depth; those parameters get
// set in the configuration screen.
//-----------------------------------------------------------------------------

#include "solvespace.h"
#include "ssg.h"
#include "filewriter/gcodefilewriter.h"

void GCodeFileWriter::StartFile() {
  sel = {};
}
void GCodeFileWriter::StartPath(RgbaColor strokeRgb, double lineWidth, bool filled,
                                RgbaColor fillRgb, hStyle hs) {}
void GCodeFileWriter::Background(RgbaColor color) {}
void GCodeFileWriter::FinishPath(RgbaColor strokeRgb, double lineWidth, bool filled,
                                 RgbaColor fillRgb, hStyle hs) {}
void GCodeFileWriter::Triangle(STriangle *tr) {}

void GCodeFileWriter::Bezier(SBezier *sb) {
  if (sb->deg == 1) {
    sel.AddEdge(sb->ctrl[0], sb->ctrl[1]);
  } else {
    BezierAsPwl(sb);
  }
}

void GCodeFileWriter::FinishAndCloseFile() {
  SPolygon sp = {};
  sel.AssemblePolygon(&sp, NULL);

  int i;
  for (i = 0; i < SS.gCode.passes; i++) {
    double depth = (SS.gCode.depth / SS.gCode.passes) * (i + 1);

    SContour *sc;
    for (sc = sp.l.First(); sc; sc = sp.l.NextAfter(sc)) {
      if (sc->l.n < 2)
        continue;

      SPoint *pt = sc->l.First();
      fprintf(f, "G00 X%s Y%s\r\n", SS.MmToString(pt->p.x).c_str(), SS.MmToString(pt->p.y).c_str());
      fprintf(f, "G01 Z%s F%s\r\n", SS.MmToString(depth).c_str(),
              SS.MmToString(SS.gCode.plungeFeed).c_str());

      pt = sc->l.NextAfter(pt);
      for (; pt; pt = sc->l.NextAfter(pt)) {
        fprintf(f, "G01 X%s Y%s F%s\r\n", SS.MmToString(pt->p.x).c_str(),
                SS.MmToString(pt->p.y).c_str(), SS.MmToString(SS.gCode.feed).c_str());
      }
      // Move up to a clearance plane above the work.
      fprintf(f, "G00 Z%s\r\n", SS.MmToString(SS.gCode.safeHeight).c_str());
    }
  }

  sp.Clear();
  sel.Clear();
  fclose(f);
}
