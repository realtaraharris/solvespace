/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <Window.h>
#include "App.h"

#include <iostream>

extern BApplication* be_app;

void ButtonUnclickedEventHook () {
	be_app->WindowAt(TOOLBAR)->PostMessage(new BMessage(LINE_TOOL_BTN_UNCLICKED));
}
