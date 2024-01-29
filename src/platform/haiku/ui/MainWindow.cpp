/*
 * Copyright 2023, 2024 Tara Harris <3769985+realtaraharris@users.noreply.github.com>
 * All rights reserved. Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include "MainWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuItem.h>
#include <StorageKit.h>
#include <View.h>

#include "App.h" // contains message enums
#include "Toolbar.h"

#include <iostream>

#define INIT_X 125
#define INIT_Y 45

MainWindow::MainWindow(void)
    : BWindow(BRect(INIT_X, INIT_Y, INIT_X + MIN_WIDTH,
                    INIT_Y + MIN_HEIGHT + MENUBAR_HEIGHT),
              "SolveSpace", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS,
              B_CURRENT_WORKSPACE),
              settings(new BMessage('sett')) {
    BRect rect(Bounds());

    SetSizeLimits(
        MIN_WIDTH, 100000, MIN_HEIGHT + MENUBAR_HEIGHT,
        100000); // the Haiku API needs a way to not set any upper bound

    menuBar = new BMenuBar(rect, "menubar");
    BMenu *fileMenu = new BMenu("File");
    fileMenu->AddItem(new BMenuItem("New", new BMessage(M_NEW_FILE), 'N'));
    fileMenu->AddItem(new BMenuItem("Open…", new BMessage(M_OPEN_FILE), 'O'));
    fileMenu->AddItem(new BMenuItem("Save", new BMessage(M_SAVE_FILE), 'S'));
    fileMenu->AddItem(new BMenuItem("Save as…", new BMessage(M_SAVE_AS_FILE), NULL));
    fileMenu->AddSeparatorItem();
    fileMenu->AddItem(new BMenuItem("Quit", new BMessage(M_QUIT_APP), 'Q'));
    menuBar->AddItem(fileMenu);

    BMenu *editMenu = new BMenu("Edit");
    undoMenuItem = new BMenuItem("Undo", new BMessage(M_UNDO), 'Z');
    editMenu->AddItem(undoMenuItem);
    redoMenuItem = new BMenuItem("Redo", new BMessage(M_REDO), 'Z', B_SHIFT_KEY);
    editMenu->AddItem(redoMenuItem);
    editMenu->AddItem(new BMenuItem("Regenerate all", new BMessage(M_NOT_IMPL), NULL));
    editMenu->AddSeparatorItem();
    editMenu->AddItem(new BMenuItem("Snap selection to grid", new BMessage(M_NOT_IMPL), '.'));
    editMenu->AddItem(new BMenuItem("Rotate imported 90°", new BMessage(M_NOT_IMPL), '9'));
    editMenu->AddSeparatorItem();
    editMenu->AddItem(new BMenuItem("Cut", new BMessage(M_NOT_IMPL), 'X'));
    editMenu->AddItem(new BMenuItem("Copy", new BMessage(M_NOT_IMPL), 'C'));
    editMenu->AddItem(new BMenuItem("Paste", new BMessage(M_NOT_IMPL), 'V'));
    editMenu->AddItem(new BMenuItem("Paste transformed", new BMessage(M_NOT_IMPL), 'T'));
    editMenu->AddItem(new BMenuItem("Delete", new BMessage(M_NOT_IMPL), NULL));
    editMenu->AddSeparatorItem();
    editMenu->AddItem(new BMenuItem("Select edge chain", new BMessage(M_NOT_IMPL), 'E'));
    editMenu->AddItem(new BMenuItem("Select all", new BMessage(M_NOT_IMPL), 'A'));
    editMenu->AddItem(new BMenuItem("Unselect all", new BMessage(M_NOT_IMPL), NULL));
    editMenu->AddSeparatorItem();
//    editMenu->AddItem(new BMenuItem("Line styles", new BMessage(M_NOT_IMPL), NULL));
//    editMenu->AddItem(new BMenuItem("View projection", new BMessage(M_NOT_IMPL), NULL));
    editMenu->AddItem(new BMenuItem("Configuration…", new BMessage(M_NOT_IMPL), NULL));
    menuBar->AddItem(editMenu);

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
    groupMenu->AddItem(new BMenuItem("Link/Assemble…",
                                     new BMessage(M_GROUP_LINK),
                                     NULL)); // Command::GROUP_LINK
    groupMenu->AddItem(new BMenuItem("Link recent",
                                     new BMessage(M_GROUP_RECENT),
                                     NULL)); // Command::GROUP_RECENT
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

    propertyBrowser = new PropertyBrowser();


    viewParameters = new ViewParameters();
    be_app->WindowAt(VIEW_PARAMETERS)
        ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));

    currentFilePath = new BPath();

    LoadSettings();
    toolWindow->Show();
    propertyBrowser->Show();
    viewParameters->Show();
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
        SS.GW.MenuView(SolveSpace::Command::ZOOM_IN);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case ZOOM_OUT: {
        SS.GW.MenuView(SolveSpace::Command::ZOOM_OUT);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case ZOOM_TO_FIT: {
        SS.GW.MenuView(SolveSpace::Command::ZOOM_TO_FIT);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case CENTER_VIEW_AT_POINT: {
        SS.GW.MenuView(SolveSpace::Command::CENTER_VIEW);
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
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case NEAREST_ORTHO_TOOL_BTN_CLICKED: {
        SS.GW.ActivateCommand(SolveSpace::Command::NEAREST_ORTHO);
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_3D: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_3D);
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_WRKPL: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_WRKPL);
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_TRANS: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_TRANS);
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_ROT: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_ROT);
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_EXTRUDE: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_EXTRUDE);
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_HELIX: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_HELIX);
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_LATHE: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_LATHE);
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_REVOLVE: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_REVOLVE);
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_LINK: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_LINK);
        SS.GW.Invalidate();
        editorView->Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_RECENT: {
        SS.GW.ActivateCommand(SolveSpace::Command::GROUP_RECENT);
        SS.GW.Invalidate();
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
    case M_NEW_FILE: {
        SS.GW.ActivateCommand(SolveSpace::Command::NEW);
		editorView->New();
        SS.GW.Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_SAVE_AS_FILE: {
        BFilePanel *fp =
            new BFilePanel(B_SAVE_PANEL, new BMessenger(this), NULL,
                           B_FILE_NODE, false, new BMessage(SAVE_AS_FILE));
        fp->SetSaveText(currentFilePath->Leaf());
        fp->Show();
        break;
    }
    case SAVE_AS_FILE: {
        // TODO: report errors to user
        if (!msg->HasRef("directory") || !msg->HasString("name")) { break; }
        entry_ref ref;
        const char *name;

        if (msg->FindRef("directory", 0, &ref) != B_OK || msg->FindString("name", &name) != B_OK) { break; }
        BEntry entry(&ref, true);

        if (entry.GetPath(currentFilePath) != B_OK) { break; }
        SS.SaveToFile(
            Platform::Path::From(std::string(currentFilePath->Path()) + "/" + std::string(name)));

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
    case M_UNDO: {
        SS.GW.ActivateCommand(SolveSpace::Command::UNDO);
        SS.GW.Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_REDO: {
        SS.GW.ActivateCommand(SolveSpace::Command::REDO);
        SS.GW.Invalidate();
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case SET_UNDO_ENABLED: {
        undoMenuItem->SetEnabled(true);
        break;
    }
    case SET_UNDO_DISABLED: {
        undoMenuItem->SetEnabled(false);
        break;
    }
    case SET_REDO_ENABLED: {
        redoMenuItem->SetEnabled(true);
        break;
    }
    case SET_REDO_DISABLED: {
        redoMenuItem->SetEnabled(false);
        break;
    }
    default: {
        BWindow::MessageReceived(msg);
        break;
    }
    }
}

status_t load_settings(BMessage *message, const char *filename) {
    status_t status = B_BAD_VALUE;
    if (!message) { return status; }

    BPath path;
    status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
    if (status != B_OK) { return status; }

    status = path.Append(filename);
    if (status != B_OK) { return status; }

    BFile file(path.Path(), B_READ_ONLY);
    status = file.InitCheck();
    if (status != B_OK) { return status; }

    status = message->Unflatten(&file);
    file.Unset();

    return status;
}

status_t save_settings(BMessage *message, const char *filename) {
    status_t status = B_BAD_VALUE;
    if (!message) { return status; }

    BPath path;
    status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
    if (status != B_OK) { return status; }

    status = path.Append(filename);
    if (status != B_OK) { return status; }

    BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
    status = file.InitCheck();
    if (status != B_OK) { return status; }

    status = message->Flatten(&file);
    file.Unset();

    return status;
}

void MainWindow::LoadSettings() {
    if (load_settings(settings, "SolveSpace_settings") != B_OK) { return; }

    BRect frame;
    if (settings->FindRect("SolveSpace Window", &frame) == B_OK) {
        MoveTo(frame.LeftTop());
        ResizeTo(frame.Width(), frame.Height());
    }

    if (settings->FindRect("Toolbar Window", &frame) == B_OK) {
        be_app->WindowAt(TOOLBAR)->MoveTo(frame.LeftTop());
        be_app->WindowAt(TOOLBAR)->ResizeTo(frame.Width(), frame.Height());
    }
    if (settings->FindRect("Property Browser Window", &frame) == B_OK) {
        be_app->WindowAt(PROPERTY_BROWSER)->MoveTo(frame.LeftTop());
        be_app->WindowAt(PROPERTY_BROWSER)->ResizeTo(frame.Width(), frame.Height());
    }
    if (settings->FindRect("View Parameters Window", &frame) == B_OK) {
        be_app->WindowAt(VIEW_PARAMETERS)->MoveTo(frame.LeftTop());
        be_app->WindowAt(VIEW_PARAMETERS)->ResizeTo(frame.Width(), frame.Height());
    }
}

void MainWindow::SaveSettings() {
    if (settings->ReplaceRect("SolveSpace Window", Frame()) != B_OK) {
        settings->AddRect("SolveSpace Window", Frame());
    }

    if (settings->ReplaceRect("Toolbar Window", be_app->WindowAt(TOOLBAR)->Frame()) != B_OK) {
        settings->AddRect("Toolbar Window", be_app->WindowAt(TOOLBAR)->Frame());
    }
    if (settings->ReplaceRect("Property Browser Window", be_app->WindowAt(PROPERTY_BROWSER)->Frame()) != B_OK) {
        settings->AddRect("Property Browser Window", be_app->WindowAt(PROPERTY_BROWSER)->Frame());
    }
    if (settings->ReplaceRect("View Parameters Window", be_app->WindowAt(VIEW_PARAMETERS)->Frame()) != B_OK) {
        settings->AddRect("View Parameters Window", be_app->WindowAt(VIEW_PARAMETERS)->Frame());
    }

    if (save_settings(settings, "SolveSpace_settings") != B_OK) {
        std::cerr << "error saving SolveSpace settings" << std::endl;
    }
}

bool MainWindow::QuitRequested(void) {
    SaveSettings();
    be_app->PostMessage(B_QUIT_REQUESTED);
    return true;
}
