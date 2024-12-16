/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#ifndef HAIKUSPACEUI_H
#define HAIKUSPACEUI_H

#include <SupportDefs.h>

class HaikuSpaceUI : public SolveSpaceUI {
  public:
	void SavePanel(uint32 messageName);
	void OpenPanel(uint32 messageName);
  void OpenSolveSpaceFile();
  void UndoEnableMenus();
  bool OkayToStartNewFile();
  void GetFilenameAndSave(bool);
  bool SaveNewFile(Platform::Path newSaveFile);
  bool LoadAutosaveFor(const Platform::Path &filename);
  int  LocateImportedFile(const Platform::Path &filename, bool canCancel);
  void GetPngExportImageFilename();
  void PngExportImage();
  void PromptForLinkedFile();
  void LinkedFileImage(const Platform::Path &filename);
  void PromptForImportFile();
  void ImportFile(const Platform::Path &importFile);
  void PromptForGroupLink(Group &g);
  void GroupLink(const Platform::Path &groupLinkFile, Group &g);

  void PromptForExportViewFile();
  void ExportView(const Platform::Path &exportFile);

  void PromptForExportWireframeFile();
  void ExportWireframe(const Platform::Path &exportFile);

  void PromptForExportSectionFile();
  void ExportSection(const Platform::Path &exportFile);

  void PromptForExportMeshFile();
  void ExportMesh(const Platform::Path &exportFile);

  void PromptForExportSurfacesFile();
  void ExportSurfaces(const Platform::Path &exportFile);

  void PromptForStopTracingFile();
  void StopTracing(const Platform::Path &exportFile);
};

#endif // HAIKUSPACEUI_H
