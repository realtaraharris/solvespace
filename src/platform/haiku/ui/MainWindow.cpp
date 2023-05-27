#include "MainWindow.h"

#include <Application.h>
#include <Menu.h>
#include <MenuItem.h>
#include <View.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <StorageKit.h>
#include <Alert.h> // BAlert

#include "Solver.h"
#include "Toolbar.h"
#include "App.h" // contains message enums

#include <iostream>

#define INIT_X 100
#define INIT_Y 100

MainWindow::MainWindow(void)
	:	BWindow(BRect(INIT_X, INIT_Y, INIT_X + MIN_WIDTH, INIT_Y + MIN_HEIGHT + MENUBAR_HEIGHT), "SolveSpace", B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS, B_CURRENT_WORKSPACE) {
	BRect rect(Bounds());

	SetSizeLimits(MIN_WIDTH, 100000, MIN_HEIGHT + MENUBAR_HEIGHT, 100000); // the Haiku API needs a way to not set any upper bound

	menuBar = new BMenuBar(rect, "menubar");	
	BMenu *fileMenu = new BMenu("File");
	fileMenu->AddItem(new BMenuItem("Open", new BMessage(M_OPEN_FILE), 'O'));
	fileMenu->AddItem(new BMenuItem("Quit", new BMessage(M_QUIT_APP), 'Q'));
	menuBar->AddItem(fileMenu);

	BMenu *viewMenu = new BMenu("View");
	viewMenu->AddItem(new BMenuItem("Zoom in", new BMessage(ZOOM_IN), '+'));
	viewMenu->AddItem(new BMenuItem("Zoom out", new BMessage(ZOOM_OUT), '-'));
	viewMenu->AddItem(new BMenuItem("Zoom to fit", new BMessage(ZOOM_TO_FIT), 'f'));
	BSeparatorItem* separator = new BSeparatorItem;
	viewMenu->AddItem(separator);
	viewMenu->AddItem(new BMenuItem("Show snap grid", new BMessage(TOGGLE_SNAP_GRID), '>'));
	menuBar->AddItem(viewMenu);

	rect.Set(0, 0, MIN_WIDTH, MIN_HEIGHT);
    editorView = new EditorView(rect);

    SetLayout(new BGroupLayout(B_VERTICAL));

    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar)
		.Add(editorView)
		.End();

    SS.Init();

	toolWindow = new AppToolbar();
	toolWindow->Show();

	propertyBrowser = new PropertyBrowser();
	propertyBrowser->Show();
}

void MainWindow::MessageReceived(BMessage *msg) {
    switch (msg->what) {
        case ZOOM_IN: {
            editorView->ZoomToMouse(1);
            SS.GW.Invalidate();
            editorView->Invalidate();
            break;
        }
        case ZOOM_OUT: {
            editorView->ZoomToMouse(-1);
            SS.GW.Invalidate();
            editorView->Invalidate();
            break;
        }
        case ZOOM_TO_FIT: {
            editorView->ZoomToFit(false, true); // includingInvisibles = false, useSelection = true
            SS.GW.Invalidate();
            editorView->Invalidate();
            break;
        }
        case TOGGLE_SNAP_GRID: {
	        SS.GW.showSnapGrid = !SS.GW.showSnapGrid;
            SS.GW.EnsureValidActives();
            SS.GW.Invalidate();
            editorView->Invalidate();
            if (SS.GW.showSnapGrid && !SS.GW.LockedInWorkplane()) {
                BAlert* alert = new BAlert("SolveSpace information dialog", "No workplane is active, so the grid will not appear.",
                NULL, NULL, "OK", B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
                alert->SetShortcut(0, B_ESCAPE);
                alert->Go();
            }
	        break;
	    }
		case LINE_TOOL_BTN_CLICKED: {
			std::cout << "LINE_TOOL_BTN" << std::endl;
			SS.GW.MenuRequest(SolveSpace::Command::LINE_SEGMENT);
			break;
		}
		case RECT_TOOL_BTN_CLICKED: {
			std::cout << "RECT_TOOL_BTN" << std::endl;
			SS.GW.MenuRequest(SolveSpace::Command::RECTANGLE);
			break;
		}
		case CIRCLE_TOOL_BTN_CLICKED: {
			std::cout << "CIRCLE_TOOL_BTN" << std::endl;
			SS.GW.MenuRequest(SolveSpace::Command::CIRCLE);
			break;
		}
		case ARC_TOOL_BTN_CLICKED: {
			std::cout << "ARC_TOOL_BTN" << std::endl;
			SS.GW.MenuRequest(SolveSpace::Command::ARC);
			break;
		}
		case TANGENT_ARC_TOOL_BTN_CLICKED: {
			std::cout << "TANGENT_ARC_TOOL_BTN" << std::endl;
			SS.GW.MenuRequest(SolveSpace::Command::TANGENT_ARC);
			break;
		}
		case CUBIC_SPLINE_TOOL_BTN_CLICKED: {
			std::cout << "CUBIC_SPLINE_TOOL_BTN" << std::endl;
			SS.GW.MenuRequest(SolveSpace::Command::CUBIC);
			break;
		}
		case DATUM_POINT_TOOL_BTN_CLICKED: {
			std::cout << "DATUM_POINT_TOOL_BTN" << std::endl;
			SS.GW.MenuRequest(SolveSpace::Command::DATUM_POINT);
			break;
		}
		case CONSTRUCTION_TOOL_BTN_CLICKED: {
			std::cout << "CONSTRUCTION_TOOL_BTN" << std::endl;
			SS.GW.MenuRequest(SolveSpace::Command::CONSTRUCTION);
			break;
		}
		case SPLIT_CURVES_TOOL_BTN_CLICKED: {
			std::cout << "SPLIT_CURVES_TOOL_BTN" << std::endl;
			SS.GW.MenuRequest(SolveSpace::Command::SPLIT_CURVES);
			break;
		}
		case TEXT_TOOL_BTN_CLICKED: {
			SS.GW.MenuRequest(SolveSpace::Command::TTF_TEXT);
			break;
		}
		case DISTANCE_DIA_TOOL_BTN_CLICKED: {
			std::cout << "DISTANCE_DIA_TOOL_BTN" << std::endl;
			SS.GW.ActivateCommand(SolveSpace::Command::DISTANCE_DIA);
			SS.GW.Invalidate();
			editorView->Invalidate();
			break;
		}
		case ANGLE_TOOL_BTN_CLICKED: {
			std::cout << "ANGLE_TOOL_BTN" << std::endl;
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
		case M_SHOW_EDITOR: {
			std::cout << "BOOOOOOM SUCCESS" << std::endl;
			break;
		}
		case M_QUIT_APP: {
			FreeAllTemporary();
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case M_OPEN_FILE: {
			BFilePanel *fp = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL, B_FILE_NODE, false, new BMessage(READ_FILE));
			fp->Show();
			break;
		}
		case READ_FILE: {
			if (msg->HasRef("refs")) {
				entry_ref ref;
				if (msg->FindRef("refs", 0, &ref) == B_OK) {
					BEntry entry(&ref, true);
					BPath path;
					entry.GetPath(&path);
					editorView->Load(std::string(path.Path()));

					be_app->WindowAt(PROPERTY_BROWSER)->PostMessage(new BMessage(SHOW_LIST_OF_GROUPS));
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
