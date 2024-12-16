#include "solvespace.h"

std::string MakeStipplePattern(StipplePattern pattern, double scale, char delimiter,
                               bool inkscapeWorkaround = false) {
  scale /= 2.0;

  // Inkscape ignores all elements that are exactly zero instead of drawing
  // them as dots.
  double zero = inkscapeWorkaround ? 1e-6 : 0;

  std::string result;
  switch (pattern) {
  case StipplePattern::CONTINUOUS: return "";

  case StipplePattern::SHORT_DASH: result = ssprintf("%.3f_%.3f", scale, scale * 2.0); break;
  case StipplePattern::DASH: result = ssprintf("%.3f_%.3f", scale, scale); break;
  case StipplePattern::DASH_DOT:
    result = ssprintf("%.3f_%.3f_%.6f_%.3f", scale, scale * 0.5, zero, scale * 0.5);
    break;
  case StipplePattern::DASH_DOT_DOT:
    result = ssprintf("%.3f_%.3f_%.6f_%.3f_%.6f_%.3f", scale, scale * 0.5, zero, scale * 0.5,
                      scale * 0.5, zero);
    break;
  case StipplePattern::DOT: result = ssprintf("%.6f_%.3f", zero, scale * 0.5); break;
  case StipplePattern::LONG_DASH: result = ssprintf("%.3f_%.3f", scale * 2.0, scale * 0.5); break;

  case StipplePattern::FREEHAND:
  case StipplePattern::ZIGZAG: ssassert(false, "Freehand and zigzag export not implemented");
  }
  std::replace(result.begin(), result.end(), '_', delimiter);
  return result;
}
