//-----------------------------------------------------------------------------
// Implementation of a cosmetic line style, which determines the color and
// other appearance of a line or curve on-screen and in exported files. Some
// styles are predefined, and others can be created by the user.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"

void TextWindow::ScreenShowListOfStyles(int link, uint32_t v) {
  SS.TW.GoToScreen(Screen::LIST_OF_STYLES);
}
void TextWindow::ScreenShowStyleInfo(int link, uint32_t v) {
  SolveSpaceUI::MenuEdit(Command::UNSELECT_ALL);
  SS.TW.GoToScreen(Screen::STYLE_INFO);
  SS.TW.shown.style.v = v;
}

void TextWindow::ScreenLoadFactoryDefaultStyles(int link, uint32_t v) {
  Style::LoadFactoryDefaults();
  SS.TW.GoToScreen(Screen::LIST_OF_STYLES);
  SS.GW.persistentDirty = true;
}

void TextWindow::ScreenCreateCustomStyle(int link, uint32_t v) {
  Style::CreateCustomStyle();
}

void TextWindow::ScreenChangeBackgroundColor(int link, uint32_t v) {
  RgbaColor rgb = SS.backgroundColor;
  SS.TW.ShowEditControlWithColorPicker(3, rgb);
  SS.TW.edit.meaning = Edit::BACKGROUND_COLOR;
}

void TextWindow::ShowListOfStyles() {
  Printf(true, "%Ft color  style-name");

  bool darkbg = false;
  for (Style &s : SK.style) {
    Printf(false, "%Bp  %Bz   %Bp   %Fl%Ll%f%D%s%E", darkbg ? 'd' : 'a', &s.color,
           darkbg ? 'd' : 'a', ScreenShowStyleInfo, s.h.v, s.DescriptionString().c_str());

    darkbg = !darkbg;
  }

  Printf(true, "  %Fl%Ll%fcreate a new custom style%E", &ScreenCreateCustomStyle);

  Printf(false, "");

  RgbaColor rgb = SS.backgroundColor;
  Printf(false, "%Ft background color (r, g, b)%E");
  Printf(false, "%Ba   %@, %@, %@ %Fl%D%f%Ll[change]%E", rgb.redF(), rgb.greenF(), rgb.blueF(),
         top[rows - 1] + 2, &ScreenChangeBackgroundColor);

  Printf(false, "");
  Printf(false, "  %Fl%Ll%fload factory defaults%E", &ScreenLoadFactoryDefaultStyles);
}

void TextWindow::ScreenChangeStyleName(int link, uint32_t v) {
  hStyle hs = {v};
  Style *s  = Style::Get(hs);
  SS.TW.ShowEditControl(12, s->name);
  SS.TW.edit.style   = hs;
  SS.TW.edit.meaning = Edit::STYLE_NAME;
}

void TextWindow::ScreenDeleteStyle(int link, uint32_t v) {
  SS.UndoRemember();
  hStyle hs = {v};
  Style *s  = SK.style.FindByIdNoOops(hs);
  if (s) {
    SK.style.RemoveById(hs);
    // And it will get recreated automatically if something is still using
    // the style, so no need to do anything else.
  }
  SS.TW.GoToScreen(Screen::LIST_OF_STYLES);
  SS.GW.Invalidate();
}

void TextWindow::ScreenChangeStylePatternType(int link, uint32_t v) {
  hStyle hs             = {v};
  Style *s              = Style::Get(hs);
  s->stippleType        = (StipplePattern)(link - 1);
  SS.GW.persistentDirty = true;
}

void TextWindow::ScreenChangeStyleMetric(int link, uint32_t v) {
  hStyle         hs = {v};
  Style         *s  = Style::Get(hs);
  double         val;
  Style::UnitsAs units;
  Edit           meaning;
  int            col;
  switch (link) {
  case 't':
    val     = s->textHeight;
    units   = s->textHeightAs;
    col     = 10;
    meaning = Edit::STYLE_TEXT_HEIGHT;
    break;

  case 's':
    val     = s->stippleScale;
    units   = s->widthAs;
    col     = 17;
    meaning = Edit::STYLE_STIPPLE_PERIOD;
    break;

  case 'w':
  case 'W':
    val     = s->width;
    units   = s->widthAs;
    col     = 9;
    meaning = Edit::STYLE_WIDTH;
    break;

  default: ssassert(false, "Unexpected link");
  }

  std::string edit_value;
  if (units == Style::UnitsAs::PIXELS) {
    edit_value = ssprintf("%.2f", val);
  } else {
    edit_value = SS.MmToString(val, true);
  }
  SS.TW.ShowEditControl(col, edit_value);
  SS.TW.edit.style   = hs;
  SS.TW.edit.meaning = meaning;
}

void TextWindow::ScreenChangeStyleTextAngle(int link, uint32_t v) {
  hStyle hs = {v};
  Style *s  = Style::Get(hs);
  SS.TW.ShowEditControl(9, ssprintf("%.2f", s->textAngle));
  SS.TW.edit.style   = hs;
  SS.TW.edit.meaning = Edit::STYLE_TEXT_ANGLE;
}

void TextWindow::ScreenChangeStyleColor(int link, uint32_t v) {
  hStyle hs = {v};
  Style *s  = Style::Get(hs);
  // Same function used for stroke and fill colors
  Edit      em;
  RgbaColor rgb;
  if (link == 's') {
    em  = Edit::STYLE_COLOR;
    rgb = s->color;
  } else if (link == 'f') {
    em  = Edit::STYLE_FILL_COLOR;
    rgb = s->fillColor;
  } else
    ssassert(false, "Unexpected link");
  SS.TW.ShowEditControlWithColorPicker(13, rgb);
  SS.TW.edit.style   = hs;
  SS.TW.edit.meaning = em;
}

void TextWindow::ScreenChangeStyleYesNo(int link, uint32_t v) {
  SS.UndoRemember();
  hStyle hs = {v};
  Style *s  = Style::Get(hs);
  switch (link) {
  // Units for the width
  case 'w':
    if (s->widthAs != Style::UnitsAs::MM) {
      s->widthAs = Style::UnitsAs::MM;
      s->width /= SS.GW.scale;
      s->stippleScale /= SS.GW.scale;
    }
    break;
  case 'W':
    if (s->widthAs != Style::UnitsAs::PIXELS) {
      s->widthAs = Style::UnitsAs::PIXELS;
      s->width *= SS.GW.scale;
      s->stippleScale *= SS.GW.scale;
    }
    break;

  // Units for the height
  case 'g':
    if (s->textHeightAs != Style::UnitsAs::MM) {
      s->textHeightAs = Style::UnitsAs::MM;
      s->textHeight /= SS.GW.scale;
    }
    break;

  case 'G':
    if (s->textHeightAs != Style::UnitsAs::PIXELS) {
      s->textHeightAs = Style::UnitsAs::PIXELS;
      s->textHeight *= SS.GW.scale;
    }
    break;

  case 'e': s->exportable = !(s->exportable); break;

  case 'v': s->visible = !(s->visible); break;

  case 'f': s->filled = !(s->filled); break;

  // Horizontal text alignment
  case 'L':
    s->textOrigin =
        (Style::TextOrigin)((uint32_t)s->textOrigin | (uint32_t)Style::TextOrigin::LEFT);
    s->textOrigin =
        (Style::TextOrigin)((uint32_t)s->textOrigin & ~(uint32_t)Style::TextOrigin::RIGHT);
    break;
  case 'H':
    s->textOrigin =
        (Style::TextOrigin)((uint32_t)s->textOrigin & ~(uint32_t)Style::TextOrigin::LEFT);
    s->textOrigin =
        (Style::TextOrigin)((uint32_t)s->textOrigin & ~(uint32_t)Style::TextOrigin::RIGHT);
    break;
  case 'R':
    s->textOrigin =
        (Style::TextOrigin)((uint32_t)s->textOrigin & ~(uint32_t)Style::TextOrigin::LEFT);
    s->textOrigin =
        (Style::TextOrigin)((uint32_t)s->textOrigin | (uint32_t)Style::TextOrigin::RIGHT);
    break;

  // Vertical text alignment
  case 'B':
    s->textOrigin = (Style::TextOrigin)((uint32_t)s->textOrigin | (uint32_t)Style::TextOrigin::BOT);
    s->textOrigin =
        (Style::TextOrigin)((uint32_t)s->textOrigin & ~(uint32_t)Style::TextOrigin::TOP);
    break;
  case 'V':
    s->textOrigin =
        (Style::TextOrigin)((uint32_t)s->textOrigin & ~(uint32_t)Style::TextOrigin::BOT);
    s->textOrigin =
        (Style::TextOrigin)((uint32_t)s->textOrigin & ~(uint32_t)Style::TextOrigin::TOP);
    break;
  case 'T':
    s->textOrigin =
        (Style::TextOrigin)((uint32_t)s->textOrigin & ~(uint32_t)Style::TextOrigin::BOT);
    s->textOrigin = (Style::TextOrigin)((uint32_t)s->textOrigin | (uint32_t)Style::TextOrigin::TOP);
    break;
  }
  SS.GW.Invalidate(/*clearPersistent=*/true);
}

bool TextWindow::EditControlDoneForStyles(const std::string &str) {
  Style *s;
  switch (edit.meaning) {
  case Edit::STYLE_STIPPLE_PERIOD:
  case Edit::STYLE_TEXT_HEIGHT:
  case Edit::STYLE_WIDTH: {
    SS.UndoRemember();
    s = Style::Get(edit.style);

    double         v;
    Style::UnitsAs units = (edit.meaning == Edit::STYLE_TEXT_HEIGHT) ? s->textHeightAs : s->widthAs;
    if (units == Style::UnitsAs::MM) {
      v = SS.StringToMm(str);
    } else {
      v = atof(str.c_str());
    }
    v = std::max(0.0, v);
    if (edit.meaning == Edit::STYLE_TEXT_HEIGHT) {
      s->textHeight = v;
    } else if (edit.meaning == Edit::STYLE_STIPPLE_PERIOD) {
      s->stippleScale = v;
    } else {
      s->width = v;
    }
    break;
  }
  case Edit::STYLE_TEXT_ANGLE:
    SS.UndoRemember();
    s            = Style::Get(edit.style);
    s->textAngle = WRAP_SYMMETRIC(atof(str.c_str()), 360);
    break;

  case Edit::BACKGROUND_COLOR:
  case Edit::STYLE_FILL_COLOR:
  case Edit::STYLE_COLOR: {
    Vector rgb;
    if (sscanf(str.c_str(), "%lf, %lf, %lf", &rgb.x, &rgb.y, &rgb.z) == 3) {
      rgb = rgb.ClampWithin(0, 1);
      if (edit.meaning == Edit::STYLE_COLOR) {
        SS.UndoRemember();
        s        = Style::Get(edit.style);
        s->color = RGBf(rgb.x, rgb.y, rgb.z);
      } else if (edit.meaning == Edit::STYLE_FILL_COLOR) {
        SS.UndoRemember();
        s            = Style::Get(edit.style);
        s->fillColor = RGBf(rgb.x, rgb.y, rgb.z);
      } else {
        SS.backgroundColor = RGBf(rgb.x, rgb.y, rgb.z);
      }
    } else {
      Error(_("Bad format: specify color as r, g, b"));
    }
    break;
  }
  case Edit::STYLE_NAME:
    if (str.empty()) {
      Error(_("Style name cannot be empty"));
    } else {
      SS.UndoRemember();
      s       = Style::Get(edit.style);
      s->name = str;
    }
    break;

  default: return false;
  }
  SS.GW.persistentDirty = true;
  return true;
}

void TextWindow::ShowStyleInfo() {
  Printf(true, "%Fl%f%Ll(back to list of styles)%E", &ScreenShowListOfStyles);

  Style *s = Style::Get(shown.style);

  if (s->h.v < Style::FIRST_CUSTOM) {
    Printf(true, "%FtSTYLE  %E%s ", s->DescriptionString().c_str());
  } else {
    Printf(true,
           "%FtSTYLE  %E%s "
           "[%Fl%Ll%D%frename%E/%Fl%Ll%D%fdel%E]",
           s->DescriptionString().c_str(), s->h.v, &ScreenChangeStyleName, s->h.v,
           &ScreenDeleteStyle);
  }
  Printf(true, "%Ft line stroke style%E");
  Printf(false, "%Ba   %Ftcolor %E%Bz  %Ba (%@, %@, %@) %D%f%Ls%Fl[change]%E", &s->color,
         s->color.redF(), s->color.greenF(), s->color.blueF(), s->h.v, ScreenChangeStyleColor);

  // The line width, and its units
  if (s->widthAs == Style::UnitsAs::PIXELS) {
    Printf(false, "   %Ftwidth%E %@ %D%f%Lp%Fl[change]%E", s->width, s->h.v,
           &ScreenChangeStyleMetric, (s->h.v < Style::FIRST_CUSTOM) ? 'w' : 'W');
  } else {
    Printf(false, "   %Ftwidth%E %s %D%f%Lp%Fl[change]%E", SS.MmToString(s->width).c_str(), s->h.v,
           &ScreenChangeStyleMetric, (s->h.v < Style::FIRST_CUSTOM) ? 'w' : 'W');
  }

  if (s->widthAs == Style::UnitsAs::PIXELS) {
    Printf(false, "%Ba   %Ftstipple width%E %@ %D%f%Lp%Fl[change]%E", s->stippleScale, s->h.v,
           &ScreenChangeStyleMetric, 's');
  } else {
    Printf(false, "%Ba   %Ftstipple width%E %s %D%f%Lp%Fl[change]%E",
           SS.MmToString(s->stippleScale).c_str(), s->h.v, &ScreenChangeStyleMetric, 's');
  }

  bool widthpx = (s->widthAs == Style::UnitsAs::PIXELS);
  if (s->h.v < Style::FIRST_CUSTOM) {
    Printf(false, "   %Ftin units of %Fdpixels%E");
  } else {
    Printf(false,
           "%Ba   %Ftin units of  %Fd"
           "%D%f%LW%s pixels%E  "
           "%D%f%Lw%s %s",
           s->h.v, &ScreenChangeStyleYesNo, widthpx ? RADIO_TRUE : RADIO_FALSE, s->h.v,
           &ScreenChangeStyleYesNo, !widthpx ? RADIO_TRUE : RADIO_FALSE, SS.UnitName());
  }

  Printf(false, "%Ba   %Ftstipple type:%E");

  const size_t patternCount                 = (size_t)StipplePattern::LAST + 1;
  const char  *patternsSource[patternCount] = {"___________", "-  -  -  - ", "- - - - - -",
                                               "__ __ __ __", "-.-.-.-.-.-", "..-..-..-..",
                                               "...........", "~~~~~~~~~~~", "__~__~__~__"};
  std::string  patterns[patternCount];

  for (uint32_t i = 0; i <= (uint32_t)StipplePattern::LAST; i++) {
    const char *str = patternsSource[i];
    do {
      switch (*str) {
      case ' ': patterns[i] += " "; break;
      case '.': patterns[i] += "\xEE\x80\x84"; break;
      case '_': patterns[i] += "\xEE\x80\x85"; break;
      case '-': patterns[i] += "\xEE\x80\x86"; break;
      case '~': patterns[i] += "\xEE\x80\x87"; break;
      default: ssassert(false, "Unexpected stipple pattern element");
      }
    } while (*(++str));
  }

  for (uint32_t i = 0; i <= (uint32_t)StipplePattern::LAST; i++) {
    const char *radio = s->stippleType == (StipplePattern)i ? RADIO_TRUE : RADIO_FALSE;
    Printf(false, "%Bp     %D%f%Lp%s %s%E", (i % 2 == 0) ? 'd' : 'a', s->h.v,
           &ScreenChangeStylePatternType, i + 1, radio, patterns[i].c_str());
  }

  if (s->h.v >= Style::FIRST_CUSTOM) {
    // The fill color, and whether contours are filled

    Printf(false, "");
    Printf(false, "%Ft contour fill style%E");
    Printf(false, "%Ba   %Ftcolor %E%Bz  %Ba (%@, %@, %@) %D%f%Lf%Fl[change]%E", &s->fillColor,
           s->fillColor.redF(), s->fillColor.greenF(), s->fillColor.blueF(), s->h.v,
           ScreenChangeStyleColor);

    Printf(false, "%Bd   %D%f%Lf%s  contours are filled%E", s->h.v, &ScreenChangeStyleYesNo,
           s->filled ? CHECK_TRUE : CHECK_FALSE);
  }

  // The text height, and its units
  Printf(false, "");
  Printf(false, "%Ft text style%E");

  if (s->textHeightAs == Style::UnitsAs::PIXELS) {
    Printf(false, "%Ba   %Ftheight %E%@ %D%f%Lt%Fl%s%E", s->textHeight, s->h.v,
           &ScreenChangeStyleMetric, "[change]");
  } else {
    Printf(false, "%Ba   %Ftheight %E%s %D%f%Lt%Fl%s%E", SS.MmToString(s->textHeight).c_str(),
           s->h.v, &ScreenChangeStyleMetric, "[change]");
  }

  bool textHeightpx = (s->textHeightAs == Style::UnitsAs::PIXELS);
  if (s->h.v < Style::FIRST_CUSTOM) {
    Printf(false, "%Bd   %Ftin units of %Fdpixels");
  } else {
    Printf(false,
           "%Bd   %Ftin units of  %Fd"
           "%D%f%LG%s pixels%E  "
           "%D%f%Lg%s %s",
           s->h.v, &ScreenChangeStyleYesNo, textHeightpx ? RADIO_TRUE : RADIO_FALSE, s->h.v,
           &ScreenChangeStyleYesNo, !textHeightpx ? RADIO_TRUE : RADIO_FALSE, SS.UnitName());
  }

  if (s->h.v >= Style::FIRST_CUSTOM) {
    Printf(false, "%Ba   %Ftangle %E%@ %D%f%Ll%Fl[change]%E", s->textAngle, s->h.v,
           &ScreenChangeStyleTextAngle);

    Printf(false, "");
    Printf(false, "%Ft text comment alignment%E");
    bool neither;
    neither = !((uint32_t)s->textOrigin &
                ((uint32_t)Style::TextOrigin::LEFT | (uint32_t)Style::TextOrigin::RIGHT));
    Printf(false,
           "%Ba   "
           "%D%f%LL%s left%E    "
           "%D%f%LH%s center%E  "
           "%D%f%LR%s right%E  ",
           s->h.v, &ScreenChangeStyleYesNo,
           ((uint32_t)s->textOrigin & (uint32_t)Style::TextOrigin::LEFT) ? RADIO_TRUE : RADIO_FALSE,
           s->h.v, &ScreenChangeStyleYesNo, neither ? RADIO_TRUE : RADIO_FALSE, s->h.v,
           &ScreenChangeStyleYesNo,
           ((uint32_t)s->textOrigin & (uint32_t)Style::TextOrigin::RIGHT) ? RADIO_TRUE
                                                                          : RADIO_FALSE);

    neither = !((uint32_t)s->textOrigin &
                ((uint32_t)Style::TextOrigin::BOT | (uint32_t)Style::TextOrigin::TOP));
    Printf(false,
           "%Bd   "
           "%D%f%LB%s bottom%E  "
           "%D%f%LV%s center%E  "
           "%D%f%LT%s top%E  ",
           s->h.v, &ScreenChangeStyleYesNo,
           ((uint32_t)s->textOrigin & (uint32_t)Style::TextOrigin::BOT) ? RADIO_TRUE : RADIO_FALSE,
           s->h.v, &ScreenChangeStyleYesNo, neither ? RADIO_TRUE : RADIO_FALSE, s->h.v,
           &ScreenChangeStyleYesNo,
           ((uint32_t)s->textOrigin & (uint32_t)Style::TextOrigin::TOP) ? RADIO_TRUE : RADIO_FALSE);
  }

  Printf(false, "");

  if (s->h.v >= Style::FIRST_CUSTOM) {
    Printf(false, "  %Fd%D%f%Lv%s  show these objects on screen%E", s->h.v, &ScreenChangeStyleYesNo,
           s->visible ? CHECK_TRUE : CHECK_FALSE);
  }

  Printf(false, "  %Fd%D%f%Le%s  export these objects%E", s->h.v, &ScreenChangeStyleYesNo,
         s->exportable ? CHECK_TRUE : CHECK_FALSE);

  if (s->h.v >= Style::FIRST_CUSTOM) {
    Printf(false, "");
    Printf(false, "To assign lines or curves to this style,");
    Printf(false, "right-click them on the drawing.");
  }
}

void TextWindow::ScreenAssignSelectionToStyle(int link, uint32_t v) {
  Style::AssignSelectionToStyle(v);
}
