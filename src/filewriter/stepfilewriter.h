#ifndef STEP_FILE_WRITER_H
#define STEP_FILE_WRITER_H

class StepFileWriter {
public:
    void ExportSurfacesTo(const Platform::Path &filename);
    void WriteHeader();
    void WriteProductHeader();
    int ExportCurve(SBezier *sb);
    int ExportCurveLoop(SBezierLoop *loop, bool inner);
    void ExportSurface(SSurface *ss, SBezierList *sbl);
    void WriteWireframe();
    void WriteFooter();

    List<int> curves;
    List<int> advancedFaces;
    FILE *f;
    int id;
};

#endif // STEP_FILE_WRITER_H