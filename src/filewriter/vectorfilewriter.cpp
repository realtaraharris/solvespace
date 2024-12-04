#include "solvespace.h"
#include "gcodefilewriter.h"
#include "dxffilewriter.h"
#include "epsfilewriter.h"
#include "pdffilewriter.h"
#include "hpglfilewriter.h"
#include "vectorfilewriter.h"

double VectorFileWriter::MmToPts (double mm) {
  // 72 points in an inch
  return (mm / 25.4) * 72;
}

VectorFileWriter *VectorFileWriter::ForFile (const Platform::Path &filename) {
  VectorFileWriter *ret;
  /*    bool needOpen = true;
      if(filename.HasExtension("dxf")) {
          static DxfFileWriter DxfWriter;
          ret = &DxfWriter;
          needOpen = false;
      } else if(filename.HasExtension("ps") || filename.HasExtension("eps")) {
          static EpsFileWriter EpsWriter;
          ret = &EpsWriter;
      } else if(filename.HasExtension("pdf")) {
          static PdfFileWriter PdfWriter;
          ret = &PdfWriter;
      } else if(filename.HasExtension("svg")) {
          static SvgFileWriter SvgWriter;
          ret = &SvgWriter;
      } else if(filename.HasExtension("plt") || filename.HasExtension("hpgl")) {
          static HpglFileWriter HpglWriter;
          ret = &HpglWriter;
      } else if(filename.HasExtension("step") || filename.HasExtension("stp")) {
          static Step2dFileWriter Step2dWriter;
          ret = &Step2dWriter;
      } else if(filename.HasExtension("txt") || filename.HasExtension("ngc")) {
          static GCodeFileWriter GCodeWriter;
          ret = &GCodeWriter;
      } else {
          Error("Can't identify output file type from file extension of "
          "filename '%s'; try "
          ".step, .stp, .dxf, .svg, .plt, .hpgl, .pdf, .txt, .ngc, "
          ".eps, or .ps.",
              filename.raw.c_str());
          return NULL;
      }
      ret->filename = filename;
      if(!needOpen) return ret;

      FILE *f = OpenFile(filename, "wb");
      if(!f) {
          Error("Couldn't write to '%s'", filename.raw.c_str());
          return NULL;
      }
      ret->f = f; */
  return ret;
}

void VectorFileWriter::SetModelviewProjection (const Vector &u, const Vector &v, const Vector &n,
                                               const Vector &origin, double cameraTan,
                                               double scale) {
  this->u         = u;
  this->v         = v;
  this->n         = n;
  this->origin    = origin;
  this->cameraTan = cameraTan;
  this->scale     = scale;
}

Vector VectorFileWriter::Transform (Vector &pos) const {
  return pos.InPerspective (u, v, n, origin, cameraTan).ScaledBy (1.0 / scale);
}

void VectorFileWriter::OutputLinesAndMesh (SBezierLoopSetSet *sblss, SMesh *sm) {
  STriangle *tr;
  SBezier   *b;

  // First calculate the bounding box.
  ptMin = Vector::From (VERY_POSITIVE, VERY_POSITIVE, VERY_POSITIVE);
  ptMax = Vector::From (VERY_NEGATIVE, VERY_NEGATIVE, VERY_NEGATIVE);
  if (sm) {
    for (tr = sm->l.First (); tr; tr = sm->l.NextAfter (tr)) {
      (tr->a).MakeMaxMin (ptMax, ptMin);
      (tr->b).MakeMaxMin (ptMax, ptMin);
      (tr->c).MakeMaxMin (ptMax, ptMin);
    }
  }
  if (sblss) {
    SBezierLoopSet *sbls;
    for (sbls = sblss->l.First (); sbls; sbls = sblss->l.NextAfter (sbls)) {
      SBezierLoop *sbl;
      for (sbl = sbls->l.First (); sbl; sbl = sbls->l.NextAfter (sbl)) {
        for (b = sbl->l.First (); b; b = sbl->l.NextAfter (b)) {
          for (int i = 0; i <= b->deg; i++) {
            (b->ctrl[i]).MakeMaxMin (ptMax, ptMin);
          }
        }
      }
    }
  }

  // And now we compute the canvas size.
  double s = 1.0 / SS.exportScale;
  if (SS.exportCanvasSizeAuto) {
    // It's based on the calculated bounding box; we grow it along each
    // boundary by the specified amount.
    ptMin.x -= s * SS.exportMargin.left;
    ptMax.x += s * SS.exportMargin.right;
    ptMin.y -= s * SS.exportMargin.bottom;
    ptMax.y += s * SS.exportMargin.top;
  } else {
    ptMin.x = (s * SS.exportCanvas.dx);
    ptMin.y = (s * SS.exportCanvas.dy);
    ptMax.x = ptMin.x + (s * SS.exportCanvas.width);
    ptMax.y = ptMin.y + (s * SS.exportCanvas.height);
  }

  StartFile ();
  if (SS.exportBackgroundColor) {
    Background (SS.backgroundColor);
  }
  if (sm && SS.exportShadedTriangles) {
    for (tr = sm->l.First (); tr; tr = sm->l.NextAfter (tr)) {
      Triangle (tr);
    }
  }
  if (sblss) {
    SBezierLoopSet *sbls;
    for (sbls = sblss->l.First (); sbls; sbls = sblss->l.NextAfter (sbls)) {
      for (SBezierLoop *sbl = sbls->l.First (); sbl; sbl = sbls->l.NextAfter (sbl)) {
        b = sbl->l.First ();
        if (!b || !Style::Exportable (b->auxA))
          continue;

        hStyle    hs        = {(uint32_t)b->auxA};
        Style    *stl       = Style::Get (hs);
        double    lineWidth = Style::WidthMm (b->auxA) * s;
        RgbaColor strokeRgb = Style::Color (hs, /*forExport=*/true);
        RgbaColor fillRgb   = Style::FillColor (hs, /*forExport=*/true);

        StartPath (strokeRgb, lineWidth, stl->filled, fillRgb, hs);
        for (b = sbl->l.First (); b; b = sbl->l.NextAfter (b)) {
          Bezier (b);
        }
        FinishPath (strokeRgb, lineWidth, stl->filled, fillRgb, hs);
      }
    }
  }
  FinishAndCloseFile ();
}

void VectorFileWriter::BezierAsPwl (SBezier *sb) {
  List<Vector> lv = {};
  sb->MakePwlInto (&lv, SS.ExportChordTolMm ());

  for (int i = 1; i < lv.n; i++) {
    SBezier sb = SBezier::From (lv[i - 1], lv[i]);
    Bezier (&sb);
  }
  lv.Clear ();
}

void VectorFileWriter::BezierAsNonrationalCubic (SBezier *sb, int depth) {
  Vector t0 = sb->TangentAt (0), t1 = sb->TangentAt (1);
  // The curve is correct, and the first derivatives are correct, at the
  // endpoints.
  SBezier bnr = SBezier::From (sb->Start (), sb->Start ().Plus (t0.ScaledBy (1.0 / 3)),
                               sb->Finish ().Minus (t1.ScaledBy (1.0 / 3)), sb->Finish ());

  double tol = SS.ExportChordTolMm ();
  // Arbitrary choice, but make it a little finer than pwl tolerance since
  // it should be easier to achieve that with the smooth curves.
  tol /= 2;

  bool closeEnough = true;
  int  i;
  for (i = 1; i <= 3; i++) {
    double t  = i / 4.0;
    Vector p0 = sb->PointAt (t), pn = bnr.PointAt (t);
    double d = (p0.Minus (pn)).Magnitude ();
    if (d > tol) {
      closeEnough = false;
    }
  }

  if (closeEnough || depth > 3) {
    Bezier (&bnr);
  } else {
    SBezier bef, aft;
    sb->SplitAt (0.5, &bef, &aft);
    BezierAsNonrationalCubic (&bef, depth + 1);
    BezierAsNonrationalCubic (&aft, depth + 1);
  }
}