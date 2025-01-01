//-----------------------------------------------------------------------------
// Routines for HPGL output
//-----------------------------------------------------------------------------
#include "solvespace.h"
#include "ssg.h"
#include "filewriter/hpglfilewriter.h"

double HpglFileWriter::MmToHpglUnits(double mm) {
  return mm * 40;
}

void HpglFileWriter::StartFile() {
  fprintf(f, "IN;\r\n");
  fprintf(f, "SP1;\r\n");
}

void HpglFileWriter::Background(RgbaColor color) {}

void HpglFileWriter::StartPath(RgbaColor strokeRgb, double lineWidth, bool filled,
                               RgbaColor fillRgb, hStyle hs) {}
void HpglFileWriter::FinishPath(RgbaColor strokeRgb, double lineWidth, bool filled,
                                RgbaColor fillRgb, hStyle hs) {}

void HpglFileWriter::Triangle(STriangle *tr) {}

void HpglFileWriter::Bezier(SBezier *sb) {
  if (sb->deg == 1) {
    fprintf(f, "PU%d,%d;\r\n", (int)MmToHpglUnits(sb->ctrl[0].x),
            (int)MmToHpglUnits(sb->ctrl[0].y));
    fprintf(f, "PD%d,%d;\r\n", (int)MmToHpglUnits(sb->ctrl[1].x),
            (int)MmToHpglUnits(sb->ctrl[1].y));
  } else {
    BezierAsPwl(sb);
  }
}

void HpglFileWriter::FinishAndCloseFile() {
  fclose(f);
}
