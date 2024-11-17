class TextWindow {
public:
    enum {
        MAX_COLS = 100,
        MIN_COLS = 45,
        MAX_ROWS = 4000
    };

    typedef struct {
        char      c;
        RgbaColor color;
    } Color;
    static const Color fgColors[];
    static const Color bgColors[];

    float bgColorTable[256*3];
    float fgColorTable[256*3];

    enum {
        CHAR_WIDTH_    = 9,
        CHAR_HEIGHT    = 16,
        LINE_HEIGHT    = 20,
        LEFT_MARGIN    = 6,
    };

#define CHECK_FALSE "\xEE\x80\x80" // U+E000
#define CHECK_TRUE  "\xEE\x80\x81"
#define RADIO_FALSE "\xEE\x80\x82"
#define RADIO_TRUE  "\xEE\x80\x83"

    int scrollPos;      // The scrollbar position, in half-row units
    int halfRows;       // The height of our window, in half-row units

    uint32_t text[MAX_ROWS][MAX_COLS];
    typedef void LinkFunction(int link, uint32_t v);
    enum { NOT_A_LINK = 0 };
    struct {
        char            fg;
        char            bg;
        RgbaColor       bgRgb;
        int             link;
        uint32_t        data;
        LinkFunction   *f;
        LinkFunction   *h;
    }       meta[MAX_ROWS][MAX_COLS];
    int hoveredRow, hoveredCol;

    int top[MAX_ROWS]; // in half-line units, or -1 for unused
    int rows;

    Platform::WindowRef window;
    std::shared_ptr<ViewportCanvas> canvas;

    void Draw(Canvas *canvas);

    void Paint();
    void MouseEvent(bool isClick, bool leftDown, double x, double y);
    void MouseLeave();
    void ScrollbarEvent(double newPos);

    enum DrawOrHitHow : uint32_t {
        PAINT = 0,
        HOVER = 1,
        CLICK = 2
    };
    void DrawOrHitTestIcons(UiCanvas *canvas, DrawOrHitHow how,
                            double mx, double my);
    Button *hoveredButton;

    Vector HsvToRgb(Vector hsv);
    std::shared_ptr<Pixmap> HsvPattern2d(int w, int h);
    std::shared_ptr<Pixmap> HsvPattern1d(double hue, double sat, int w, int h);
    void ColorPickerDone();
    bool DrawOrHitTestColorPicker(UiCanvas *canvas, DrawOrHitHow how,
                                  bool leftDown, double x, double y);

    void Init();
    void MakeColorTable(const Color *in, float *out);
    void Printf(bool half, const char *fmt, ...);
    void ClearScreen();

    void Show();
    void Resize();

    // State for the screen that we are showing in the text window.
    enum class Screen : uint32_t {
        LIST_OF_GROUPS      = 0,
        GROUP_INFO          = 1,
        GROUP_SOLVE_INFO    = 2,
        CONFIGURATION       = 3,
        STEP_DIMENSION      = 4,
        LIST_OF_STYLES      = 5,
        STYLE_INFO          = 6,
        PASTE_TRANSFORMED   = 7,
        EDIT_VIEW           = 8,
        TANGENT_ARC         = 9
    };
    typedef struct {
        Screen  screen;

        hGroup      group;
        hStyle      style;

        hConstraint constraint;

        struct {
            int         times;
            Vector      trans;
            double      theta;
            Vector      origin;
            double      scale;
        }           paste;
    } ShownState;
    ShownState shown;

    enum class Edit : uint32_t {
        NOTHING               = 0,
        // For multiple groups
        TIMES_REPEATED        = 1,
        GROUP_NAME            = 2,
        GROUP_SCALE           = 3,
        GROUP_COLOR           = 4,
        GROUP_OPACITY         = 5,
        // For the configuration screen
        LIGHT_DIRECTION       = 100,
        LIGHT_INTENSITY       = 101,
        COLOR                 = 102,
        CHORD_TOLERANCE       = 103,
        MAX_SEGMENTS          = 104,
        CAMERA_TANGENT        = 105,
        GRID_SPACING          = 106,
        DIGITS_AFTER_DECIMAL  = 107,
        DIGITS_AFTER_DECIMAL_DEGREE = 108,
        EXPORT_SCALE          = 109,
        EXPORT_OFFSET         = 110,
        CANVAS_SIZE           = 111,
        G_CODE_DEPTH          = 112,
        G_CODE_SAFE_HEIGHT    = 113,
        G_CODE_PASSES         = 114,
        G_CODE_FEED           = 115,
        G_CODE_PLUNGE_FEED    = 116,
        AUTOSAVE_INTERVAL     = 117,
        LIGHT_AMBIENT         = 118,
        FIND_CONSTRAINT_TIMEOUT = 119,
        EXPLODE_DISTANCE      = 120,
        // For TTF text
        TTF_TEXT              = 300,
        // For the step dimension screen
        STEP_DIM_FINISH       = 400,
        STEP_DIM_STEPS        = 401,
        // For the styles stuff
        STYLE_WIDTH           = 500,
        STYLE_TEXT_HEIGHT     = 501,
        STYLE_TEXT_ANGLE      = 502,
        STYLE_COLOR           = 503,
        STYLE_FILL_COLOR      = 504,
        STYLE_NAME            = 505,
        BACKGROUND_COLOR      = 506,
        STYLE_STIPPLE_PERIOD  = 508,
        // For paste transforming
        PASTE_TIMES_REPEATED  = 600,
        PASTE_ANGLE           = 601,
        PASTE_SCALE           = 602,
        // For view
        VIEW_SCALE            = 700,
        VIEW_ORIGIN           = 701,
        VIEW_PROJ_RIGHT       = 702,
        VIEW_PROJ_UP          = 703,
        // For tangent arc
        TANGENT_ARC_RADIUS    = 800,
        // For helix pitch
        HELIX_PITCH           = 802
    };
    struct {
        bool        showAgain;
        Edit        meaning;
        int         i;
        hGroup      group;
        hRequest    request;
        hStyle      style;
    } edit;

    static void ReportHowGroupSolved(hGroup hg);

    struct {
        int     halfRow;
        int     col;

        struct {
            RgbaColor rgb;
            double    h, s, v;
            bool      show;
            bool      picker1dActive;
            bool      picker2dActive;
        }       colorPicker;
    } editControl;

    void HideEditControl();
    void ShowEditControl(int col, const std::string &str, int halfRow = -1);
    void ShowEditControlWithColorPicker(int col, RgbaColor rgb);

    void ClearSuper();

    void ShowHeader(bool withNav);
    // These are self-contained screens, that show some information about
    // the sketch.
    void ShowListOfGroups();
    void ShowGroupInfo();
    void ShowGroupSolveInfo();
    void ShowConfiguration();
    void ShowListOfStyles();
    void ShowStyleInfo();
    void ShowStepDimension();
    void ShowPasteTransformed();
    void ShowEditView();
    void ShowTangentArc();
    // Special screen, based on selection
    void DescribeSelection();

    void GoToScreen(Screen screen);

    // All of these are callbacks from the GUI code; first from when
    // we're describing an entity
    static void ScreenEditTtfText(int link, uint32_t v);
    static void ScreenSetTtfFont(int link, uint32_t v);
    static void ScreenUnselectAll(int link, uint32_t v);

    // when we're describing a constraint
    static void ScreenConstraintToggleReference(int link, uint32_t v);
    static void ScreenConstraintShowAsRadius(int link, uint32_t v);

    // and the rest from the stuff in textscreens.cpp
    static void ScreenSelectGroup(int link, uint32_t v);
    static void ScreenActivateGroup(int link, uint32_t v);
    static void ScreenToggleGroupShown(int link, uint32_t v);
    static void ScreenHowGroupSolved(int link, uint32_t v);
    static void ScreenShowGroupsSpecial(int link, uint32_t v);
    static void ScreenDeleteGroup(int link, uint32_t v);

    static void ScreenHoverGroupWorkplane(int link, uint32_t v);
    static void ScreenHoverRequest(int link, uint32_t v);
    static void ScreenHoverEntity(int link, uint32_t v);
    static void ScreenHoverConstraint(int link, uint32_t v);
    static void ScreenSelectRequest(int link, uint32_t v);
    static void ScreenSelectEntity(int link, uint32_t v);
    static void ScreenSelectConstraint(int link, uint32_t v);

    static void ScreenChangeGroupOption(int link, uint32_t v);
    static void ScreenColor(int link, uint32_t v);
    static void ScreenOpacity(int link, uint32_t v);

    static void ScreenShowListOfStyles(int link, uint32_t v);
    static void ScreenShowStyleInfo(int link, uint32_t v);
    static void ScreenDeleteStyle(int link, uint32_t v);
    static void ScreenChangeStylePatternType(int link, uint32_t v);
    static void ScreenChangeStyleYesNo(int link, uint32_t v);
    static void ScreenCreateCustomStyle(int link, uint32_t v);
    static void ScreenLoadFactoryDefaultStyles(int link, uint32_t v);
    static void ScreenAssignSelectionToStyle(int link, uint32_t v);

    static void ScreenShowConfiguration(int link, uint32_t v);
    static void ScreenShowEditView(int link, uint32_t v);
    static void ScreenGoToWebsite(int link, uint32_t v);

    static void ScreenChangeFixExportColors(int link, uint32_t v);
    static void ScreenChangeExportBackgroundColor(int link, uint32_t v);
    static void ScreenChangeBackFaces(int link, uint32_t v);
    static void ScreenChangeShowContourAreas(int link, uint32_t v);
    static void ScreenChangeCheckClosedContour(int link, uint32_t v);
    static void ScreenChangeTurntableNav(int link, uint32_t v);
    static void ScreenChangeImmediatelyEditDimension(int link, uint32_t v);
    static void ScreenChangeAutomaticLineConstraints(int link, uint32_t v);
    static void ScreenChangePwlCurves(int link, uint32_t v);
    static void ScreenChangeCanvasSizeAuto(int link, uint32_t v);
    static void ScreenChangeCanvasSize(int link, uint32_t v);
    static void ScreenChangeShadedTriangles(int link, uint32_t v);

    static void ScreenAllowRedundant(int link, uint32_t v);

    struct {
        bool    isDistance;
        double  finish;
        int     steps;

        Platform::TimerRef timer;
        int64_t time;
        int     step;
    } stepDim;
    static void ScreenStepDimSteps(int link, uint32_t v);
    static void ScreenStepDimFinish(int link, uint32_t v);
    static void ScreenStepDimGo(int link, uint32_t v);

    static void ScreenChangeTangentArc(int link, uint32_t v);

    static void ScreenPasteTransformed(int link, uint32_t v);

    static void ScreenHome(int link, uint32_t v);

    // These ones do stuff with the edit control
    static void ScreenChangeExprA(int link, uint32_t v);
    static void ScreenChangeGroupName(int link, uint32_t v);
    static void ScreenChangeGroupScale(int link, uint32_t v);
    static void ScreenChangeHelixPitch(int link, uint32_t v);
    static void ScreenChangePitchOption(int link, uint32_t v);
    static void ScreenChangeLightDirection(int link, uint32_t v);
    static void ScreenChangeLightIntensity(int link, uint32_t v);
    static void ScreenChangeLightAmbient(int link, uint32_t v);
    static void ScreenChangeColor(int link, uint32_t v);
    static void ScreenChangeChordTolerance(int link, uint32_t v);
    static void ScreenChangeMaxSegments(int link, uint32_t v);
    static void ScreenChangeExportChordTolerance(int link, uint32_t v);
    static void ScreenChangeExportMaxSegments(int link, uint32_t v);
    static void ScreenChangeCameraTangent(int link, uint32_t v);
    static void ScreenChangeGridSpacing(int link, uint32_t v);
    static void ScreenChangeExplodeDistance(int link, uint32_t v);
    static void ScreenChangeDigitsAfterDecimal(int link, uint32_t v);
    static void ScreenChangeDigitsAfterDecimalDegree(int link, uint32_t v);
    static void ScreenChangeUseSIPrefixes(int link, uint32_t v);
    static void ScreenChangeExportScale(int link, uint32_t v);
    static void ScreenChangeExportOffset(int link, uint32_t v);
    static void ScreenChangeGCodeParameter(int link, uint32_t v);
    static void ScreenChangeAutosaveInterval(int link, uint32_t v);
    static void ScreenChangeFindConstraintTimeout(int link, uint32_t v);
    static void ScreenChangeStyleName(int link, uint32_t v);
    static void ScreenChangeStyleMetric(int link, uint32_t v);
    static void ScreenChangeStyleTextAngle(int link, uint32_t v);
    static void ScreenChangeStyleColor(int link, uint32_t v);
    static void ScreenChangeBackgroundColor(int link, uint32_t v);
    static void ScreenChangePasteTransformed(int link, uint32_t v);
    static void ScreenChangeViewScale(int link, uint32_t v);
    static void ScreenChangeViewToFullScale(int link, uint32_t v);
    static void ScreenChangeViewOrigin(int link, uint32_t v);
    static void ScreenChangeViewProjection(int link, uint32_t v);

    bool EditControlDoneForStyles(const std::string &s);
    bool EditControlDoneForConfiguration(const std::string &s);
    bool EditControlDoneForPaste(const std::string &s);
    bool EditControlDoneForView(const std::string &s);
    void EditControlDone(std::string s);
};
