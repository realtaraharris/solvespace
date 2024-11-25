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
    fileMenu->AddItem(new BMenuItem("Export image…", new BMessage(M_EXPORT_IMAGE), NULL));
    fileMenu->AddItem(new BMenuItem("Export 2D view…", new BMessage(M_EXPORT_VIEW), NULL));
    fileMenu->AddItem(new BMenuItem("Export 2D section…", new BMessage(M_EXPORT_SECTION), NULL));
    fileMenu->AddItem(new BMenuItem("Export 3D wireframe…", new BMessage(M_EXPORT_WIREFRAME), NULL));
    fileMenu->AddItem(new BMenuItem("Export triangle mesh…", new BMessage(M_EXPORT_MESH), NULL));
    fileMenu->AddItem(new BMenuItem("Export surfaces…", new BMessage(M_EXPORT_SURFACES), NULL));
    fileMenu->AddItem(new BMenuItem("Import…", new BMessage(M_IMPORT), NULL));
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
                                     'W')); // Command::GROUP_WRKPL
    groupMenu->AddSeparatorItem();
    groupMenu->AddItem(new BMenuItem("Step translating",
                                     new BMessage(M_GROUP_TRANS),
                                     'T')); // Command::GROUP_TRANS
    groupMenu->AddItem(new BMenuItem("Step rotating", new BMessage(M_GROUP_ROT),
                                     'R')); // Command::GROUP_ROT
    groupMenu->AddSeparatorItem();
    groupMenu->AddItem(new BMenuItem("Extrude", new BMessage(M_GROUP_EXTRUDE),
                                     'X', B_SHIFT_KEY)); // Command::GROUP_EXTRUDE
    groupMenu->AddItem(new BMenuItem("Helix", new BMessage(M_GROUP_HELIX),
                                     'H')); // Command::GROUP_HELIX
    groupMenu->AddItem(new BMenuItem("Lathe", new BMessage(M_GROUP_LATHE),
                                     'L')); // Command::GROUP_LATHE
    groupMenu->AddItem(new BMenuItem("Revolve", new BMessage(M_GROUP_REVOLVE),
                                     'V')); // Command::GROUP_REVOLVE
    groupMenu->AddSeparatorItem();
    groupMenu->AddItem(new BMenuItem("Link/Assemble…",
                                     new BMessage(M_GROUP_LINK),
                                     NULL)); // Command::GROUP_LINK
    groupMenu->AddItem(new BMenuItem("Link recent",
                                     new BMessage(M_GROUP_RECENT),
                                     NULL)); // Command::GROUP_RECENT
    menuBar->AddItem(groupMenu);

    BMenu *sketchMenu = new BMenu("Sketch");
    sketchMenu->AddItem(new BMenuItem("In workplane", new BMessage(M_SEL_WORKPLANE),
                                     '2')); // Command::SEL_WORKPLANE
    sketchMenu->AddItem(new BMenuItem("Anywhere in 3D",
                                     new BMessage(M_FREE_IN_3D),
                                     '3')); // Command::FREE_IN_3D
    sketchMenu->AddSeparatorItem();
    sketchMenu->AddItem(new BMenuItem("Datum point",
                                     new BMessage(M_DATUM_POINT),
                                     'P')); // Command::DATUM_POINT
    sketchMenu->AddItem(new BMenuItem("Workplane", new BMessage(M_WORKPLANE),
                                     NULL)); // Command::WORKPLANE
    sketchMenu->AddSeparatorItem();
    sketchMenu->AddItem(new BMenuItem("Line segment", new BMessage(M_LINE_SEGMENT),
                                     'S')); // Command::LINE_SEGMENT
    sketchMenu->AddItem(new BMenuItem("Construction line segment", new BMessage(M_CONSTR_SEGMENT),
                                     'S', B_SHIFT_KEY)); // Command::CONSTR_SEGMENT
    sketchMenu->AddItem(new BMenuItem("Rectangle", new BMessage(M_RECTANGLE),
                                     'R')); // Command::RECTANGLE
    sketchMenu->AddItem(new BMenuItem("Circle", new BMessage(M_CIRCLE),
                                     'C')); // Command::CIRCLE
    sketchMenu->AddItem(new BMenuItem("Arc of a circle", new BMessage(M_ARC),
                                     'A')); // Command::ARC
    sketchMenu->AddItem(new BMenuItem("Bezier cubic spline", new BMessage(M_CUBIC),
                                     'B')); // Command::CUBIC
    sketchMenu->AddSeparatorItem();
    sketchMenu->AddItem(new BMenuItem("Text in font",
                                     new BMessage(M_TTF_TEXT),
                                     'T')); // Command::TTF_TEXT
    sketchMenu->AddItem(new BMenuItem("Image…",
                                     new BMessage(M_IMAGE),
                                     NULL)); // Command::IMAGE
    sketchMenu->AddSeparatorItem();
    sketchMenu->AddItem(new BMenuItem("Toggle construction",
                                     new BMessage(M_CONSTRUCTION),
                                     'G')); // Command::CONSTRUCTION
    sketchMenu->AddItem(new BMenuItem("Tangent arc at point",
                                     new BMessage(M_TANGENT_ARC),
                                     'A', B_SHIFT_KEY)); // Command::TANGENT_ARC
    sketchMenu->AddItem(new BMenuItem("Split curves at intersection",
                                     new BMessage(M_SPLIT_CURVES),
                                     'I')); // Command::SPLIT_CURVES

    menuBar->AddItem(sketchMenu);

    BMenu *constrainMenu = new BMenu("Constrain");
    constrainMenu->AddItem(new BMenuItem("Distance / diameter", new BMessage(M_DISTANCE_DIA),
                                     'D')); // Command::DISTANCE_DIA
    constrainMenu->AddItem(new BMenuItem("Reference dimension",
                                     new BMessage(M_REF_DISTANCE),
                                     'D', B_SHIFT_KEY)); // Command::REF_DISTANCE
    constrainMenu->AddItem(new BMenuItem("Angle",
                                     new BMessage(M_ANGLE),
                                     'N', B_CONTROL_KEY)); // Command::ANGLE
    constrainMenu->AddItem(new BMenuItem("Reference angle",
                                     new BMessage(M_REF_ANGLE),
                                     'N', B_CONTROL_KEY | B_SHIFT_KEY)); // Command::REF_ANGLE
    constrainMenu->AddItem(new BMenuItem("Other supplementary angle",
                                     new BMessage(M_OTHER_ANGLE),
                                     'U')); // Command::OTHER_ANGLE
    constrainMenu->AddItem(new BMenuItem("Toggle reference dim",
                                     new BMessage(M_REFERENCE),
                                     'E')); // Command::REFERENCE
    constrainMenu->AddSeparatorItem();
    constrainMenu->AddItem(new BMenuItem("Horizontal",
                                     new BMessage(M_HORIZONTAL),
                                     'H')); // Command::HORIZONTAL
    constrainMenu->AddItem(new BMenuItem("Vertical",
                                     new BMessage(M_VERTICAL),
                                     'V')); // Command::VERTICAL
    constrainMenu->AddSeparatorItem();
    constrainMenu->AddItem(new BMenuItem("On point / curve /plane",
                                     new BMessage(M_ON_ENTITY),
                                     'O')); // Command::ON_ENTITY
    constrainMenu->AddItem(new BMenuItem("Equal length / radius / angle",
                                     new BMessage(M_EQUAL),
                                     'Q', B_SHIFT_KEY)); // Command::EQUAL
    constrainMenu->AddItem(new BMenuItem("Length / arc ratio",
                                     new BMessage(M_RATIO),
                                     'Z', B_SHIFT_KEY)); // Command::RATIO
    constrainMenu->AddItem(new BMenuItem("Length / arc difference",
                                     new BMessage(M_DIFFERENCE),
                                     'J')); // Command::DIFFERENCE
    constrainMenu->AddItem(new BMenuItem("At midpoint",
                                     new BMessage(M_AT_MIDPOINT),
                                     'M')); // Command::AT_MIDPOINT
    constrainMenu->AddItem(new BMenuItem("Symmetric",
                                     new BMessage(M_SYMMETRIC),
                                     'Y')); // Command::SYMMETRIC
    constrainMenu->AddItem(new BMenuItem("Parallel / tangent",
                                     new BMessage(M_PARALLEL),
                                     'L')); // Command::PARALLEL
    constrainMenu->AddItem(new BMenuItem("Perpendicular",
                                     new BMessage(M_PERPENDICULAR),
                                     '[')); // Command::PERPENDICULAR
    constrainMenu->AddItem(new BMenuItem("Same orientation",
                                     new BMessage(M_ORIENTED_SAME),
                                     'X')); // Command::ORIENTED_SAME
    constrainMenu->AddItem(new BMenuItem("Lock point where dragged",
                                     new BMessage(M_WHERE_DRAGGED),
                                     ']')); // Command::WHERE_DRAGGED
    constrainMenu->AddSeparatorItem();
    constrainMenu->AddItem(new BMenuItem("Comment",
                                     new BMessage(M_COMMENT),
                                     ';')); // Command::COMMENT

    menuBar->AddItem(constrainMenu);

    BMenu *analyzeMenu = new BMenu("Analyze");
    analyzeMenu->AddItem(new BMenuItem("Measure volume", new BMessage(M_VOLUME),
                                     'V', B_CONTROL_KEY | B_SHIFT_KEY)); // Command::VOLUME
    analyzeMenu->AddItem(new BMenuItem("Measure area", new BMessage(M_AREA),
                                     'A', B_CONTROL_KEY | B_SHIFT_KEY)); // Command::AREA
    analyzeMenu->AddItem(new BMenuItem("Measure perimeter", new BMessage(M_PERIMETER),
                                     'P', B_CONTROL_KEY | B_SHIFT_KEY)); // Command::PERIMETER
    analyzeMenu->AddItem(new BMenuItem("Show interfering parts", new BMessage(M_INTERFERENCE),
                                     'I', B_CONTROL_KEY | B_SHIFT_KEY)); // Command::INTERFERENCE
    analyzeMenu->AddItem(new BMenuItem("Show naked edges", new BMessage(M_NAKED_EDGES),
                                     'N', B_CONTROL_KEY | B_SHIFT_KEY)); // Command::NAKED_EDGES
    analyzeMenu->AddItem(new BMenuItem("Show center of mass", new BMessage(M_CENTER_OF_MASS),
                                     'C', B_CONTROL_KEY | B_SHIFT_KEY)); // Command::CENTER_OF_MASS
    analyzeMenu->AddSeparatorItem();
    analyzeMenu->AddItem(new BMenuItem("Show underconstrained points", new BMessage(M_SHOW_DOF),
                                     'F', B_CONTROL_KEY | B_SHIFT_KEY)); // Command::SHOW_DOF
    analyzeMenu->AddSeparatorItem();
    analyzeMenu->AddItem(new BMenuItem("Trace point", new BMessage(M_TRACE_PT),
                                     'T', B_CONTROL_KEY | B_SHIFT_KEY)); // Command::TRACE_PT
    analyzeMenu->AddItem(new BMenuItem("Stop tracing…", new BMessage(M_STOP_TRACING),
                                     'D', B_CONTROL_KEY | B_SHIFT_KEY)); // Command::STOP_TRACING
    analyzeMenu->AddItem(new BMenuItem("Step dimension…", new BMessage(M_STEP_DIM),
                                     'S', B_CONTROL_KEY | B_SHIFT_KEY)); // Command::STEP_DIM
    menuBar->AddItem(analyzeMenu);

    BMenu *helpMenu = new BMenu("Help");
    helpMenu->AddItem(new BMenuItem("About",
		                                 new BMessage(M_ABOUT),
                                     NULL)); // Command::ABOUT
    menuBar->AddItem(helpMenu);

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

    editorView->New();
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
            SS.MenuClipboard(SolveSpace::Command::DELETE);
            break;
        }
        case 's': {
						be_app->WindowAt(TOOLBAR)
								->PostMessage(new BMessage(LINE_TOOL_BTN_CLICKED));
            break;
        }
        case 'r': {
            be_app->WindowAt(TOOLBAR)
                ->PostMessage(new BMessage(RECT_TOOL_BTN_CLICKED));
            break;
        }
        case 'c': {
            be_app->WindowAt(TOOLBAR)
                ->PostMessage(new BMessage(CIRCLE_TOOL_BTN_CLICKED));
            break;
        }
        case 'a': {
            if (mods & B_SHIFT_KEY) {
                be_app->WindowAt(TOOLBAR)
                    ->PostMessage(new BMessage(TANGENT_ARC_TOOL_BTN_CLICKED));
            } else {
                be_app->WindowAt(TOOLBAR)
                    ->PostMessage(new BMessage(ARC_TOOL_BTN_CLICKED));
            }
            break;
        }
        case 'b': {
            be_app->WindowAt(TOOLBAR)
                ->PostMessage(new BMessage(CUBIC_SPLINE_TOOL_BTN_CLICKED));
            break;
        }
        case 'p': {
            be_app->WindowAt(TOOLBAR)
                ->PostMessage(new BMessage(DATUM_POINT_TOOL_BTN_CLICKED));
            break;
        }
        case 'g': {
            be_app->WindowAt(TOOLBAR)
                ->PostMessage(new BMessage(CONSTRUCTION_TOOL_BTN_CLICKED));
            break;
        }
        case 'i': {
            be_app->WindowAt(TOOLBAR)
                ->PostMessage(new BMessage(SPLIT_CURVES_TOOL_BTN_CLICKED));
            break;
        }
        case 't': {
            be_app->WindowAt(TOOLBAR)
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
        SS.MenuView(SolveSpace::Command::ZOOM_IN);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case ZOOM_OUT: {
        SS.MenuView(SolveSpace::Command::ZOOM_OUT);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case ZOOM_TO_FIT: {
        SS.MenuView(SolveSpace::Command::ZOOM_TO_FIT);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case CENTER_VIEW_AT_POINT: {
        SS.MenuView(SolveSpace::Command::CENTER_VIEW);
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

    case RECT_TOOL_BTN_CLICKED: {
        SS.MenuRequest(SolveSpace::Command::RECTANGLE);
        break;
    }
    case CIRCLE_TOOL_BTN_CLICKED: {
        SS.MenuRequest(SolveSpace::Command::CIRCLE);
        break;
    }
    case ARC_TOOL_BTN_CLICKED: {
        SS.MenuRequest(SolveSpace::Command::ARC);
        break;
    }
    case TANGENT_ARC_TOOL_BTN_CLICKED: {
        SS.MenuRequest(SolveSpace::Command::TANGENT_ARC);
        break;
    }
    case CUBIC_SPLINE_TOOL_BTN_CLICKED: {
        SS.MenuRequest(SolveSpace::Command::CUBIC);
        break;
    }
    case DATUM_POINT_TOOL_BTN_CLICKED: {
        SS.MenuRequest(SolveSpace::Command::DATUM_POINT);
        break;
    }
    case CONSTRUCTION_TOOL_BTN_CLICKED: {
        SS.MenuRequest(SolveSpace::Command::CONSTRUCTION);
        break;
    }
    case SPLIT_CURVES_TOOL_BTN_CLICKED: {
        SS.MenuRequest(SolveSpace::Command::SPLIT_CURVES);
        break;
    }
    case TEXT_TOOL_BTN_CLICKED: {
        SS.MenuRequest(SolveSpace::Command::TTF_TEXT);
        break;
    }
    case DISTANCE_DIA_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::DISTANCE_DIA);
        break;
    }
    case ANGLE_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::ANGLE);
        break;
    }
    case HORIZONTAL_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::HORIZONTAL);
        break;
    }
    case VERTICAL_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::VERTICAL);
        break;
    }
    case PARALLEL_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::PARALLEL);
        break;
    }
    case CONSTRAIN_PERP_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::PERPENDICULAR);
        break;
    }
    case PT_ON_LINE_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::ON_ENTITY);
        break;
    }
    case CONSTRAIN_SYMMETRIC_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::SYMMETRIC);
        break;
    }
    case CONSTRAIN_EQUAL_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::EQUAL);
        break;
    }
    case CONSTRAIN_ORIENTED_SAME_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::ORIENTED_SAME);
        break;
    }
    case OTHER_ANGLE_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::OTHER_ANGLE);
        break;
    }
    case REF_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::REFERENCE);
        break;
    }
    case NEAREST_ISO_TOOL_BTN_CLICKED: {
        SS.MenuConstrain(SolveSpace::Command::NEAREST_ISO);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case NEAREST_ORTHO_TOOL_BTN_CLICKED: {
        SS.MenuView(SolveSpace::Command::NEAREST_ORTHO);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_3D: {
        SS.MenuGroup(SolveSpace::Command::GROUP_3D);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_WRKPL: {
        SS.MenuGroup(SolveSpace::Command::GROUP_WRKPL);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_TRANS: {
        SS.MenuGroup(SolveSpace::Command::GROUP_TRANS);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_ROT: {
        SS.MenuGroup(SolveSpace::Command::GROUP_ROT);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_EXTRUDE: {
        SS.MenuGroup(SolveSpace::Command::GROUP_EXTRUDE);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_HELIX: {
        SS.MenuGroup(SolveSpace::Command::GROUP_HELIX);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_LATHE: {
        SS.MenuGroup(SolveSpace::Command::GROUP_LATHE);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_REVOLVE: {
        SS.MenuGroup(SolveSpace::Command::GROUP_REVOLVE);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_LINK: {
        SS.MenuGroup(SolveSpace::Command::GROUP_LINK);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_GROUP_RECENT: {
        SS.MenuGroup(SolveSpace::Command::GROUP_RECENT);
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
        SS.MenuFile(SolveSpace::Command::NEW);
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
    case M_EXPORT_IMAGE: {
        BFilePanel *fp =
            new BFilePanel(B_SAVE_PANEL, new BMessenger(this), NULL,
                           B_FILE_NODE, false, new BMessage(EXPORT_IMAGE));
        fp->SetSaveText(currentFilePath->Leaf()); // TODO: strip extension and replace with png
        fp->Show();
        break;
    }
    case EXPORT_IMAGE: {
        // TODO: extract identical code used elsewhere, e.g., the SAVE_AS_FILE case handler
        if (!msg->HasRef("directory") || !msg->HasString("name")) { break; }
        entry_ref ref;
        const char *name;
        BPath path;

        if (msg->FindRef("directory", 0, &ref) != B_OK || msg->FindString("name", &name) != B_OK) { break; }
        BEntry entry(&ref, true);

        if (entry.GetPath(&path) != B_OK) { break; }

        SS.ExportAsPngTo(
            Platform::Path::From(std::string(path.Path()) + "/" + std::string(name))
        );

        break;
    }
    case M_EXPORT_VIEW: {
        SS.MenuFile(SolveSpace::Command::EXPORT_VIEW);
        break;
    }
    case M_EXPORT_SECTION: {
        SS.MenuFile(SolveSpace::Command::EXPORT_SECTION);
        break;
    }
    case M_EXPORT_WIREFRAME: {
        SS.MenuFile(SolveSpace::Command::EXPORT_WIREFRAME);
        break;
    }
    case M_EXPORT_MESH: {
        SS.MenuFile(SolveSpace::Command::EXPORT_MESH);
        break;
    }
    case M_EXPORT_SURFACES: {
        SS.MenuFile(SolveSpace::Command::EXPORT_SURFACES);
        break;
    }
    case M_IMPORT: {
				SS.MenuFile(SolveSpace::Command::IMPORT);
        break;
    }

    case M_UNDO: {
		    SS.MenuEdit(SolveSpace::Command::UNDO);
        be_app->WindowAt(VIEW_PARAMETERS)
            ->PostMessage(new BMessage(UPDATE_VIEW_PARAMETERS));
        break;
    }
    case M_REDO: {
        SS.MenuEdit(SolveSpace::Command::REDO);
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
