/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <Application.h>
#include <Window.h>

#include "App.h"
#include "HaikuSpaceUI.h"

void HaikuSpaceUI::UndoEnableMenus() {
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(undo.cnt > 0 ? SET_UNDO_ENABLED : SET_UNDO_DISABLED));
    be_app->WindowAt(MAIN_WINDOW)->PostMessage(new BMessage(redo.cnt > 0 ? SET_REDO_ENABLED : SET_REDO_DISABLED));
}
