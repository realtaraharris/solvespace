//-----------------------------------------------------------------------------
// Our platform support functions for the headless (no OpenGL) test runner.
//
// Copyright 2016 whitequark
//-----------------------------------------------------------------------------

#include <Window.h>

#include "App.h"

#include "solvespace.h"
#include "ssg.h"

extern BApplication *be_app;

namespace SolveSpace {

  //-----------------------------------------------------------------------------
  // Rendering
  //-----------------------------------------------------------------------------

  std::shared_ptr<ViewportCanvas> CreateRenderer() {
    return std::make_shared<AggPixmapRenderer>();
  }

  namespace Platform {

    //-----------------------------------------------------------------------------
    // Fatal errors
    //-----------------------------------------------------------------------------

    void FatalError(const std::string &message) {
      fprintf(stderr, "%s", message.c_str());
      abort();
    }

    //-----------------------------------------------------------------------------
    // Settings
    //-----------------------------------------------------------------------------

    class SettingsImplDummy final : public Settings {
  public:
      void FreezeInt(const std::string &key, uint32_t value) override {}

      uint32_t ThawInt(const std::string &key, uint32_t defaultValue = 0) override {
        return defaultValue;
      }

      void FreezeFloat(const std::string &key, double value) override {}

      double ThawFloat(const std::string &key, double defaultValue = 0.0) override {
        return defaultValue;
      }

      void FreezeString(const std::string &key, const std::string &value) override {}

      std::string ThawString(const std::string &key,
                             const std::string &defaultValue = "") override {
        return defaultValue;
      }
    };

    SettingsRef GetSettings() {
      static std::shared_ptr<SettingsImplDummy> settings = std::make_shared<SettingsImplDummy>();
      return settings;
    }

    //-----------------------------------------------------------------------------
    // Timers
    //-----------------------------------------------------------------------------

    class TimerImplDummy final : public Timer {
  public:
      void RunAfter(unsigned milliseconds) override {}
    };

    TimerRef CreateTimer() {
      return std::make_shared<TimerImplDummy>();
    }

    //-----------------------------------------------------------------------------
    // Application-wide APIs
    //-----------------------------------------------------------------------------

    std::vector<Platform::Path> fontFiles;
    std::vector<Platform::Path> GetFontFiles() {
      return fontFiles;
    }
  } // namespace Platform
} // namespace SolveSpace