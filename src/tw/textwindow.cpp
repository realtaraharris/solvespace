//-----------------------------------------------------------------------------
// Helper functions for the text-based browser window.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"
#include "ssg.h"
#include "ttf.h"

namespace SolveSpace {

  class Button {
public:
    virtual std::string Tooltip() = 0;
    virtual void Draw(UiCanvas *uiCanvas, int x, int y, bool asHovered) = 0;
    virtual int AdvanceWidth() = 0;
    virtual void Click() = 0;
  };

  class SpacerButton : public Button {
public:
    std::string Tooltip() override { return ""; }

    void Draw(UiCanvas *uiCanvas, int x, int y, bool asHovered) override {
      // Draw a darker-grey spacer in between the groups of icons.
      uiCanvas->DrawRect(x, x + 4, y, y - 24,
                         /*fillColor=*/{45, 45, 45, 255},
                         /*outlineColor=*/{});
    }

    int AdvanceWidth() override { return 12; }

    void Click() override {}
  };

  class ShowHideButton : public Button {
public:
    bool *variable;
    std::string tooltip;
    std::string iconName;
    std::shared_ptr<Pixmap> icon;

    ShowHideButton(bool *variable, std::string iconName, std::string tooltip)
        : variable(variable), tooltip(tooltip), iconName(iconName) {}

    std::string Tooltip() override { return ((*variable) ? "Hide " : "Show ") + tooltip; }

    void Draw(UiCanvas *uiCanvas, int x, int y, bool asHovered) override {
      if (icon == NULL) {
        icon = LoadPng("icons/text-window/" + iconName + ".png");
      }

      uiCanvas->DrawPixmap(icon, x, y - 24);
      if (asHovered) {
        uiCanvas->DrawRect(x - 2, x + 26, y + 2, y - 26,
                           /*fillColor=*/{255, 255, 0, 75},
                           /*outlineColor=*/{});
      }
      if (!*(variable)) {
        int s = 0, f = 24;
        RgbaColor color = {255, 0, 0, 150};
        uiCanvas->DrawLine(x + s, y - s, x + f, y - f, color, 2);
        uiCanvas->DrawLine(x + s, y - f, x + f, y - s, color, 2);
      }
    }

    int AdvanceWidth() override { return 32; }

    void Click() override { SS.GW.ToggleBool(variable); }
  };

  class FacesButton : public ShowHideButton {
public:
    FacesButton() : ShowHideButton(&(SS.GW.showFaces), "faces", "") {}

    std::string Tooltip() override {
      if (*variable) {
        return "Don't make faces selectable with mouse";
      } else {
        return "Make faces selectable with mouse";
      }
    }
  };

  class OccludedLinesButton : public Button {
public:
    std::shared_ptr<Pixmap> visibleIcon;
    std::shared_ptr<Pixmap> stippledIcon;
    std::shared_ptr<Pixmap> invisibleIcon;

    std::string Tooltip() override {
      switch (SS.GW.drawOccludedAs) {
      case GraphicsWindow::DrawOccludedAs::INVISIBLE: return "Stipple occluded lines";

      case GraphicsWindow::DrawOccludedAs::STIPPLED: return "Draw occluded lines";

      case GraphicsWindow::DrawOccludedAs::VISIBLE: return "Don't draw occluded lines";

      default: ssassert(false, "Unexpected mode");
      }
    }

    void Draw(UiCanvas *uiCanvas, int x, int y, bool asHovered) override {
      if (visibleIcon == NULL) {
        visibleIcon = LoadPng("icons/text-window/occluded-visible.png");
      }
      if (stippledIcon == NULL) {
        stippledIcon = LoadPng("icons/text-window/occluded-stippled.png");
      }
      if (invisibleIcon == NULL) {
        invisibleIcon = LoadPng("icons/text-window/occluded-invisible.png");
      }

      std::shared_ptr<Pixmap> icon;
      switch (SS.GW.drawOccludedAs) {
      case GraphicsWindow::DrawOccludedAs::INVISIBLE: icon = invisibleIcon; break;
      case GraphicsWindow::DrawOccludedAs::STIPPLED: icon = stippledIcon; break;
      case GraphicsWindow::DrawOccludedAs::VISIBLE: icon = visibleIcon; break;
      }

      uiCanvas->DrawPixmap(icon, x, y - 24);
      if (asHovered) {
        uiCanvas->DrawRect(x - 2, x + 26, y + 2, y - 26,
                           /*fillColor=*/{255, 255, 0, 75},
                           /*outlineColor=*/{});
      }
    }

    int AdvanceWidth() override { return 32; }

    void Click() override {
      switch (SS.GW.drawOccludedAs) {
      case GraphicsWindow::DrawOccludedAs::INVISIBLE:
        SS.GW.drawOccludedAs = GraphicsWindow::DrawOccludedAs::STIPPLED;
        break;

      case GraphicsWindow::DrawOccludedAs::STIPPLED:
        SS.GW.drawOccludedAs = GraphicsWindow::DrawOccludedAs::VISIBLE;
        break;

      case GraphicsWindow::DrawOccludedAs::VISIBLE:
        SS.GW.drawOccludedAs = GraphicsWindow::DrawOccludedAs::INVISIBLE;
        break;
      }

      SS.GenerateAll();
      SS.GW.Invalidate();
      SS.ScheduleShowTW();
    }
  };

  static SpacerButton spacerButton;

  static ShowHideButton workplanesButton = {&(SS.GW.showWorkplanes), "workplane",
                                            "workplanes from inactive groups"};
  static ShowHideButton normalsButton = {&(SS.GW.showNormals), "normal", "normals"};
  static ShowHideButton pointsButton = {&(SS.GW.showPoints), "point", "points"};
  static ShowHideButton constructionButton = {&(SS.GW.showConstruction), "construction",
                                              "construction entities"};
  static ShowHideButton constraintsButton = {&(SS.GW.showConstraints), "constraint",
                                             "constraints and dimensions"};
  static FacesButton facesButton;
  static ShowHideButton shadedButton = {&(SS.GW.showShaded), "shaded",
                                        "shaded view of solid model"};
  static ShowHideButton edgesButton = {&(SS.GW.showEdges), "edges", "edges of solid model"};
  static ShowHideButton outlinesButton = {&(SS.GW.showOutlines), "outlines",
                                          "outline of solid model"};
  static ShowHideButton meshButton = {&(SS.GW.showMesh), "mesh", "triangle mesh of solid model"};
  static OccludedLinesButton occludedLinesButton;

  static Button *buttons[] = {
      &workplanesButton,    &normalsButton,  &pointsButton, &constructionButton,
      &constraintsButton,   &facesButton,    &spacerButton, &shadedButton,
      &edgesButton,         &outlinesButton, &meshButton,   &spacerButton,
      &occludedLinesButton,
  };

  /** Foreground color codes. */
  const TextWindow::Color TextWindow::fgColors[] = {
      {'d', RGBi(255, 255, 255)},                           // Default   : white
      {'l', RGBi(100, 200, 255)},                           // links     : blue
      {'t', RGBi(255, 200, 100)},                           // tree/text : yellow
      {'h', RGBi(90, 90, 90)},    {'s', RGBi(40, 255, 40)}, // Ok        : green
      {'m', RGBi(200, 200, 0)},   {'r', RGBi(0, 0, 0)},     // Reverse   : black
      {'x', RGBi(255, 20, 20)},                             // Error     : red
      {'i', RGBi(0, 255, 255)},                             // Info      : cyan
      {'g', RGBi(128, 128, 128)},                           // Disabled  : gray
      {'b', RGBi(200, 200, 200)}, {0, RGBi(0, 0, 0)}};
  /** Background color codes. */
  const TextWindow::Color TextWindow::bgColors[] = {
      {'d', RGBi(0, 0, 0)}, // Default   : black
      {'t', RGBi(34, 15, 15)},
      {'a', RGBi(25, 25, 25)},    // Alternate : dark gray
      {'r', RGBi(255, 255, 255)}, // Reverse   : white
      {0, RGBi(0, 0, 0)}};

  void TextWindow::MakeColorTable(const Color *in, float *out) {
    int i;
    for (i = 0; in[i].c != 0; i++) {
      int c = in[i].c;
      ssassert(c >= 0 && c <= 255, "Unexpected color index");
      out[c * 3 + 0] = in[i].color.redF();
      out[c * 3 + 1] = in[i].color.greenF();
      out[c * 3 + 2] = in[i].color.blueF();
    }
  }

  void TextWindow::Init() {
    if (!window) {
//      window = Platform::CreateWindow(Platform::Window::Kind::TOOL, SS.GW.window);
      if (window) {
        canvas = CreateRenderer();

        using namespace std::placeholders;
        window->onClose = []() {
          SS.TW.HideEditControl();
          SS.GW.showTextWindow = false;
          SS.GW.EnsureValidActives();
        };
        window->onMouseEvent = [this](Platform::MouseEvent event) {
          using Platform::MouseEvent;

          if (event.type == MouseEvent::Type::PRESS || event.type == MouseEvent::Type::DBL_PRESS ||
              event.type == MouseEvent::Type::MOTION) {
            bool isClick = (event.type != MouseEvent::Type::MOTION);
            bool leftDown = (event.button == MouseEvent::Button::LEFT);
            this->MouseEvent(isClick, leftDown, event.x, event.y);
            return true;
          } else if (event.type == MouseEvent::Type::LEAVE) {
            MouseLeave();
            return true;
          } else if (event.type == MouseEvent::Type::SCROLL_VERT) {
            if (event.scrollDelta == 0) {
              return true;
            }
            if (abs(event.scrollDelta) < 0.2) {
              if (event.scrollDelta > 0) {
                event.scrollDelta = 0.2;
              } else {
                event.scrollDelta = -0.2;
              }
            }
            double offset = LINE_HEIGHT / 2 * event.scrollDelta;
            ScrollbarEvent(window->GetScrollbarPosition() - offset);
          }
          return false;
        };
        window->onRender = std::bind(&TextWindow::Paint, this);
        window->onEditingDone = std::bind(&TextWindow::EditControlDone, this, _1);
        window->onScrollbarAdjusted = std::bind(&TextWindow::ScrollbarEvent, this, _1);
        window->SetMinContentSize(370, 370);
      }
    }

    ClearSuper();
  }

  void TextWindow::ClearSuper() {
    // Ugly hack, but not so ugly as the next line
    Platform::WindowRef oldWindow = std::move(window);
    std::shared_ptr<ViewportCanvas> oldCanvas = canvas;

    // Cannot use *this = {} here because TextWindow instances
    // are 2.4MB long; this causes stack overflows in prologue
    // when built with MSVC, even with optimizations.
    memset(this, 0, sizeof(*this));

    // Return old canvas
    window = std::move(oldWindow);
    canvas = oldCanvas;

    HideEditControl();

    MakeColorTable(fgColors, fgColorTable);
    MakeColorTable(bgColors, bgColorTable);

    ClearScreen();
    //    Show();
  }

  void TextWindow::HideEditControl() {
    editControl.colorPicker.show = false;
    if (window) {
      window->HideEditor();
      window->Invalidate();
    }
  }

  void TextWindow::ShowEditControl(int col, const std::string &str, int halfRow) {
    if (halfRow < 0)
      halfRow = top[hoveredRow];
    editControl.halfRow = halfRow;
    editControl.col = col;

    int x = LEFT_MARGIN + CHAR_WIDTH_ * col;
    int y = (halfRow - SS.TW.scrollPos) * (LINE_HEIGHT / 2);

    double width, height;
//    window->GetContentSize(&width, &height);
//    window->ShowEditor(x, y + LINE_HEIGHT - 2, LINE_HEIGHT - 4, width - x, /*isMonospace=*/true, str);
  }

  void TextWindow::ShowEditControlWithColorPicker(int col, RgbaColor rgb) {
    SS.ScheduleShowTW();

    editControl.colorPicker.show = true;
    editControl.colorPicker.rgb = rgb;
    editControl.colorPicker.h = 0;
    editControl.colorPicker.s = 0;
    editControl.colorPicker.v = 1;
    ShowEditControl(col, ssprintf("%.2f, %.2f, %.2f", rgb.redF(), rgb.greenF(), rgb.blueF()));
  }

  void TextWindow::ClearScreen() {
    int i, j;
    for (i = 0; i < MAX_ROWS; i++) {
      for (j = 0; j < MAX_COLS; j++) {
        text[i][j] = ' ';
        meta[i][j].fg = 'd';
        meta[i][j].bg = 'd';
        meta[i][j].link = NOT_A_LINK;
      }
      top[i] = i * 2;
    }
    rows = 0;
  }

  // This message was added when someone had too many fonts for the text window
  // Scrolling seemed to be broken, but was actually at the MAX_ROWS.
  static const char *endString = "    **** End of Text Screen ****";

  void TextWindow::Printf(bool halfLine, const char *fmt, ...) {
    if (!canvas)
      return;

    if (rows >= MAX_ROWS)
      return;

    if (rows >= MAX_ROWS - 2 && (fmt != endString)) {
      // twice due to some half-row issues on resizing
      Printf(halfLine, endString);
      Printf(halfLine, endString);
      return;
    }

    va_list vl;
    va_start(vl, fmt);

    int r, c;
    r = rows;
    top[r] = (r == 0) ? 0 : (top[r - 1] + (halfLine ? 3 : 2));
    rows++;

    for (c = 0; c < MAX_COLS; c++) {
      text[r][c] = ' ';
      meta[r][c].link = NOT_A_LINK;
    }

    char fg = 'd';
    char bg = 'd';
    RgbaColor bgRgb = RGBi(0, 0, 0);
    int link = NOT_A_LINK;
    uint32_t data = 0;
    LinkFunction *f = NULL, *h = NULL;

    c = 0;
    while (*fmt) {
      char buf[1024];

      if (*fmt == '%') {
        fmt++;
        if (*fmt == '\0')
          goto done;
        strcpy(buf, "");
        switch (*fmt) {
        case 'd': {
          int v = va_arg(vl, int);
          sprintf(buf, "%d", v);
          break;
        }
        case 'x': {
          unsigned int v = va_arg(vl, unsigned int);
          sprintf(buf, "%08x", v);
          break;
        }
        case '@': {
          double v = va_arg(vl, double);
          sprintf(buf, "%.2f", v);
          break;
        }
        case '2': {
          double v = va_arg(vl, double);
          sprintf(buf, "%s%.2f", v < 0 ? "" : " ", v);
          break;
        }
        case '3': {
          double v = va_arg(vl, double);
          sprintf(buf, "%s%.3f", v < 0 ? "" : " ", v);
          break;
        }
        case '#': {
          double v = va_arg(vl, double);
          sprintf(buf, "%.3f", v);
          break;
        }
        case 's': {
          char *s = va_arg(vl, char *);
          memcpy(buf, s, std::min(sizeof(buf), strlen(s) + 1));
          break;
        }
        case 'c': {
          // 'char' is promoted to 'int' when passed through '...'
          int v = va_arg(vl, int);
          if (v == 0) {
            strcpy(buf, "");
          } else {
            sprintf(buf, "%c", v);
          }
          break;
        }
        case 'E':
          fg = 'd';
          // leave the background, though
          link = NOT_A_LINK;
          data = 0;
          f = NULL;
          h = NULL;
          break;

        case 'F':
        case 'B': {
          char cc = fmt[1]; // color code
          RgbaColor *rgbPtr = NULL;
          switch (cc) {
          case 0: goto done; // truncated directive
          case 'p': cc = (char)va_arg(vl, int); break;
          case 'z': rgbPtr = va_arg(vl, RgbaColor *); break;
          }
          if (*fmt == 'F') {
            fg = cc;
          } else {
            bg = cc;
            if (rgbPtr)
              bgRgb = *rgbPtr;
          }
          fmt++;
          break;
        }
        case 'L':
          if (fmt[1] == '\0')
            goto done;
          fmt++;
          if (*fmt == 'p') {
            link = va_arg(vl, int);
          } else {
            link = *fmt;
          }
          break;

        case 'f': f = va_arg(vl, LinkFunction *); break;

        case 'h': h = va_arg(vl, LinkFunction *); break;

        case 'D': {
          unsigned int v = va_arg(vl, unsigned int);
          data = (uint32_t)v;
          break;
        }
        case '%': strcpy(buf, "%"); break;
        }
      } else {
        utf8_iterator it2(fmt), it1 = it2++;
        strncpy(buf, fmt, it2 - it1);
        buf[it2 - it1] = '\0';
      }

      for (utf8_iterator it(buf); *it; ++it) {
        for (size_t i = 0; i < canvas->GetBitmapFont()->GetWidth(*it); i++) {
          if (c >= MAX_COLS)
            goto done;
          text[r][c] = (i == 0) ? *it : ' ';
          meta[r][c].fg = fg;
          meta[r][c].bg = bg;
          meta[r][c].bgRgb = bgRgb;
          meta[r][c].link = link;
          meta[r][c].data = data;
          meta[r][c].f = f;
          meta[r][c].h = h;
          c++;
        }
      }

      utf8_iterator it(fmt);
      it++;
      fmt = it.ptr();
    }
    while (c < MAX_COLS) {
      meta[r][c].fg = fg;
      meta[r][c].bg = bg;
      meta[r][c].bgRgb = bgRgb;
      c++;
    }

  done:
    va_end(vl);
  }

  void TextWindow::Show() {
    if (SS.GW.pending.operation == GraphicsWindow::Pending::NONE) {
      SS.GW.ClearPending(/*scheduleShowTW=*/false);
    }

    SS.GW.GroupSelection();
    auto const &gs = SS.GW.gs;

    // Make sure these tests agree with test used to draw indicator line on
    // main list of groups screen.
    if (SS.GW.pending.description) {
      // A pending operation (that must be completed with the mouse in
      // the graphics window) will preempt our usual display.
      HideEditControl();
      ShowHeader(false);
      Printf(false, "");
      Printf(false, "%s", SS.GW.pending.description);
      Printf(true, "%Fl%f%Ll(cancel operation)%E", &TextWindow::ScreenUnselectAll);
    } else if ((gs.n > 0 || gs.constraints > 0) && shown.screen != Screen::PASTE_TRANSFORMED) {
      if (edit.meaning != Edit::TTF_TEXT)
        HideEditControl();
      ShowHeader(false);
      DescribeSelection();
    } else {
      if (edit.meaning == Edit::TTF_TEXT)
        HideEditControl();
      ShowHeader(true);
      switch (shown.screen) {
      default:
        shown.screen = Screen::LIST_OF_GROUPS;
        // fall through
      case Screen::LIST_OF_GROUPS: ShowListOfGroups(); break;
      case Screen::GROUP_INFO: ShowGroupInfo(); break;
      case Screen::GROUP_SOLVE_INFO: ShowGroupSolveInfo(); break;
      case Screen::CONFIGURATION: ShowConfiguration(); break;
      case Screen::STEP_DIMENSION: ShowStepDimension(); break;
      case Screen::LIST_OF_STYLES: ShowListOfStyles(); break;
      case Screen::STYLE_INFO: ShowStyleInfo(); break;
      case Screen::PASTE_TRANSFORMED: ShowPasteTransformed(); break;
      case Screen::EDIT_VIEW: ShowEditView(); break;
      case Screen::TANGENT_ARC: ShowTangentArc(); break;
      }
    }
    Printf(false, "");

    // Make sure there's room for the color picker
    if (editControl.colorPicker.show) {
      int pickerHeight = 25;
      int halfRow = editControl.halfRow;
      if (top[rows - 1] - halfRow < pickerHeight && rows < MAX_ROWS) {
        rows++;
        top[rows - 1] = halfRow + pickerHeight;
      }
    }

    if (window)
      Resize();
  }

  void TextWindow::Resize() {
    double width, height;
//    window->GetContentSize(&width, &height);

    halfRows = (int)height / (LINE_HEIGHT / 2);

    int bottom = top[rows - 1] + 2;
    scrollPos = std::min(scrollPos, bottom - halfRows);
    scrollPos = std::max(scrollPos, 0);

    window->ConfigureScrollbar(0, top[rows - 1] + 1, halfRows);
    window->SetScrollbarPosition(scrollPos);
    window->SetScrollbarVisible(top[rows - 1] + 1 > halfRows);
    window->Invalidate();
  }

  void TextWindow::DrawOrHitTestIcons(UiCanvas *uiCanvas, TextWindow::DrawOrHitHow how, double mx,
                                      double my) {
    double width, height;
//    window->GetContentSize(&width, &height);

    int x = 20, y = 33 + LINE_HEIGHT;
    y -= scrollPos * (LINE_HEIGHT / 2);

    if (how == PAINT) {
      int top = y - 28, bot = y + 4;
      uiCanvas->DrawRect(0, (int)width, top, bot,
                         /*fillColor=*/{30, 30, 30, 255}, /*outlineColor=*/{});
    }

    Button *oldHovered = hoveredButton;
    if (how != PAINT) {
      hoveredButton = NULL;
    }

    double hoveredX, hoveredY;
    for (Button *button : buttons) {
      if (how == PAINT) {
        button->Draw(uiCanvas, x, y, (button == hoveredButton));
      } else if (mx > x - 2 && mx < x + 26 && my < y + 2 && my > y - 26) {
        hoveredButton = button;
        hoveredX = x - 2;
        hoveredY = y - 26;
        if (how == CLICK) {
          button->Click();
        }
      }

      x += button->AdvanceWidth();
    }

    if (how != PAINT && hoveredButton != oldHovered) {
      if (hoveredButton == NULL) {
        window->SetTooltip("", 0, 0, 0, 0);
      } else {
        window->SetTooltip(hoveredButton->Tooltip(), hoveredX, hoveredY, 28, 28);
      }
      window->Invalidate();
    }
  }

  //----------------------------------------------------------------------------
  // Given (x, y, z) = (h, s, v) in [0,6), [0,1], [0,1], return (x, y, z) =
  // (r, g, b) all in [0, 1].
  //----------------------------------------------------------------------------
  Vector TextWindow::HsvToRgb(Vector hsv) {
    if (hsv.x >= 6)
      hsv.x -= 6;

    Vector rgb;
    double hmod2 = hsv.x;
    while (hmod2 >= 2)
      hmod2 -= 2;
    double x = (1 - std::fabs(hmod2 - 1));
    if (hsv.x < 1) {
      rgb = Vector::From(1, x, 0);
    } else if (hsv.x < 2) {
      rgb = Vector::From(x, 1, 0);
    } else if (hsv.x < 3) {
      rgb = Vector::From(0, 1, x);
    } else if (hsv.x < 4) {
      rgb = Vector::From(0, x, 1);
    } else if (hsv.x < 5) {
      rgb = Vector::From(x, 0, 1);
    } else {
      rgb = Vector::From(1, 0, x);
    }
    double c = hsv.y * hsv.z;
    double m = 1 - hsv.z;
    rgb = rgb.ScaledBy(c);
    rgb = rgb.Plus(Vector::From(m, m, m));

    return rgb;
  }

  std::shared_ptr<Pixmap> TextWindow::HsvPattern2d(int w, int h) {
    std::shared_ptr<Pixmap> pixmap = Pixmap::Create(Pixmap::Format::RGB, w, h);
    for (size_t j = 0; j < pixmap->height; j++) {
      size_t p = pixmap->stride * j;
      for (size_t i = 0; i < pixmap->width; i++) {
        Vector hsv = Vector::From(6.0 * i / (pixmap->width - 1), 1.0 * j / (pixmap->height - 1), 1);
        Vector rgb = HsvToRgb(hsv);
        rgb = rgb.ScaledBy(255);
        pixmap->data[p++] = (uint8_t)rgb.x;
        pixmap->data[p++] = (uint8_t)rgb.y;
        pixmap->data[p++] = (uint8_t)rgb.z;
      }
    }
    return pixmap;
  }

  std::shared_ptr<Pixmap> TextWindow::HsvPattern1d(double hue, double sat, int w, int h) {
    std::shared_ptr<Pixmap> pixmap = Pixmap::Create(Pixmap::Format::RGB, w, h);
    for (size_t i = 0; i < pixmap->height; i++) {
      size_t p = i * pixmap->stride;
      for (size_t j = 0; j < pixmap->width; j++) {
        Vector hsv = Vector::From(6 * hue, sat, 1.0 * (pixmap->width - 1 - j) / pixmap->width);
        Vector rgb = HsvToRgb(hsv);
        rgb = rgb.ScaledBy(255);
        pixmap->data[p++] = (uint8_t)rgb.x;
        pixmap->data[p++] = (uint8_t)rgb.y;
        pixmap->data[p++] = (uint8_t)rgb.z;
      }
    }
    return pixmap;
  }

  void TextWindow::ColorPickerDone() {
    RgbaColor rgb = editControl.colorPicker.rgb;
    EditControlDone(ssprintf("%.2f, %.2f, %.3f", rgb.redF(), rgb.greenF(), rgb.blueF()));
  }

  bool TextWindow::DrawOrHitTestColorPicker(UiCanvas *uiCanvas, DrawOrHitHow how, bool leftDown,
                                            double x, double y) {
    using Platform::Window;

    bool mousePointerAsHand = false;

    if (how == HOVER && !leftDown) {
      editControl.colorPicker.picker1dActive = false;
      editControl.colorPicker.picker2dActive = false;
    }

    if (!editControl.colorPicker.show)
      return false;
    if (how == CLICK || (how == HOVER && leftDown))
      window->Invalidate();

    static const RgbaColor BaseColor[12] = {
        RGBi(255, 0, 0),   RGBi(0, 255, 0),   RGBi(0, 0, 255),

        RGBi(0, 255, 255), RGBi(255, 0, 255), RGBi(255, 255, 0),

        RGBi(255, 127, 0), RGBi(255, 0, 127), RGBi(0, 255, 127),
        RGBi(127, 255, 0), RGBi(127, 0, 255), RGBi(0, 127, 255),
    };

    double width, height;
//    window->GetContentSize(&width, &height);

    int px = LEFT_MARGIN + CHAR_WIDTH_ * editControl.col;
    int py = (editControl.halfRow - SS.TW.scrollPos) * (LINE_HEIGHT / 2);

    py += LINE_HEIGHT + 5;

    static const int WIDTH = 16, HEIGHT = 12;
    static const int PITCH = 18, SIZE = 15;

    px = std::min(px, (int)width - (WIDTH * PITCH + 40));

    int pxm = px + WIDTH * PITCH + 11, pym = py + HEIGHT * PITCH + 7;

    int bw = 6;
    if (how == PAINT) {
      uiCanvas->DrawRect(px, pxm + bw, py, pym + bw,
                         /*fillColor=*/{50, 50, 50, 255},
                         /*outlineColor=*/{},
                         /*zIndex=*/1);
      uiCanvas->DrawRect(px + (bw / 2), pxm + (bw / 2), py + (bw / 2), pym + (bw / 2),
                         /*fillColor=*/{0, 0, 0, 255},
                         /*outlineColor=*/{},
                         /*zIndex=*/1);
    } else {
      if (x < px || x > pxm + (bw / 2) || y < py || y > pym + (bw / 2)) {
        return false;
      }
    }
    px += (bw / 2);
    py += (bw / 2);

    int i, j;
    for (i = 0; i < WIDTH / 2; i++) {
      for (j = 0; j < HEIGHT; j++) {
        Vector rgb;
        RgbaColor d;
        if (i == 0 && j < 8) {
          d = SS.modelColor[j];
          rgb = Vector::From(d.redF(), d.greenF(), d.blueF());
        } else if (i == 0) {
          double a = (j - 8.0) / 3.0;
          rgb = Vector::From(a, a, a);
        } else {
          d = BaseColor[j];
          rgb = Vector::From(d.redF(), d.greenF(), d.blueF());
          if (i >= 2 && i <= 4) {
            double a = (i == 2) ? 0.2 : (i == 3) ? 0.3 : 0.4;
            rgb = rgb.Plus(Vector::From(a, a, a));
          }
          if (i >= 5 && i <= 7) {
            double a = (i == 5) ? 0.7 : (i == 6) ? 0.4 : 0.18;
            rgb = rgb.ScaledBy(a);
          }
        }

        rgb = rgb.ClampWithin(0, 1);
        int sx = px + 5 + PITCH * (i + 8) + 4, sy = py + 5 + PITCH * j;

        if (how == PAINT) {
          uiCanvas->DrawRect(sx, sx + SIZE, sy, sy + SIZE,
                             /*fillColor=*/RGBf(rgb.x, rgb.y, rgb.z),
                             /*outlineColor=*/{},
                             /*zIndex=*/2);
        } else if (how == CLICK) {
          if (x >= sx && x <= sx + SIZE && y >= sy && y <= sy + SIZE) {
            editControl.colorPicker.rgb = RGBf(rgb.x, rgb.y, rgb.z);
            ColorPickerDone();
          }
        } else if (how == HOVER) {
          if (x >= sx && x <= sx + SIZE && y >= sy && y <= sy + SIZE) {
            mousePointerAsHand = true;
          }
        }
      }
    }

    int hxm, hym;
    int hx = px + 5, hy = py + 5;
    hxm = hx + PITCH * 7 + SIZE;
    hym = hy + PITCH * 2 + SIZE;
    if (how == PAINT) {
      uiCanvas->DrawRect(hx, hxm, hy, hym,
                         /*fillColor=*/editControl.colorPicker.rgb,
                         /*outlineColor=*/{},
                         /*zIndex=*/2);
    } else if (how == CLICK) {
      if (x >= hx && x <= hxm && y >= hy && y <= hym) {
        ColorPickerDone();
      }
    } else if (how == HOVER) {
      if (x >= hx && x <= hxm && y >= hy && y <= hym) {
        mousePointerAsHand = true;
      }
    }

    hy += PITCH * 3;

    hxm = hx + PITCH * 7 + SIZE;
    hym = hy + PITCH * 1 + SIZE;
    // The one-dimensional thing to pick the color's value
    if (how == PAINT) {
      uiCanvas->DrawPixmap(
          HsvPattern1d(editControl.colorPicker.h, editControl.colorPicker.s, hxm - hx, hym - hy),
          hx, hy, /*zIndex=*/2);

      int cx = hx + (int)((hxm - hx) * (1.0 - editControl.colorPicker.v));
      uiCanvas->DrawLine(cx, hy, cx, hym,
                         /*fillColor=*/{0, 0, 0, 255},
                         /*outlineColor=*/{},
                         /*zIndex=*/3);
    } else if (how == CLICK ||
               (how == HOVER && leftDown && editControl.colorPicker.picker1dActive)) {
      if (x >= hx && x <= hxm && y >= hy && y <= hym) {
        editControl.colorPicker.v = 1 - (x - hx) / (hxm - hx);

        Vector rgb = HsvToRgb(Vector::From(6 * editControl.colorPicker.h, editControl.colorPicker.s,
                                           editControl.colorPicker.v));
        editControl.colorPicker.rgb = RGBf(rgb.x, rgb.y, rgb.z);

        editControl.colorPicker.picker1dActive = true;
      }
    }
    // and advance our vertical position
    hy += PITCH * 2;

    hxm = hx + PITCH * 7 + SIZE;
    hym = hy + PITCH * 6 + SIZE;
    // Two-dimensional thing to pick a color by hue and saturation
    if (how == PAINT) {
      uiCanvas->DrawPixmap(HsvPattern2d(hxm - hx, hym - hy), hx, hy,
                           /*zIndex=*/2);

      int cx = hx + (int)((hxm - hx) * editControl.colorPicker.h),
          cy = hy + (int)((hym - hy) * editControl.colorPicker.s);
      uiCanvas->DrawLine(cx - 5, cy, cx + 5, cy,
                         /*fillColor=*/{255, 255, 255, 255},
                         /*outlineColor=*/{},
                         /*zIndex=*/3);
      uiCanvas->DrawLine(cx, cy - 5, cx, cy + 5,
                         /*fillColor=*/{255, 255, 255, 255},
                         /*outlineColor=*/{},
                         /*zIndex=*/3);
    } else if (how == CLICK ||
               (how == HOVER && leftDown && editControl.colorPicker.picker2dActive)) {
      if (x >= hx && x <= hxm && y >= hy && y <= hym) {
        double h = (x - hx) / (hxm - hx), s = (y - hy) / (hym - hy);
        editControl.colorPicker.h = h;
        editControl.colorPicker.s = s;

        Vector rgb = HsvToRgb(Vector::From(6 * editControl.colorPicker.h, editControl.colorPicker.s,
                                           editControl.colorPicker.v));
        editControl.colorPicker.rgb = RGBf(rgb.x, rgb.y, rgb.z);

        editControl.colorPicker.picker2dActive = true;
      }
    }

    window->SetCursor(mousePointerAsHand ? Window::Cursor::HAND : Window::Cursor::POINTER);
    return true;
  }

  void TextWindow::Paint() {
    if (!canvas)
      return;

    double width, height;
//    window->GetContentSize(&width, &height);
    if (halfRows != (int)height / (LINE_HEIGHT / 2))
      Resize();

    Camera camera = {};
    camera.width = width;
    camera.height = height;
    camera.pixelRatio = window->GetDevicePixelRatio();
    camera.gridFit = (window->GetDevicePixelRatio() == 1);
    camera.LoadIdentity();
    camera.offset.x = -camera.width / 2.0;
    camera.offset.y = -camera.height / 2.0;

    Lighting lighting = {};
    lighting.backgroundColor = RGBi(0, 0, 0);

    canvas->SetLighting(lighting);
    canvas->SetCamera(camera);
    canvas->StartFrame();

    UiCanvas uiCanvas = {};
    uiCanvas.canvas = canvas;
    uiCanvas.flip = true;

    int r, c, a;
    for (a = 0; a < 2; a++) {
      for (r = 0; r < rows; r++) {
        int ltop = top[r];
        if (ltop < (scrollPos - 1))
          continue;
        if (ltop > scrollPos + halfRows)
          break;

        for (c = 0; c < std::min(((int)width / CHAR_WIDTH_) + 1, (int)MAX_COLS); c++) {
          int x = LEFT_MARGIN + c * CHAR_WIDTH_;
          int y = (ltop - scrollPos) * (LINE_HEIGHT / 2) + 4;

          int fg = meta[r][c].fg;
          int bg = meta[r][c].bg;

          // On the first pass, all the background quads; on the next
          // pass, all the foreground (i.e., font) quads.
          if (a == 0) {
            RgbaColor bgRgb = meta[r][c].bgRgb;
            int bh = LINE_HEIGHT, adj = 0;
            if (bg == 'z') {
              bh = CHAR_HEIGHT;
              adj += 2;
            } else {
              bgRgb = RgbaColor::FromFloat(bgColorTable[bg * 3 + 0], bgColorTable[bg * 3 + 1],
                                           bgColorTable[bg * 3 + 2]);
            }

            if (bg != 'd') {
              // Move the quad down a bit, so that the descenders
              // still have the correct background.
              uiCanvas.DrawRect(x, x + CHAR_WIDTH_, y + adj, y + adj + bh,
                                /*fillColor=*/bgRgb, /*outlineColor=*/{});
            }
          } else if (a == 1) {
            RgbaColor fgRgb = RgbaColor::FromFloat(
                fgColorTable[fg * 3 + 0], fgColorTable[fg * 3 + 1], fgColorTable[fg * 3 + 2]);
            if (text[r][c] != ' ') {
              uiCanvas.DrawBitmapChar(text[r][c], x, y + CHAR_HEIGHT, fgRgb);
            }

            // If this is a link and it's hovered, then draw the
            // underline
            if (meta[r][c].link && meta[r][c].link != 'n' && (r == hoveredRow && c == hoveredCol)) {
              int cs = c, cf = c;
              while (cs >= 0 && meta[r][cs].link && meta[r][cs].f == meta[r][c].f &&
                     meta[r][cs].data == meta[r][c].data) {
                cs--;
              }
              cs++;

              while (meta[r][cf].link && meta[r][cf].f == meta[r][c].f &&
                     meta[r][cf].data == meta[r][c].data) {
                cf++;
              }

              // But don't underline checkboxes or radio buttons
              while (((text[r][cs] >= 0xe000 && text[r][cs] <= 0xefff) || text[r][cs] == ' ') &&
                     cs < cf) {
                cs++;
              }

              // Always use the color of the rightmost character
              // in the link, so that underline is consistent color
              fg = meta[r][cf - 1].fg;
              fgRgb = RgbaColor::FromFloat(fgColorTable[fg * 3 + 0], fgColorTable[fg * 3 + 1],
                                           fgColorTable[fg * 3 + 2]);
              int yp = y + CHAR_HEIGHT;
              uiCanvas.DrawLine(LEFT_MARGIN + cs * CHAR_WIDTH_, yp, LEFT_MARGIN + cf * CHAR_WIDTH_,
                                yp, fgRgb);
            }
          }
        }
      }
    }

    // The line to indicate the column of radio buttons that indicates the
    // active group.
    SS.GW.GroupSelection();
    auto const &gs = SS.GW.gs;
    // Make sure this test agrees with test to determine which screen is drawn
    if (!SS.GW.pending.description && gs.n == 0 && gs.constraints == 0 &&
        shown.screen == Screen::LIST_OF_GROUPS) {
      int x = 29, y = 70 + LINE_HEIGHT;
      y -= scrollPos * (LINE_HEIGHT / 2);

      RgbaColor color = RgbaColor::FromFloat(fgColorTable['t' * 3 + 0], fgColorTable['t' * 3 + 1],
                                             fgColorTable['t' * 3 + 2]);
      uiCanvas.DrawLine(x, y, x, y + 40, color);
    }

    // The header has some icons that are drawn separately from the text
    DrawOrHitTestIcons(&uiCanvas, PAINT, 0, 0);

    // And we may show a color picker for certain editable fields
    DrawOrHitTestColorPicker(&uiCanvas, PAINT, false, 0, 0);

    canvas->FlushFrame();
    canvas->FinishFrame();
    canvas->Clear();
  }

  void TextWindow::MouseEvent(bool leftClick, bool leftDown, double x, double y) {
    using Platform::Window;

//    if (SS.TW.window->IsEditorVisible() || SS.GW.window->IsEditorVisible()) {
      if (DrawOrHitTestColorPicker(NULL, leftClick ? CLICK : HOVER, leftDown, x, y)) {
        return;
      }

      if (leftClick) {
        HideEditControl();
//        SS.GW.window->HideEditor();
      } else {
        window->SetCursor(Window::Cursor::POINTER);
      }
      return;
//    }

    DrawOrHitTestIcons(NULL, leftClick ? CLICK : HOVER, x, y);

    GraphicsWindow::Selection ps = SS.GW.hover;
    SS.GW.hover.Clear();

    int prevHoveredRow = hoveredRow, prevHoveredCol = hoveredCol;
    hoveredRow = 0;
    hoveredCol = 0;

    // Find the corresponding character in the text buffer
    int c = (int)((x - LEFT_MARGIN) / CHAR_WIDTH_);
    int hh = (LINE_HEIGHT) / 2;
    y += scrollPos * hh;
    int r;
    for (r = 0; r < rows; r++) {
      if (y >= top[r] * hh && y <= (top[r] + 2) * hh) {
        break;
      }
    }
    if (r >= 0 && c >= 0 && r < rows && c < MAX_COLS) {
      window->SetCursor(Window::Cursor::POINTER);

      hoveredRow = r;
      hoveredCol = c;

      const auto &item = meta[r][c];
      if (leftClick) {
        if (item.link && item.f) {
          (item.f)(item.link, item.data);
          Show();
          SS.GW.Invalidate();
        }
      } else {
        if (item.link) {
          window->SetCursor(Window::Cursor::HAND);
          if (item.h) {
            (item.h)(item.link, item.data);
          }
        } else {
          window->SetCursor(Window::Cursor::POINTER);
        }
      }
    }

    if ((!ps.Equals(&(SS.GW.hover))) || prevHoveredRow != hoveredRow ||
        prevHoveredCol != hoveredCol) {
      SS.GW.Invalidate();
      window->Invalidate();
    }
  }

  void TextWindow::MouseLeave() {
    hoveredButton = NULL;
    hoveredRow = 0;
    hoveredCol = 0;
    window->Invalidate();
  }

  void TextWindow::ScrollbarEvent(double newPos) {
    if (window->IsEditorVisible()) {
      // An edit field is active. Do not move the scrollbar.
      return;
    }

    int bottom = top[rows - 1] + 2;
    newPos = std::min((int)newPos, bottom - halfRows);
    newPos = std::max((int)newPos, 0);
    if (newPos != scrollPos) {
      scrollPos = (int)newPos;
      window->SetScrollbarPosition(scrollPos);
      window->Invalidate();
    }
  }

// formerly describescreen.cpp
//-----------------------------------------------------------------------------
// The screens when an entity is selected, that show some description of it--
// endpoints of the lines, diameter of the circle, etc.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"

  void TextWindow::ScreenUnselectAll(int link, uint32_t v) {
    SolveSpaceUI::MenuEdit(Command::UNSELECT_ALL);
  }

  void TextWindow::ScreenEditTtfText(int link, uint32_t v) {
    hRequest hr = {v};
    Request *r = SK.GetRequest(hr);

    SS.TW.ShowEditControl(10, r->str);
    SS.TW.edit.meaning = Edit::TTF_TEXT;
    SS.TW.edit.request = hr;
  }

  void TextWindow::ScreenSetTtfFont(int link, uint32_t v) {
    int i = (int)v;
    if (i < 0)
      return;
    if (i >= SS.fonts.l.n)
      return;

    SS.GW.GroupSelection();
    auto const &gs = SS.GW.gs;
    if (gs.entities != 1 || gs.n != 1)
      return;

    Entity *e = SK.entity.FindByIdNoOops(gs.entity[0]);
    if (!e || e->type != Entity::Type::TTF_TEXT || !e->h.isFromRequest())
      return;

    Request *r = SK.request.FindByIdNoOops(e->h.request());
    if (!r)
      return;

    SS.UndoRemember();
    r->font = SS.fonts.l[i].FontFileBaseName();
    SS.MarkGroupDirty(r->group);
    SS.ScheduleShowTW();
  }

  void TextWindow::ScreenConstraintToggleReference(int link, uint32_t v) {
    hConstraint hc = {v};
    Constraint *c = SK.GetConstraint(hc);

    SS.UndoRemember();
    c->reference = !c->reference;

    SS.MarkGroupDirty(c->group);
    SS.ScheduleShowTW();
  }

  void TextWindow::ScreenConstraintShowAsRadius(int link, uint32_t v) {
    hConstraint hc = {v};
    Constraint *c = SK.GetConstraint(hc);

    SS.UndoRemember();
    c->other = !c->other;

    SS.ScheduleShowTW();
  }

  void TextWindow::DescribeSelection() {
    Printf(false, "");

    auto const &gs = SS.GW.gs;
    if (gs.n == 1 && (gs.points == 1 || gs.entities == 1)) {
      Entity *e = SK.GetEntity(gs.points == 1 ? gs.point[0] : gs.entity[0]);
      Vector p = Vector(0, 0, 0);

#define COSTR_NO_LINK(p) \
  SS.MmToString((p).x).c_str(), SS.MmToString((p).y).c_str(), SS.MmToString((p).z).c_str()
#define PT_AS_STR_NO_LINK "(%Fi%s%Fd, %Fi%s%Fd, %Fi%s%Fd)"
#define PT_AS_NUM "(%Fi%3%Fd, %Fi%3%Fd, %Fi%3%Fd)"
#define COSTR(e, p) \
  e->h, (&TextWindow::ScreenSelectEntity), (&TextWindow::ScreenHoverEntity), COSTR_NO_LINK(p)
#define PT_AS_STR "%Ll%D%f%h" PT_AS_STR_NO_LINK "%E"
#define CO_LINK(e, p) \
  e->h, (&TextWindow::ScreenSelectEntity), (&TextWindow::ScreenHoverEntity), CO(p)
#define PT_AS_NUM_LINK "%Ll%D%f%h" PT_AS_NUM "%E"

      switch (e->type) {
      case Entity::Type::POINT_IN_3D:
      case Entity::Type::POINT_IN_2D:
      case Entity::Type::POINT_N_TRANS:
      case Entity::Type::POINT_N_ROT_TRANS:
      case Entity::Type::POINT_N_COPY:
      case Entity::Type::POINT_N_ROT_AA:
      case Entity::Type::POINT_N_ROT_AXIS_TRANS:
        p = e->PointGetNum();
        Printf(false, "%FtPOINT%E at " PT_AS_STR, COSTR(e, p));
        break;

      case Entity::Type::NORMAL_IN_3D:
      case Entity::Type::NORMAL_IN_2D:
      case Entity::Type::NORMAL_N_COPY:
      case Entity::Type::NORMAL_N_ROT:
      case Entity::Type::NORMAL_N_ROT_AA: {
        Quaternion q = e->NormalGetNum();
        p = q.RotationN();
        Printf(false, "%FtNORMAL / COORDINATE SYSTEM%E");
        Printf(true, "  basis n = " PT_AS_NUM, CO(p));
        p = q.RotationU();
        Printf(false, "        u = " PT_AS_NUM, CO(p));
        p = q.RotationV();
        Printf(false, "        v = " PT_AS_NUM, CO(p));
        break;
      }
      case Entity::Type::WORKPLANE: {
        p = SK.GetEntity(e->point[0])->PointGetNum();
        Printf(false, "%FtWORKPLANE%E");
        Printf(true, "   origin = " PT_AS_STR, COSTR(SK.GetEntity(e->point[0]), p));
        Quaternion q = e->Normal()->NormalGetNum();
        p = q.RotationN();
        Printf(true, "   normal = " PT_AS_NUM_LINK, CO_LINK(e->Normal(), p));
        break;
      }
      case Entity::Type::LINE_SEGMENT: {
        Vector p0 = SK.GetEntity(e->point[0])->PointGetNum();
        p = p0;
        Printf(false, "%FtLINE SEGMENT%E");
        Printf(true, "   thru " PT_AS_STR, COSTR(SK.GetEntity(e->point[0]), p));
        Vector p1 = SK.GetEntity(e->point[1])->PointGetNum();
        p = p1;
        Printf(false, "        " PT_AS_STR, COSTR(SK.GetEntity(e->point[1]), p));
        Printf(true, "   len = %Fi%s%E", SS.MmToString((p1.Minus(p0).Magnitude())).c_str());
        break;
      }
      case Entity::Type::CUBIC_PERIODIC:
      case Entity::Type::CUBIC:
        int pts;
        if (e->type == Entity::Type::CUBIC_PERIODIC) {
          Printf(false, "%FtPERIODIC C2 CUBIC SPLINE%E");
          pts = (3 + e->extraPoints);
        } else if (e->extraPoints > 0) {
          Printf(false, "%FtINTERPOLATING C2 CUBIC SPLINE%E");
          pts = (4 + e->extraPoints);
        } else {
          Printf(false, "%FtCUBIC BEZIER CURVE%E");
          pts = 4;
        }
        for (int i = 0; i < pts; i++) {
          p = SK.GetEntity(e->point[i])->PointGetNum();
          Printf((i == 0), "   p%d = " PT_AS_STR, i, COSTR(SK.GetEntity(e->point[i]), p));
        }
        break;

      case Entity::Type::ARC_OF_CIRCLE: {
        Printf(false, "%FtARC OF A CIRCLE%E");
        p = SK.GetEntity(e->point[0])->PointGetNum();
        Printf(true, "     center = " PT_AS_STR, COSTR(SK.GetEntity(e->point[0]), p));
        p = SK.GetEntity(e->point[1])->PointGetNum();
        Printf(true, "  endpoints = " PT_AS_STR, COSTR(SK.GetEntity(e->point[1]), p));
        p = SK.GetEntity(e->point[2])->PointGetNum();
        Printf(false, "              " PT_AS_STR, COSTR(SK.GetEntity(e->point[2]), p));
        double r = e->CircleGetRadiusNum();
        Printf(true, "   diameter =  %Fi%s", SS.MmToString(r * 2).c_str());
        Printf(false, "     radius =  %Fi%s", SS.MmToString(r).c_str());
        double thetas, thetaf, dtheta;
        e->ArcGetAngles(&thetas, &thetaf, &dtheta);
        Printf(false, "    arc len =  %Fi%s", SS.MmToString(dtheta * r).c_str());
        break;
      }
      case Entity::Type::CIRCLE: {
        Printf(false, "%FtCIRCLE%E");
        p = SK.GetEntity(e->point[0])->PointGetNum();
        Printf(true, "        center = " PT_AS_STR, COSTR(SK.GetEntity(e->point[0]), p));
        double r = e->CircleGetRadiusNum();
        Printf(true, "      diameter =  %Fi%s", SS.MmToString(r * 2).c_str());
        Printf(false, "        radius =  %Fi%s", SS.MmToString(r).c_str());
        Printf(false, " circumference =  %Fi%s", SS.MmToString(2 * M_PI * r).c_str());
        break;
      }
      case Entity::Type::FACE_NORMAL_PT:
      case Entity::Type::FACE_XPROD:
      case Entity::Type::FACE_N_ROT_TRANS:
      case Entity::Type::FACE_N_ROT_AA:
      case Entity::Type::FACE_N_TRANS:
      case Entity::Type::FACE_ROT_NORMAL_PT:
      case Entity::Type::FACE_N_ROT_AXIS_TRANS:
        Printf(false, "%FtPLANE FACE%E");
        p = e->FaceGetNormalNum();
        Printf(true, "   normal = " PT_AS_NUM, CO(p));
        p = e->FaceGetPointNum();
        Printf(false, "     thru = " PT_AS_STR, COSTR(e, p));
        break;

      case Entity::Type::TTF_TEXT: {
        Printf(false, "%FtTRUETYPE FONT TEXT%E");
        Printf(true, "  font = '%Fi%s%E'", e->font.c_str());
        if (e->h.isFromRequest()) {
          Printf(false, "  text = '%Fi%s%E' %Fl%Ll%f%D[change]%E", e->str.c_str(),
                 &ScreenEditTtfText, e->h.request().v);
          Printf(true, "  select new font");
          SS.fonts.LoadAll();
          // Not using range-for here because we use i inside the output.
          for (int i = 0; i < SS.fonts.l.n; i++) {
            TtfFont *tf = &(SS.fonts.l[i]);
            if (e->font == tf->FontFileBaseName()) {
              Printf(false, "%Bp    %s", (i & 1) ? 'd' : 'a', tf->name.c_str());
            } else {
              Printf(false, "%Bp    %f%D%Fl%Ll%s%E%Bp", (i & 1) ? 'd' : 'a', &ScreenSetTtfFont, i,
                     tf->name.c_str(), (i & 1) ? 'd' : 'a');
            }
          }
        } else {
          Printf(false, "  text = '%Fi%s%E'", e->str.c_str());
        }
        break;
      }
      case Entity::Type::IMAGE: {
        Printf(false, "%FtIMAGE%E");
        Platform::Path relativePath = e->file.RelativeTo(SS.saveFile.Parent());
        if (relativePath.IsEmpty()) {
          Printf(true, "  file = '%Fi%s%E'", e->file.raw.c_str());
        } else {
          Printf(true, "  file = '%Fi%s%E'", relativePath.raw.c_str());
        }
        break;
      }

      default: Printf(true, "%Ft?? ENTITY%E"); break;
      }

      Printf(false, "");
      if (e->h.isFromRequest()) {
        Request *r = SK.GetRequest(e->h.request());
        if (e->h == r->h.entity(0)) {
          Printf(false, "%FtFROM REQUEST%E  %s", r->DescriptionString().c_str());
        } else {
          Printf(false, "%FtFROM REQUEST%E  %Fl%Ll%D%f%h%s%E", r->h.v,
                 (&TextWindow::ScreenSelectRequest), &(TextWindow::ScreenHoverRequest),
                 r->DescriptionString().c_str());
        }
      }
      Group *g = SK.GetGroup(e->group);
      Printf(false, "%FtIN GROUP%E      %Fl%Ll%D%f%s%E", g->h.v, (&TextWindow::ScreenSelectGroup),
             g->DescriptionString().c_str());
      if (e->workplane == Entity::FREE_IN_3D) {
        Printf(false, "%FtNOT LOCKED IN WORKPLANE%E");
      } else {
        Entity *w = SK.GetEntity(e->workplane);
        if (w->h.isFromRequest()) {
          Printf(false, "%FtIN WORKPLANE%E  %Fl%Ll%D%f%h%s%E", w->h.request().v,
                 (&TextWindow::ScreenSelectRequest), &(TextWindow::ScreenHoverRequest),
                 w->DescriptionString().c_str());
        } else {
          Printf(false, "%FtIN WORKPLANE%E  %Fl%Ll%D%f%h%s%E", w->h.group().v,
                 (&TextWindow::ScreenSelectGroup), (&TextWindow::ScreenHoverGroupWorkplane),
                 w->DescriptionString().c_str());
        }
      }
      if (e->IsStylable()) {
        if (e->style.v) {
          Style *s = Style::Get(e->style);
          Printf(false, "%FtIN STYLE%E      %Fl%Ll%D%f%s%E", s->h.v,
                 (&TextWindow::ScreenShowStyleInfo), s->DescriptionString().c_str());
        } else {
          Printf(false, "%FtIN STYLE%E      none");
        }
      }
      if (e->construction) {
        Printf(false, "%FtCONSTRUCTION");
      }

      std::vector<hConstraint> lhc = {};
      auto FindConstraints = [&](hEntity he) {
        for (const Constraint &c : SK.constraint) {
          if (!(c.ptA == he || c.ptB == he || c.entityA == he || c.entityB == he ||
                c.entityC == he || c.entityD == he))
            continue;
          lhc.push_back(c.h);
        }
      };
      FindConstraints(e->h);
      if (!e->IsPoint()) {
        for (int i = 0; i < MAX_POINTS_IN_ENTITY; i++) {
          if (e->point[i].v == 0)
            break;
          FindConstraints(e->point[i]);
        }
      }

      std::sort(lhc.begin(), lhc.end());
      lhc.erase(std::unique(lhc.begin(), lhc.end()), lhc.end());

      auto ListConstraints = [&](bool reference) {
        bool first = true;
        int a = 0;
        for (hConstraint hc : lhc) {
          Constraint *c = SK.GetConstraint(hc);
          if (c->reference != reference)
            continue;
          if (first) {
            first = false;
            if (reference) {
              Printf(true, "%FtMEASURED BY:%E");
            } else {
              Printf(true, "%FtCONSTRAINED BY:%E");
            }
          }
          Printf(false, "%Bp   %Fl%Ll%D%f%h%s%E", (a & 1) ? 'd' : 'a', c->h.v,
                 (&TextWindow::ScreenSelectConstraint), (&TextWindow::ScreenHoverConstraint),
                 c->DescriptionString().c_str());
          a++;
        }
      };
      ListConstraints(/*reference=*/false);
      ListConstraints(/*reference=*/true);
    } else if (gs.n == 2 && gs.points == 2) {
      Printf(false, "%FtTWO POINTS");
      Vector p0 = SK.GetEntity(gs.point[0])->PointGetNum();
      Printf(true, "   at " PT_AS_STR, COSTR(SK.GetEntity(gs.point[0]), p0));
      Vector p1 = SK.GetEntity(gs.point[1])->PointGetNum();
      Printf(false, "      " PT_AS_STR, COSTR(SK.GetEntity(gs.point[1]), p1));
      Vector dv = p1.Minus(p0);
      Printf(true, "  d = %Fi%s", SS.MmToString(dv.Magnitude()).c_str());
      Printf(false, "  d(x, y, z) = " PT_AS_STR_NO_LINK, COSTR_NO_LINK(dv));
    } else if (gs.n == 2 && gs.points == 1 && gs.circlesOrArcs == 1) {
      Entity *ec = SK.GetEntity(gs.entity[0]);
      if (ec->type == Entity::Type::CIRCLE) {
        Printf(false, "%FtPOINT AND A CIRCLE");
      } else if (ec->type == Entity::Type::ARC_OF_CIRCLE) {
        Printf(false, "%FtPOINT AND AN ARC");
      } else
        ssassert(false, "Unexpected entity type");
      Vector p = SK.GetEntity(gs.point[0])->PointGetNum();
      Printf(true, "        pt at " PT_AS_STR, COSTR(SK.GetEntity(gs.point[0]), p));
      Vector c = SK.GetEntity(ec->point[0])->PointGetNum();
      Printf(true, "     center = " PT_AS_STR, COSTR(SK.GetEntity(ec->point[0]), c));
      double r = ec->CircleGetRadiusNum();
      Printf(false, "   diameter =  %Fi%s", SS.MmToString(r * 2).c_str());
      Printf(false, "     radius =  %Fi%s", SS.MmToString(r).c_str());
      double d = (p.Minus(c)).Magnitude() - r;
      Printf(true, "   distance = %Fi%s", SS.MmToString(d).c_str());
    } else if (gs.n == 2 && gs.faces == 1 && gs.points == 1) {
      Printf(false, "%FtA POINT AND A PLANE FACE");
      Vector pt = SK.GetEntity(gs.point[0])->PointGetNum();
      Printf(true, "        point = " PT_AS_STR, COSTR(SK.GetEntity(gs.point[0]), pt));
      Vector n = SK.GetEntity(gs.face[0])->FaceGetNormalNum();
      Printf(true, " plane normal = " PT_AS_NUM, CO(n));
      Vector pl = SK.GetEntity(gs.face[0])->FaceGetPointNum();
      Printf(false, "   plane thru = " PT_AS_STR, COSTR(SK.GetEntity(gs.face[0]), pl));
      double dd = n.Dot(pl) - n.Dot(pt);
      Printf(true, "     distance = %Fi%s", SS.MmToString(dd).c_str());
    } else if (gs.n == 3 && gs.points == 2 && gs.vectors == 1) {
      Printf(false, "%FtTWO POINTS AND A VECTOR");
      Vector p0 = SK.GetEntity(gs.point[0])->PointGetNum();
      Printf(true, "  pointA = " PT_AS_STR, COSTR(SK.GetEntity(gs.point[0]), p0));
      Vector p1 = SK.GetEntity(gs.point[1])->PointGetNum();
      Printf(false, "  pointB = " PT_AS_STR, COSTR(SK.GetEntity(gs.point[1]), p1));
      Vector v = SK.GetEntity(gs.vector[0])->VectorGetNum();
      v = v.WithMagnitude(1);
      Printf(true, "  vector = " PT_AS_NUM_LINK, CO_LINK(SK.GetEntity(gs.vector[0]), v));
      double d = (p1.Minus(p0)).Dot(v);
      Printf(true, "  proj_d = %Fi%s", SS.MmToString(d).c_str());
    } else if (gs.n == 2 && gs.lineSegments == 1 && gs.points == 1) {
      Entity *ln = SK.GetEntity(gs.entity[0]);
      Vector lp0 = SK.GetEntity(ln->point[0])->PointGetNum(),
             lp1 = SK.GetEntity(ln->point[1])->PointGetNum();
      Printf(false, "%FtLINE SEGMENT AND POINT%E");
      Printf(true, "   ln thru " PT_AS_STR, COSTR(SK.GetEntity(ln->point[0]), lp0));
      Printf(false, "           " PT_AS_STR, COSTR(SK.GetEntity(ln->point[1]), lp1));
      Entity *p = SK.GetEntity(gs.point[0]);
      Vector pp = p->PointGetNum();
      Printf(true, "     point " PT_AS_STR, COSTR(p, pp));
      Printf(true, " pt-ln distance = %Fi%s%E",
             SS.MmToString(pp.DistanceToLine(lp0, lp1.Minus(lp0))).c_str());
      hEntity wrkpl = SS.GW.ActiveWorkplane();
      if (wrkpl != Entity::FREE_IN_3D && !(p->workplane == wrkpl && ln->workplane == wrkpl)) {
        Vector ppw = pp.ProjectInto(wrkpl);
        Vector lp0w = lp0.ProjectInto(wrkpl);
        Vector lp1w = lp1.ProjectInto(wrkpl);
        Printf(false, "    or distance = %Fi%s%E (in workplane)",
               SS.MmToString(ppw.DistanceToLine(lp0w, lp1w.Minus(lp0w))).c_str());
      }
    } else if (gs.n == 2 && gs.vectors == 2) {
      Printf(false, "%FtTWO VECTORS");

      Vector v0 = SK.GetEntity(gs.entity[0])->VectorGetNum(),
             v1 = SK.GetEntity(gs.entity[1])->VectorGetNum();
      v0 = v0.WithMagnitude(1);
      v1 = v1.WithMagnitude(1);

      Printf(true, "  vectorA = " PT_AS_NUM_LINK, CO_LINK(SK.GetEntity(gs.entity[0]), v0));
      Printf(false, "  vectorB = " PT_AS_NUM_LINK, CO_LINK(SK.GetEntity(gs.entity[1]), v1));

      double theta = acos(v0.Dot(v1));
      Printf(true, "    angle = %Fi%2%E degrees", theta * 180 / PI);
      while (theta < PI / 2)
        theta += PI;
      while (theta > PI / 2)
        theta -= PI;
      Printf(false, " or angle = %Fi%2%E (mod 180)", theta * 180 / PI);
    } else if (gs.n == 2 && gs.faces == 2) {
      Printf(false, "%FtTWO PLANE FACES");

      Vector n0 = SK.GetEntity(gs.face[0])->FaceGetNormalNum();
      Printf(true, " planeA normal = " PT_AS_NUM, CO(n0));
      Vector p0 = SK.GetEntity(gs.face[0])->FaceGetPointNum();
      Printf(false, "   planeA thru = " PT_AS_STR, COSTR(SK.GetEntity(gs.face[0]), p0));

      Vector n1 = SK.GetEntity(gs.face[1])->FaceGetNormalNum();
      Printf(true, " planeB normal = " PT_AS_NUM, CO(n1));
      Vector p1 = SK.GetEntity(gs.face[1])->FaceGetPointNum();
      Printf(false, "   planeB thru = " PT_AS_STR, COSTR(SK.GetEntity(gs.face[1]), p1));

      double theta = acos(n0.Dot(n1));
      Printf(true, "         angle = %Fi%2%E degrees", theta * 180 / PI);
      while (theta < PI / 2)
        theta += PI;
      while (theta > PI / 2)
        theta -= PI;
      Printf(false, "      or angle = %Fi%2%E (mod 180)", theta * 180 / PI);

      if (std::fabs(theta) < 0.01) {
        double d = (p1.Minus(p0)).Dot(n0);
        Printf(true, "      distance = %Fi%s", SS.MmToString(d).c_str());
      }
    } else if (gs.n == 0 && gs.constraints == 1) {
      Constraint *c = SK.GetConstraint(gs.constraint[0]);
      const std::string &desc = c->DescriptionString().c_str();

      if (c->type == Constraint::Type::COMMENT) {
        Printf(false, "%FtCOMMENT%E  %s", desc.c_str());
        if (c->ptA != Entity::NO_ENTITY) {
          Vector p = SK.GetEntity(c->ptA)->PointGetNum();
          Printf(true, "  attached to point at: " PT_AS_STR, COSTR(SK.GetEntity(c->ptA), p));
          Vector dv = c->disp.offset;
          Printf(false, "    distance = %Fi%s", SS.MmToString(dv.Magnitude()).c_str());
          Printf(false, "  d(x, y, z) = " PT_AS_STR_NO_LINK, COSTR_NO_LINK(dv));
        }
      } else if (c->HasLabel()) {
        if (c->reference) {
          Printf(false, "%FtREFERENCE%E  %s", desc.c_str());
        } else {
          Printf(false, "%FtDIMENSION%E  %s", desc.c_str());
        }
        Printf(true, "  %Fd%f%D%Ll%s  reference", &ScreenConstraintToggleReference,
               gs.constraint[0].v, c->reference ? CHECK_TRUE : CHECK_FALSE);
        if (c->type == Constraint::Type::DIAMETER) {
          Printf(false, "  %Fd%f%D%Ll%s  use radius", &ScreenConstraintShowAsRadius,
                 gs.constraint[0].v, c->other ? CHECK_TRUE : CHECK_FALSE);
        }
      } else {
        Printf(false, "%FtCONSTRAINT%E  %s", desc.c_str());
      }

      if (c->IsProjectible()) {
        if (c->workplane == Entity::FREE_IN_3D) {
          Printf(true, "%FtNOT PROJECTED TO WORKPLANE%E");
        } else {
          Entity *w = SK.GetEntity(c->workplane);
          if (w->h.isFromRequest()) {
            Printf(true, "%FtIN WORKPLANE%E  %Fl%Ll%D%f%h%s%E", w->h.request().v,
                   (&TextWindow::ScreenSelectRequest), &(TextWindow::ScreenHoverRequest),
                   w->DescriptionString().c_str());
          } else {
            Printf(true, "%FtIN WORKPLANE%E  %Fl%Ll%D%f%h%s%E", w->h.group().v,
                   (&TextWindow::ScreenSelectGroup), (&TextWindow::ScreenHoverGroupWorkplane),
                   w->DescriptionString().c_str());
          }
        }
      }

      std::vector<hEntity> lhe = {};
      lhe.push_back(c->ptA);
      lhe.push_back(c->ptB);
      lhe.push_back(c->entityA);
      lhe.push_back(c->entityB);
      lhe.push_back(c->entityC);
      lhe.push_back(c->entityD);

      auto it = std::remove_if(lhe.begin(), lhe.end(), [](hEntity he) {
        return he == Entity::NO_ENTITY || !he.isFromRequest();
      });
      lhe.erase(it, lhe.end());

      if (!lhe.empty()) {
        if (c->reference) {
          Printf(true, "%FtMEASURES:%E");
        } else {
          Printf(true, "%FtCONSTRAINS:%E");
        }

        int a = 0;
        for (hEntity he : lhe) {
          Entity *e = SK.GetEntity(he);
          Printf(false, "%Bp   %Fl%Ll%D%f%h%s%E", (a & 1) ? 'd' : 'a', e->h.v,
                 (&TextWindow::ScreenSelectEntity), &(TextWindow::ScreenHoverEntity),
                 e->DescriptionString().c_str());
          a++;
        }
      }
    } else {
      int n = SS.GW.selection.n;
      Printf(false, "%FtSELECTED:%E %d item%s", n, n == 1 ? "" : "s");
    }

    if (shown.screen == Screen::STYLE_INFO && shown.style.v >= Style::FIRST_CUSTOM &&
        gs.stylables > 0) {
      // If we are showing a screen for a particular style, then offer the
      // option to assign our selected entities to that style.
      Style *s = Style::Get(shown.style);
      Printf(true, "%Fl%D%f%Ll(assign to style %s)%E", shown.style.v, &ScreenAssignSelectionToStyle,
             s->DescriptionString().c_str());
    }
    // If any of the selected entities have an assigned style, then offer
    // the option to remove that style.
    bool styleAssigned = false;
    for (int i = 0; i < gs.entities; i++) {
      Entity *e = SK.GetEntity(gs.entity[i]);
      if (e->style.v != 0) {
        styleAssigned = true;
      }
    }
    for (int i = 0; i < gs.constraints; i++) {
      Constraint *c = SK.GetConstraint(gs.constraint[i]);
      if (c->type == Constraint::Type::COMMENT && c->disp.style.v != 0) {
        styleAssigned = true;
      }
    }
    if (styleAssigned) {
      Printf(true, "%Fl%D%f%Ll(remove assigned style)%E", 0, &ScreenAssignSelectionToStyle);
    }

    Printf(true, "%Fl%f%Ll(unselect all)%E", &TextWindow::ScreenUnselectAll);
  }

  void TextWindow::GoToScreen(Screen screen) {
    shown.screen = screen;
  }

} // namespace SolveSpace
