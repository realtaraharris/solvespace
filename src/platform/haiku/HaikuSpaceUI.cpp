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

#include "App.h"
#include "../../solvespace.h"
#include "ssg.h"
#include "filewriter/stepfilewriter.h"
#include "filewriter/vectorfilewriter.h"

// https://www.iana.org/assignments/media-types/media-types.xhtml
#define GENERATE_REF_FILTER_IMPL(filterName, permittedMimeTypes) \
bool filterName::Filter( \
  const entry_ref *entryRef, \
	BNode *node, \
	struct stat_beos *stat, \
	const char *fileType \
) { \
  char type[B_MIME_TYPE_LENGTH]; \
  BNodeInfo nodeInfo(node); \
  if (node->IsDirectory()) { return true; } \
  nodeInfo.GetType(type); \
  for (const std::string &pmt : permittedMimeTypes) { \
    const BString mask(pmt.c_str()); \
    if (mask.Compare(type, mask.CountChars()) == 0) { \
      return true; \
    } \
  } \
  return false; \
}

// - "SolveSpace models", "slvs"
GENERATE_REF_FILTER_IMPL(SolveSpaceModelFileFilter, std::vector({"application/solvespace"}))

// - "SolveSpace models", "slvs"
// ? "IDF circuit board", "emn"
// - "STL triangle mesh", "stl"
GENERATE_REF_FILTER_IMPL(SolveSpaceLinkFileFilter, std::vector({"application/solvespace", "model/emn", "model/stl"}))

// - "PNG image", "png"
GENERATE_REF_FILTER_IMPL(RasterFileFilter, std::vector({"image/png"}))

// - "STL mesh", "stl"
// - "Wavefront OBJ mesh", "obj"
// - "Three.js-compatible mesh, with viewer", "html"
// - "Three.js-compatible mesh, mesh only", "js"
// - "VRML text file", "wrl"
GENERATE_REF_FILTER_IMPL(MeshFileFilter, std::vector({"model/stl", "model/obj", "text/html", "text/javascript", "model/vrml"}))

// - "STEP file", "step", "stp"
GENERATE_REF_FILTER_IMPL(SurfaceFileFilter, std::vector({"model/step"}))

// - "PDF file", "pdf"
// ? "Encapsulated PostScript", "eps"
// - "PostScript", "ps"
// - "Scalable Vector Graphics", "svg"
// - "STEP file", "step", "stp"
// - "DXF file (AutoCAD 2007)", "dxf" (text)
// - "HPGL file", "plt", "hpgl"
// ? "G Code", "ngc", "txt"
GENERATE_REF_FILTER_IMPL(VectorFileFilter, std::vector({"application/pdf", "image/x-eps", "application/postscript", "image/svg+xml", "model/step", "image/vnd.dxf", "application/vnd.hp-hpgl", "gcode"}))

// - "STEP file", "step", "stp"
// - "DXF file (AutoCAD 2007)" "dxf" (text)
GENERATE_REF_FILTER_IMPL(Vector3dFileFilter, std::vector({"model/step", "image/vnd.dxf"}))

// - "DXF file (AutoCAD 2007)", "dxf" (text)
// - "DWG file (AutoCAD 2007)", "dwg" (binary)
GENERATE_REF_FILTER_IMPL(ImportFileFilter, std::vector({"image/vnd.dxf", "image/vnd.dwg"}))

// - "Comma-separated values", "csv"
GENERATE_REF_FILTER_IMPL(CsvFileFilter, std::vector({"text/csv"}))

void HaikuSpaceUI::SavePanel(uint32 messageName) {
  BFilePanel *fp =
      new BFilePanel(B_SAVE_PANEL, NULL, NULL, B_FILE_NODE, false, new BMessage(messageName));
  fp->Show();
}

void HaikuSpaceUI::OpenPanel(uint32 messageName, BRefFilter *fileFilter) {
  BFilePanel *fp = new BFilePanel(B_OPEN_PANEL, NULL, NULL, B_FILE_NODE, false,
                                  new BMessage(messageName), fileFilter);
  fp->Show();
}

HaikuSpaceUI::HaikuSpaceUI() {
  solveSpaceModelFileFilter = new SolveSpaceModelFileFilter();
  solveSpaceLinkFileFilter = new SolveSpaceLinkFileFilter();
  rasterFileFilter = new RasterFileFilter();
  meshFileFilter = new MeshFileFilter();
  surfaceFileFilter = new SurfaceFileFilter();
  vectorFileFilter = new VectorFileFilter();
  vector3dFileFilter = new Vector3dFileFilter();
  importFileFilter = new ImportFileFilter();
  csvFileFilter = new CsvFileFilter();
}

void HaikuSpaceUI::OpenSolveSpaceFile() {
  OpenPanel(READ_FILE, solveSpaceModelFileFilter);
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

  if (!SS.saveFile.IsEmpty()) {
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
  OpenPanel(LINKED_FILE_IMAGE, rasterFileFilter);
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
  OpenPanel(IMPORT_FILE, importFileFilter);
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
    OpenPanel(GROUP_LINK_FILE, solveSpaceLinkFileFilter);
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