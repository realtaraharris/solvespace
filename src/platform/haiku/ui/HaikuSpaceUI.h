/*
 * Copyright 2023, 2024 Tara Harris <3769985+realtaraharris@users.noreply.github.com>
 * All rights reserved. Distributed under the terms of the GPLv3 and MIT licenses.
 */

#ifndef HAIKUSPACEUI_H
#define HAIKUSPACEUI_H

class HaikuSpaceUI : public SolveSpaceUI {
public:
    void UndoEnableMenus();
	bool OkayToStartNewFile();
	bool GetFilenameAndSave(bool);
};

#endif // HAIKUSPACEUI_H