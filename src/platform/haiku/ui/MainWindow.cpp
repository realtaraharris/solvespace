#include "MainWindow.h"

#include <Alert.h> // BAlert
#include <Application.h>
#include <Button.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuItem.h>
#include <StorageKit.h>
#include <View.h>

#include "App.h" // contains message enums
#include "Solver.h"
#include "Toolbar.h"

#include <iostream>

#define INIT_X 125
#define INIT_Y 45

MainWindow::MainWindow(void)
    : BWindow(BRect(INIT_X, INIT_Y, INIT_X + MIN_WIDTH,
                    INIT_Y + MIN_HEIGHT + MENUBAR_HEIGHT),
              "SolveSpace", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS,
              B_CURRENT_WORKSPACE) {
    BRect rect(Bounds());

    SetSizeLimits(
        MIN_WIDTH, 100000, MIN_HEIGHT + MENUBAR_HEIGHT,
        100000); // the Haiku API needs a way to not set any upper bound

    menuBar = new BMenuBar(rect, "menubar");
    BMenu *fileMenu = new BMenu("File");
    fileMenu->AddItem(new BMenuItem("Open", new BMessage(M_OPEN_FILE), 'O'));
    fileMenu->AddItem(new BMenuItem("Save", new BMessage(M_SAVE_FILE), 'S'));
    fileMenu->AddItem(new BMenuItem("Quit", new BMessage(M_QUIT_APP), 'Q'));
    menuBar->AddItem(fileMenu);

    BMenu *viewMenu = new BMenu("View");
    viewMenu->AddItem(new BMenuItem("Zoom in", new BMessage(ZOOM_IN), '+'));
    viewMenu->AddItem(new BMenuItem("Zoom out", new BMessage(ZOOM_OUT), '-'));
    viewMenu->AddItem(
        new BMenuItem("Zoom to fit", new BMessage(ZOOM_TO_FIT), 'f'));
    viewMenu->AddSeparatorItem();
    viewMenu->AddItem(new BMenuItem(
        "Align view to workplane", new BMessage(ALIGN_VIEW_TO_WORKPLANE), 'W'));
    viewMenu->AddItem(new BMenuItem(
        "Nearest ortho view", new BMessage(NEAREST_ORTHO_TOOL_BTN_CLICKED),
        '2')); // B_F2_KEY
    viewMenu->AddItem(new BMenuItem("Nearest isometric view",
                                    new BMessage(NEAREST_ISO_TOOL_BTN_CLICKED),
                                    '3')); // B_F3_KEY
    viewMenu->AddItem(new BMenuItem("Center view at point",
                                    new BMessage(CENTER_VIEW_AT_POINT),
                                    '4')); // B_F4_KEY
    viewMenu->AddSeparatorItem();

    viewMenu->AddItem(
        new BMenuItem("Show snap grid", new BMessage(TOGGLE_SNAP_GRID), '>'));
    menuBar->AddItem(viewMenu);

    BMenu *groupMenu = new BMenu("Group");
    groupMenu->AddItem(new BMenuItem("Sketch in 3D", new BMessage(M_GROUP_3D),
                                     '3')); // Command::GROUP_3D
    groupMenu->AddItem(new BMenuItem("Sketch in new workplane",
                                     new BMessage(M_GROUP_WRKPL),
                                     'w')); // Command::GROUP_WRKPL
    groupMenu->AddSeparatorItem();
    groupMenu->AddItem(new BMenuItem("Step translating",
                                     new BMessage(M_GROUP_TRANS),
                                     't')); // Command::GROUP_TRANS
    groupMenu->AddItem(new BMenuItem("Step rotating", new BMessage(M_GROUP_ROT),
                                     'r')); // Command::GROUP_ROT
    groupMenu->AddSeparatorItem();
    groupMenu->AddItem(new BMenuItem("Extrude", new BMessage(M_GROUP_EXTRUDE),
                                     'x')); // Command::GROUP_EXTRUDE
    groupMenu->AddItem(new BMenuItem("Helix", new BMessage(M_GROUP_HELIX),
                                     'h')); // Command::GROUP_HELIX
    groupMenu->AddItem(new BMenuItem("Lathe", new BMessage(M_GROUP_LATHE),
                                     'l')); // Command::GROUP_LATHE
    groupMenu->AddItem(new BMenuItem("Revolve", new BMessage(M_GROUP_REVOLVE),
                                     'v')); // Command::GROUP_REVOLVE
    groupMenu->AddSeparatorItem();
    groupMenu->AddItem(new BMenuItem("Link/Assemble...",
                                     new BMessage(M_GROUP_LINK),
                                     "")); // Command::GROUP_LINK
    groupMenu->AddItem(new BMenuItem("Link recent",
                                     new BMessage(M_GROUP_RECENT),
                                     "")); // Command::GROUP_RECENT
    menuBar->AddItem(groupMenu);

    editorView = new EditorView();

    SetLayout(new BGroupLayout(B_VERTICAL));

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .Add(menuBar)
        .Add(editorView)
        .End();

    SS.Init();

    const BSize toolbarIconSize =
        be_control_look->ComposeIconSize(B_LARGE_ICON);
    BRect toolbarRect = BRect(BPoint(10, 35), BSize(0, 0));
    toolWindow = new AppToolbar(toolbarRect, toolbarIconSize);
    toolWindow->ResizeToPreferred();
    toolWindow->Show();

    propertyBrowser = new PropertyBrowser();
    propertyBrowser->Show();

    viewParameters = new ViewParameters();
    be_app->WindowAt(VIEW_PARAMETERS)
        ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
    viewParameters->Show();

    currentFilePath = new BPath();
}

void MainWindow::MessageReceived(BMessage *msg) {
    switch (msg->what) {
    // TODO: move this into KeyboardShortcuts.cpp
    case B_KEY_DOWN: {
        int32 key, raw;
        msg->FindInt32("key", &key);
        msg->FindInt32("raw_char", &raw);
        uint32_t mods = modifiers();
        switch (key) {
        case B_F3_KEY: {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(NEAREST_ISO_TOOL_BTN_CLICKED));
                break;
        }
        }
        switch ((char)raw) {
        case B_DELETE: {
            SS.GW.MenuClipboard(SolveSpace::Command::DELETE);
            break;
        }
        case 's': {
            // TODO: tell the toolbar to toggle the line tool button
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(LINE_TOOL_BTN_CLICKED));
            break;
        }
        case 'r': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(RECT_TOOL_BTN_CLICKED));
            break;
        }
        case 'c': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(CIRCLE_TOOL_BTN_CLICKED));
            break;
        }
        case 'a': {
            if (mods & B_SHIFT_KEY) {
                be_app->WindowAt(MAIN_WINDOW)
                    ->PostMessage(new BMessage(TANGENT_ARC_TOOL_BTN_CLICKED));
            } else {
                be_app->WindowAt(MAIN_WINDOW)
                    ->PostMessage(new BMessage(ARC_TOOL_BTN_CLICKED));
            }
            break;
        }
        case 'b': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(CUBIC_SPLINE_TOOL_BTN_CLICKED));
            break;
        }
        case 'p': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(DATUM_POINT_TOOL_BTN_CLICKED));
            break;
        }
        case 'g': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(CONSTRUCTION_TOOL_BTN_CLICKED));
            break;
        }
        case 'i': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(SPLIT_CURVES_TOOL_BTN_CLICKED));
            break;
        }
        case 't': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(TEXT_TOOL_BTN_CLICKED));
            break;
        }

        case 'd': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(DISTANCE_DIA_TOOL_BTN_CLICKED));
            break;
        }
        case 'n': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(ANGLE_TOOL_BTN_CLICKED));
            break;
        }
        case 'h': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(HORIZONTAL_TOOL_BTN_CLICKED));
            break;
        }
        case 'v': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(VERTICAL_TOOL_BTN_CLICKED));
            break;
        }
        case 'l': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(PARALLEL_TOOL_BTN_CLICKED));
            break;
        }
        case '[': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(CONSTRAIN_PERP_TOOL_BTN_CLICKED));
            break;
        }
        case 'o': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(PT_ON_LINE_TOOL_BTN_CLICKED));
            break;
        }
        case 'y': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(CONSTRAIN_SYMMETRIC_TOOL_BTN_CLICKED));
            break;
        }
        case 'q': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(CONSTRAIN_EQUAL_TOOL_BTN_CLICKED));
            break;
        }
        case 'x': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(CONSTRAIN_ORIENTED_SAME_TOOL_BTN_CLICKED));
            break;
        }
        case 'u': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(OTHER_ANGLE_TOOL_BTN_CLICKED));
            break;
        }
        case 'e': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(REF_TOOL_BTN_CLICKED));
            break;
        }
        case 'w': {
            be_app->WindowAt(MAIN_WINDOW)
                ->PostMessage(new BMessage(NEAREST_ORTHO_TOOL_BTN_CLICKED));
            break;
        }
        }
        break;
    }
    case ZOOM_IN: {
        editorView->ZoomToMouse(1);
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case ZOOM_OUT: {
        editorView->ZoomToMouse(-1);
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case ZOOM_TO_FIT: {
        editorView->ZoomToFit(
            false, true); // includingInvisibles = false, useSelection = true
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case CENTER_VIEW_AT_POINT: {
        SS.GW.MenuView(SolveSpace::Command::CENTER_VIEW);
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case TOGGLE_SNAP_GRID: {
        SS.GW.showSnapGrid = !SS.GW.showSnapGrid;
        SS.GW.EnsureValidActives();
        SS.GW.Invalidate();
        editorView->Invalidate();
        if (SS.GW.showSnapGrid && !SS.GW.LockedInWorkplane()) {
            BAlert *alert = new BAlert(
                "SolveSpace information dialog",
                "No workplane is active, so the grid will not appear.", NULL,
                NULL, "OK", B_WIDTH_AS_USUAL, B_OFFSET_SPACING,
                B_WARNING_ALERT);
            alert->SetShortcut(0, B_ESCAPE);
            alert->Go();
        }
        break;
    }
    case LINE_TOOL_BTN_CLICKED: {
        SS.GW.MenuRequest(SolveSpace::Command::LINE_SEGMENT);
        break;
    }
    case RECT_TOOL_BTN_CLICKED: {
        SS.GW.MenuRequest(SolveSpace::Command::RECTANGLE);
        break;
    }
    case CIRCLE_TOOL_BTN_CLICKED: {
        SS.GW.MenuRequest(SolveSpace::Command::CIRCLE);
        break;
    }
    case ARC_TOOL_BTN_CLICKED: {
        SS.GW.MenuRequest(SolveSpace::Command::ARC);
        break;
    }
    case TANGENT_ARC_TOOL_BTN_CLICKED: {
        SS.GW.MenuRequest(SolveSpace::Command::TANGENT_ARC);
        break;
    }
    case CUBIC_SPLINE_TOOL_BTN_CLICKED: {
        SS.GW.MenuRequest(SolveSpace::Command::CUBIC);
        break;
    }
    case DATUM_POINT_TOOL_BTN_CLICKED: {
        SS.GW.MenuRequest(SolveSpace::Command::DATUM_POINT);
        break;
    }
    case CONSTRUCTION_TOOL_BTN_CLICKED: {
        SS.GW.MenuRequest(SolveSpace::Command::CONSTRUCTION);
        break;
    }
    case SPLIT_CURVES_TOOL_BTN_CLICKED: {
        SS.GW.MenuRequest(SolveSpace::Command::SPLIT_CURVES);
        break;
    }
    case TEXT_TOOL_BTN_CLICKED: {
        SS.GW.MenuRequest(SolveSpace::Command::TTF_TEXT);
        break;
    }
    case DISTANCE_DIA_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::DISTANCE_DIA);
        SS.GW.Invalidate();
        editorView->Invalidate();
        break;
    }
    case ANGLE_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::ANGLE);
        SS.GW.Invalidate();
        editorView->Invalidate();
        break;
    }
    case HORIZONTAL_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::HORIZONTAL);
        SS.GW.Invalidate();
        editorView->Invalidate();
        break;
    }
    case VERTICAL_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::VERTICAL);
        SS.GW.Invalidate();
        editorView->Invalidate();
        break;
    }
    case PARALLEL_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::PARALLEL);
        SS.GW.Invalidate();
        editorView->Invalidate();
        break;
    }
    case CONSTRAIN_PERP_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::PERPENDICULAR);
        SS.GW.Invalidate();
        editorView->Invalidate();
        break;
    }
    case PT_ON_LINE_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::ON_ENTITY);
        SS.GW.Invalidate();
        editorView->Invalidate();
        break;
    }
    case CONSTRAIN_SYMMETRIC_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::SYMMETRIC);
        SS.GW.Invalidate();
        editorView->Invalidate();
        break;
    }
    case CONSTRAIN_EQUAL_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::EQUAL);
        SS.GW.Invalidate();
        editorView->Invalidate();
        break;
    }
    case CONSTRAIN_ORIENTED_SAME_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::ORIENTED_SAME);
        SS.GW.Invalidate();
        editorView->Invalidate();
        break;
    }
    case OTHER_ANGLE_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::OTHER_ANGLE);
        SS.GW.Invalidate();
        editorView->Invalidate();
        break;
    }
    case REF_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::REFERENCE);
        SS.GW.Invalidate();
        editorView->Invalidate();
        break;
    }
    case NEAREST_ISO_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::NEAREST_ISO);
        SS.GW.Invalidate();
        editorView->SyncCamera();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case NEAREST_ORTHO_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::NEAREST_ORTHO);
        SS.GW.Invalidate();
        editorView->SyncCamera();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_3D: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_3D);
        SS.GW.Invalidate();
        editorView->SyncCamera();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_WRKPL: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_WRKPL);
        SS.GW.Invalidate();
        editorView->SyncCamera();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_TRANS: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_TRANS);
        SS.GW.Invalidate();
        editorView->SyncCamera();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_ROT: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_ROT);
        SS.GW.Invalidate();
        editorView->SyncCamera();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_EXTRUDE: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_EXTRUDE);
        SS.GW.Invalidate();
        editorView->SyncCamera();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_HELIX: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_HELIX);
        SS.GW.Invalidate();
        editorView->SyncCamera();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_LATHE: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_LATHE);
        SS.GW.Invalidate();
        editorView->SyncCamera();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_REVOLVE: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_REVOLVE);
        SS.GW.Invalidate();
        editorView->SyncCamera();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_LINK: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_LINK);
        SS.GW.Invalidate();
        editorView->SyncCamera();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_RECENT: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_RECENT);
        SS.GW.Invalidate();
        editorView->SyncCamera();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_SHOW_EDITOR: {
        break;
    }
    case M_QUIT_APP: {
        FreeAllTemporary();
        be_app->PostMessage(B_QUIT_REQUESTED);
        break;
    }
    case M_OPEN_FILE: {
        BFilePanel *fp =
            new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL,
                           B_FILE_NODE, false, new BMessage(READ_FILE));
        fp->Show();
        break;
    }
    case M_SAVE_FILE: {
        SS.SaveToFile(
            Platform::Path::From(std::string(currentFilePath->Path())));
        break;
    }
    case READ_FILE: {
        if (msg->HasRef("refs")) {
            entry_ref ref;
            if (msg->FindRef("refs", 0, &ref) == B_OK) {
                BEntry entry(&ref, true);
                entry.GetPath(currentFilePath);
                editorView->Load(std::string(currentFilePath->Path()));

                be_app->WindowAt(PROPERTY_BROWSER)
                    ->PostMessage(new BMessage(SHOW_LIST_OF_GROUPS));
            }
        }
        break;
    }
    default: {
        BWindow::MessageReceived(msg);
        break;
    }
    }
}

bool MainWindow::QuitRequested(void) {
    SolveSpace::Platform::CleanupForQuitting();
    be_app->PostMessage(B_QUIT_REQUESTED);
    return true;
}
