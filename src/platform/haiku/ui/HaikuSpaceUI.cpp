/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <Application.h>
#include <Window.h>
#include <Alert.h>

#include "App.h"
#include "../../../solvespace.h"
#include "HaikuSpaceUI.h"

void HaikuSpaceUI::UndoEnableMenus() {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(undo.cnt > 0 ? SET_UNDO_ENABLED : SET_UNDO_DISABLED));
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(redo.cnt > 0 ? SET_REDO_ENABLED : SET_REDO_DISABLED));
}

bool HaikuSpaceUI::OkayToStartNewFile() {
    if(!unsaved) return true;

	std::string message;

    if(!SolveSpace::SS.saveFile.IsEmpty()) {
		message = "Do you want to save the changes you made to the sketch " + saveFile.raw + "?"; // .c_str());
    } else {
		message = "Do you want to save the changes made to the new sketch?";
    }
	message += " Your changes will be lost if you don't save them.";

	BAlert *alert = new BAlert(
		"Modified file",
		message.c_str(), "Don't save", "Cancel", "Save", B_WIDTH_AS_USUAL, B_OFFSET_SPACING,
		B_WARNING_ALERT);
	alert->SetShortcut(0, B_ESCAPE);
    switch(alert->Go()) {
        case 0: // don't save
            RemoveAutosave();
            return true;
		case 1: // cancel
			return false;
        case 2: // save
            return GetFilenameAndSave(/* saveAs = */false);
    }

	return false;
}

bool HaikuSpaceUI::GetFilenameAndSave(bool saveAs) {}