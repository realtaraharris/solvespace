#pragma once

class StepFileWriter {
  public:
  void ExportSurfacesTo (const Platform::Path &filename);
  void WriteHeader ();
  void WriteProductHeader ();
  int  ExportCurve (SBezier *sb);
  int  ExportCurveLoop (SBezierLoop *loop, bool inner);
  void ExportSurface (SSurface *ss, SBezierList *sbl);
  void WriteWireframe ();
  void WriteFooter ();

  List<int> curves;
  List<int> advancedFaces;
  FILE     *f;
  int       id;
};