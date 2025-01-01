//-----------------------------------------------------------------------------
// Implementation of a cosmetic line style, which determines the color and
// other appearance of a line or curve on-screen and in exported files. Some
// styles are predefined, and others can be created by the user.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"
#include "ssg.h"

const Style::Default Style::Defaults[] = {
    {{ACTIVE_GRP}, "ActiveGrp", RGBf(1.0, 1.0, 1.0), 1.5, 4, true, StipplePattern::CONTINUOUS},
    {{CONSTRUCTION},
     "Construction",
     RGBf(0.1, 0.7, 0.1),
     1.5,
     0,
     false,
     StipplePattern::CONTINUOUS},
    {{INACTIVE_GRP}, "InactiveGrp", RGBf(0.5, 0.3, 0.0), 1.5, 3, true, StipplePattern::CONTINUOUS},
    {{DATUM}, "Datum", RGBf(0.0, 0.8, 0.0), 1.5, 0, true, StipplePattern::CONTINUOUS},
    {{SOLID_EDGE}, "SolidEdge", RGBf(0.8, 0.8, 0.8), 1.0, 2, true, StipplePattern::CONTINUOUS},
    {{CONSTRAINT}, "Constraint", RGBf(1.0, 0.1, 1.0), 1.0, 0, true, StipplePattern::CONTINUOUS},
    {{SELECTED}, "Selected", RGBf(1.0, 0.0, 0.0), 1.5, 0, true, StipplePattern::CONTINUOUS},
    {{HOVERED}, "Hovered", RGBf(1.0, 1.0, 0.0), 1.5, 0, true, StipplePattern::CONTINUOUS},
    {{CONTOUR_FILL}, "ContourFill", RGBf(0.0, 0.1, 0.1), 1.0, 0, true, StipplePattern::CONTINUOUS},
    {{NORMALS}, "Normals", RGBf(0.0, 0.4, 0.4), 1.0, 0, true, StipplePattern::CONTINUOUS},
    {{ANALYZE}, "Analyze", RGBf(0.0, 1.0, 1.0), 3.0, 0, true, StipplePattern::CONTINUOUS},
    {{DRAW_ERROR}, "DrawError", RGBf(1.0, 0.0, 0.0), 8.0, 0, true, StipplePattern::CONTINUOUS},
    {{DIM_SOLID}, "DimSolid", RGBf(0.1, 0.1, 0.1), 1.0, 0, true, StipplePattern::CONTINUOUS},
    {{HIDDEN_EDGE}, "HiddenEdge", RGBf(0.8, 0.8, 0.8), 1.0, 1, true, StipplePattern::DASH},
    {{OUTLINE}, "Outline", RGBf(0.8, 0.8, 0.8), 3.0, 5, true, StipplePattern::CONTINUOUS},
    {{0}, NULL, RGBf(0.0, 0.0, 0.0), 0.0, 0, true, StipplePattern::CONTINUOUS}};

std::string Style::CnfColor(const std::string &prefix) {
  return "Style_" + prefix + "_Color";
}
std::string Style::CnfWidth(const std::string &prefix) {
  return "Style_" + prefix + "_Width";
}
std::string Style::CnfStippleType(const std::string &prefix) {
  return "Style_" + prefix + "_StippleType";
}
std::string Style::CnfStippleScale(const std::string &prefix) {
  return "Style_" + prefix + "_StippleScale";
}
std::string Style::CnfTextHeight(const std::string &prefix) {
  return "Style_" + prefix + "_TextHeight";
}
std::string Style::CnfExportable(const std::string &prefix) {
  return "Style_" + prefix + "_Exportable";
}

std::string Style::CnfPrefixToName(const std::string &prefix) {
  std::string name = "#def-";

  for (size_t i = 0; i < prefix.length(); i++) {
    if (isupper(prefix[i]) && i != 0)
      name += '-';
    name += tolower(prefix[i]);
  }

  return name;
}

void Style::CreateAllDefaultStyles() {
  const Default *d;
  for (d = &(Defaults[0]); d->h.v; d++) {
    (void)Get(d->h);
  }
}

void Style::CreateDefaultStyle(hStyle h) {
  bool           isDefaultStyle = true;
  const Default *d;
  for (d = &(Defaults[0]); d->h.v; d++) {
    if (d->h == h)
      break;
  }
  if (!d->h.v) {
    // Not a default style; so just create it the same as our default
    // active group entity style.
    d              = &(Defaults[0]);
    isDefaultStyle = false;
  }

  Style ns = {};
  FillDefaultStyle(&ns, d);
  ns.h = h;
  if (isDefaultStyle) {
    ns.name = CnfPrefixToName(d->cnfPrefix);
  } else {
    ns.name = "new-custom-style";
  }

  SK.style.Add(&ns);
}

void Style::FillDefaultStyle(Style *s, const Default *d, bool factory) {
  Platform::SettingsRef settings = Platform::GetSettings();

  if (d == NULL)
    d = &Defaults[0];
  s->color = (factory) ? d->color : settings->ThawColor(CnfColor(d->cnfPrefix), d->color);
  s->width = (factory) ? d->width : settings->ThawFloat(CnfWidth(d->cnfPrefix), (float)(d->width));
  s->widthAs      = UnitsAs::PIXELS;
  s->textHeight   = (factory) ? 11.5 : settings->ThawFloat(CnfTextHeight(d->cnfPrefix), 11.5);
  s->textHeightAs = UnitsAs::PIXELS;
  s->textOrigin   = TextOrigin::NONE;
  s->textAngle    = 0;
  s->visible      = true;
  s->exportable =
      (factory) ? d->exportable : settings->ThawBool(CnfExportable(d->cnfPrefix), d->exportable);
  s->filled       = false;
  s->fillColor    = RGBf(0.3, 0.3, 0.3);
  s->stippleType  = (factory)
                        ? d->stippleType
                        : Style::StipplePatternFromString(settings->ThawString(
                             CnfStippleType(d->cnfPrefix), StipplePatternName(d->stippleType)));
  s->stippleScale = (factory) ? 15.0 : settings->ThawFloat(CnfStippleScale(d->cnfPrefix), 15.0);
  s->zIndex       = d->zIndex;
}

void Style::LoadFactoryDefaults() {
  const Default *d;
  for (d = &(Defaults[0]); d->h.v; d++) {
    Style *s = Get(d->h);
    FillDefaultStyle(s, d, /*factory=*/true);
  }
  SS.backgroundColor = RGBi(0, 0, 0);
}

void Style::FreezeDefaultStyles(Platform::SettingsRef settings) {
  const Default *d;
  for (d = &(Defaults[0]); d->h.v; d++) {
    settings->FreezeColor(CnfColor(d->cnfPrefix), Color(d->h));
    settings->FreezeFloat(CnfWidth(d->cnfPrefix), (float)Width(d->h));
    settings->FreezeString(CnfStippleType(d->cnfPrefix), StipplePatternName(d->h));
    settings->FreezeFloat(CnfStippleScale(d->cnfPrefix), (float)StippleScale(d->h));
    settings->FreezeFloat(CnfTextHeight(d->cnfPrefix), (float)TextHeight(d->h));
    settings->FreezeBool(CnfExportable(d->cnfPrefix), Exportable(d->h.v));
  }
}

uint32_t Style::CreateCustomStyle(bool rememberForUndo) {
  if (rememberForUndo)
    SS.UndoRemember();
  uint32_t vs = std::max((uint32_t)Style::FIRST_CUSTOM, SK.style.MaximumId() + 1);
  hStyle   hs = {vs};
  (void)Style::Get(hs);
  return hs.v;
}

void Style::AssignSelectionToStyle(uint32_t v) {
  bool showError = false;
  SS.GW.GroupSelection();

  SS.UndoRemember();
  int i;
  for (i = 0; i < SS.GW.gs.entities; i++) {
    hEntity he = SS.GW.gs.entity[i];
    Entity *e  = SK.GetEntity(he);
    if (!e->IsStylable())
      continue;

    if (!he.isFromRequest()) {
      showError = true;
      continue;
    }

    hRequest hr = he.request();
    Request *r  = SK.GetRequest(hr);
    r->style.v  = v;
    SS.MarkGroupDirty(r->group);
  }
  for (i = 0; i < SS.GW.gs.constraints; i++) {
    hConstraint hc = SS.GW.gs.constraint[i];
    Constraint *c  = SK.GetConstraint(hc);
    if (!c->IsStylable())
      continue;

    c->disp.style.v = v;
    SS.MarkGroupDirty(c->group);
  }

  if (showError) {
    Error(_("Can't assign style to an entity that's derived from another "
            "entity; try assigning a style to this entity's parent."));
  }

  SS.GW.ClearSelection();
  SS.GW.Invalidate();

  // And show that style's info screen in the text window.
  SS.TW.GoToScreen(TextWindow::Screen::STYLE_INFO);
  SS.TW.shown.style.v = v;
  SS.ScheduleShowTW();
}

//-----------------------------------------------------------------------------
// Look up a style by its handle. If that style does not exist, then create
// the style, according to our table of default styles.
//-----------------------------------------------------------------------------
Style *Style::Get(hStyle h) {
  if (h.v == 0)
    h.v = ACTIVE_GRP;

  Style *s = SK.style.FindByIdNoOops(h);
  if (s) {
    // It exists, good.
    return s;
  } else {
    // It doesn't exist; so we should create it and then return that.
    CreateDefaultStyle(h);
    return SK.style.FindById(h);
  }
}

//-----------------------------------------------------------------------------
// A couple of wrappers, so that I can call these functions with either an
// hStyle or with the integer corresponding to that hStyle.v.
//-----------------------------------------------------------------------------
RgbaColor Style::Color(int s, bool forExport) {
  hStyle hs = {(uint32_t)s};
  return Color(hs, forExport);
}
double Style::Width(int s) {
  hStyle hs = {(uint32_t)s};
  return Width(hs);
}

//-----------------------------------------------------------------------------
// If a color is almost white, then we can rewrite it to black, just so that
// it won't disappear on file formats with a light background.
//-----------------------------------------------------------------------------
RgbaColor Style::RewriteColor(RgbaColor rgbin) {
  Vector rgb = Vector::From(rgbin.redF(), rgbin.greenF(), rgbin.blueF());
  rgb        = rgb.Minus(Vector::From(1, 1, 1));
  if (rgb.Magnitude() < 0.4 && SS.fixExportColors) {
    // This is an almost-white color in a default style, which is
    // good for the default on-screen view (black bg) but probably
    // not desired in the exported files, which typically are shown
    // against white backgrounds.
    return RGBi(0, 0, 0);
  } else {
    return rgbin;
  }
}

//-----------------------------------------------------------------------------
// Return the stroke color associated with our style as 8-bit RGB.
//-----------------------------------------------------------------------------
RgbaColor Style::Color(hStyle h, bool forExport) {
  Style *s = Get(h);
  if (forExport) {
    return RewriteColor(s->color);
  } else {
    return s->color;
  }
}

//-----------------------------------------------------------------------------
// Return the fill color associated with our style as 8-bit RGB.
//-----------------------------------------------------------------------------
RgbaColor Style::FillColor(hStyle h, bool forExport) {
  Style *s = Get(h);
  if (forExport) {
    return RewriteColor(s->fillColor);
  } else {
    return s->fillColor;
  }
}

//-----------------------------------------------------------------------------
// Return the width associated with our style in pixels..
//-----------------------------------------------------------------------------
double Style::Width(hStyle h) {
  Style *s = Get(h);
  switch (s->widthAs) {
  case UnitsAs::MM: return s->width * SS.GW.scale;
  case UnitsAs::PIXELS: return s->width;
  }
  ssassert(false, "Unexpected units");
}

//-----------------------------------------------------------------------------
// Return the width associated with our style in millimeters..
//-----------------------------------------------------------------------------
double Style::WidthMm(int hs) {
  double widthpx = Width(hs);
  return widthpx / SS.GW.scale;
}

//-----------------------------------------------------------------------------
// Return the associated text height, in pixels.
//-----------------------------------------------------------------------------
double Style::TextHeight(hStyle h) {
  Style *s = Get(h);
  switch (s->textHeightAs) {
  case UnitsAs::MM: return s->textHeight * SS.GW.scale;
  case UnitsAs::PIXELS: return s->textHeight;
  }
  ssassert(false, "Unexpected units");
}

double Style::DefaultTextHeight() {
  hStyle hs{Style::CONSTRAINT};
  return TextHeight(hs);
}

//-----------------------------------------------------------------------------
// Return the parameters of this style, as a canvas stroke.
//-----------------------------------------------------------------------------
Canvas::Stroke Style::Stroke(hStyle hs) {
  Canvas::Stroke stroke = {};
  Style         *style  = Style::Get(hs);
  stroke.color          = style->color;
  stroke.stipplePattern = style->stippleType;
  stroke.stippleScale   = style->stippleScale;
  stroke.width          = style->width;
  switch (style->widthAs) {
  case Style::UnitsAs::PIXELS: stroke.unit = Canvas::Unit::PX; break;
  case Style::UnitsAs::MM: stroke.unit = Canvas::Unit::MM; break;
  }
  return stroke;
}

Canvas::Stroke Style::Stroke(int hsv) {
  hStyle hs = {(uint32_t)hsv};
  return Style::Stroke(hs);
}

//-----------------------------------------------------------------------------
// Should lines and curves from this style appear in the output file? Only
// if it's both shown and exportable.
//-----------------------------------------------------------------------------
bool Style::Exportable(int si) {
  hStyle hs = {(uint32_t)si};
  Style *s  = Get(hs);
  return (s->exportable) && (s->visible);
}

//-----------------------------------------------------------------------------
// Return the appropriate style for our entity. If the entity has a style
// explicitly assigned, then it's that style. Otherwise it's the appropriate
// default style.
//-----------------------------------------------------------------------------
hStyle Style::ForEntity(hEntity he) {
  Entity *e = SK.GetEntity(he);
  // If the entity has a special style, use that. If that style doesn't
  // exist yet, then it will get created automatically later.
  if (e->style.v != 0) {
    return e->style;
  }

  // Otherwise, we use the default rules.
  hStyle hs;
  if (e->group != SS.GW.activeGroup) {
    hs.v = INACTIVE_GRP;
  } else if (e->construction) {
    hs.v = CONSTRUCTION;
  } else {
    hs.v = ACTIVE_GRP;
  }
  return hs;
}

StipplePattern Style::StipplePatternFromString(std::string name) {
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  if (name == "continuous") {
    return StipplePattern::CONTINUOUS;
  } else if (name == "shortdash") {
    return StipplePattern::SHORT_DASH;
  } else if (name == "dash") {
    return StipplePattern::DASH;
  } else if (name == "longdash") {
    return StipplePattern::LONG_DASH;
  } else if (name == "dashdot") {
    return StipplePattern::DASH_DOT;
  } else if (name == "dashdotdot") {
    return StipplePattern::DASH_DOT_DOT;
  } else if (name == "dot") {
    return StipplePattern::DOT;
  } else if (name == "freehand") {
    return StipplePattern::FREEHAND;
  } else if (name == "zigzag") {
    return StipplePattern::ZIGZAG;
  }

  return StipplePattern::CONTINUOUS;
}

StipplePattern Style::PatternType(hStyle hs) {
  Style *s = Get(hs);
  return s->stippleType;
}

std::string Style::StipplePatternName(hStyle hs) {
  Style *s = Get(hs);
  return StipplePatternName(s->stippleType);
}

std::string Style::StipplePatternName(StipplePattern stippleType) {
  switch (stippleType) {
  case StipplePattern::CONTINUOUS: return "Continuous";
  case StipplePattern::SHORT_DASH: return "ShortDash";
  case StipplePattern::DASH: return "Dash";
  case StipplePattern::LONG_DASH: return "LongDash";
  case StipplePattern::DASH_DOT: return "DashDot";
  case StipplePattern::DASH_DOT_DOT: return "DashDotDot";
  case StipplePattern::DOT: return "Dot";
  case StipplePattern::FREEHAND: return "FreeHand";
  case StipplePattern::ZIGZAG: return "ZigZag";
  }

  return "Continuous";
}

double Style::StippleScale(hStyle hs) {
  Style *s = Get(hs);
  return s->stippleScale;
}

double Style::StippleScaleMm(hStyle hs) {
  Style *s = Get(hs);
  if (s->widthAs == UnitsAs::MM) {
    return s->stippleScale;
  } else if (s->widthAs == UnitsAs::PIXELS) {
    return s->stippleScale / SS.GW.scale;
  }
  return 1.0;
}

std::string Style::DescriptionString() const {
  if (name.empty()) {
    return ssprintf("s%03x-(unnamed)", h.v);
  } else {
    return ssprintf("s%03x-%s", h.v, name.c_str());
  }
}
