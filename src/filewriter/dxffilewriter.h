#ifndef DXF_FILE_WRITER_H
#define DXF_FILE_WRITER_H

#include "vectorfilewriter.h"

class DxfFileWriter : public VectorFileWriter {
  public:
  struct BezierPath {
    std::vector<SBezier *> beziers;
  };

  std::vector<BezierPath>          paths;
  IdList<Constraint, hConstraint> *constraint;

  static const char *lineTypeName (StipplePattern stippleType);

  bool OutputConstraints (IdList<Constraint, hConstraint> *constraint) override;

  void StartPath (RgbaColor strokeRgb, double lineWidth, bool filled, RgbaColor fillRgb,
                  hStyle hs) override;
  void FinishPath (RgbaColor strokeRgb, double lineWidth, bool filled, RgbaColor fillRgb,
                   hStyle hs) override;
  void Triangle (STriangle *tr) override;
  void Bezier (SBezier *sb) override;
  void Background (RgbaColor color) override;
  void StartFile () override;
  void FinishAndCloseFile () override;
  bool HasCanvasSize () const override { return false; }
  bool CanOutputMesh () const override { return false; }
  bool NeedToOutput (Constraint *c);
};
#endif // DXF_FILE_WRITER_H