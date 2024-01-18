#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <MenuBar.h>
#include <Window.h>

#include "EditorView.h"
#include "PropertyBrowser.h"
#include "ViewParameters.h"

class MainWindow : public BWindow {
  public:
    MainWindow(void);
    void MessageReceived(BMessage *msg);
    bool QuitRequested(void);
    void LoadSettings();
    void SaveSettings();
  private:
    BMenuBar *menuBar;

    BMenuItem *undoMenuItem;
    BMenuItem *redoMenuItem;

    BWindow *toolWindow;
    PropertyBrowser *propertyBrowser;
    ViewParameters *viewParameters;
    EditorView *editorView;
    BPath *currentFilePath;

    BMessage* settings;
};

#endif
