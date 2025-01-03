//-----------------------------------------------------------------------------
// Utility functions, mostly various kinds of vector math (working on real
// numbers, not working on quantities in the symbolic algebra system).
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"
#include "platform/EventHooks.h"

void SolveSpace::AssertFailure(const char *file, unsigned line, const char *function,
                               const char *condition, const char *message) {
  std::string formattedMsg;
  formattedMsg += ssprintf("File %s, line %u, function %s:\n", file, line, function);
  formattedMsg += ssprintf("Assertion failed: %s.\n", condition);
  formattedMsg += ssprintf("Message: %s.\n", message);
  SolveSpace::Platform::FatalError(formattedMsg);
}

std::string SolveSpace::ssprintf(const char *fmt, ...) {
  va_list va;

  va_start(va, fmt);
  int size = vsnprintf(NULL, 0, fmt, va);
  ssassert(size >= 0, "vsnprintf could not encode string");
  va_end(va);

  std::string result;
  result.resize(size + 1);

  va_start(va, fmt);
  vsnprintf(&result[0], size + 1, fmt, va);
  va_end(va);

  result.resize(size);
  return result;
}

char32_t utf8_iterator::operator* () {
  const uint8_t *it = (const uint8_t *)this->p;
  char32_t result = *it;

  if ((result & 0x80) != 0) {
    unsigned int mask = 0x40;

    do {
      result <<= 6;
      unsigned int c = (*++it);
      mask <<= 5;
      result += c - 0x80;
    } while ((result & mask) != 0);

    result &= mask - 1;
  }

  this->n = (const char *)(it + 1);
  return result;
}

int64_t SolveSpace::GetMilliseconds() {
  auto timestamp = std::chrono::steady_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count();
}

//-----------------------------------------------------------------------------
// Format the string for our message box appropriately, and then display
// that string.
//-----------------------------------------------------------------------------
static void MessageBox(const char *fmt, va_list va, bool error,
                       std::function<void()> onDismiss = std::function<void()>()) {
  va_list va_size;
  va_copy(va_size, va);
  int size = vsnprintf(NULL, 0, fmt, va_size);
  ssassert(size >= 0, "vsnprintf could not encode string");
  va_end(va_size);

  std::string text;
  text.resize(size);

  vsnprintf(&text[0], size + 1, fmt, va);

  // Split message text using a heuristic for better presentation.
  size_t separatorAt = 0;
  while (separatorAt != std::string::npos) {
    size_t dotAt = text.find('.', separatorAt + 1), colonAt = text.find(':', separatorAt + 1);
    separatorAt = std::min(dotAt, colonAt);
    if (separatorAt == std::string::npos ||
        (separatorAt + 1 < text.size() && isspace(text[separatorAt + 1]))) {
      break;
    }
  }
  std::string message = text;
  std::string description;
  if (separatorAt != std::string::npos) {
    message = text.substr(0, separatorAt + 1);
    if (separatorAt + 1 < text.size()) {
      description = text.substr(separatorAt + 1);
    }
  }

  if (description.length() > 0) {
    std::string::iterator it = description.begin();
    while (isspace(*it))
      it++;
    description = description.substr(it - description.begin());
  }

  if (error) {
    WarningDialog(message, description, onDismiss);
  } else {
    ErrorDialog(message, description, onDismiss);
  }
}
void SolveSpace::Error(const char *fmt, ...) {
  va_list f;
  va_start(f, fmt);
  MessageBox(fmt, f, /*error=*/true);
  va_end(f);
}
void SolveSpace::Message(const char *fmt, ...) {
  va_list f;
  va_start(f, fmt);
  MessageBox(fmt, f, /*error=*/false);
  va_end(f);
}
void SolveSpace::MessageAndRun(std::function<void()> onDismiss, const char *fmt, ...) {
  va_list f;
  va_start(f, fmt);
  MessageBox(fmt, f, /*error=*/false, onDismiss);
  va_end(f);
}

size_t VectorHash::operator() (const Vector &v) const {
  const size_t size = (size_t)pow(std::numeric_limits<size_t>::max(), 1.0 / 3.0) - 1;
  const double eps = 4.0 * LENGTH_EPS;

  double x = std::fabs(v.x) / eps;
  double y = std::fabs(v.y) / eps;
  double z = std::fabs(v.y) / eps;

  size_t xs = size_t(fmod(x, (double)size));
  size_t ys = size_t(fmod(y, (double)size));
  size_t zs = size_t(fmod(z, (double)size));

  return (zs * size + ys) * size + xs;
}

bool VectorPred::operator() (Vector a, Vector b) const {
  return a.Equals(b, LENGTH_EPS);
}

const std::vector<double> &SolveSpace::StipplePatternDashes(StipplePattern pattern) {
  static bool initialized;
  static std::vector<double> dashes[(size_t)StipplePattern::LAST + 1];
  if (!initialized) {
    // Inkscape ignores all elements that are exactly zero instead of drawing
    // them as dots, so set those to 1e-6.
    dashes[(size_t)StipplePattern::CONTINUOUS] = {};
    dashes[(size_t)StipplePattern::SHORT_DASH] = {1.0, 2.0};
    dashes[(size_t)StipplePattern::DASH] = {1.0, 1.0};
    dashes[(size_t)StipplePattern::DASH_DOT] = {1.0, 0.5, 1e-6, 0.5};
    dashes[(size_t)StipplePattern::DASH_DOT_DOT] = {1.0, 0.5, 1e-6, 0.5, 1e-6, 0.5};
    dashes[(size_t)StipplePattern::DOT] = {1e-6, 0.5};
    dashes[(size_t)StipplePattern::LONG_DASH] = {2.0, 0.5};
    dashes[(size_t)StipplePattern::FREEHAND] = {1.0, 2.0};
    dashes[(size_t)StipplePattern::ZIGZAG] = {1.0, 2.0};
  }

  return dashes[(size_t)pattern];
}

double SolveSpace::StipplePatternLength(StipplePattern pattern) {
  static bool initialized;
  static double lengths[(size_t)StipplePattern::LAST + 1];
  if (!initialized) {
    for (size_t i = 0; i < (size_t)StipplePattern::LAST; i++) {
      const std::vector<double> &dashes = StipplePatternDashes((StipplePattern)i);
      double length = 0.0;
      for (double dash : dashes) {
        length += dash;
      }
      lengths[i] = length;
    }
  }

  return lengths[(size_t)pattern];
}
