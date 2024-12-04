/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include <Alert.h>
#include <Application.h>
#include <Window.h>

#include "../../solvespace.h"
#include "App.h"
#include "HaikuSpaceUI.h"
#include <FilePanel.h>

void HaikuSpaceUI::OpenSolveSpaceFile () {
  BFilePanel *fp = new BFilePanel (B_OPEN_PANEL, &be_app_messenger, NULL, B_FILE_NODE, false,
                                   new BMessage (READ_FILE));
  fp->Show ();
}

void HaikuSpaceUI::UndoEnableMenus () {
  be_app->WindowAt (MAIN_WINDOW)
      ->PostMessage (new BMessage (undo.cnt > 0 ? SET_UNDO_ENABLED : SET_UNDO_DISABLED));
  be_app->WindowAt (MAIN_WINDOW)
      ->PostMessage (new BMessage (redo.cnt > 0 ? SET_REDO_ENABLED : SET_REDO_DISABLED));
}

bool HaikuSpaceUI::OkayToStartNewFile () {
  if (!unsaved) {
    return true;
  }

  std::string message;

  if (!SolveSpace::SS.saveFile.IsEmpty ()) {
    message = "Do you want to save the changes you made to the sketch " + saveFile.raw +
              "?"; // .c_str());
  } else {
    message = "Do you want to save the changes made to the new sketch?";
  }
  message += " Your changes will be lost if you don't save them.";

  BAlert *alert = new BAlert ("Modified file", message.c_str (), "Don't save", "Cancel", "Save",
                              B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
  alert->SetShortcut (0, B_ESCAPE);
  switch (alert->Go ()) {
  case 0: // don't save
    RemoveAutosave ();
    return true;
  case 1: // cancel
    return false;
  case 2: // save
    GetFilenameAndSave (/* saveAs = */ false);
    return false;
  }

  return false;
}

void HaikuSpaceUI::GetFilenameAndSave (bool saveAs) {
  if (saveAs || saveFile.IsEmpty ()) {
    BFilePanel *fp = new BFilePanel (B_SAVE_PANEL, &be_app_messenger, NULL, B_FILE_NODE, false,
                                     new BMessage (SAVE_AS_FILE) // calls HaikuSpaceUI::SaveNewFile
    );
    if (!saveFile.IsEmpty ()) {
      fp->SetSaveText (saveFile.raw.c_str ());
    }
    fp->Show ();
  }
}

bool HaikuSpaceUI::SaveNewFile (
    Platform::Path newSaveFile) { // called by the SAVE_AS_FILE bMessage handler
  if (!SaveToFile (newSaveFile)) {
    return false;
  }

  AddToRecentList (newSaveFile);
  RemoveAutosave ();
  saveFile = newSaveFile;
  unsaved  = false;

  return true;
}

bool HaikuSpaceUI::LoadAutosaveFor (const Platform::Path &filename) {
  Platform::Path autosaveFile = filename.WithExtension (BACKUP_EXT);

  FILE *f = OpenFile (autosaveFile, "rb");
  if (!f) {
    return false;
  }
  fclose (f);

  BAlert *alert = new BAlert ("Autosave Available",
                              "An autosave file is available for this sketch. Do you want "
                              "to load the autosave file instead?",
                              "Load autosave", "Don't load", NULL, B_WIDTH_AS_USUAL,
                              B_OFFSET_SPACING, B_WARNING_ALERT);
  alert->SetShortcut (0, B_ESCAPE);
  if (alert->Go () == 0) {
    unsaved = true;
    return LoadFromFile (autosaveFile, /*canCancel=*/true);
  }

  return false;
}

int HaikuSpaceUI::LocateImportedFile (const Platform::Path &filename, bool canCancel) {
  BAlert *alert = new BAlert ("Missing file",
                              "The linked file “%s” is not present. Do you want to locate "
                              "it manually? If you decline, any geometry that depends on "
                              "the missing file will be permanently removed.",
                              "Yes", "No", canCancel ? "Cancel" : NULL, B_WIDTH_AS_USUAL,
                              B_OFFSET_SPACING, B_INFO_ALERT);
  alert->SetShortcut (0, B_ESCAPE);

  // FIXME(async): asyncify this call
  return alert->Go (); // 0: YES, 1: NO, 2: CANCEL
}

void HaikuSpaceUI::GetPngExportImageFilename () {
  BFilePanel *fp =
      new BFilePanel (B_SAVE_PANEL, &be_app_messenger, NULL, B_FILE_NODE, false,
                      new BMessage (PNG_EXPORT_IMAGE) // calls HaikuSpaceUI::PngExportImage
      );
  if (!saveFile.IsEmpty ()) {
    fp->SetSaveText (pngExportImageFilename.raw.c_str ());
  }
  fp->Show ();
}

void HaikuSpaceUI::PngExportImage () {
  SS.ExportAsPngTo (pngExportImageFilename);
}
