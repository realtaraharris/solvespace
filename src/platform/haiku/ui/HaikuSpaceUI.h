/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef HAIKUSPACEUI_H
#define HAIKUSPACEUI_H

#include "App.h"
#include "../../../solvespace.h"

class HaikuSpaceUI : public SolveSpaceUI {
public:
    void UndoEnableMenus();
};

#endif // HAIKUSPACEUI_H