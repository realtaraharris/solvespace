//-----------------------------------------------------------------------------
// Routines for SVG output
//-----------------------------------------------------------------------------

#include "solvespace.h"
#include "svgfilewriter.h"
#include "stipplepattern.h"

void SvgFileWriter::StartFile () {
  fprintf (f,
           "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" "
           "\"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">\r\n"
           "<svg xmlns=\"http://www.w3.org/2000/svg\"  "
           "xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
           "width='%.3fmm' height='%.3fmm' "
           "viewBox=\"0 0 %.3f %.3f\">\r\n"
           "\r\n"
           "<title>Exported SVG</title>\r\n"
           "\r\n",
           (ptMax.x - ptMin.x), (ptMax.y - ptMin.y), (ptMax.x - ptMin.x), (ptMax.y - ptMin.y));

  fprintf (f, "<style><![CDATA[\r\n");
  fprintf (f, "polygon {\r\n");
  fprintf (f, "shape-rendering:crispEdges;\r\n");
  // crispEdges turns of anti-aliasing, which tends to cause hairline
  // cracks between triangles; but there still is some cracking, so
  // specify a stroke width too, hope for around a pixel
  double sw = std::max (ptMax.x - ptMin.x, ptMax.y - ptMin.y) / 1000;
  fprintf (f, "stroke-width:%f;\r\n", sw);
  fprintf (f, "}\r\n");

  auto export_style = [&] (hStyle hs) {
    Style         *s            = Style::Get (hs);
    RgbaColor      strokeRgb    = Style::Color (hs, /*forExport=*/true);
    RgbaColor      fillRgb      = Style::FillColor (hs, /*forExport=*/true);
    StipplePattern pattern      = Style::PatternType (hs);
    double         stippleScale = Style::StippleScaleMm (hs);

    fprintf (f, ".s%x {\r\n", hs.v);
    fprintf (f, "stroke:#%02x%02x%02x;\r\n", strokeRgb.red, strokeRgb.green, strokeRgb.blue);
    // don't know why we have to take a half of the width
    fprintf (f, "stroke-width:%f;\r\n", Style::WidthMm (hs.v) / 2.0);
    fprintf (f, "stroke-linecap:round;\r\n");
    fprintf (f, "stroke-linejoin:round;\r\n");
    std::string patternStr = MakeStipplePattern (pattern, stippleScale, ',',
                                                 /*inkscapeWorkaround=*/true);
    if (!patternStr.empty ()) {
      fprintf (f, "stroke-dasharray:%s;\r\n", patternStr.c_str ());
    }
    if (s->filled) {
      fprintf (f, "fill:#%02x%02x%02x;\r\n", fillRgb.red, fillRgb.green, fillRgb.blue);
    } else {
      fprintf (f, "fill:none;\r\n");
    }
    fprintf (f, "}\r\n");
  };

  export_style ({Style::NO_STYLE});
  for (auto &style : SK.style) {
    Style *s = &style;
    export_style (s->h);
  }
  fprintf (f, "]]></style>\r\n");
}

void SvgFileWriter::Background (RgbaColor color) {
  fprintf (f,
           "<style><![CDATA[\r\n"
           "svg {\r\n"
           "background-color:#%02x%02x%02x;\r\n"
           "}\r\n"
           "]]></style>\r\n",
           color.red, color.green, color.blue);
}

void SvgFileWriter::StartPath (RgbaColor strokeRgb, double lineWidth, bool filled,
                               RgbaColor fillRgb, hStyle hs) {
  fprintf (f, "<path d='");
  prevPt = Vector::From (VERY_POSITIVE, VERY_POSITIVE, VERY_POSITIVE);
}
void SvgFileWriter::FinishPath (RgbaColor strokeRgb, double lineWidth, bool filled,
                                RgbaColor fillRgb, hStyle hs) {
  std::string fill;
  if (filled) {
    fill = ssprintf ("fill='#%02x%02x%02x'", fillRgb.red, fillRgb.green, fillRgb.blue);
  }
  std::string cls = ssprintf ("s%x", hs.v);
  fprintf (f, "' class='%s' %s/>\r\n", cls.c_str (), fill.c_str ());
}

void SvgFileWriter::MaybeMoveTo (Vector st, Vector fi) {
  // SVG uses a coordinate system with the origin at top left, +y down
  if (!prevPt.Equals (st)) {
    fprintf (f, "M%.3f %.3f ", (st.x - ptMin.x), (ptMax.y - st.y));
  }
  prevPt = fi;
}

void SvgFileWriter::Triangle (STriangle *tr) {
  fprintf (f,
           "<polygon points='%.3f,%.3f %.3f,%.3f %.3f,%.3f' "
           "stroke='#%02x%02x%02x' "
           "fill='#%02x%02x%02x'/>\r\n",
           (tr->a.x - ptMin.x), (ptMax.y - tr->a.y), (tr->b.x - ptMin.x), (ptMax.y - tr->b.y),
           (tr->c.x - ptMin.x), (ptMax.y - tr->c.y), tr->meta.color.red, tr->meta.color.green,
           tr->meta.color.blue, tr->meta.color.red, tr->meta.color.green, tr->meta.color.blue);
}

void SvgFileWriter::Bezier (SBezier *sb) {
  Vector c, n = Vector::From (0, 0, 1);
  double r;
  if (sb->deg == 1) {
    MaybeMoveTo (sb->ctrl[0], sb->ctrl[1]);
    fprintf (f, "L%.3f,%.3f ", (sb->ctrl[1].x - ptMin.x), (ptMax.y - sb->ctrl[1].y));
  } else if (sb->IsCircle (n, &c, &r)) {
    Vector p0 = sb->ctrl[0], p1 = sb->ctrl[2];
    double theta0 = atan2 (p0.y - c.y, p0.x - c.x), theta1 = atan2 (p1.y - c.y, p1.x - c.x),
           dtheta = WRAP_SYMMETRIC (theta1 - theta0, 2 * PI);
    // The arc must be less than 180 degrees, or else it couldn't have
    // been represented as a single rational Bezier. So large-arc-flag
    // must be false. sweep-flag is determined by the sign of dtheta.
    // Note that clockwise and counter-clockwise are backwards in SVG's
    // mirrored csys.
    MaybeMoveTo (p0, p1);
    fprintf (f, "A%.3f,%.3f 0 0,%d %.3f,%.3f ", r, r, (dtheta < 0) ? 1 : 0, p1.x - ptMin.x,
             ptMax.y - p1.y);
  } else if (!sb->IsRational ()) {
    if (sb->deg == 2) {
      MaybeMoveTo (sb->ctrl[0], sb->ctrl[2]);
      fprintf (f, "Q%.3f,%.3f %.3f,%.3f ", sb->ctrl[1].x - ptMin.x, ptMax.y - sb->ctrl[1].y,
               sb->ctrl[2].x - ptMin.x, ptMax.y - sb->ctrl[2].y);
    } else if (sb->deg == 3) {
      MaybeMoveTo (sb->ctrl[0], sb->ctrl[3]);
      fprintf (f, "C%.3f,%.3f %.3f,%.3f %.3f,%.3f ", sb->ctrl[1].x - ptMin.x,
               ptMax.y - sb->ctrl[1].y, sb->ctrl[2].x - ptMin.x, ptMax.y - sb->ctrl[2].y,
               sb->ctrl[3].x - ptMin.x, ptMax.y - sb->ctrl[3].y);
    }
  } else {
    BezierAsNonrationalCubic (sb);
  }
}

void SvgFileWriter::FinishAndCloseFile () {
  fprintf (f, "\r\n</svg>\r\n");
  fclose (f);
}
