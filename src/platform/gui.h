//-----------------------------------------------------------------------------
// An abstraction for platform-dependent GUI functionality.
//
// Copyright 2018 whitequark
//-----------------------------------------------------------------------------

#pragma once

namespace SolveSpace {
  class RgbaColor;

  namespace Platform {

    //-----------------------------------------------------------------------------
    // Events
    //-----------------------------------------------------------------------------

    // A mouse input event.
    class MouseEvent {
  public:
      enum class Type {
        MOTION,
        PRESS,
        DBL_PRESS,
        RELEASE,
        SCROLL_VERT,
        LEAVE,
      };

      enum class Button {
        NONE,
        LEFT,
        MIDDLE,
        RIGHT,
      };

      Type   type;
      double x;
      double y;
      bool   shiftDown;
      bool   controlDown;
      union {
        Button button;      // for Type::{MOTION,PRESS,DBL_PRESS,RELEASE}
        double scrollDelta; // for Type::SCROLL_VERT
      };
    };

    //-----------------------------------------------------------------------------
    // Interfaces
    //-----------------------------------------------------------------------------

    // Handling fatal errors.
    [[noreturn]]
    void FatalError (const std::string &message);

    // A native settings store.
    class Settings {
  public:
      virtual ~Settings () = default;

      virtual void     FreezeInt (const std::string &key, uint32_t value)          = 0;
      virtual uint32_t ThawInt (const std::string &key, uint32_t defaultValue = 0) = 0;

      virtual void   FreezeFloat (const std::string &key, double value)            = 0;
      virtual double ThawFloat (const std::string &key, double defaultValue = 0.0) = 0;

      virtual void        FreezeString (const std::string &key, const std::string &value) = 0;
      virtual std::string ThawString (const std::string &key,
                                      const std::string &defaultValue = "")               = 0;

      virtual void FreezeBool (const std::string &key, bool value);
      virtual bool ThawBool (const std::string &key, bool defaultValue = false);

      virtual void      FreezeColor (const std::string &key, RgbaColor value);
      virtual RgbaColor ThawColor (const std::string &key, RgbaColor defaultValue);
    };

    typedef std::shared_ptr<Settings> SettingsRef;

    SettingsRef GetSettings ();

    // A native single-shot timer.
    class Timer {
  public:
      std::function<void ()> onTimeout;

      virtual ~Timer () = default;

      virtual void RunAfter (unsigned milliseconds) = 0;
      virtual void RunAfterNextFrame () { RunAfter (1); }
      virtual void RunAfterProcessingEvents () { RunAfter (0); }
    };

    typedef std::shared_ptr<Timer> TimerRef;

    TimerRef CreateTimer ();

    //-----------------------------------------------------------------------------
    // Application-wide APIs
    //-----------------------------------------------------------------------------

    std::vector<Platform::Path> GetFontFiles ();
  } // namespace Platform
} // namespace SolveSpace
