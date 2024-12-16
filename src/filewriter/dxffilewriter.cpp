//-----------------------------------------------------------------------------
// Routines for DXF export
//-----------------------------------------------------------------------------

#include "solvespace.h"
#include "filewriter/dxffilewriter.h"
#include "libdxfrw.h"

class DxfWriteInterface : public DRW_Interface {
  public:
  DxfFileWriter *writer;
  dxfRW         *dxf;

  std::set<std::string> messages;

  static DRW_Coord toCoord(const Vector &v) { return DRW_Coord(v.x, v.y, v.z); }

  Vector xfrm(Vector v) { return writer->Transform(v); }

  void writeTextstyles() override {
    DRW_Textstyle ts;
    ts.name = "unicode";
    ts.font = "unicode";
    dxf->writeTextstyle(&ts);
  }

  void writeLayers() override {
    DRW_Layer layer;

    layer.name = "dimensions";
    dxf->writeLayer(&layer);
    layer.name = "text";
    dxf->writeLayer(&layer);

    std::set<uint32_t> usedStyles;

    for (DxfFileWriter::BezierPath &path : writer->paths) {
      for (SBezier *sb : path.beziers) {
        usedStyles.insert((uint32_t)sb->auxA);
      }
    }

    for (uint32_t v : usedStyles) {
      Style *s   = Style::Get(hStyle{v});
      layer.name = s->DescriptionString();
      dxf->writeLayer(&layer);
    }
  }

  void writeLTypes() override {
    for (uint32_t i = 0; i <= (uint32_t)StipplePattern::LAST; i++) {
      StipplePattern st = (StipplePattern)i;
      DRW_LType      type;
      // LibreCAD requires the line type to have one of these exact names,
      // or otherwise it overwrites it with its own (continuous) style.
      type.name = DxfFileWriter::lineTypeName(st);
      double sw = 1.0;
      switch (st) {
      case StipplePattern::CONTINUOUS: break;

      case StipplePattern::SHORT_DASH:
        type.path.push_back(sw);
        type.path.push_back(-sw * 2.0);
        break;

      case StipplePattern::DASH:
        type.path.push_back(sw);
        type.path.push_back(-sw);
        break;

      case StipplePattern::LONG_DASH:
        type.path.push_back(sw * 2.0);
        type.path.push_back(-sw);
        break;

      case StipplePattern::DASH_DOT:
        type.path.push_back(sw);
        type.path.push_back(-sw);
        type.path.push_back(0.0);
        type.path.push_back(-sw);
        break;

      case StipplePattern::DOT:
        type.path.push_back(sw);
        type.path.push_back(0.0);
        break;

      case StipplePattern::DASH_DOT_DOT:
        type.path.push_back(sw);
        type.path.push_back(-sw);
        type.path.push_back(0.0);
        type.path.push_back(-sw);
        type.path.push_back(0.0);
        type.path.push_back(-sw);
        break;

      case StipplePattern::FREEHAND:
      case StipplePattern::ZIGZAG:
        // Not implemented; exported as continuous.
        break;
      }
      dxf->writeLineType(&type);
    }
  }

  void writePolylines() {
    PolylineBuilder builder;

    for (DxfFileWriter::BezierPath &path : writer->paths) {
      for (SBezier *sb : path.beziers) {
        if (sb->deg != 1)
          continue;
        builder.AddEdge(sb->ctrl[0], sb->ctrl[1], (uint32_t)sb->auxA);
      }
    }

    DRW_Polyline polyline;

    auto startFunc = [&](PolylineBuilder::Vertex *start, PolylineBuilder::Vertex *next,
                         PolylineBuilder::Edge *e) {
      hStyle hs = {e->kind};
      polyline  = {};
      assignEntityDefaults(&polyline, hs);

      if (!(EXACT(start->pos.z == 0.0) && EXACT(next->pos.z == 0.0))) {
        polyline.flags |= 8 /* 3d polyline */;
      }
      polyline.vertlist.push_back(new DRW_Vertex(start->pos.x, start->pos.y, start->pos.z, 0.0));
      polyline.vertlist.push_back(new DRW_Vertex(next->pos.x, next->pos.y, next->pos.z, 0.0));
    };

    auto nextFunc = [&](PolylineBuilder::Vertex *next, PolylineBuilder::Edge *e) {
      if (!EXACT(next->pos.z == 0.0)) {
        polyline.flags |= 8 /* 3d polyline */;
      }
      polyline.vertlist.push_back(new DRW_Vertex(next->pos.x, next->pos.y, next->pos.z, 0.0));
    };

    auto endFunc = [&]() { dxf->writePolyline(&polyline); };

    auto aloneFunc = [&](PolylineBuilder::Edge *e) {
      hStyle hs = {e->kind};
      writeLine(e->a->pos, e->b->pos, hs);
    };

    builder.Generate(startFunc, nextFunc, aloneFunc, endFunc);
  }

  void writeEntities() override {
    writePolylines();

    for (DxfFileWriter::BezierPath &path : writer->paths) {
      for (SBezier *sb : path.beziers) {
        if (sb->deg == 1)
          continue;
        writeBezier(sb);
      }
    }

    if (writer->constraint) {
      for (Constraint &c : *writer->constraint) {
        if (!writer->NeedToOutput(&c))
          continue;
        switch (c.type) {
        case Constraint::Type::PT_PT_DISTANCE: {
          Vector ap  = SK.GetEntity(c.ptA)->PointGetNum();
          Vector bp  = SK.GetEntity(c.ptB)->PointGetNum();
          Vector ref = ((ap.Plus(bp)).ScaledBy(0.5)).Plus(c.disp.offset);
          writeAlignedDimension(xfrm(ap), xfrm(bp), xfrm(ref), xfrm(ref), c.Label(), c.GetStyle(),
                                c.valA);
          break;
        }

        case Constraint::Type::PT_LINE_DISTANCE: {
          Vector  pt   = SK.GetEntity(c.ptA)->PointGetNum();
          Entity *line = SK.GetEntity(c.entityA);
          Vector  lA   = SK.GetEntity(line->point[0])->PointGetNum();
          Vector  lB   = SK.GetEntity(line->point[1])->PointGetNum();
          Vector  dl   = lB.Minus(lA);

          Vector closest = pt.ClosestPointOnLine(lA, dl);

          if (pt.Equals(closest))
            break;

          Vector ref        = ((closest.Plus(pt)).ScaledBy(0.5)).Plus(c.disp.offset);
          Vector refClosest = ref.ClosestPointOnLine(lA, dl);

          double ddl = dl.Dot(dl);
          if (std::fabs(ddl) > LENGTH_EPS * LENGTH_EPS) {
            double t = refClosest.Minus(lA).Dot(dl) / ddl;
            if (t < 0.0) {
              refClosest = lA;
            } else if (t > 1.0) {
              refClosest = lB;
            }
          }

          Vector xdl = xfrm(lB).Minus(xfrm(lA));
          writeLinearDimension(xfrm(pt), xfrm(refClosest), xfrm(ref), xfrm(ref), c.Label(),
                               atan2(xdl.y, xdl.x) / PI * 180.0 + 90.0, 0.0, c.GetStyle(), c.valA);
          break;
        }

        case Constraint::Type::DIAMETER: {
          Entity    *circle = SK.GetEntity(c.entityA);
          Vector     center = SK.GetEntity(circle->point[0])->PointGetNum();
          Quaternion q      = SK.GetEntity(circle->normal)->NormalGetNum();
          Vector     n      = q.RotationN().WithMagnitude(1);
          double     r      = circle->CircleGetRadiusNum();

          Vector ref = center.Plus(c.disp.offset);
          // Force the label into the same plane as the circle.
          ref = ref.Minus(n.ScaledBy(n.Dot(ref) - n.Dot(center)));

          Vector rad = ref.Minus(center).WithMagnitude(r);
          if (/*isRadius*/ c.other) {
            writeRadialDimension(xfrm(center), xfrm(center.Plus(rad)), xfrm(ref), c.Label(),
                                 c.GetStyle(), c.valA);
          } else {
            writeDiametricDimension(xfrm(center.Minus(rad)), xfrm(center.Plus(rad)), xfrm(ref),
                                    c.Label(), c.GetStyle(), c.valA);
          }
          break;
        }

        case Constraint::Type::ANGLE: {
          Entity *a = SK.GetEntity(c.entityA);
          Entity *b = SK.GetEntity(c.entityB);

          Vector a0 = a->VectorGetStartPoint();
          Vector b0 = b->VectorGetStartPoint();
          Vector da = a->VectorGetNum();
          Vector db = b->VectorGetNum();
          if (/*otherAngle*/ c.other) {
            a0 = a0.Plus(da);
            da = da.ScaledBy(-1);
          }

          Vector                          ref = c.disp.offset;
          VectorAtIntersectionOfLines_ret eeep =
              VectorAtIntersectionOfLines(a0, a0.Plus(da), b0, b0.Plus(db), false);
          Vector pi = eeep.intersectionPoint;
          if (!eeep.skewed)
            ref = pi.Plus(c.disp.offset);

          Vector norm = da.Cross(db);
          Vector dna  = norm.Cross(da).WithMagnitude(1.0);

          double thetaf = acos(da.DirectionCosineWith(db));

          // Calculate median
          Vector m =
              da.WithMagnitude(1.0).ScaledBy(cos(thetaf / 2)).Plus(dna.ScaledBy(sin(thetaf / 2)));
          Vector rm = ref.Minus(pi);

          // Test which side we have to place an arc
          if (m.Dot(rm) < 0) {
            da  = da.ScaledBy(-1);
            dna = dna.ScaledBy(-1);
            db  = db.ScaledBy(-1);
          }

          Vector bisect = da.WithMagnitude(1.0)
                              .ScaledBy(cos(thetaf / 2.0))
                              .Plus(dna.ScaledBy(sin(thetaf / 2.0)));

          ref = pi.Plus(bisect.WithMagnitude(c.disp.offset.Magnitude()));

          // Get lines again to write exact line.
          a0 = a->VectorGetStartPoint();
          b0 = b->VectorGetStartPoint();
          da = a->VectorGetNum();
          db = b->VectorGetNum();

          writeAngularDimension(xfrm(a0), xfrm(a0.Plus(da)), xfrm(b0), xfrm(b0.Plus(db)), xfrm(ref),
                                xfrm(ref), c.Label(), c.GetStyle(), c.valA);
          break;
        }

        case Constraint::Type::COMMENT: {
          Style *st = SK.style.FindById(c.GetStyle());
          writeText(xfrm(c.disp.offset), c.Label(), Style::TextHeight(c.GetStyle()) / SS.GW.scale,
                    st->textAngle, st->textOrigin, c.GetStyle());
          break;
        }

        default:
          // Other types of constraints do not have a DXF dimension equivalent.
          break;
        }
      }
    }
  }

  int findDxfColor(const RgbaColor &src) {
    int    best    = 0;
    double minDist = VERY_POSITIVE;
    Vector srcv    = Vector::From(src.redF(), src.greenF(), src.blueF());
    for (int i = 1; i < 256; i++) {
      RgbaColor dst  = RGBi(DRW::dxfColors[i][0], DRW::dxfColors[i][1], DRW::dxfColors[i][2]);
      Vector    dstv = Vector::From(dst.redF(), dst.greenF(), dst.blueF());
      double    dist = srcv.Minus(dstv).Magnitude();
      if (dist < minDist || best == 0) {
        best    = i;
        minDist = dist;
      }
    }
    return best;
  }

  void assignEntityDefaults(DRW_Entity *entity, hStyle hs) {
    Style    *s        = Style::Get(hs);
    RgbaColor color    = Style::Color(hs, /*forExport=*/true);
    entity->color24    = color.ToPackedIntBGRA();
    entity->color      = findDxfColor(color);
    entity->layer      = s->DescriptionString();
    entity->lineType   = DxfFileWriter::lineTypeName(s->stippleType);
    entity->ltypeScale = Style::StippleScaleMm(s->h);
    entity->setWidthMm(Style::WidthMm(hs.v));

    if (s->stippleType == StipplePattern::FREEHAND) {
      messages.insert(_("freehand lines were replaced with continuous lines"));
    } else if (s->stippleType == StipplePattern::ZIGZAG) {
      messages.insert(_("zigzag lines were replaced with continuous lines"));
    }
  }

  void assignDimensionDefaults(DRW_Dimension *dimension, hStyle hs) {
    assignEntityDefaults(dimension, hs);
    dimension->layer = "dimensions";
  }

  void writeLine(const Vector &p0, const Vector &p1, hStyle hs) {
    DRW_Line line;
    assignEntityDefaults(&line, hs);
    line.basePoint = toCoord(p0);
    line.secPoint  = toCoord(p1);
    dxf->writeLine(&line);
  }

  void writeArc(const Vector &c, double r, double sa, double ea, hStyle hs) {
    DRW_Arc arc;
    assignEntityDefaults(&arc, hs);
    arc.radious   = r;
    arc.basePoint = toCoord(c);
    arc.staangle  = sa;
    arc.endangle  = ea;
    dxf->writeArc(&arc);
  }

  void writeBezierAsPwl(SBezier *sb) {
    List<Vector> lv = {};
    sb->MakePwlInto(&lv, SS.ExportChordTolMm());
    hStyle       hs = {(uint32_t)sb->auxA};
    DRW_Polyline polyline;
    assignEntityDefaults(&polyline, hs);
    for (int i = 0; i < lv.n; i++) {
      Vector     *v      = &lv[i];
      DRW_Vertex *vertex = new DRW_Vertex(v->x, v->y, v->z, 0.0);
      polyline.vertlist.push_back(vertex);
    }
    dxf->writePolyline(&polyline);
    lv.Clear();
  }

  void makeKnotsFor(DRW_Spline *spline) {
    // QCad/LibreCAD require this for some reason.
    if (spline->degree == 3) {
      spline->nknots = 8;
      spline->knotslist.push_back(0.0);
      spline->knotslist.push_back(0.0);
      spline->knotslist.push_back(0.0);
      spline->knotslist.push_back(0.0);
      spline->knotslist.push_back(1.0);
      spline->knotslist.push_back(1.0);
      spline->knotslist.push_back(1.0);
      spline->knotslist.push_back(1.0);
    } else if (spline->degree == 2) {
      spline->nknots = 6;
      spline->knotslist.push_back(0.0);
      spline->knotslist.push_back(0.0);
      spline->knotslist.push_back(0.0);
      spline->knotslist.push_back(1.0);
      spline->knotslist.push_back(1.0);
      spline->knotslist.push_back(1.0);
    } else
      ssassert(false, "Unexpected degree of spline");
  }

  void writeSpline(SBezier *sb) {
    bool       isRational = sb->IsRational();
    hStyle     hs         = {(uint32_t)sb->auxA};
    DRW_Spline spline;
    assignEntityDefaults(&spline, hs);
    spline.flags    = (isRational) ? 0x04 : 0x08;
    spline.degree   = sb->deg;
    spline.ncontrol = sb->deg + 1;
    makeKnotsFor(&spline);
    for (int i = 0; i <= sb->deg; i++) {
      spline.controllist.push_back(new DRW_Coord(sb->ctrl[i].x, sb->ctrl[i].y, sb->ctrl[i].z));
      if (isRational)
        spline.weightlist.push_back(sb->weight[i]);
    }
    dxf->writeSpline(&spline);
  }

  void writeBezier(SBezier *sb) {
    hStyle hs = {(uint32_t)sb->auxA};
    Vector c;
    Vector n = Vector::From(0.0, 0.0, 1.0);
    double r;

    if (sb->deg == 1) {
      // Line
      writeLine(sb->ctrl[0], sb->ctrl[1], hs);
    } else if (sb->IsInPlane(n, 0) && sb->IsCircle(n, &c, &r)) {
      // Circle perpendicular to camera
      double theta0 = atan2(sb->ctrl[0].y - c.y, sb->ctrl[0].x - c.x);
      double theta1 = atan2(sb->ctrl[2].y - c.y, sb->ctrl[2].x - c.x);
      double dtheta = WRAP_SYMMETRIC(theta1 - theta0, 2.0 * PI);
      if (dtheta < 0.0) {
        std::swap(theta0, theta1);
      }

      writeArc(c, r, theta0, theta1, hs);
    } else if (sb->IsRational()) {
      // Rational bezier
      // We'd like to export rational beziers exactly, but the resulting DXF
      // files can only be read by AutoCAD; LibreCAD/QCad simply do not
      // implement the feature. So, export as piecewise linear for compatibility.
      writeBezierAsPwl(sb);
    } else {
      // Any other curve
      writeSpline(sb);
    }
  }

  void writeAlignedDimension(Vector def1, Vector def2, Vector dimp, Vector textp,
                             const std::string &text, hStyle hs, double actual) {
    DRW_DimAligned dim;
    assignDimensionDefaults(&dim, hs);
    dim.setDef1Point(toCoord(def1));
    dim.setDef2Point(toCoord(def2));
    dim.setDimPoint(toCoord(dimp));
    dim.setTextPoint(toCoord(textp));
    dim.setText(text);
    dim.setActualMeasurement(actual);
    dxf->writeDimension(&dim);
  }

  void writeLinearDimension(Vector def1, Vector def2, Vector dimp, Vector textp,
                            const std::string &text, double angle, double oblique, hStyle hs,
                            double actual) {
    DRW_DimLinear dim;
    assignDimensionDefaults(&dim, hs);
    dim.setDef1Point(toCoord(def1));
    dim.setDef2Point(toCoord(def2));
    dim.setDimPoint(toCoord(dimp));
    dim.setTextPoint(toCoord(textp));
    dim.setText(text);
    dim.setAngle(angle);
    dim.setOblique(oblique);
    dim.setActualMeasurement(actual);
    dxf->writeDimension(&dim);
  }

  void writeRadialDimension(Vector center, Vector radius, Vector textp, const std::string &text,
                            hStyle hs, double actual) {
    DRW_DimRadial dim;
    assignDimensionDefaults(&dim, hs);
    dim.setCenterPoint(toCoord(center));
    dim.setDiameterPoint(toCoord(radius));
    dim.setTextPoint(toCoord(textp));
    dim.setText(text);
    dim.setActualMeasurement(actual);
    dxf->writeDimension(&dim);
  }

  void writeDiametricDimension(Vector def1, Vector def2, Vector textp, const std::string &text,
                               hStyle hs, double actual) {
    DRW_DimDiametric dim;
    assignDimensionDefaults(&dim, hs);
    dim.setDiameter1Point(toCoord(def1));
    dim.setDiameter2Point(toCoord(def2));
    dim.setTextPoint(toCoord(textp));
    dim.setText(text);
    dim.setActualMeasurement(actual);
    dxf->writeDimension(&dim);
  }

  void writeAngularDimension(Vector fl1, Vector fl2, Vector sl1, Vector sl2, Vector dimp,
                             Vector textp, const std::string &text, hStyle hs, double actual) {
    DRW_DimAngular dim;
    assignDimensionDefaults(&dim, hs);
    dim.setFirstLine1(toCoord(fl1));
    dim.setFirstLine2(toCoord(fl2));
    dim.setSecondLine1(toCoord(sl1));
    dim.setSecondLine2(toCoord(sl2));
    dim.setDimPoint(toCoord(dimp));
    dim.setTextPoint(toCoord(textp));
    dim.setText(text);
    dim.setActualMeasurement(actual * PI / 180.0);
    dxf->writeDimension(&dim);
  }

  void writeText(Vector textp, const std::string &text, double height, double angle,
                 Style::TextOrigin origin, hStyle hs) {
    DRW_Text txt;
    assignEntityDefaults(&txt, hs);
    txt.layer     = "text";
    txt.style     = "unicode";
    txt.basePoint = toCoord(textp);
    txt.secPoint  = txt.basePoint;
    txt.text      = text;
    txt.height    = height;
    txt.angle     = angle;
    txt.alignH    = DRW_Text::HCenter;
    if ((uint32_t)origin & (uint32_t)Style::TextOrigin::LEFT) {
      txt.alignH = DRW_Text::HLeft;
    } else if ((uint32_t)origin & (uint32_t)Style::TextOrigin::RIGHT) {
      txt.alignH = DRW_Text::HRight;
    }
    txt.alignV = DRW_Text::VMiddle;
    if ((uint32_t)origin & (uint32_t)Style::TextOrigin::TOP) {
      txt.alignV = DRW_Text::VTop;
    } else if ((uint32_t)origin & (uint32_t)Style::TextOrigin::BOT) {
      txt.alignV = DRW_Text::VBaseLine;
    }
    dxf->writeText(&txt);
  }
};

bool DxfFileWriter::OutputConstraints(IdList<Constraint, hConstraint> *constraint) {
  this->constraint = constraint;
  return true;
}

void DxfFileWriter::StartFile() {
  paths.clear();
}

void DxfFileWriter::Background(RgbaColor color) {}

void DxfFileWriter::StartPath(RgbaColor strokeRgb, double lineWidth, bool filled, RgbaColor fillRgb,
                              hStyle hs) {
  BezierPath path = {};
  paths.push_back(path);
}
void DxfFileWriter::FinishPath(RgbaColor strokeRgb, double lineWidth, bool filled,
                               RgbaColor fillRgb, hStyle hs) {}

void DxfFileWriter::Triangle(STriangle *tr) {}

void DxfFileWriter::Bezier(SBezier *sb) {
  paths.back().beziers.push_back(sb);
}

void DxfFileWriter::FinishAndCloseFile() {
  dxfRW dxf;

  DxfWriteInterface interface = {};
  interface.writer            = this;
  interface.dxf               = &dxf;

  std::stringstream stream;
  dxf.write(stream, &interface, DRW::AC1021, /*bin=*/false);
  paths.clear();
  constraint = NULL;

  if (!WriteFile(filename, stream.str())) {
    Error("Couldn't write to '%s'", filename.raw.c_str());
    return;
  }

  if (!interface.messages.empty()) {
    std::string text = _("Some aspects of the drawing have no DXF equivalent and "
                         "were not exported:\n");
    for (const std::string &message : interface.messages) {
      text += " * " + message + "\n";
    }
    Message(text.c_str());
  }
}

bool DxfFileWriter::NeedToOutput(Constraint *c) {
  switch (c->type) {
  case Constraint::Type::PT_PT_DISTANCE:
  case Constraint::Type::PT_LINE_DISTANCE:
  case Constraint::Type::DIAMETER:
  case Constraint::Type::ANGLE:
  case Constraint::Type::COMMENT: return c->IsVisible();

  default: // See writeEntities().
    break;
  }
  return false;
}

const char *DxfFileWriter::lineTypeName(StipplePattern stippleType) {
  switch (stippleType) {
  case StipplePattern::CONTINUOUS: return "CONTINUOUS";
  case StipplePattern::SHORT_DASH: return "DASHED";
  case StipplePattern::DASH: return "DASHED";
  case StipplePattern::LONG_DASH: return "DASHEDX2";
  case StipplePattern::DASH_DOT: return "DASHDOT";
  case StipplePattern::DASH_DOT_DOT: return "DIVIDE";
  case StipplePattern::DOT: return "DOT";

  case StipplePattern::FREEHAND:
  case StipplePattern::ZIGZAG:
    /* no corresponding DXF line type */
    break;
  }

  return "CONTINUOUS";
}
