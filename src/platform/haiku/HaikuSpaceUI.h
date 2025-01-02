/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#pragma once

#include <FilePanel.h>

#define GENERATE_REF_FILTER_PROTO(filterName) \
class filterName : public BRefFilter { \
  public: bool Filter(const entry_ref *entryRef, BNode *node, \
	struct stat_beos *stat, const char *fileType); \
};

GENERATE_REF_FILTER_PROTO(SolveSpaceModelFileFilter)
GENERATE_REF_FILTER_PROTO(SolveSpaceLinkFileFilter)
GENERATE_REF_FILTER_PROTO(RasterFileFilter)
GENERATE_REF_FILTER_PROTO(MeshFileFilter)
GENERATE_REF_FILTER_PROTO(SurfaceFileFilter)
GENERATE_REF_FILTER_PROTO(VectorFileFilter)
GENERATE_REF_FILTER_PROTO(Vector3dFileFilter)
GENERATE_REF_FILTER_PROTO(ImportFileFilter)
GENERATE_REF_FILTER_PROTO(CsvFileFilter)

class HaikuSpaceUI : public SolveSpaceUI {
  private:
  BRefFilter *solveSpaceModelFileFilter;
  BRefFilter *solveSpaceLinkFileFilter;
  BRefFilter *rasterFileFilter;
  BRefFilter *meshFileFilter;
  BRefFilter *surfaceFileFilter;
  BRefFilter *vectorFileFilter;
  BRefFilter *vector3dFileFilter;
  BRefFilter *importFileFilter;
  BRefFilter *csvFileFilter;

  public:
  HaikuSpaceUI();
  void SavePanel(uint32 messageName);
  void OpenPanel(uint32 messageName, BRefFilter *fileFilter);
  void OpenSolveSpaceFile();
  void UndoEnableMenus();
  bool OkayToStartNewFile();
  void GetFilenameAndSave(bool);
  bool SaveNewFile(Platform::Path newSaveFile);
  bool LoadAutosaveFor(const Platform::Path &filename);
  int  LocateImportedFile(const Platform::Path &filename, bool canCancel);
  void GetPngExportImageFilename();
  void PngExportImage(Platform::Path fp);
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
