//-----------------------------------------------------------------------------
// Routines for EPS output
//-----------------------------------------------------------------------------

#include "solvespace.h"
#include "filewriter/epsfilewriter.h"
#include "stipplepattern.h"

void EpsFileWriter::StartFile () {
  fprintf (f,
           "%%!PS-Adobe-2.0\r\n"
           "%%%%Creator: SolveSpace\r\n"
           "%%%%Title: title\r\n"
           "%%%%Pages: 0\r\n"
           "%%%%PageOrder: Ascend\r\n"
           "%%%%BoundingBox: 0 0 %d %d\r\n"
           "%%%%HiResBoundingBox: 0 0 %.3f %.3f\r\n"
           "%%%%EndComments\r\n"
           "\r\n"
           "gsave\r\n"
           "\r\n",
           (int)ceil (MmToPts (ptMax.x - ptMin.x)), (int)ceil (MmToPts (ptMax.y - ptMin.y)),
           MmToPts (ptMax.x - ptMin.x), MmToPts (ptMax.y - ptMin.y));
}

void EpsFileWriter::Background (RgbaColor color) {
  double width  = ptMax.x - ptMin.x;
  double height = ptMax.y - ptMin.y;

  fprintf (f,
           "%.3f %.3f %.3f setrgbcolor\r\n"
           "newpath\r\n"
           "    %.3f %.3f moveto\r\n"
           "    %.3f %.3f lineto\r\n"
           "    %.3f %.3f lineto\r\n"
           "    %.3f %.3f lineto\r\n"
           "    closepath\r\n"
           "gsave fill grestore\r\n",
           color.redF (), color.greenF (), color.blueF (), MmToPts (0), MmToPts (0),
           MmToPts (width), MmToPts (0), MmToPts (width), MmToPts (height), MmToPts (0),
           MmToPts (height));

  // same issue with cracks, stroke it to avoid them
  double sw = std::max (width, height) / 1000;
  fprintf (f,
           "1 setlinejoin\r\n"
           "1 setlinecap\r\n"
           "%.3f setlinewidth\r\n"
           "gsave stroke grestore\r\n",
           MmToPts (sw));
}

void EpsFileWriter::StartPath (RgbaColor strokeRgb, double lineWidth, bool filled,
                               RgbaColor fillRgb, hStyle hs) {
  fprintf (f, "newpath\r\n");
  prevPt = Vector::From (VERY_POSITIVE, VERY_POSITIVE, VERY_POSITIVE);
}
void EpsFileWriter::FinishPath (RgbaColor strokeRgb, double lineWidth, bool filled,
                                RgbaColor fillRgb, hStyle hs) {
  StipplePattern pattern      = Style::PatternType (hs);
  double         stippleScale = MmToPts (Style::StippleScaleMm (hs));

  fprintf (f,
           "    %.3f setlinewidth\r\n"
           "    %.3f %.3f %.3f setrgbcolor\r\n"
           "    1 setlinejoin\r\n" // rounded
           "    1 setlinecap\r\n"  // rounded
           "    [%s] 0 setdash\r\n"
           "    gsave stroke grestore\r\n",
           MmToPts (lineWidth), strokeRgb.redF (), strokeRgb.greenF (), strokeRgb.blueF (),
           MakeStipplePattern (pattern, stippleScale, ' ').c_str ());
  if (filled) {
    fprintf (f,
             "    %.3f %.3f %.3f setrgbcolor\r\n"
             "    gsave fill grestore\r\n",
             fillRgb.redF (), fillRgb.greenF (), fillRgb.blueF ());
  }
}

void EpsFileWriter::MaybeMoveTo (Vector st, Vector fi) {
  if (!prevPt.Equals (st)) {
    fprintf (f, "    %.3f %.3f moveto\r\n", MmToPts (st.x - ptMin.x), MmToPts (st.y - ptMin.y));
  }
  prevPt = fi;
}

void EpsFileWriter::Triangle (STriangle *tr) {
  fprintf (f,
           "%.3f %.3f %.3f setrgbcolor\r\n"
           "newpath\r\n"
           "    %.3f %.3f moveto\r\n"
           "    %.3f %.3f lineto\r\n"
           "    %.3f %.3f lineto\r\n"
           "    closepath\r\n"
           "gsave fill grestore\r\n",
           tr->meta.color.redF (), tr->meta.color.greenF (), tr->meta.color.blueF (),
           MmToPts (tr->a.x - ptMin.x), MmToPts (tr->a.y - ptMin.y), MmToPts (tr->b.x - ptMin.x),
           MmToPts (tr->b.y - ptMin.y), MmToPts (tr->c.x - ptMin.x), MmToPts (tr->c.y - ptMin.y));

  // same issue with cracks, stroke it to avoid them
  double sw = std::max (ptMax.x - ptMin.x, ptMax.y - ptMin.y) / 1000;
  fprintf (f,
           "1 setlinejoin\r\n"
           "1 setlinecap\r\n"
           "%.3f setlinewidth\r\n"
           "gsave stroke grestore\r\n",
           MmToPts (sw));
}

void EpsFileWriter::Bezier (SBezier *sb) {
  Vector c, n = Vector::From (0, 0, 1);
  double r;
  if (sb->deg == 1) {
    MaybeMoveTo (sb->ctrl[0], sb->ctrl[1]);
    fprintf (f, "    %.3f %.3f lineto\r\n", MmToPts (sb->ctrl[1].x - ptMin.x),
             MmToPts (sb->ctrl[1].y - ptMin.y));
  } else if (sb->IsCircle (n, &c, &r)) {
    Vector p0 = sb->ctrl[0], p1 = sb->ctrl[2];
    double theta0 = atan2 (p0.y - c.y, p0.x - c.x), theta1 = atan2 (p1.y - c.y, p1.x - c.x),
           dtheta = WRAP_SYMMETRIC (theta1 - theta0, 2 * PI);
    MaybeMoveTo (p0, p1);
    fprintf (f, "    %.3f %.3f %.3f %.3f %.3f %s\r\n", MmToPts (c.x - ptMin.x),
             MmToPts (c.y - ptMin.y), MmToPts (r), theta0 * 180 / PI, theta1 * 180 / PI,
             dtheta < 0 ? "arcn" : "arc");
  } else if (sb->deg == 3 && !sb->IsRational ()) {
    MaybeMoveTo (sb->ctrl[0], sb->ctrl[3]);
    fprintf (f, "    %.3f %.3f %.3f %.3f %.3f %.3f curveto\r\n", MmToPts (sb->ctrl[1].x - ptMin.x),
             MmToPts (sb->ctrl[1].y - ptMin.y), MmToPts (sb->ctrl[2].x - ptMin.x),
             MmToPts (sb->ctrl[2].y - ptMin.y), MmToPts (sb->ctrl[3].x - ptMin.x),
             MmToPts (sb->ctrl[3].y - ptMin.y));
  } else {
    BezierAsNonrationalCubic (sb);
  }
}

void EpsFileWriter::FinishAndCloseFile () {
  fprintf (f, "\r\n"
              "grestore\r\n"
              "\r\n");
  fclose (f);
}
