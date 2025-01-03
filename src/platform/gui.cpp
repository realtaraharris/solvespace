//-----------------------------------------------------------------------------
// Platform-dependent GUI functionality that has only minor differences.
//
// Copyright 2018 whitequark
//-----------------------------------------------------------------------------
#include "solvespace.h"

namespace SolveSpace {
  namespace Platform {

    //-----------------------------------------------------------------------------
    // Settings
    //-----------------------------------------------------------------------------

    void Settings::FreezeBool(const std::string &key, bool value) {
      FreezeInt(key, (int)value);
    }

    bool Settings::ThawBool(const std::string &key, bool defaultValue) {
      return ThawInt(key, (int)defaultValue) != 0;
    }

    void Settings::FreezeColor(const std::string &key, RgbaColor value) {
      FreezeInt(key, value.ToPackedInt());
    }

    RgbaColor Settings::ThawColor(const std::string &key, RgbaColor defaultValue) {
      return RgbaColor::FromPackedInt(ThawInt(key, defaultValue.ToPackedInt()));
    }
  } // namespace Platform
} // namespace SolveSpace
