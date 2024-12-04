class GraphicsWindow {
  public:
  void Init (double width, double height, int pixelDeviceRatio);

  Platform::WindowRef window;

  void DeleteSelection ();
  void CopySelection ();
  void PasteClipboard (Vector trans, double theta, double scale);

  Platform::MenuRef openRecentMenu;
  Platform::MenuRef linkRecentMenu;

  Platform::MenuItemRef showGridMenuItem;
  Platform::MenuItemRef dimSolidModelMenuItem;
  Platform::MenuItemRef perspectiveProjMenuItem;
  Platform::MenuItemRef explodeMenuItem;
  Platform::MenuItemRef showToolbarMenuItem;
  Platform::MenuItemRef showTextWndMenuItem;
  Platform::MenuItemRef fullScreenMenuItem;

  Platform::MenuItemRef unitsMmMenuItem;
  Platform::MenuItemRef unitsMetersMenuItem;
  Platform::MenuItemRef unitsInchesMenuItem;
  Platform::MenuItemRef unitsFeetInchesMenuItem;

  Platform::MenuItemRef inWorkplaneMenuItem;
  Platform::MenuItemRef in3dMenuItem;

  Platform::MenuItemRef undoMenuItem;
  Platform::MenuItemRef redoMenuItem;

  std::shared_ptr<ViewportCanvas> canvas;
  std::shared_ptr<BatchCanvas>    persistentCanvas;
  bool                            persistentDirty;

  bool overrideCamera; // hack to make the Paint method work for both tests and the graphical UI
  // These parameters define the map from 2d screen coordinates to the
  // coordinates of the 3d sketch points. We will use an axonometric
  // projection.
  double width;
  double height;
  int    devicePixelRatio;
  Vector offset    = Vector (0, 0, 0);
  Vector projRight = Vector (0, 0, 0);
  Vector projUp    = Vector (0, 0, 0);
  double scale;
  struct {
    bool    mouseDown;
    Vector  offset    = Vector (0, 0, 0);
    Vector  projRight = Vector (0, 0, 0);
    Vector  projUp    = Vector (0, 0, 0);
    Point2d mouse;
    Point2d mouseOnButtonDown;
    Vector  marqueePoint = Vector (0, 0, 0);
    bool    startedMoving;
  } orig;
  // We need to detect when the projection is changed to invalidate
  // caches for drawn items.
  struct {
    Vector offset    = Vector (0, 0, 0);
    Vector projRight = Vector (0, 0, 0);
    Vector projUp    = Vector (0, 0, 0);
    double scale;
  } cached;

  // Most recent mouse position, updated every time the mouse moves.
  Point2d currentMousePosition;

  // When the user is dragging a point, don't solve multiple times without
  // allowing a paint in between. The extra solves are wasted if they're
  // not displayed.
  bool havePainted;

  // Some state for the context menu.
  struct {
    bool active;
  } context;

  Camera   GetCamera () const;
  Lighting GetLighting () const;

  void    NormalizeProjectionVectors ();
  Point2d ProjectPoint (Vector p);
  Vector  ProjectPoint3 (Vector p);
  Vector  ProjectPoint4 (Vector p, double *w);
  Vector  UnProjectPoint (Point2d p);
  Vector  UnProjectPoint3 (Vector p);

  Platform::TimerRef animateTimer;
  void               AnimateOnto (Quaternion quatf, Vector offsetf);
  void               AnimateOntoWorkplane ();

  Vector VectorFromProjs (Vector rightUpForward);
  void   HandlePointForZoomToFit (Vector p, Point2d *pmax, Point2d *pmin, double *wmin,
                                  bool usePerspective, const Camera &camera);
  void   ZoomToMouse (double delta);
  void   LoopOverPoints (const std::vector<Entity *>     &entities,
                         const std::vector<Constraint *> &constraints,
                         const std::vector<hEntity> &faces, Point2d *pmax, Point2d *pmin,
                         double *wmin, bool usePerspective, bool includeMesh, const Camera &camera);
  void   ZoomToFit (bool includingInvisibles = false, bool useSelection = false);
  double ZoomToFit (const Camera &camera, bool includingInvisibles = false,
                    bool useSelection = false);

  hGroup  activeGroup;
  void    EnsureValidActives ();
  bool    LockedInWorkplane ();
  void    SetWorkplaneFreeIn3d ();
  hEntity ActiveWorkplane ();
  void    ForceTextWindowShown ();

  // Operations that must be completed by doing something with the mouse
  // are noted here.
  enum class Pending : uint32_t {
    NONE                     = 0,
    COMMAND                  = 1,
    DRAGGING_POINTS          = 2,
    DRAGGING_NEW_POINT       = 3,
    DRAGGING_NEW_LINE_POINT  = 4,
    DRAGGING_NEW_CUBIC_POINT = 5,
    DRAGGING_NEW_ARC_POINT   = 6,
    DRAGGING_CONSTRAINT      = 7,
    DRAGGING_RADIUS          = 8,
    DRAGGING_NORMAL          = 9,
    DRAGGING_NEW_RADIUS      = 10,
    DRAGGING_MARQUEE         = 11,
  };

  struct {
    Pending operation;
    Command command;

    hRequest       request;
    hEntity        point;
    List<hEntity>  points;
    List<hRequest> requests;
    hEntity        circle;
    hEntity        normal;
    hConstraint    constraint;

    const char    *description;
    Platform::Path filename;

    bool             hasSuggestion;
    Constraint::Type suggestion;
  } pending;
  void ClearPending (int which, bool scheduleShowTW = true);
  bool IsFromPending (hRequest r);
  void AddToPending (hRequest r);
  void ReplacePending (hRequest before, hRequest after);

  // The constraint that is being edited with the on-screen textbox.
  hConstraint constraintBeingEdited;

  bool SuggestLineConstraint (hRequest lineSegment, Constraint::Type *type);

  Vector   SnapToGrid (Vector p);
  Vector   SnapToEntityByScreenPoint (Point2d pp, hEntity he);
  bool     ConstrainPointByHovered (hEntity pt, const Point2d *projected = NULL);
  void     DeleteTaggedRequests ();
  hRequest AddRequest (Request::Type type, bool rememberForUndo);
  hRequest AddRequest (Request::Type type);

  class ParametricCurve {
public:
    bool   isLine; // else circle
    Vector p0 = Vector (0, 0, 0), p1 = Vector (0, 0, 0);
    Vector u = Vector (0, 0, 0), v = Vector (0, 0, 0);
    double r, theta0, theta1, dtheta;

    void   MakeFromEntity (hEntity he, bool reverse);
    Vector PointAt (double t);
    Vector TangentAt (double t);
    double LengthForAuto ();

    void CreateRequestTrimmedTo (double t, bool reuseOrig, hEntity orig, hEntity arc,
                                 bool arcFinish, bool pointf);
    void ConstrainPointIfCoincident (hEntity hpt);
  };
  void    MakeTangentArc ();
  void    SplitLinesOrCurves ();
  hEntity SplitEntity (hEntity he, Vector pinter);
  hEntity SplitLine (hEntity he, Vector pinter);
  hEntity SplitCircle (hEntity he, Vector pinter);
  hEntity SplitCubic (hEntity he, Vector pinter);
  void    ReplacePointInConstraints (hEntity oldpt, hEntity newpt);
  void    RemoveConstraintsForPointBeingDeleted (hEntity hpt);
  void    FixConstraintsForRequestBeingDeleted (hRequest hr);
  void    FixConstraintsForPointBeingDeleted (hEntity hpt);
  void    EditConstraint (hConstraint constraint);

  // A selected entity.
  class Selection {
public:
    int tag;

    hEntity     entity;
    hConstraint constraint;
    bool        emphasized;

    void Draw (bool isHovered, Canvas *canvas);

    void Clear ();
    bool IsEmpty ();
    bool Equals (Selection *b);
    bool HasEndpoints ();
  };

  // A hovered entity, with its location relative to the cursor.
  class Hover {
public:
    int       zIndex;
    double    distance;
    double    depth;
    Selection selection;
  };

  List<Hover>     hoverList;
  Selection       hover;
  bool            hoverWasSelectedOnMousedown;
  List<Selection> selection;

  Selection ChooseFromHoverToSelect ();
  Selection ChooseFromHoverToDrag ();
  void      HitTestMakeSelection (Point2d mp);
  void      ClearSelection ();
  void      ClearNonexistentSelectionItems ();
  /// This structure is filled by a call to GroupSelection().
  struct {
    std::vector<hEntity>     point;
    std::vector<hEntity>     entity;
    std::vector<hEntity>     anyNormal;
    std::vector<hEntity>     vector;
    std::vector<hEntity>     face;
    std::vector<hConstraint> constraint;
    int                      points;
    int                      entities;
    int                      workplanes;
    int                      faces;
    int                      lineSegments;
    int                      circlesOrArcs;
    int                      arcs;
    int                      cubics;
    int                      periodicCubics;
    int                      anyNormals;
    int                      vectors;
    int                      constraints;
    int                      stylables;
    int                      constraintLabels;
    int                      withEndpoints;
    int                      n; ///< Number of selected items
  } gs;
  void GroupSelection ();
  bool IsSelected (Selection *s);
  bool IsSelected (hEntity he);
  void MakeSelected (hEntity he);
  void MakeSelected (hConstraint hc);
  void MakeSelected (Selection *s);
  void MakeUnselected (hEntity he, bool coincidentPointTrick);
  void MakeUnselected (Selection *s, bool coincidentPointTrick);
  void SelectByMarquee ();
  void ClearSuper (int which);

  // This sets what gets displayed.
  bool showWorkplanes;
  bool showNormals;
  bool showPoints;
  bool showConstruction;
  bool showConstraints;
  bool showTextWindow;
  bool showShaded;
  bool showEdges;
  bool showOutlines;
  bool showFaces;
  bool showFacesDrawing;
  bool showFacesNonDrawing;
  bool showMesh;
  void ToggleBool (bool *v);

  enum class DrawOccludedAs { INVISIBLE, STIPPLED, VISIBLE };
  DrawOccludedAs drawOccludedAs;

  bool showSnapGrid;
  bool dimSolidModel;
  void DrawSnapGrid (Canvas *canvas);

  void AddPointToDraggedList (hEntity hp);
  void StartDraggingByEntity (hEntity he);
  void StartDraggingBySelection ();
  void UpdateDraggedNum (Vector *pos, double mx, double my);
  void UpdateDraggedPoint (hEntity hp, double mx, double my);

  void Invalidate (bool clearPersistent = false);
  void DrawEntities (Canvas *canvas, bool persistent);
  void DrawPersistent (Canvas *canvas);
  void Draw (Canvas *canvas);
  void Paint ();

  bool MouseEvent (Platform::MouseEvent event);
  void MouseMoved (double x, double y, bool leftDown, bool middleDown, bool rightDown,
                   bool shiftDown, bool ctrlDown);
  void MouseLeftDown (double x, double y, bool shiftDown, bool ctrlDown);
  void MouseLeftUp (double x, double y, bool shiftDown, bool ctrlDown);
  void MouseLeftDoubleClick (double x, double y);
  void MouseMiddleOrRightDown (double x, double y);
  void MouseRightUp (double x, double y);
  void MouseScroll (double delta);
  void MouseLeave ();
  bool KeyboardEvent (Platform::KeyboardEvent event);
  void EditControlDone (const std::string &s);

  int64_t last6DofTime;
  hGroup  last6DofGroup;
  void    SixDofEvent (Platform::SixDofEvent event);
};
