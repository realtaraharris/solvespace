//-----------------------------------------------------------------------------
// Our platform support functions for the headless (no OpenGL) test runner.
//
// Copyright 2016 whitequark
//-----------------------------------------------------------------------------
#include "solvespace.h"
#include <Window.h>

#include "App.h"

extern BApplication* be_app;

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
    void FreezeInt(const std::string &key, uint32_t value) override {
    }

    uint32_t ThawInt(const std::string &key, uint32_t defaultValue = 0) override {
        return defaultValue;
    }

    void FreezeFloat(const std::string &key, double value) override {
    }

    double ThawFloat(const std::string &key, double defaultValue = 0.0) override {
        return defaultValue;
    }

    void FreezeString(const std::string &key, const std::string &value) override {
    }

    std::string ThawString(const std::string &key, const std::string &defaultValue = "") override {
        return defaultValue;
    }
};

SettingsRef GetSettings() {
    static std::shared_ptr<SettingsImplDummy> settings =
                std::make_shared<SettingsImplDummy>();
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
// Menus
//-----------------------------------------------------------------------------

MenuRef CreateMenu() {
    return std::shared_ptr<Menu>();
}

MenuBarRef GetOrCreateMainMenu(bool *unique) {
    *unique = false;
    return std::shared_ptr<MenuBar>();
}

// Don't do put much functionality in here; just stub this out to aid future removal
class WindowImplHaiku final : public Window {
//    HaikuWindow       haikuWindow;
public:
    WindowImplHaiku(Window::Kind kind) {}

    virtual double GetPixelDensity() override { return 1.0; }

    virtual void GetContentSize(double *width, double *height) override {
	const BSize size = be_app->WindowAt(0)->Size();
	*width = size.Width();
	*height = size.Height() - MENUBAR_HEIGHT;
    }

    virtual int GetDevicePixelRatio() override { return 1; }
    // Returns (fractional) font scale, to be applied on top of (integral) device pixel ratio.
    virtual double GetDeviceFontScale() {
        return GetPixelDensity() / GetDevicePixelRatio() / 96.0;
    }

    virtual bool IsVisible() override { return true; }
    virtual void SetVisible(bool visible) override {}
    virtual void Focus() override {}

    virtual bool IsFullScreen() override { return false; }
    virtual void SetFullScreen(bool fullScreen) override {}

    virtual void SetTitle(const std::string &title) override {}
    virtual bool SetTitleForFilename(const Path &filename) { return false; }

    virtual void SetMenuBar(MenuBarRef menuBar) override {}
    virtual void SetMinContentSize(double width, double height) override {}

    virtual void FreezePosition(SettingsRef settings, const std::string &key) override {}
    virtual void ThawPosition(SettingsRef settings, const std::string &key) override {}

    virtual void SetCursor(Cursor cursor) override {}
    virtual void SetTooltip(const std::string &text, double x, double y,
                            double width, double height) override {}

    virtual bool IsEditorVisible() override { return true; }
    virtual void ShowEditor(double x, double y, double fontHeight, double minWidth,
                            bool isMonospace, const std::string &text) override {
        dbp("in ShowEditor!");
        be_app->WindowAt(0)->PostMessage(new BMessage(M_SHOW_EDITOR));
    }
    virtual void HideEditor() override {}

    virtual void SetScrollbarVisible(bool visible) override {}
    virtual void ConfigureScrollbar(double min, double max, double pageSize) override {}
    virtual double GetScrollbarPosition() override { return 0; }
    virtual void SetScrollbarPosition(double pos) override {}

    virtual void Invalidate() override {}
};

//-----------------------------------------------------------------------------
// Windows
//-----------------------------------------------------------------------------

WindowRef CreateWindow(Window::Kind kind, WindowRef parentWindow) {
//    return std::shared_ptr<Window>();

    auto window = std::make_shared<WindowImplHaiku>(kind);
/*  if(parentWindow) {
        window->haikuWindow.set_transient_for(
            std::static_pointer_cast<WindowImplHaiku>(parentWindow)->haikuWindow);
    } */
    return window;
}

void Request3DConnexionEventsForWindow(WindowRef window) {}

//-----------------------------------------------------------------------------
// Message dialogs
//-----------------------------------------------------------------------------

MessageDialogRef CreateMessageDialog(WindowRef parentWindow) {
    return std::shared_ptr<MessageDialog>();
}

//-----------------------------------------------------------------------------
// File dialogs
//-----------------------------------------------------------------------------

FileDialogRef CreateOpenFileDialog(WindowRef parentWindow) {
    return std::shared_ptr<FileDialog>();
}

FileDialogRef CreateSaveFileDialog(WindowRef parentWindow) {
    return std::shared_ptr<FileDialog>();
}

//-----------------------------------------------------------------------------
// Application-wide APIs
//-----------------------------------------------------------------------------

std::vector<Platform::Path> fontFiles;
std::vector<Platform::Path> GetFontFiles() {
    return fontFiles;
}

void OpenInBrowser(const std::string &url) {}

std::vector<std::string> InitGui(int argc, char **argv) {
    return {};
}

void RunGui() {}

void ExitGui() {
    exit(0);
}

void ClearGui() {}

}

}
