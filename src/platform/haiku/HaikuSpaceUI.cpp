/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include <Alert.h>
#include <Application.h>
#include <NodeInfo.h>
#include <Window.h>
#include <SupportDefs.h>

#include "../../solvespace.h"
#include "filewriter/stepfilewriter.h"
#include "filewriter/vectorfilewriter.h"
#include "App.h"
#include <FilePanel.h>
#include "HaikuSpaceUI.h"

void HaikuSpaceUI::SavePanel(uint32 messageName) {
  BFilePanel *fp =
      new BFilePanel(B_SAVE_PANEL, NULL, NULL, B_FILE_NODE, false, new BMessage(messageName));
  fp->Show();
}

void HaikuSpaceUI::OpenPanel(uint32 messageName) {
  BFilePanel *fp =
      new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, false, new BMessage(messageName));
  fp->Show();
}

void HaikuSpaceUI::OpenSolveSpaceFile() {
  OpenPanel(READ_FILE);
}

void HaikuSpaceUI::UndoEnableMenus() {
  be_app->WindowAt(MAIN_WINDOW)
      ->PostMessage(new BMessage(undo.cnt > 0 ? SET_UNDO_ENABLED : SET_UNDO_DISABLED));
  be_app->WindowAt(MAIN_WINDOW)
      ->PostMessage(new BMessage(redo.cnt > 0 ? SET_REDO_ENABLED : SET_REDO_DISABLED));
}

bool HaikuSpaceUI::OkayToStartNewFile() {
  if (!unsaved) {
    return true;
  }

  std::string message;

  if (!SolveSpace::SS.saveFile.IsEmpty()) {
    message = "Do you want to save the changes you made to the sketch " + saveFile.raw +
              "?"; // .c_str());
  } else {
    message = "Do you want to save the changes made to the new sketch?";
  }
  message += " Your changes will be lost if you don't save them.";

  BAlert *alert = new BAlert("Modified file", message.c_str(), "Don't save", "Cancel", "Save",
                             B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
  alert->SetShortcut(0, B_ESCAPE);
  switch (alert->Go()) {
  case 0: // don't save
    RemoveAutosave();
    return true;
  case 1: // cancel
    return false;
  case 2: // save
    GetFilenameAndSave(/* saveAs = */ false);
    return false;
  }

  return false;
}

void HaikuSpaceUI::GetFilenameAndSave(bool saveAs) {
  if (saveAs || saveFile.IsEmpty()) {
    BFilePanel *fp = new BFilePanel(B_SAVE_PANEL, &be_app_messenger, NULL, B_FILE_NODE, false,
                                    new BMessage(SAVE_AS_FILE) // calls HaikuSpaceUI::SaveNewFile
    );
    if (!saveFile.IsEmpty()) {
      fp->SetSaveText(saveFile.raw.c_str());
    }
    fp->Show();
  } else { // we know the file name! just save the file now
    if (SaveToFile(saveFile)) {
      AddToRecentList(saveFile);
      RemoveAutosave();
      unsaved = false;
    }
  }
}

bool HaikuSpaceUI::SaveNewFile(
    Platform::Path newSaveFile) { // called by the SAVE_AS_FILE bMessage handler
  if (!SaveToFile(newSaveFile)) {
    return false;
  }

  AddToRecentList(newSaveFile);
  RemoveAutosave();
  saveFile = newSaveFile;
  unsaved  = false;

  return true;
}

bool HaikuSpaceUI::LoadAutosaveFor(const Platform::Path &filename) {
  Platform::Path autosaveFile = filename.WithExtension(BACKUP_EXT);

  FILE *f = OpenFile(autosaveFile, "rb");
  if (!f) {
    return false;
  }
  fclose(f);

  BAlert *alert = new BAlert("Autosave Available",
                             "An autosave file is available for this sketch. Do you want "
                             "to load the autosave file instead?",
                             "Load autosave", "Don't load", NULL, B_WIDTH_AS_USUAL,
                             B_OFFSET_SPACING, B_WARNING_ALERT);
  alert->SetShortcut(0, B_ESCAPE);
  if (alert->Go() == 0) {
    unsaved = true;
    std::cout << "unsaved = true (HaikuSpaceUI::LoadAutosaveFor)" << std::endl;
    return LoadFromFile(autosaveFile, /*canCancel=*/true);
  }

  return false;
}

int HaikuSpaceUI::LocateImportedFile(const Platform::Path &filename, bool canCancel) {
  BAlert *alert = new BAlert(
      "Missing file",
      std::format(
          "The linked file {0} is not present. Do you want to locate it manually? If you decline, "
          "any geometry that depends on the missing file will be permanently removed.",
          filename.raw)
          .c_str(),
      "Yes", "No", canCancel ? "Cancel" : NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_INFO_ALERT);
  alert->SetShortcut(0, B_ESCAPE);

  // FIXME(async): asyncify this call
  return alert->Go(); // 0: YES, 1: NO, 2: CANCEL
}

void HaikuSpaceUI::GetPngExportImageFilename() {
  BFilePanel *fp =
      new BFilePanel(B_SAVE_PANEL, &be_app_messenger, NULL, B_FILE_NODE, false,
                     new BMessage(PNG_EXPORT_IMAGE) // calls HaikuSpaceUI::PngExportImage
      );
  if (!saveFile.IsEmpty()) {
    fp->SetSaveText(pngExportImageFilename.raw.c_str());
  }
  fp->Show();
}

void HaikuSpaceUI::PngExportImage(Platform::Path fp) {
  SS.ExportAsPngTo(fp);
}

void HaikuSpaceUI::PromptForLinkedFile() {
  OpenPanel(LINKED_FILE_IMAGE);
}

void HaikuSpaceUI::LinkedFileImage(const Platform::Path &filename) {
  std::shared_ptr<Pixmap> pixmap = Pixmap::ReadPng(filename);
  if (pixmap == NULL) {
    Error("The image '%s' is corrupted.", filename.raw.c_str());
  } else {
    // we know where the file is now, good
    SS.images[filename] = pixmap;
  }
}

void HaikuSpaceUI::PromptForImportFile() {
  OpenPanel(IMPORT_FILE);
}

namespace SolveSpace {
  void ImportDxf(const Platform::Path &file);
  void ImportDwg(const Platform::Path &file);
} // namespace SolveSpace

void HaikuSpaceUI::ImportFile(const Platform::Path &importFile) {
  if (importFile.HasExtension("dxf")) {
    ImportDxf(importFile);
  } else if (importFile.HasExtension("dwg")) {
    ImportDwg(importFile);
  } else {
    Error(_("Can't identify file type from file extension of filename '%s'; try .dxf or .dwg."),
          importFile.raw.c_str());
    return;
  }

  SS.GenerateAll(SolveSpaceUI::Generate::UNTIL_ACTIVE);
}

void HaikuSpaceUI::PromptForGroupLink(Group &g) {
  g.type        = Group::Type::LINKED;
  g.meshCombine = Group::CombineAs::ASSEMBLE;
  if (g.linkFile.IsEmpty()) {
    OpenPanel(GROUP_LINK_FILE);
  }
}

void HaikuSpaceUI::GroupLink(const Platform::Path &groupLinkFile, Group &g) {
  g.linkFile = groupLinkFile;

  // assign the default name of the group based on the name of the linked file
  g.name = g.linkFile.FileStem();
  for (size_t i = 0; i < g.name.length(); i++) {
    if (!(isalnum(g.name[i]) || (unsigned)g.name[i] >= 0x80)) { // convert punctuation to dashes
      g.name[i] = '-';
    }
  }
}

void HaikuSpaceUI::PromptForExportViewFile() {
  SavePanel(EXPORT_VIEW);
}

void HaikuSpaceUI::ExportView(const Platform::Path &exportFile) {
  // warn the user if they're exporting constraints inappropriately
  if (SS.GW.showConstraints &&
      (exportFile.HasExtension("txt") || fabs(SS.exportOffset) > LENGTH_EPS)) {
    Message(_("Constraints are currently shown, and will be exported "
              "in the toolpath. This is probably not what you want; "
              "hide them by clicking the link at the top of the "
              "text window."));
  }

  SS.ExportViewOrWireframeTo(exportFile, /*exportWireframe=*/false);
}

void HaikuSpaceUI::PromptForExportWireframeFile() {
  SavePanel(EXPORT_WIREFRAME);
}

void HaikuSpaceUI::ExportWireframe(const Platform::Path &exportFile) {
  SS.ExportViewOrWireframeTo(exportFile, /*exportWireframe*/ true);
}

void HaikuSpaceUI::PromptForExportSectionFile() {
  SavePanel(EXPORT_SECTION);
}
void HaikuSpaceUI::ExportSection(const Platform::Path &exportFile) {
  SS.ExportSectionTo(exportFile);
}

void HaikuSpaceUI::PromptForExportMeshFile() {
  SavePanel(EXPORT_MESH);
}
void HaikuSpaceUI::ExportMesh(const Platform::Path &exportFile) {
  SS.ExportMeshTo(exportFile);
}

void HaikuSpaceUI::PromptForExportSurfacesFile() {
  SavePanel(EXPORT_SURFACES);
}
void HaikuSpaceUI::ExportSurfaces(const Platform::Path &exportFile) {
  StepFileWriter sfw = {};
  sfw.ExportSurfacesTo(exportFile);
}

void HaikuSpaceUI::PromptForStopTracingFile() {
  if (SS.traced.point == Entity::NO_ENTITY) {
    return;
  }

  SavePanel(STOP_TRACING);
}
void HaikuSpaceUI::StopTracing(const Platform::Path &exportFile) {
  FILE *f = OpenFile(exportFile, "wb");
  if (f) {
    int       i;
    SContour *sc = &(SS.traced.path);
    for (i = 0; i < sc->l.n; i++) {
      Vector p = sc->l[i].p;
      double s = SS.exportScale;
      fprintf(f, "%.10f, %.10f, %.10f\r\n", p.x / s, p.y / s, p.z / s);
    }
    fclose(f);
  } else {
    Error(_("Couldn't write to '%s'"), exportFile.raw.c_str());
  }
  // clear the trace, and stop tracing
  SS.traced.point = Entity::NO_ENTITY;
  SS.traced.path.l.Clear();
  SS.GW.Invalidate();
}

/*
  std::vector<FileFilter> SolveSpaceModelFileFilters = {
    {CN_ ("file-type", "SolveSpace models"), {"slvs"}},
  };
  std::vector<FileFilter> SolveSpaceLinkFileFilters = {
    {CN_ ("file-type", "ALL"), {"slvs", "emn", "stl"}},
    {CN_ ("file-type", "SolveSpace models"), {"slvs"}},
    {CN_ ("file-type", "IDF circuit board"), {"emn"}},
    {CN_ ("file-type", "STL triangle mesh"), {"stl"}},
  };
  std::vector<FileFilter> RasterFileFilters = {
    {CN_ ("file-type", "PNG image"), {"png"}},
  };
  std::vector<FileFilter> MeshFileFilters = {
    {CN_ ("file-type", "STL mesh"), {"stl"}},
    {CN_ ("file-type", "Wavefront OBJ mesh"), {"obj"}},
    {CN_ ("file-type", "Three.js-compatible mesh, with viewer"), {"html"}},
    {CN_ ("file-type", "Three.js-compatible mesh, mesh only"), {"js"}},
    {CN_ ("file-type", "VRML text file"), {"wrl"}},
  };
  std::vector<FileFilter> SurfaceFileFilters = {
    {CN_ ("file-type", "STEP file"), {"step", "stp"}},
  };
  std::vector<FileFilter> VectorFileFilters = {
    {CN_ ("file-type", "PDF file"), {"pdf"}},
    {CN_ ("file-type", "Encapsulated PostScript"), {"eps", "ps"}},
    {CN_ ("file-type", "Scalable Vector Graphics"), {"svg"}},
    {CN_ ("file-type", "STEP file"), {"step", "stp"}},
    {CN_ ("file-type", "DXF file (AutoCAD 2007)"), {"dxf"}},
    {CN_ ("file-type", "HPGL file"), {"plt", "hpgl"}},
    {CN_ ("file-type", "G Code"), {"ngc", "txt"}},
  };
  std::vector<FileFilter> Vector3dFileFilters = {
    {CN_ ("file-type", "STEP file"), {"step", "stp"}},
    {CN_ ("file-type", "DXF file (AutoCAD 2007)"), {"dxf"}},
  };
  std::vector<FileFilter> ImportFileFilters = {
    {CN_ ("file-type", "AutoCAD DXF and DWG files"), {"dxf", "dwg"}},
  };
  std::vector<FileFilter> CsvFileFilters = {
    {CN_ ("file-type", "Comma-separated values"), {"csv"}},
  };
*/
