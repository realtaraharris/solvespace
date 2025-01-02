//-----------------------------------------------------------------------------
// Entry point in to the program, our registry-stored settings and top-level
// housekeeping when we open, save, and create new files.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "cpp2util.h"
#include "solvespace.h"
#include "ssg.h"
#include "config.h"
#include "filewriter/vectorfilewriter.h"

#if defined(HAIKU_GUI)
HaikuSpaceUI SS = {};
#else
SolveSpaceUI SS = {};
#endif

Sketch SolveSpace::SK = {};

void SolveSpaceUI::Init() {
  // check that the resource system works
  dbp("%s", LoadString("banner.txt").data());

  Platform::SettingsRef settings = Platform::GetSettings();

  okayToStartNewFile = true;
  SS.tangentArcRadius = 10.0;
  SS.explodeDistance = 1.0;

  // Then, load the registry settings.
  // Default list of colors for the model material
  modelColor[0] = settings->ThawColor("ModelColor_0", RGBi(150, 150, 150));
  modelColor[1] = settings->ThawColor("ModelColor_1", RGBi(100, 100, 100));
  modelColor[2] = settings->ThawColor("ModelColor_2", RGBi(30, 30, 30));
  modelColor[3] = settings->ThawColor("ModelColor_3", RGBi(150, 0, 0));
  modelColor[4] = settings->ThawColor("ModelColor_4", RGBi(0, 100, 0));
  modelColor[5] = settings->ThawColor("ModelColor_5", RGBi(0, 80, 80));
  modelColor[6] = settings->ThawColor("ModelColor_6", RGBi(0, 0, 130));
  modelColor[7] = settings->ThawColor("ModelColor_7", RGBi(80, 0, 80));
  // Light intensities
  lightIntensity[0] = settings->ThawFloat("LightIntensity_0", 1.0);
  lightIntensity[1] = settings->ThawFloat("LightIntensity_1", 0.5);
  ambientIntensity = settings->ThawFloat("Light_Ambient", 0.3);
  // Light positions
  lightDir[0].x = settings->ThawFloat("LightDir_0_Right", -1.0);
  lightDir[0].y = settings->ThawFloat("LightDir_0_Up", 1.0);
  lightDir[0].z = settings->ThawFloat("LightDir_0_Forward", 0.0);
  lightDir[1].x = settings->ThawFloat("LightDir_1_Right", 1.0);
  lightDir[1].y = settings->ThawFloat("LightDir_1_Up", 0.0);
  lightDir[1].z = settings->ThawFloat("LightDir_1_Forward", 0.0);

  exportMode = false;
  // Chord tolerance
  chordTol = settings->ThawFloat("ChordTolerancePct", 0.1);
  // Max pwl segments to generate
  maxSegments = settings->ThawInt("MaxSegments", 20);
  // Chord tolerance
  exportChordTol = settings->ThawFloat("ExportChordTolerance", 0.1);
  // Max pwl segments to generate
  exportMaxSegments = settings->ThawInt("ExportMaxSegments", 64);
  // Timeout value for finding redundant constrains (ms)
  timeoutRedundantConstr = settings->ThawInt("TimeoutRedundantConstraints", 1000);
  // View units
  viewUnits = (Unit)settings->ThawInt("ViewUnits", (uint32_t)Unit::MM);
  // Number of digits after the decimal point
  afterDecimalMm = settings->ThawInt("AfterDecimalMm", 2);
  afterDecimalInch = settings->ThawInt("AfterDecimalInch", 3);
  afterDecimalDegree = settings->ThawInt("AfterDecimalDegree", 2);
  useSIPrefixes = settings->ThawBool("UseSIPrefixes", false);
  // Camera tangent (determines perspective)
  cameraTangent = settings->ThawFloat("CameraTangent", 0.3f / 1e3);
  // Grid spacing
  gridSpacing = settings->ThawFloat("GridSpacing", 5.0);
  // Export scale factor
  exportScale = settings->ThawFloat("ExportScale", 1.0);
  // Export offset (cutter radius comp)
  exportOffset = settings->ThawFloat("ExportOffset", 0.0);
  // Rewrite exported colors close to white into black (assuming white bg)
  fixExportColors = settings->ThawBool("FixExportColors", true);
  // Export background color
  exportBackgroundColor = settings->ThawBool("ExportBackgroundColor", false);
  // Draw back faces of triangles (when mesh is leaky/self-intersecting)
  drawBackFaces = settings->ThawBool("DrawBackFaces", true);
  // Use turntable mouse navigation
  turntableNav = settings->ThawBool("TurntableNav", false);
  // Immediately edit dimension
  immediatelyEditDimension = settings->ThawBool("ImmediatelyEditDimension", true);
  // Check that contours are closed and not self-intersecting
  checkClosedContour = settings->ThawBool("CheckClosedContour", true);
  // Enable automatic constrains for lines
  automaticLineConstraints = settings->ThawBool("AutomaticLineConstraints", true);
  // Draw closed polygons areas
  showContourAreas = settings->ThawBool("ShowContourAreas", false);
  // Export shaded triangles in a 2d view
  exportShadedTriangles = settings->ThawBool("ExportShadedTriangles", true);
  // Export pwl curves (instead of exact) always
  exportPwlCurves = settings->ThawBool("ExportPwlCurves", false);
  // Background color on-screen
  backgroundColor = settings->ThawColor("BackgroundColor", RGBi(0, 0, 0));
  // Whether export canvas size is fixed or derived from bbox
  exportCanvasSizeAuto = settings->ThawBool("ExportCanvasSizeAuto", true);
  // Margins for automatic canvas size
  exportMargin.left = settings->ThawFloat("ExportMargin_Left", 5.0);
  exportMargin.right = settings->ThawFloat("ExportMargin_Right", 5.0);
  exportMargin.bottom = settings->ThawFloat("ExportMargin_Bottom", 5.0);
  exportMargin.top = settings->ThawFloat("ExportMargin_Top", 5.0);
  // Dimensions for fixed canvas size
  exportCanvas.width = settings->ThawFloat("ExportCanvas_Width", 100.0);
  exportCanvas.height = settings->ThawFloat("ExportCanvas_Height", 100.0);
  exportCanvas.dx = settings->ThawFloat("ExportCanvas_Dx", 5.0);
  exportCanvas.dy = settings->ThawFloat("ExportCanvas_Dy", 5.0);
  // Extra parameters when exporting G code
  gCode.depth = settings->ThawFloat("GCode_Depth", 10.0);
  gCode.safeHeight = settings->ThawFloat("GCode_SafeHeight", 5.0);
  gCode.passes = settings->ThawInt("GCode_Passes", 1);
  gCode.feed = settings->ThawFloat("GCode_Feed", 10.0);
  gCode.plungeFeed = settings->ThawFloat("GCode_PlungeFeed", 10.0);
  // Show toolbar in the graphics window
  showToolbar = settings->ThawBool("ShowToolbar", true);
  // Recent files menus
  for (size_t i = 0; i < MAX_RECENT; i++) {
    std::string rawPath = settings->ThawString("RecentFile_" + std::to_string(i), "");
    if (rawPath.empty())
      continue;
    recentFiles.push_back(Platform::Path::From(rawPath));
  }
  // Autosave timer
  autosaveInterval = settings->ThawInt("AutosaveInterval", 5);
  // Locale
  std::string locale = settings->ThawString("Locale", "");
  if (!locale.empty()) {
    SetLocale(locale);
  }

  refreshTimer = Platform::CreateTimer();
  refreshTimer->onTimeout = std::bind(&SolveSpaceUI::Refresh, &SS);

  autosaveTimer = Platform::CreateTimer();
  autosaveTimer->onTimeout = std::bind(&SolveSpaceUI::Autosave, &SS);

  // The default styles (colors, line widths, etc.) are also stored in the
  // configuration file, but we will automatically load those as we need
  // them.

  ScheduleAutosave();

  NewFile();
  AfterNewFile();
}

void SolveSpaceUI::OpenSolveSpaceFile() {
  assert("SolveSpaceUI::OpenSolveSpaceFile() shouldn't be called");
}

bool SolveSpaceUI::LoadAutosaveFor(const Platform::Path &filename) {
  assert("SolveSpaceUI::LoadAutosaveFor() shouldn't be called");
  return false;
}

bool SolveSpaceUI::Load(const Platform::Path &filename) {
  bool autosaveLoaded = LoadAutosaveFor(filename);
  bool fileLoaded = autosaveLoaded || LoadFromFile(filename, /*canCancel=*/true);
  if (fileLoaded) {
    saveFile = filename;
    AddToRecentList(filename);
  } else {
    saveFile.Clear();
    NewFile();
  }
  AfterNewFile();
  unsaved = autosaveLoaded;
  std::cout << "unsaved = " << unsaved << " (SolveSpaceUI::Load)" << std::endl;

  return fileLoaded;
}

void SolveSpaceUI::Exit() {
  Platform::SettingsRef settings = Platform::GetSettings();

  // Recent files
  for (size_t i = 0; i < MAX_RECENT; i++) {
    std::string rawPath;
    if (recentFiles.size() > i) {
      rawPath = recentFiles[i].raw;
    }
    settings->FreezeString("RecentFile_" + std::to_string(i), rawPath);
  }
  // Model colors
  for (size_t i = 0; i < MODEL_COLORS; i++)
    settings->FreezeColor("ModelColor_" + std::to_string(i), modelColor[i]);
  // Light intensities
  settings->FreezeFloat("LightIntensity_0", (float)lightIntensity[0]);
  settings->FreezeFloat("LightIntensity_1", (float)lightIntensity[1]);
  settings->FreezeFloat("Light_Ambient", (float)ambientIntensity);
  // Light directions
  settings->FreezeFloat("LightDir_0_Right", (float)lightDir[0].x);
  settings->FreezeFloat("LightDir_0_Up", (float)lightDir[0].y);
  settings->FreezeFloat("LightDir_0_Forward", (float)lightDir[0].z);
  settings->FreezeFloat("LightDir_1_Right", (float)lightDir[1].x);
  settings->FreezeFloat("LightDir_1_Up", (float)lightDir[1].y);
  settings->FreezeFloat("LightDir_1_Forward", (float)lightDir[1].z);
  // Chord tolerance
  settings->FreezeFloat("ChordTolerancePct", (float)chordTol);
  // Max pwl segments to generate
  settings->FreezeInt("MaxSegments", (uint32_t)maxSegments);
  // Export Chord tolerance
  settings->FreezeFloat("ExportChordTolerance", (float)exportChordTol);
  // Export Max pwl segments to generate
  settings->FreezeInt("ExportMaxSegments", (uint32_t)exportMaxSegments);
  // Timeout for finding which constraints to fix Jacobian
  settings->FreezeInt("TimeoutRedundantConstraints", (uint32_t)timeoutRedundantConstr);
  // View units
  settings->FreezeInt("ViewUnits", (uint32_t)viewUnits);
  // Number of digits after the decimal point
  settings->FreezeInt("AfterDecimalMm", (uint32_t)afterDecimalMm);
  settings->FreezeInt("AfterDecimalInch", (uint32_t)afterDecimalInch);
  settings->FreezeInt("AfterDecimalDegree", (uint32_t)afterDecimalDegree);
  settings->FreezeBool("UseSIPrefixes", useSIPrefixes);
  // Camera tangent (determines perspective)
  settings->FreezeFloat("CameraTangent", (float)cameraTangent);
  // Grid spacing
  settings->FreezeFloat("GridSpacing", gridSpacing);
  // Export scale
  settings->FreezeFloat("ExportScale", exportScale);
  // Export offset (cutter radius comp)
  settings->FreezeFloat("ExportOffset", exportOffset);
  // Rewrite exported colors close to white into black (assuming white bg)
  settings->FreezeBool("FixExportColors", fixExportColors);
  // Export background color
  settings->FreezeBool("ExportBackgroundColor", exportBackgroundColor);
  // Draw back faces of triangles (when mesh is leaky/self-intersecting)
  settings->FreezeBool("DrawBackFaces", drawBackFaces);
  // Draw closed polygons areas
  settings->FreezeBool("ShowContourAreas", showContourAreas);
  // Check that contours are closed and not self-intersecting
  settings->FreezeBool("CheckClosedContour", checkClosedContour);
  // Use turntable mouse navigation
  settings->FreezeBool("TurntableNav", turntableNav);
  // Immediately edit dimensions
  settings->FreezeBool("ImmediatelyEditDimension", immediatelyEditDimension);
  // Enable automatic constrains for lines
  settings->FreezeBool("AutomaticLineConstraints", automaticLineConstraints);
  // Export shaded triangles in a 2d view
  settings->FreezeBool("ExportShadedTriangles", exportShadedTriangles);
  // Export pwl curves (instead of exact) always
  settings->FreezeBool("ExportPwlCurves", exportPwlCurves);
  // Background color on-screen
  settings->FreezeColor("BackgroundColor", backgroundColor);
  // Whether export canvas size is fixed or derived from bbox
  settings->FreezeBool("ExportCanvasSizeAuto", exportCanvasSizeAuto);
  // Margins for automatic canvas size
  settings->FreezeFloat("ExportMargin_Left", exportMargin.left);
  settings->FreezeFloat("ExportMargin_Right", exportMargin.right);
  settings->FreezeFloat("ExportMargin_Bottom", exportMargin.bottom);
  settings->FreezeFloat("ExportMargin_Top", exportMargin.top);
  // Dimensions for fixed canvas size
  settings->FreezeFloat("ExportCanvas_Width", exportCanvas.width);
  settings->FreezeFloat("ExportCanvas_Height", exportCanvas.height);
  settings->FreezeFloat("ExportCanvas_Dx", exportCanvas.dx);
  settings->FreezeFloat("ExportCanvas_Dy", exportCanvas.dy);
  // Extra parameters when exporting G code
  settings->FreezeFloat("GCode_Depth", gCode.depth);
  settings->FreezeInt("GCode_Passes", gCode.passes);
  settings->FreezeFloat("GCode_Feed", gCode.feed);
  settings->FreezeFloat("GCode_PlungeFeed", gCode.plungeFeed);
  // Show toolbar in the graphics window
  settings->FreezeBool("ShowToolbar", showToolbar);
  // Autosave timer
  settings->FreezeInt("AutosaveInterval", autosaveInterval);

  // And the default styles, colors and line widths and such.
  Style::FreezeDefaultStyles(settings);

  FreeAllTemporary();

  Platform::ExitGui();
}

void SolveSpaceUI::Refresh() {
  // generateAll must happen bfore updating displays
  if (scheduledGenerateAll) {
    GenerateAll(Generate::DIRTY, /*andFindFree=*/false, /*genForBBox=*/false);
    scheduledGenerateAll = false;
  }
  if (scheduledShowTW) {
    TW.Show();
    scheduledShowTW = false;
  }
}

void SolveSpaceUI::ScheduleGenerateAll() {
  scheduledGenerateAll = true;
  //    refreshTimer->RunAfterProcessingEvents();
}

void SolveSpaceUI::ScheduleShowTW() {
  scheduledShowTW = true;
  //    refreshTimer->RunAfterProcessingEvents();
}

void SolveSpaceUI::ScheduleAutosave() {
  autosaveTimer->RunAfter(autosaveInterval * 60 * 1000);
}

double SolveSpaceUI::MmPerUnit() {
  switch (viewUnits) {
  case Unit::INCHES: return 25.4;
  case Unit::FEET_INCHES: return 25.4; // The 'unit' is still inches
  case Unit::METERS: return 1000.0;
  case Unit::MM: return 1.0;
  }
  return 1.0;
}
const char *SolveSpaceUI::UnitName() {
  switch (viewUnits) {
  case Unit::INCHES: return "in";
  case Unit::FEET_INCHES: return "in";
  case Unit::METERS: return "m";
  case Unit::MM: return "mm";
  }
  return "";
}

std::string SolveSpaceUI::MmToString(double v, bool editable) {
  v /= MmPerUnit();
  // The syntax 2' 6" for feet and inches is not something we can (currently)
  // parse back from a string so if editable is true, we treat FEET_INCHES the
  // same as INCHES and just return the unadorned decimal number of inches.
  if (viewUnits == Unit::FEET_INCHES && !editable) {
    // Now convert v from inches to 64'ths of an inch, to make rounding easier.
    v = floor((v + (1.0 / 128.0)) * 64.0);
    int feet = (int)(v / (12.0 * 64.0));
    v = v - (feet * 12.0 * 64.0);
    // v is now the feet-less remainder in 1/64 inches
    int inches = (int)(v / 64.0);
    int numerator = (int)(v - ((double)inches * 64.0));
    int denominator = 64;
    // Divide down to smallest denominator where the numerator is still a whole number
    while ((numerator != 0) && ((numerator & 1) == 0)) {
      numerator /= 2;
      denominator /= 2;
    }
    std::ostringstream str;
    if (feet != 0) {
      str << feet << "'-";
    }
    // For something like 0.5, show 1/2" rather than 0 1/2"
    if (!(feet == 0 && inches == 0 && numerator != 0)) {
      str << inches;
    }
    if (numerator != 0) {
      str << " " << numerator << "/" << denominator;
    }
    str << "\"";
    return str.str();
  }

  int digits = UnitDigitsAfterDecimal();
  double minimum = 0.5 * pow(10, -digits);
  while ((v < minimum) && (v > LENGTH_EPS)) {
    digits++;
    minimum *= 0.1;
  }
  return ssprintf("%.*f", digits, v);
}
static const char *DimToString(int dim) {
  switch (dim) {
  case 3: return "³";
  case 2: return "²";
  case 1: return "";
  default: ssassert(false, "Unexpected dimension");
  }
}
static std::pair<int, std::string> SelectSIPrefixMm(int ord, int dim) {
  // decide what units to use depending on the order of magnitude of the
  // measure in meters and the dimension (1,2,3 lenear, area, volume)
  switch (dim) {
  case 0:
  case 1:
    if (ord >= 3)
      return {3, "km"};
    else if (ord >= 0)
      return {0, "m"};
    else if (ord >= -2)
      return {-2, "cm"};
    else if (ord >= -3)
      return {-3, "mm"};
    else if (ord >= -6)
      return {-6, "µm"};
    else
      return {-9, "nm"};
    break;
  case 2:
    if (ord >= 5)
      return {3, "km"};
    else if (ord >= 0)
      return {0, "m"};
    else if (ord >= -2)
      return {-2, "cm"};
    else if (ord >= -6)
      return {-3, "mm"};
    else if (ord >= -13)
      return {-6, "µm"};
    else
      return {-9, "nm"};
    break;
  case 3:
    if (ord >= 7)
      return {3, "km"};
    else if (ord >= 0)
      return {0, "m"};
    else if (ord >= -5)
      return {-2, "cm"};
    else if (ord >= -11)
      return {-3, "mm"};
    else
      return {-6, "µm"};
    break;
  default: dbp("dimensions over 3 not supported"); break;
  }
  return {0, "m"};
}
static std::pair<int, std::string> SelectSIPrefixInch(int deg) {
  if (deg >= 0)
    return {0, "in"};
  else if (deg >= -3)
    return {-3, "mil"};
  else
    return {-6, "µin"};
}
std::string SolveSpaceUI::MmToStringSI(double v, int dim) {
  bool compact = false;
  if (dim == 0) {
    if (!useSIPrefixes)
      return MmToString(v);
    compact = true;
    dim = 1;
  }

  bool inches = (viewUnits == Unit::INCHES) || (viewUnits == Unit::FEET_INCHES);
  v /= pow(inches ? 25.4 : 1000, dim);
  int vdeg = (int)(log10(fabs(v)));
  std::string unit;
  if (fabs(v) > 0.0) {
    int sdeg = 0;
    std::tie(sdeg, unit) = inches ? SelectSIPrefixInch(vdeg / dim) : SelectSIPrefixMm(vdeg, dim);
    v /= pow(10.0, sdeg * dim);
  }
  if (viewUnits == Unit::FEET_INCHES && fabs(v) > pow(12.0, dim)) {
    unit = "ft";
    v /= pow(12.0, dim);
  }
  int pdeg = (int)ceil(log10(fabs(v) + 1e-10));
  return ssprintf("%.*g%s%s%s", pdeg + UnitDigitsAfterDecimal(), v, compact ? "" : " ",
                  unit.c_str(), DimToString(dim));
}
std::string SolveSpaceUI::DegreeToString(double v) {
  if (fabs(v - floor(v)) > 1e-10) {
    return ssprintf("%.*f", afterDecimalDegree, v);
  } else {
    return ssprintf("%.0f", v);
  }
}
double SolveSpaceUI::ExprToMm(Expr *e) {
  return (e->Eval()) * MmPerUnit();
}
double SolveSpaceUI::StringToMm(const std::string &str) {
  return std::stod(str) * MmPerUnit();
}
double SolveSpaceUI::ChordTolMm() {
  if (exportMode)
    return ExportChordTolMm();
  return chordTolCalculated;
}
double SolveSpaceUI::ExportChordTolMm() {
  return exportChordTol / exportScale;
}
int SolveSpaceUI::GetMaxSegments() {
  if (exportMode)
    return exportMaxSegments;
  return maxSegments;
}
int SolveSpaceUI::UnitDigitsAfterDecimal() {
  return (viewUnits == Unit::INCHES || viewUnits == Unit::FEET_INCHES) ? afterDecimalInch
                                                                       : afterDecimalMm;
}
void SolveSpaceUI::SetUnitDigitsAfterDecimal(int v) {
  if (viewUnits == Unit::INCHES || viewUnits == Unit::FEET_INCHES) {
    afterDecimalInch = v;
  } else {
    afterDecimalMm = v;
  }
}

double SolveSpaceUI::CameraTangent() {
  if (!usePerspectiveProj) {
    return 0;
  } else {
    return cameraTangent;
  }
}

void SolveSpaceUI::AfterNewFile() {
  // Clear out the traced point, which is no longer valid
  traced.point = Entity::NO_ENTITY;
  traced.path.l.Clear();
  // and the naked edges
  nakedEdges.Clear();

  // Quit export mode
  justExportedInfo.draw = false;
  centerOfMass.draw = false;
  exportMode = false;

  // GenerateAll() expects the view to be valid, because it uses that to
  // fill in default values for extrusion depths etc. (which won't matter
  // here, but just don't let it work on garbage)
  SS.GW.offset = Vector::From(0, 0, 0);
  SS.GW.projRight = Vector::From(1, 0, 0);
  SS.GW.projUp = Vector::From(0, 1, 0);

  GenerateAll(Generate::ALL);

  // TODO: resetting the graphics window should not require passing a size or pixel device ratio
  //    GW.Init();
  //    TW.Init();

  unsaved = false;

  GW.ZoomToFit();

  // Create all the default styles; they'll get created on the fly anyways,
  // but can't hurt to do it now.
  Style::CreateAllDefaultStyles();
}

void SolveSpaceUI::AddToRecentList(const Platform::Path &filename) {
  auto it = std::find_if(recentFiles.begin(), recentFiles.end(),
                         [&](const Platform::Path &p) { return p.Equals(filename); });
  if (it != recentFiles.end()) {
    recentFiles.erase(it);
  }

  if (recentFiles.size() > MAX_RECENT) {
    recentFiles.erase(recentFiles.begin() + MAX_RECENT);
  }

  recentFiles.insert(recentFiles.begin(), filename);
}

void SolveSpaceUI::Autosave() {
  ScheduleAutosave();

  if (!saveFile.IsEmpty() && unsaved) {
    SaveToFile(saveFile.WithExtension(BACKUP_EXT));
  }
}

void SolveSpaceUI::RemoveAutosave() {
  Platform::Path autosaveFile = saveFile.WithExtension(BACKUP_EXT);
  RemoveFile(autosaveFile);
}

bool SolveSpaceUI::OkayToStartNewFile() {
  assert("SolveSpaceUI::OkayToStartNewFile() shouldn't be called");
  return false;
}

void SolveSpaceUI::GetPngExportImageFilename() {
  assert("SolveSpaceUI::GetPngExportImageFilename() shouldn't be called");
}

void SolveSpaceUI::ShowNakedEdges(bool reportOnlyWhenNotOkay) {
  SS.nakedEdges.Clear();

  Group *g = SK.GetGroup(SS.GW.activeGroup);
  SMesh *m = &(g->displayMesh);
  SKdNode *root = SKdNode::From(m);
  bool inters, leaks;
  root->MakeCertainEdgesInto(&(SS.nakedEdges), EdgeKind::NAKED_OR_SELF_INTER,
                             /*coplanarIsInter=*/true, &inters, &leaks);

  if (reportOnlyWhenNotOkay && !inters && !leaks && SS.nakedEdges.l.IsEmpty()) {
    return;
  }
  SS.GW.Invalidate();

  const char *intersMsg = inters ? _("The mesh is self-intersecting (NOT okay, invalid).")
                                 : _("The mesh is not self-intersecting (okay, valid).");
  const char *leaksMsg = leaks ? _("The mesh has naked edges (NOT okay, invalid).")
                               : _("The mesh is watertight (okay, valid).");

  std::string cntMsg = ssprintf(_("\n\nThe model contains %d triangles, from %d surfaces."),
                                g->displayMesh.l.n, g->runningShell.surface.n);

  if (SS.nakedEdges.l.IsEmpty()) {
    Message(_("%s\n\n%s\n\nZero problematic edges, good.%s"), intersMsg, leaksMsg, cntMsg.c_str());
  } else {
    Error(_("%s\n\n%s\n\n%d problematic edges, bad.%s"), intersMsg, leaksMsg, SS.nakedEdges.l.n,
          cntMsg.c_str());
  }
}

void SolveSpaceUI::Clear() {
  sys.Clear();
  for (int i = 0; i < MAX_UNDO; i++) {
    if (i < undo.cnt)
      undo.d[i].Clear();
    if (i < redo.cnt)
      redo.d[i].Clear();
  }
  GW.openRecentMenu = NULL;
  GW.linkRecentMenu = NULL;
  GW.showGridMenuItem = NULL;
  GW.dimSolidModelMenuItem = NULL;
  GW.perspectiveProjMenuItem = NULL;
  GW.explodeMenuItem = NULL;
  GW.showToolbarMenuItem = NULL;
  GW.showTextWndMenuItem = NULL;
  GW.fullScreenMenuItem = NULL;
  GW.unitsMmMenuItem = NULL;
  GW.unitsMetersMenuItem = NULL;
  GW.unitsInchesMenuItem = NULL;
  GW.unitsFeetInchesMenuItem = NULL;
  GW.inWorkplaneMenuItem = NULL;
  GW.in3dMenuItem = NULL;
  GW.undoMenuItem = NULL;
  GW.redoMenuItem = NULL;
}

#include "geometry/vector.hpp"