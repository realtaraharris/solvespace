#include "solvespace.h"
#include "../platform/EventHooks.h"
#include "filewriter/stepfilewriter.h"
#include "filewriter/vectorfilewriter.h"
#include "config.h"

namespace SolveSpace {
  void ImportDxf(const Platform::Path &file);
  void ImportDwg(const Platform::Path &file);
}

void SolveSpaceUI::MenuFile(Command id) {
    Platform::SettingsRef settings = Platform::GetSettings();

    switch(id) {
        case Command::NEW:
            if(!SS.OkayToStartNewFile()) break;

            SS.saveFile.Clear();
            SS.NewFile();
            SS.AfterNewFile();
            break;

        case Command::OPEN: {
            if(!SS.OkayToStartNewFile()) break;

            Platform::FileDialogRef dialog = Platform::CreateOpenFileDialog(SS.GW.window);
            dialog->AddFilters(Platform::SolveSpaceModelFileFilters);
            dialog->ThawChoices(settings, "Sketch");
            if(dialog->RunModal()) {
                dialog->FreezeChoices(settings, "Sketch");
                SS.Load(dialog->GetFilename());
            }
            break;
        }

        case Command::SAVE:
            //SS.GetFilenameAndSave(/*saveAs=*/false);
            break;

        case Command::SAVE_AS:
            //SS.GetFilenameAndSave(/*saveAs=*/true);
            break;

        case Command::EXPORT_IMAGE: {
            Platform::FileDialogRef dialog = Platform::CreateSaveFileDialog(SS.GW.window);
            dialog->AddFilters(Platform::RasterFileFilters);
            dialog->ThawChoices(settings, "ExportImage");
            dialog->SuggestFilename(SS.saveFile);
            if(dialog->RunModal()) {
                dialog->FreezeChoices(settings, "ExportImage");
                SS.ExportAsPngTo(dialog->GetFilename());
            }
            break;
        }

        case Command::EXPORT_VIEW: {
            Platform::FileDialogRef dialog = Platform::CreateSaveFileDialog(SS.GW.window);
            dialog->AddFilters(Platform::VectorFileFilters);
            dialog->ThawChoices(settings, "ExportView");
            dialog->SuggestFilename(SS.saveFile);
            if(!dialog->RunModal()) break;
            dialog->FreezeChoices(settings, "ExportView");

            // If the user is exporting something where it would be
            // inappropriate to include the constraints, then warn.
            if(SS.GW.showConstraints &&
                (dialog->GetFilename().HasExtension("txt") ||
                 fabs(SS.exportOffset) > LENGTH_EPS))
            {
                Message(_("Constraints are currently shown, and will be exported "
                          "in the toolpath. This is probably not what you want; "
                          "hide them by clicking the link at the top of the "
                          "text window."));
            }

            SS.ExportViewOrWireframeTo(dialog->GetFilename(), /*exportWireframe=*/false);
            break;
        }

        case Command::EXPORT_WIREFRAME: {
            Platform::FileDialogRef dialog = Platform::CreateSaveFileDialog(SS.GW.window);
            dialog->AddFilters(Platform::Vector3dFileFilters);
            dialog->ThawChoices(settings, "ExportWireframe");
            dialog->SuggestFilename(SS.saveFile);
            if(!dialog->RunModal()) break;
            dialog->FreezeChoices(settings, "ExportWireframe");

            SS.ExportViewOrWireframeTo(dialog->GetFilename(), /*exportWireframe*/true);
            break;
        }

        case Command::EXPORT_SECTION: {
            Platform::FileDialogRef dialog = Platform::CreateSaveFileDialog(SS.GW.window);
            dialog->AddFilters(Platform::VectorFileFilters);
            dialog->ThawChoices(settings, "ExportSection");
            dialog->SuggestFilename(SS.saveFile);
            if(!dialog->RunModal()) break;
            dialog->FreezeChoices(settings, "ExportSection");

            SS.ExportSectionTo(dialog->GetFilename());
            break;
        }

        case Command::EXPORT_MESH: {
            Platform::FileDialogRef dialog = Platform::CreateSaveFileDialog(SS.GW.window);
            dialog->AddFilters(Platform::MeshFileFilters);
            dialog->ThawChoices(settings, "ExportMesh");
            dialog->SuggestFilename(SS.saveFile);
            if(!dialog->RunModal()) break;
            dialog->FreezeChoices(settings, "ExportMesh");

            SS.ExportMeshTo(dialog->GetFilename());
            break;
        }

        case Command::EXPORT_SURFACES: {
            Platform::FileDialogRef dialog = Platform::CreateSaveFileDialog(SS.GW.window);
            dialog->AddFilters(Platform::SurfaceFileFilters);
            dialog->ThawChoices(settings, "ExportSurfaces");
            dialog->SuggestFilename(SS.saveFile);
            if(!dialog->RunModal()) break;
            dialog->FreezeChoices(settings, "ExportSurfaces");

            StepFileWriter sfw = {};
            sfw.ExportSurfacesTo(dialog->GetFilename());
            break;
        }

        case Command::IMPORT: {
            Platform::FileDialogRef dialog = Platform::CreateOpenFileDialog(SS.GW.window);
            dialog->AddFilters(Platform::ImportFileFilters);
            dialog->ThawChoices(settings, "Import");
            if(!dialog->RunModal()) break;
            dialog->FreezeChoices(settings, "Import");

            Platform::Path importFile = dialog->GetFilename();
            if(importFile.HasExtension("dxf")) {
                ImportDxf(importFile);
            } else if(importFile.HasExtension("dwg")) {
                ImportDwg(importFile);
            } else {
                Error(_("Can't identify file type from file extension of "
                        "filename '%s'; try .dxf or .dwg."), importFile.raw.c_str());
                break;
            }

            SS.GenerateAll(SolveSpaceUI::Generate::UNTIL_ACTIVE);
            SS.ScheduleShowTW();
            break;
        }

        case Command::EXIT:
            if(!SS.OkayToStartNewFile()) break;
            SS.Exit();
            break;

        default: ssassert(false, "Unexpected menu ID");
    }
}

void SolveSpaceUI::MenuAnalyze(Command id) {
    Platform::SettingsRef settings = Platform::GetSettings();

    SS.GW.GroupSelection();
    auto const &gs = SS.GW.gs;

    switch(id) {
        case Command::STEP_DIM:
            if(gs.constraints == 1 && gs.n == 0) {
                Constraint *c = SK.GetConstraint(gs.constraint[0]);
                if(c->HasLabel() && !c->reference) {
                    SS.TW.stepDim.finish = c->valA;
                    SS.TW.stepDim.steps = 10;
                    SS.TW.stepDim.isDistance =
                        (c->type != Constraint::Type::ANGLE) &&
                        (c->type != Constraint::Type::LENGTH_RATIO) &&
                        (c->type != Constraint::Type::ARC_ARC_LEN_RATIO) &&
                        (c->type != Constraint::Type::ARC_LINE_LEN_RATIO) &&
                        (c->type != Constraint::Type::LENGTH_DIFFERENCE) &&
                        (c->type != Constraint::Type::ARC_ARC_DIFFERENCE) &&
                        (c->type != Constraint::Type::ARC_LINE_DIFFERENCE) ;
                    SS.TW.shown.constraint = c->h;
                    SS.TW.shown.screen = TextWindow::Screen::STEP_DIMENSION;

                    // The step params are specified in the text window,
                    // so force that to be shown.
                    SS.GW.ForceTextWindowShown();

                    SS.ScheduleShowTW();
                    SS.GW.ClearSelection();
                } else {
                    Error(_("Constraint must have a label, and must not be "
                            "a reference dimension."));
                }
            } else {
                Error(_("Bad selection for step dimension; select a constraint."));
            }
            break;

        case Command::NAKED_EDGES: {
            ShowNakedEdges(/*reportOnlyWhenNotOkay=*/false);
            break;
        }

        case Command::INTERFERENCE: {
            SS.nakedEdges.Clear();

            SMesh *m = &(SK.GetGroup(SS.GW.activeGroup)->displayMesh);
            SKdNode *root = SKdNode::From(m);
            bool inters, leaks;
            root->MakeCertainEdgesInto(&(SS.nakedEdges),
                EdgeKind::SELF_INTER, /*coplanarIsInter=*/false, &inters, &leaks);

            SS.GW.Invalidate();

            if(inters) {
                Error("%d edges interfere with other triangles, bad.",
                    SS.nakedEdges.l.n);
            } else {
                Message(_("The assembly does not interfere, good."));
            }
            break;
        }

        case Command::CENTER_OF_MASS: {
            SS.UpdateCenterOfMass();
            SS.centerOfMass.draw = true;
            SS.GW.Invalidate();
            break;
        }

        case Command::VOLUME: {
            Group *g = SK.GetGroup(SS.GW.activeGroup);
            double totalVol = g->displayMesh.CalculateVolume();
            std::string msg = ssprintf(
                _("The volume of the solid model is:\n\n"
                  "    %s"),
                SS.MmToStringSI(totalVol, /*dim=*/3).c_str());

            SMesh curMesh = {};
            g->thisShell.TriangulateInto(&curMesh);
            double curVol = curMesh.CalculateVolume();
            if(curVol > 0.0) {
                msg += ssprintf(
                    _("\nThe volume of current group mesh is:\n\n"
                      "    %s"),
                    SS.MmToStringSI(curVol, /*dim=*/3).c_str());
            }

            msg += _("\n\nCurved surfaces have been approximated as triangles.\n"
                     "This introduces error, typically of around 1%.");
            Message("%s", msg.c_str());
            break;
        }

        case Command::AREA: {
            Group *g = SK.GetGroup(SS.GW.activeGroup);
            SS.GW.GroupSelection();

            if(gs.faces > 0) {
                std::vector<uint32_t> faces;
                faces.push_back(gs.face[0].v);
                if(gs.faces > 1) faces.push_back(gs.face[1].v);
                double area = g->displayMesh.CalculateSurfaceArea(faces);
                Message(_("The surface area of the selected faces is:\n\n"
                          "    %s\n\n"
                          "Curves have been approximated as piecewise linear.\n"
                          "This introduces error, typically of around 1%%."),
                    SS.MmToStringSI(area, /*dim=*/2).c_str());
                break;
            }

            if(g->polyError.how != PolyError::GOOD) {
                Error(_("This group does not contain a correctly-formed "
                        "2d closed area. It is open, not coplanar, or self-"
                        "intersecting."));
                break;
            }
            SEdgeList sel = {};
            g->polyLoops.MakeEdgesInto(&sel);
            SPolygon sp = {};
            sel.AssemblePolygon(&sp, NULL, /*keepDir=*/true);
            sp.normal = sp.ComputeNormal();
            sp.FixContourDirections();
            double area = sp.SignedArea();
            Message(_("The area of the region sketched in this group is:\n\n"
                      "    %s\n\n"
                      "Curves have been approximated as piecewise linear.\n"
                      "This introduces error, typically of around 1%%."),
                SS.MmToStringSI(area, /*dim=*/2).c_str());
            sel.Clear();
            sp.Clear();
            break;
        }

        case Command::PERIMETER: {
            if(gs.n > 0 && gs.n == gs.entities) {
                double perimeter = 0.0;
                for(int i = 0; i < gs.entities; i++) {
                    Entity *en = SK.entity.FindById(gs.entity[i]);
                    SEdgeList *el = en->GetOrGenerateEdges();
                    for(const SEdge &e : el->l) {
                        perimeter += e.b.Minus(e.a).Magnitude();
                    }
                }
                Message(_("The total length of the selected entities is:\n\n"
                          "    %s\n\n"
                          "Curves have been approximated as piecewise linear.\n"
                          "This introduces error, typically of around 1%%."),
                    SS.MmToStringSI(perimeter, /*dim=*/1).c_str());
            } else {
                Error(_("Bad selection for perimeter; select line segments, arcs, and curves."));
            }
            break;
        }

        case Command::SHOW_DOF:
            // This works like a normal solve, except that it calculates
            // which variables are free/bound at the same time.
            SS.GenerateAll(SolveSpaceUI::Generate::ALL, /*andFindFree=*/true);
            break;

        case Command::TRACE_PT:
            if(gs.points == 1 && gs.n == 1) {
                SS.traced.point = gs.point[0];
                SS.GW.ClearSelection();
            } else {
                Error(_("Bad selection for trace; select a single point."));
            }
            break;

        case Command::STOP_TRACING: {
            if (SS.traced.point == Entity::NO_ENTITY) {
                break;
            }
            Platform::FileDialogRef dialog = Platform::CreateSaveFileDialog(SS.GW.window);
            dialog->AddFilters(Platform::CsvFileFilters);
            dialog->ThawChoices(settings, "Trace");
            dialog->SetFilename(SS.saveFile);
            if(dialog->RunModal()) {
                dialog->FreezeChoices(settings, "Trace");

                FILE *f = OpenFile(dialog->GetFilename(), "wb");
                if(f) {
                    int i;
                    SContour *sc = &(SS.traced.path);
                    for(i = 0; i < sc->l.n; i++) {
                        Vector p = sc->l[i].p;
                        double s = SS.exportScale;
                        fprintf(f, "%.10f, %.10f, %.10f\r\n",
                            p.x/s, p.y/s, p.z/s);
                    }
                    fclose(f);
                } else {
                    Error(_("Couldn't write to '%s'"), dialog->GetFilename().raw.c_str());
                }
            }
            // Clear the trace, and stop tracing
            SS.traced.point = Entity::NO_ENTITY;
            SS.traced.path.l.Clear();
            SS.GW.Invalidate();
            break;
        }

        default: ssassert(false, "Unexpected menu ID");
    }
}

void SolveSpaceUI::MenuView(Command id) {
    switch(id) {
        case Command::ZOOM_IN:
            SS.GW.ZoomToMouse(1);
            break;

        case Command::ZOOM_OUT:
            SS.GW.ZoomToMouse(-1);
            break;

        case Command::ZOOM_TO_FIT:
            SS.GW.ZoomToFit(/*includingInvisibles=*/false, /*useSelection=*/true);
            SS.ScheduleShowTW();
            break;

        case Command::SHOW_GRID:
            SS.GW.showSnapGrid = !SS.GW.showSnapGrid;
            SS.GW.EnsureValidActives();
            SS.GW.Invalidate();
            if(SS.GW.showSnapGrid && !SS.GW.LockedInWorkplane()) {
                Message(_("No workplane is active, so the grid will not appear."));
            }
            break;

        case Command::DIM_SOLID_MODEL:
            SS.GW.dimSolidModel = !SS.GW.dimSolidModel;
            SS.GW.EnsureValidActives();
            SS.GW.Invalidate(/*clearPersistent=*/true);
            break;

        case Command::PERSPECTIVE_PROJ:
            SS.usePerspectiveProj = !SS.usePerspectiveProj;
            SS.GW.EnsureValidActives();
            SS.GW.Invalidate();
            if(SS.cameraTangent < 1e-6) {
                Error(_("The perspective factor is set to zero, so the view will "
                        "always be a parallel projection.\n\n"
                        "For a perspective projection, modify the perspective "
                        "factor in the configuration screen. A value around 0.3 "
                        "is typical."));
            }
            break;

        case Command::EXPLODE_SKETCH:
            SS.explode = !SS.explode;
            SS.GW.EnsureValidActives();
            SS.MarkGroupDirty(SS.GW.activeGroup, true);
            break;

        case Command::ONTO_WORKPLANE:
            if(SS.GW.LockedInWorkplane()) {
                SS.GW.AnimateOntoWorkplane();
                break;
            }  // if not in 2d mode use ORTHO logic
            // fallthrough
        case Command::NEAREST_ORTHO:
        case Command::NEAREST_ISO: {
            static const Vector ortho[3] = {
                Vector::From(1, 0, 0),
                Vector::From(0, 1, 0),
                Vector::From(0, 0, 1)
            };
            double sqrt2 = sqrt(2.0), sqrt6 = sqrt(6.0);
            Quaternion quat0 = Quaternion::From(SS.GW.projRight, SS.GW.projUp);
            Quaternion quatf = quat0;
            double dmin = 1e10;

            // There are 24 possible views; 3*2*2*2
            int i, j, negi, negj;
            for(i = 0; i < 3; i++) {
                for(j = 0; j < 3; j++) {
                    if(i == j) continue;
                    for(negi = 0; negi < 2; negi++) {
                        for(negj = 0; negj < 2; negj++) {
                            Vector ou = ortho[i], ov = ortho[j];
                            // TODO: do this only if we're animating. otherwise these lines flip the camera incorrectly
                            // if(negi) ou = ou.ScaledBy(-1);
                            // if(negj) ov = ov.ScaledBy(-1);
                            Vector on = ou.Cross(ov);

                            Vector u, v;
                            if(id == Command::NEAREST_ORTHO || id == Command::ONTO_WORKPLANE) {
                                u = ou;
                                v = ov;
                            } else {
                                u =
                                    ou.ScaledBy(1/sqrt2).Plus(
                                    on.ScaledBy(-1/sqrt2));
                                v =
                                    ou.ScaledBy(-1/sqrt6).Plus(
                                    ov.ScaledBy(2/sqrt6).Plus(
                                    on.ScaledBy(-1/sqrt6)));
                            }

                            Quaternion quatt = Quaternion::From(u, v);
                            double d = std::min(
                                (quatt.Minus(quat0)).Magnitude(),
                                (quatt.Plus(quat0)).Magnitude());
                            if(d < dmin) {
                                dmin = d;
                                quatf = quatt;
                            }
                        }
                    }
                }
            }

            SS.GW.AnimateOnto(quatf, SS.GW.offset);
            break;
        }

        case Command::CENTER_VIEW:
            SS.GW.GroupSelection();
            if(SS.GW.gs.n == 1 && SS.GW.gs.points == 1) {
                Quaternion quat0;
                // Offset is the selected point, quaternion is same as before
                Vector pt = SK.GetEntity(SS.GW.gs.point[0])->PointGetNum();
                quat0 = Quaternion::From(SS.GW.projRight, SS.GW.projUp);
                SS.GW.ClearSelection();
                SS.GW.AnimateOnto(quat0, pt.ScaledBy(-1));
            } else {
                Error(_("Select a point; this point will become the center "
                        "of the view on screen."));
            }
            break;

        case Command::SHOW_TOOLBAR:
            SS.showToolbar = !SS.showToolbar;
            SS.GW.EnsureValidActives();
            SS.GW.Invalidate();
            break;

        case Command::SHOW_TEXT_WND:
            SS.GW.showTextWindow = !SS.GW.showTextWindow;
            SS.GW.EnsureValidActives();
            break;

        case Command::UNITS_INCHES:
            SS.viewUnits = Unit::INCHES;
            SS.ScheduleShowTW();
            SS.GW.EnsureValidActives();
            break;

        case Command::UNITS_FEET_INCHES:
            SS.viewUnits = Unit::FEET_INCHES;
            SS.ScheduleShowTW();
            SS.GW.EnsureValidActives();
            break;

        case Command::UNITS_MM:
            SS.viewUnits = Unit::MM;
            SS.ScheduleShowTW();
            SS.GW.EnsureValidActives();
            break;

        case Command::UNITS_METERS:
            SS.viewUnits = Unit::METERS;
            SS.ScheduleShowTW();
            SS.GW.EnsureValidActives();
            break;

        case Command::FULL_SCREEN:
            SS.GW.window->SetFullScreen(!SS.GW.window->IsFullScreen());
            SS.GW.EnsureValidActives();
            break;

        default: ssassert(false, "Unexpected menu ID");
    }
    SS.GW.Invalidate();
}

void SolveSpaceUI::MenuEdit(Command id) {
    switch(id) {
        case Command::UNSELECT_ALL:
            SS.GW.GroupSelection();
            // If there's nothing selected to de-select, and no operation
            // to cancel, then perhaps they want to return to the home
            // screen in the text window.
            if(SS.GW.gs.n               == 0 &&
               SS.GW.gs.constraints     == 0 &&
               SS.GW.pending.operation  == GraphicsWindow::Pending::NONE)
            {
                if(!(SS.TW.window->IsEditorVisible() ||
                     SS.GW.window->IsEditorVisible()))
                {
                    if(SS.TW.shown.screen == TextWindow::Screen::STYLE_INFO) {
                        SS.TW.GoToScreen(TextWindow::Screen::LIST_OF_STYLES);
                    } else {
                        SS.TW.ClearSuper();
                    }
                }
            }
            SS.GW.ClearSuper(7);
            SS.TW.HideEditControl();
            SS.nakedEdges.Clear();
            SS.justExportedInfo.draw = false;
            SS.centerOfMass.draw = false;
            // This clears the marks drawn to indicate which points are
            // still free to drag.
            for(Param &p : SK.param) {
                p.free = false;
            }
            if(SS.exportMode) {
                SS.exportMode = false;
                SS.GenerateAll(SolveSpaceUI::Generate::ALL);
            }
            SS.GW.persistentDirty = true;
            break;

        case Command::SELECT_ALL: {
            for(Entity &e : SK.entity) {
                if(e.group != SS.GW.activeGroup) continue;
                if(e.IsFace() || e.IsDistance()) continue;
                if(!e.IsVisible()) continue;

                SS.GW.MakeSelected(e.h);
            }
            SS.GW.Invalidate();
            SS.ScheduleShowTW();
            break;
        }

        case Command::SELECT_CHAIN: {
            int newlySelected = 0;
            bool didSomething;
            do {
                didSomething = false;
                for(Entity &e : SK.entity) {
                    if(e.group != SS.GW.activeGroup) continue;
                    if(!e.HasEndpoints()) continue;
                    if(!e.IsVisible()) continue;

                    Vector st = e.EndpointStart(),
                           fi = e.EndpointFinish();

                    bool onChain = false, alreadySelected = false;
                    List<GraphicsWindow::Selection> *ls = &(SS.GW.selection);
                    for(GraphicsWindow::Selection *s = ls->First(); s; s = ls->NextAfter(s)) {
                        if(!s->entity.v) continue;
                        if(s->entity == e.h) {
                            alreadySelected = true;
                            continue;
                        }
                        Entity *se = SK.GetEntity(s->entity);
                        if(!se->HasEndpoints()) continue;

                        Vector sst = se->EndpointStart(),
                               sfi = se->EndpointFinish();

                        if(sst.Equals(st) || sst.Equals(fi) ||
                           sfi.Equals(st) || sfi.Equals(fi))
                        {
                            onChain = true;
                        }
                    }
                    if(onChain && !alreadySelected) {
                        SS.GW.MakeSelected(e.h);
                        newlySelected++;
                        didSomething = true;
                    }
                }
            } while(didSomething);
            SS.GW.Invalidate();
            SS.ScheduleShowTW();
            if(newlySelected == 0) {
                Error(_("No additional entities share endpoints with the selected entities."));
            }
            break;
        }

        case Command::ROTATE_90: {
            SS.GW.GroupSelection();
            Entity *e = NULL;
            if(SS.GW.gs.n == 1 && SS.GW.gs.points == 1) {
                e = SK.GetEntity(SS.GW.gs.point[0]);
            } else if(SS.GW.gs.n == 1 && SS.GW.gs.entities == 1) {
                e = SK.GetEntity(SS.GW.gs.entity[0]);
            }
            SS.GW.ClearSelection();

            hGroup hg = e ? e->group : SS.GW.activeGroup;
            Group *g = SK.GetGroup(hg);
            if(g->type != Group::Type::LINKED) {
                Error(_("To use this command, select a point or other "
                        "entity from an linked part, or make a link "
                        "group the active group."));
                break;
            }

            SS.UndoRemember();
            // Rotate by ninety degrees about the coordinate axis closest
            // to the screen normal.
            Vector norm = SS.GW.projRight.Cross(SS.GW.projUp);
            norm = norm.ClosestOrtho();
            norm = norm.WithMagnitude(1);
            Quaternion qaa = Quaternion::From(norm, PI/2);

            g->TransformImportedBy(Vector::From(0, 0, 0), qaa);

            // and regenerate as necessary.
            SS.MarkGroupDirty(hg);
            break;
        }

        case Command::SNAP_TO_GRID: {
            if(!SS.GW.LockedInWorkplane()) {
                Error(_("No workplane is active. Activate a workplane "
                        "(with Sketch -> In Workplane) to define the plane "
                        "for the snap grid."));
                break;
            }
            SS.GW.GroupSelection();
            if(SS.GW.gs.points == 0 && SS.GW.gs.constraintLabels == 0) {
                Error(_("Can't snap these items to grid; select points, "
                        "text comments, or constraints with a label. "
                        "To snap a line, select its endpoints."));
                break;
            }
            SS.UndoRemember();

            List<GraphicsWindow::Selection> *ls = &(SS.GW.selection);
            for(GraphicsWindow::Selection *s = ls->First(); s; s = ls->NextAfter(s)) {
                if(s->entity.v) {
                    hEntity hp = s->entity;
                    Entity *ep = SK.GetEntity(hp);
                    if(!ep->IsPoint()) continue;

                    Vector p = ep->PointGetNum();
                    ep->PointForceTo(SS.GW.SnapToGrid(p));
                    SS.GW.pending.points.Add(&hp);
                    SS.MarkGroupDirty(ep->group);
                } else if(s->constraint.v) {
                    Constraint *c = SK.GetConstraint(s->constraint);
                    std::vector<Vector> refs;
                    c->GetReferencePoints(SS.GW.GetCamera(), &refs);
                    c->disp.offset = c->disp.offset.Plus(SS.GW.SnapToGrid(refs[0]).Minus(refs[0]));
                }
            }
            // Regenerate, with these points marked as dragged so that they
            // get placed as close as possible to our snap grid.
            SS.GW.ClearSelection();
            break;
        }

        case Command::UNDO:
            SS.UndoUndo();
            break;

        case Command::REDO:
            SS.UndoRedo();
            break;

        case Command::REGEN_ALL:
            SS.images.clear();
            SS.ReloadAllLinked(SS.saveFile);
            SS.GenerateAll(SolveSpaceUI::Generate::UNTIL_ACTIVE);
            SS.ScheduleShowTW();
            break;

        case Command::EDIT_LINE_STYLES:
            SS.TW.GoToScreen(TextWindow::Screen::LIST_OF_STYLES);
            SS.GW.ForceTextWindowShown();
            SS.ScheduleShowTW();
            break;
        case Command::VIEW_PROJECTION:
            SS.TW.GoToScreen(TextWindow::Screen::EDIT_VIEW);
            SS.GW.ForceTextWindowShown();
            SS.ScheduleShowTW();
            break;
        case Command::CONFIGURATION:
            SS.TW.GoToScreen(TextWindow::Screen::CONFIGURATION);
            SS.GW.ForceTextWindowShown();
            SS.ScheduleShowTW();
            break;

        default: ssassert(false, "Unexpected menu ID");
    }
}

void SolveSpaceUI::MenuClipboard(Command id) {
    if(id != Command::DELETE && !SS.GW.LockedInWorkplane()) {
        Error(_("Cut, paste, and copy work only in a workplane.\n\n"
                "Activate one with Sketch -> In Workplane."));
        return;
    }

    switch(id) {
        case Command::PASTE: {
            SS.UndoRemember();
            Vector trans = SS.GW.projRight.ScaledBy(80/SS.GW.scale).Plus(
                           SS.GW.projUp   .ScaledBy(40/SS.GW.scale));
            SS.GW.ClearSelection();
            SS.GW.PasteClipboard(trans, 0, 1);
            break;
        }

        case Command::PASTE_TRANSFORM: {
            if(SS.clipboard.r.IsEmpty()) {
                Error(_("Clipboard is empty; nothing to paste."));
                break;
            }

            Entity *wrkpl  = SK.GetEntity(SS.GW.ActiveWorkplane());
            Vector p = SK.GetEntity(wrkpl->point[0])->PointGetNum();
            SS.TW.shown.paste.times  = 1;
            SS.TW.shown.paste.trans  = Vector::From(0, 0, 0);
            SS.TW.shown.paste.theta  = 0;
            SS.TW.shown.paste.origin = p;
            SS.TW.shown.paste.scale  = 1;
            SS.TW.GoToScreen(TextWindow::Screen::PASTE_TRANSFORMED);
            SS.GW.ForceTextWindowShown();
            SS.ScheduleShowTW();
            break;
        }

        case Command::COPY:
            SS.GW.CopySelection();
            SS.GW.ClearSelection();
            break;

        case Command::CUT:
            SS.UndoRemember();
            SS.GW.CopySelection();
            SS.GW.DeleteSelection();
            break;

        case Command::DELETE:
            SS.UndoRemember();
            SS.GW.DeleteSelection();
            break;

        default: ssassert(false, "Unexpected menu ID");
    }
}

void SolveSpaceUI::MenuRequest(Command id) {
    const char *s;
    switch(id) {
        case Command::SEL_WORKPLANE: {
            SS.GW.GroupSelection();
            Group *g = SK.GetGroup(SS.GW.activeGroup);

            if(SS.GW.gs.n == 1 && SS.GW.gs.workplanes == 1) {
                // A user-selected workplane
                g->activeWorkplane = SS.GW.gs.entity[0];
                SS.GW.EnsureValidActives();
                SS.ScheduleShowTW();
            } else if(g->type == Group::Type::DRAWING_WORKPLANE) {
                // The group's default workplane
                g->activeWorkplane = g->h.entity(0);
                MessageAndRun([] {
                    // Align the view with the selected workplane
                    SS.GW.ClearSuper(8);
                    SS.GW.AnimateOntoWorkplane();
                }, _("No workplane selected. Activating default workplane "
                     "for this group."));
            } else {
                Error(_("No workplane is selected, and the active group does "
                        "not have a default workplane. Try selecting a "
                        "workplane, or activating a sketch-in-new-workplane "
                        "group."));
                //update checkboxes in the menus
                SS.GW.EnsureValidActives();
            }
            break;
        }
        case Command::FREE_IN_3D:
            SS.GW.SetWorkplaneFreeIn3d();
            SS.GW.EnsureValidActives();
            SS.ScheduleShowTW();
            SS.GW.Invalidate();
            break;

        case Command::TANGENT_ARC:
            SS.GW.GroupSelection();

            if (SS.GW.gs.n == 1 && SS.GW.gs.points == 1) {
                SS.GW.MakeTangentArc();
            } else {
                Error(_("Bad selection for tangent arc at point. Select a "
                        "single point, or select nothing to set up arc "
                        "parameters."));
            }
						TangentArcToolButtonUnclickedEventHook();
            break;

        case Command::ARC: s = _("click point on arc (draws anti-clockwise)"); goto c;
        case Command::DATUM_POINT: s = _("click to place datum point"); goto c;
        case Command::LINE_SEGMENT: s = _("click first point of line segment"); goto c;
        case Command::CONSTR_SEGMENT:
            s = _("click first point of construction line segment"); goto c;
        case Command::CUBIC: s = _("click first point of cubic segment"); goto c;
        case Command::CIRCLE: s = _("click center of circle"); goto c;
        case Command::WORKPLANE: s = _("click origin of workplane"); goto c;
        case Command::RECTANGLE: s = _("click one corner of rectangle"); goto c;
        case Command::TTF_TEXT: s = _("click top left of text"); goto c;
        case Command::IMAGE:
            if(!SS.ReloadLinkedImage(SS.saveFile, &SS.GW.pending.filename,
                                     /*canCancel=*/true)) {
                return;
            }
            s = _("click top left of image"); goto c;
c:
            SS.GW.pending.operation = GraphicsWindow::Pending::COMMAND;
            SS.GW.pending.command = id;
            SS.GW.pending.description = s;
            SS.ScheduleShowTW();
            SS.GW.Invalidate(); // repaint toolbar
            break;

        case Command::CONSTRUCTION: {
            // if we are drawing
            if(SS.GW.pending.operation == GraphicsWindow::Pending::DRAGGING_NEW_POINT ||
               SS.GW.pending.operation == GraphicsWindow::Pending::DRAGGING_NEW_LINE_POINT ||
               SS.GW.pending.operation == GraphicsWindow::Pending::DRAGGING_NEW_ARC_POINT ||
               SS.GW.pending.operation == GraphicsWindow::Pending::DRAGGING_NEW_CUBIC_POINT ||
               SS.GW.pending.operation == GraphicsWindow::Pending::DRAGGING_NEW_RADIUS) {
                for(auto &hr : SS.GW.pending.requests) {
                    Request* r = SK.GetRequest(hr);
                    r->construction = !(r->construction);
                    SS.MarkGroupDirty(r->group);
                }
                SS.GW.Invalidate();
                break;
            }
            SS.GW.GroupSelection();
            if(SS.GW.gs.entities == 0) {
                Error(_("No entities are selected. Select entities before "
                        "trying to toggle their construction state."));
                break;
            }
            SS.UndoRemember();
            int i;
            for(i = 0; i < SS.GW.gs.entities; i++) {
                hEntity he = SS.GW.gs.entity[i];
                if(!he.isFromRequest()) continue;
                Request *r = SK.GetRequest(he.request());
                r->construction = !(r->construction);
                SS.MarkGroupDirty(r->group);
            }
            SS.GW.ClearSelection();
						ConstructionToolButtonUnclickedEventHook();
            break;
        }

        case Command::SPLIT_CURVES:
            SS.GW.SplitLinesOrCurves();
						SplitCurvesToolButtonUnclickedEventHook();
            break;

        default: ssassert(false, "Unexpected menu ID");
    }
}

void SolveSpaceUI::MenuConstrain(Command id) {
    Constraint c = {};
    c.group = SS.GW.activeGroup;
    c.workplane = SS.GW.ActiveWorkplane();

    SS.GW.GroupSelection();
    auto const &gs = SS.GW.gs;

    switch(id) {
        case Command::DISTANCE_DIA:
        case Command::REF_DISTANCE: {
            if(gs.points == 2 && gs.n == 2) {
                c.type = Constraint::Type::PT_PT_DISTANCE;
                c.ptA = gs.point[0];
                c.ptB = gs.point[1];
            } else if(gs.lineSegments == 1 && gs.n == 1) {
                c.type = Constraint::Type::PT_PT_DISTANCE;
                Entity *e = SK.GetEntity(gs.entity[0]);
                c.ptA = e->point[0];
                c.ptB = e->point[1];
            } else if(gs.vectors == 1 && gs.points == 2 && gs.n == 3) {
                c.type = Constraint::Type::PROJ_PT_DISTANCE;
                c.ptA = gs.point[0];
                c.ptB = gs.point[1];
                c.entityA = gs.vector[0];
            } else if(gs.workplanes == 1 && gs.points == 1 && gs.n == 2) {
                c.type = Constraint::Type::PT_PLANE_DISTANCE;
                c.ptA = gs.point[0];
                c.entityA = gs.entity[0];
            } else if(gs.lineSegments == 1 && gs.points == 1 && gs.n == 2) {
                c.type = Constraint::Type::PT_LINE_DISTANCE;
                c.ptA = gs.point[0];
                c.entityA = gs.entity[0];
            } else if(gs.faces == 1 && gs.points == 1 && gs.n == 2) {
                c.type = Constraint::Type::PT_FACE_DISTANCE;
                c.ptA = gs.point[0];
                c.entityA = gs.face[0];
            } else if(gs.circlesOrArcs == 1 && gs.n == 1) {
                c.type = Constraint::Type::DIAMETER;
                c.entityA = gs.entity[0];
            } else {
                Error(_("Bad selection for distance / diameter constraint. This "
                        "constraint can apply to:\n\n"
                        "    * two points (distance between points)\n"
                        "    * a line segment (length)\n"
                        "    * two points and a line segment or normal (projected distance)\n"
                        "    * a workplane and a point (minimum distance)\n"
                        "    * a line segment and a point (minimum distance)\n"
                        "    * a plane face and a point (minimum distance)\n"
                        "    * a circle or an arc (diameter)\n"));
                return;
            }
            if(c.type == Constraint::Type::PT_PT_DISTANCE || c.type == Constraint::Type::PROJ_PT_DISTANCE) {
                Vector n = SS.GW.projRight.Cross(SS.GW.projUp);
                Vector a = SK.GetEntity(c.ptA)->PointGetNum();
                Vector b = SK.GetEntity(c.ptB)->PointGetNum();
                c.disp.offset = n.Cross(a.Minus(b));
                c.disp.offset = (c.disp.offset).WithMagnitude(50/SS.GW.scale);
            } else {
                c.disp.offset = Vector::From(0, 0, 0);
            }

            if(id == Command::REF_DISTANCE) {
                c.reference = true;
            }

            c.valA = 0;
            c.ModifyToSatisfy();
            Constraint::AddConstraint(&c);
            break;
        }

        case Command::ON_ENTITY:
            if(gs.points == 2 && gs.n == 2) {
                c.type = Constraint::Type::POINTS_COINCIDENT;
                c.ptA = gs.point[0];
                c.ptB = gs.point[1];
            } else if(gs.points == 1 && gs.workplanes == 1 && gs.n == 2) {
                c.type = Constraint::Type::PT_IN_PLANE;
                c.ptA = gs.point[0];
                c.entityA = gs.entity[0];
            } else if(gs.points == 1 && gs.lineSegments == 1 && gs.n == 2) {
                c.type = Constraint::Type::PT_ON_LINE;
                c.ptA = gs.point[0];
                c.entityA = gs.entity[0];
            } else if(gs.points == 1 && gs.circlesOrArcs == 1 && gs.n == 2) {
                c.type = Constraint::Type::PT_ON_CIRCLE;
                c.ptA = gs.point[0];
                c.entityA = gs.entity[0];
            } else if(gs.points == 1 && gs.faces == 1 && gs.n == 2) {
                c.type = Constraint::Type::PT_ON_FACE;
                c.ptA = gs.point[0];
                c.entityA = gs.face[0];
            } else {
                Error(_("Bad selection for on point / curve / plane constraint. "
                        "This constraint can apply to:\n\n"
                        "    * two points (points coincident)\n"
                        "    * a point and a workplane (point in plane)\n"
                        "    * a point and a line segment (point on line)\n"
                        "    * a point and a circle or arc (point on curve)\n"
                        "    * a point and a plane face (point on face)\n"));
                return;
            }
            Constraint::AddConstraint(&c);
            break;

        case Command::EQUAL:
            if(gs.lineSegments == 2 && gs.n == 2) {
                c.type = Constraint::Type::EQUAL_LENGTH_LINES;
                c.entityA = gs.entity[0];
                c.entityB = gs.entity[1];
            } else if(gs.lineSegments == 2 && gs.points == 2 && gs.n == 4) {
                c.type = Constraint::Type::EQ_PT_LN_DISTANCES;
                c.entityA = gs.entity[0];
                c.ptA = gs.point[0];
                c.entityB = gs.entity[1];
                c.ptB = gs.point[1];
            } else if(gs.lineSegments == 1 && gs.points == 2 && gs.n == 3) {
                // The same line segment for the distances, but different
                // points.
                c.type = Constraint::Type::EQ_PT_LN_DISTANCES;
                c.entityA = gs.entity[0];
                c.ptA = gs.point[0];
                c.entityB = gs.entity[0];
                c.ptB = gs.point[1];
            } else if(gs.lineSegments == 2 && gs.points == 1 && gs.n == 3) {
                c.type = Constraint::Type::EQ_LEN_PT_LINE_D;
                c.entityA = gs.entity[0];
                c.entityB = gs.entity[1];
                c.ptA = gs.point[0];
            } else if(gs.vectors == 4 && gs.n == 4) {
                c.type = Constraint::Type::EQUAL_ANGLE;
                c.entityA = gs.vector[0];
                c.entityB = gs.vector[1];
                c.entityC = gs.vector[2];
                c.entityD = gs.vector[3];
            } else if(gs.vectors == 3 && gs.n == 3) {
                c.type = Constraint::Type::EQUAL_ANGLE;
                c.entityA = gs.vector[0];
                c.entityB = gs.vector[1];
                c.entityC = gs.vector[1];
                c.entityD = gs.vector[2];
            } else if(gs.circlesOrArcs == 2 && gs.n == 2) {
                c.type = Constraint::Type::EQUAL_RADIUS;
                c.entityA = gs.entity[0];
                c.entityB = gs.entity[1];
            } else if(gs.arcs == 1 && gs.lineSegments == 1 && gs.n == 2) {
                c.type = Constraint::Type::EQUAL_LINE_ARC_LEN;
                if(SK.GetEntity(gs.entity[0])->type == Entity::Type::ARC_OF_CIRCLE) {
                    c.entityA = gs.entity[1];
                    c.entityB = gs.entity[0];
                } else {
                    c.entityA = gs.entity[0];
                    c.entityB = gs.entity[1];
                }
            } else {
                Error(_("Bad selection for equal length / radius constraint. "
                        "This constraint can apply to:\n\n"
                        "    * two line segments (equal length)\n"
                        "    * two line segments and two points "
                                "(equal point-line distances)\n"
                        "    * a line segment and two points "
                                "(equal point-line distances)\n"
                        "    * a line segment, and a point and line segment "
                                "(point-line distance equals length)\n"
                        "    * four line segments or normals "
                                "(equal angle between A,B and C,D)\n"
                        "    * three line segments or normals "
                                "(equal angle between A,B and B,C)\n"
                        "    * two circles or arcs (equal radius)\n"
                        "    * a line segment and an arc "
                                "(line segment length equals arc length)\n"));
                return;
            }
            if(c.type == Constraint::Type::EQUAL_ANGLE) {
                // Infer the nearest supplementary angle from the sketch.
                Vector a1 = SK.GetEntity(c.entityA)->VectorGetNum(),
                       b1 = SK.GetEntity(c.entityB)->VectorGetNum(),
                       a2 = SK.GetEntity(c.entityC)->VectorGetNum(),
                       b2 = SK.GetEntity(c.entityD)->VectorGetNum();
                double d1 = a1.Dot(b1), d2 = a2.Dot(b2);

                if(d1*d2 < 0) {
                    c.other = true;
                }
            }
            Constraint::AddConstraint(&c);
            break;

        case Command::RATIO:
            if(gs.lineSegments == 2 && gs.n == 2) {
                c.type = Constraint::Type::LENGTH_RATIO;
                c.entityA = gs.entity[0];
                c.entityB = gs.entity[1];
            }
            else if(gs.arcs == 2 && gs.n == 2) {
                c.type = Constraint::Type::ARC_ARC_LEN_RATIO;
                c.entityA = gs.entity[0];
                c.entityB = gs.entity[1];
            }
            else if(gs.lineSegments == 1 && gs.arcs == 1 && gs.n == 2) {
                c.type = Constraint::Type::ARC_LINE_LEN_RATIO;
                if(SK.GetEntity(gs.entity[0])->type == Entity::Type::ARC_OF_CIRCLE) {
                    c.entityA = gs.entity[1];
                    c.entityB = gs.entity[0];
                } else {
                    c.entityA = gs.entity[0];
                    c.entityB = gs.entity[1];
                }
            } else {
                Error(_("Bad selection for length ratio constraint. This "
                        "constraint can apply to:\n\n"
                        "    * two line segments\n"
                        "    * two arcs\n"
                        "    * one arc and one line segment\n"));
                return;
            }

            c.valA = 0;
            c.ModifyToSatisfy();
            Constraint::AddConstraint(&c);
            break;

        case Command::DIFFERENCE:
            if(gs.lineSegments == 2 && gs.n == 2) {
                c.type = Constraint::Type::LENGTH_DIFFERENCE;
                c.entityA = gs.entity[0];
                c.entityB = gs.entity[1];
            }
            else if(gs.arcs == 2 && gs.n == 2) {
                c.type = Constraint::Type::ARC_ARC_DIFFERENCE;
                c.entityA = gs.entity[0];
                c.entityB = gs.entity[1];
            }
            else if(gs.lineSegments == 1 && gs.arcs == 1 && gs.n == 2) {
                c.type = Constraint::Type::ARC_LINE_DIFFERENCE;
                if(SK.GetEntity(gs.entity[0])->type == Entity::Type::ARC_OF_CIRCLE) {
                    c.entityA = gs.entity[1];
                    c.entityB = gs.entity[0];
                } else {
                    c.entityA = gs.entity[0];
                    c.entityB = gs.entity[1];
                }
            } else {
                Error(_("Bad selection for length difference constraint. This "
                        "constraint can apply to:\n\n"
                        "    * two line segments\n"
                        "    * two arcs\n"
                        "    * one arc and one line segment\n"));
                return;
            }

            c.valA = 0;
            c.ModifyToSatisfy();
            Constraint::AddConstraint(&c);
            break;

        case Command::AT_MIDPOINT:
            if(gs.lineSegments == 1 && gs.points == 1 && gs.n == 2) {
                c.type = Constraint::Type::AT_MIDPOINT;
                c.entityA = gs.entity[0];
                c.ptA = gs.point[0];

                // If a point is at-midpoint, then no reason to also constrain
                // it on-line; so auto-remove that.  Handle as one undo group.
                SS.UndoRemember();
                Constraint::DeleteAllConstraintsFor(Constraint::Type::PT_ON_LINE, c.entityA, c.ptA);
                Constraint::AddConstraint(&c, /*rememberForUndo=*/false);
                break;
            } else if(gs.lineSegments == 1 && gs.workplanes == 1 && gs.n == 2) {
                c.type = Constraint::Type::AT_MIDPOINT;
                int i = SK.GetEntity(gs.entity[0])->IsWorkplane() ? 1 : 0;
                c.entityA = gs.entity[i];
                c.entityB = gs.entity[1-i];
            } else {
                Error(_("Bad selection for at midpoint constraint. This "
                        "constraint can apply to:\n\n"
                        "    * a line segment and a point "
                              "(point at midpoint)\n"
                        "    * a line segment and a workplane "
                              "(line's midpoint on plane)\n"));
                return;
            }
            Constraint::AddConstraint(&c);
            break;

        case Command::SYMMETRIC:
            if(gs.points == 2 &&
                                ((gs.workplanes == 1 && gs.n == 3) ||
                                 (gs.n == 2)))
            {
                if(gs.entities > 0)
                    c.entityA = gs.entity[0];
                c.ptA = gs.point[0];
                c.ptB = gs.point[1];
                c.type = Constraint::Type::SYMMETRIC;
            } else if(gs.lineSegments == 1 &&
                                ((gs.workplanes == 1 && gs.n == 2) ||
                                 (gs.n == 1)))
            {
                Entity *line;
                if(SK.GetEntity(gs.entity[0])->IsWorkplane()) {
                    line = SK.GetEntity(gs.entity[1]);
                    c.entityA = gs.entity[0];
                } else {
                    line = SK.GetEntity(gs.entity[0]);
                }
                c.ptA = line->point[0];
                c.ptB = line->point[1];
                c.type = Constraint::Type::SYMMETRIC;
            } else if(SS.GW.LockedInWorkplane()
                        && gs.lineSegments == 2 && gs.n == 2)
            {
                Entity *l0 = SK.GetEntity(gs.entity[0]),
                       *l1 = SK.GetEntity(gs.entity[1]);

                if((l1->group != SS.GW.activeGroup) ||
                   (l1->construction && !(l0->construction)))
                {
                    std::swap(l0, l1);
                }
                c.ptA = l1->point[0];
                c.ptB = l1->point[1];
                c.entityA = l0->h;
                c.type = Constraint::Type::SYMMETRIC_LINE;
            } else if(SS.GW.LockedInWorkplane()
                        && gs.lineSegments == 1 && gs.points == 2 && gs.n == 3)
            {
                c.ptA = gs.point[0];
                c.ptB = gs.point[1];
                c.entityA = gs.entity[0];
                c.type = Constraint::Type::SYMMETRIC_LINE;
            } else {
                Error(_("Bad selection for symmetric constraint. This constraint "
                        "can apply to:\n\n"
                        "    * two points or a line segment "
                            "(symmetric about workplane's coordinate axis)\n"
                        "    * line segment, and two points or a line segment "
                            "(symmetric about line segment)\n"
                        "    * workplane, and two points or a line segment "
                            "(symmetric about workplane)\n"));
                return;
            }
            // We may remove constraints so remember manually
            if(c.entityA == Entity::NO_ENTITY) {
                // Horizontal / vertical symmetry, implicit symmetry plane
                // normal to the workplane
                if(c.workplane == Entity::FREE_IN_3D) {
                    Error(_("A workplane must be active when constraining "
                            "symmetric without an explicit symmetry plane."));
                    return;
                }
                Vector pa = SK.GetEntity(c.ptA)->PointGetNum();
                Vector pb = SK.GetEntity(c.ptB)->PointGetNum();
                Vector dp = pa.Minus(pb);
                Entity *norm = SK.GetEntity(c.workplane)->Normal();;
                Vector u = norm->NormalU(), v = norm->NormalV();
                if(std::fabs(dp.Dot(u)) > std::fabs(dp.Dot(v))) {
                    c.type = Constraint::Type::SYMMETRIC_HORIZ;
                } else {
                    c.type = Constraint::Type::SYMMETRIC_VERT;
                }
                if(gs.lineSegments == 1) {
                    // If this line segment is already constrained horiz or
                    // vert, then auto-remove that redundant constraint.
                    // Handle as one undo group.
                    SS.UndoRemember();
                    Constraint::DeleteAllConstraintsFor(Constraint::Type::HORIZONTAL, (gs.entity[0]),
                        Entity::NO_ENTITY);
                    Constraint::DeleteAllConstraintsFor(Constraint::Type::VERTICAL, (gs.entity[0]),
                        Entity::NO_ENTITY);
                    Constraint::AddConstraint(&c, /*rememberForUndo=*/false);
                    break;
                }
            }
            Constraint::AddConstraint(&c);
            break;

        case Command::VERTICAL:
        case Command::HORIZONTAL: {
            hEntity ha, hb;
            if(c.workplane == Entity::FREE_IN_3D) {
                Error(_("Activate a workplane (with Sketch -> In Workplane) before "
                        "applying a horizontal or vertical constraint."));
                return;
            }
            if(gs.lineSegments == 1 && gs.n == 1) {
                c.entityA = gs.entity[0];
                Entity *e = SK.GetEntity(c.entityA);
                ha = e->point[0];
                hb = e->point[1];
            } else if(gs.points == 2 && gs.n == 2) {
                ha = c.ptA = gs.point[0];
                hb = c.ptB = gs.point[1];
            } else {
                Error(_("Bad selection for horizontal / vertical constraint. "
                        "This constraint can apply to:\n\n"
                        "    * two points\n"
                        "    * a line segment\n"));
                return;
            }
            if(id == Command::HORIZONTAL) {
                c.type = Constraint::Type::HORIZONTAL;
            } else {
                c.type = Constraint::Type::VERTICAL;
            }
            Constraint::AddConstraint(&c);
            break;
        }

        case Command::ORIENTED_SAME: {
            if(gs.anyNormals == 2 && gs.n == 2) {
                c.type = Constraint::Type::SAME_ORIENTATION;
                c.entityA = gs.anyNormal[0];
                c.entityB = gs.anyNormal[1];
            } else {
                Error(_("Bad selection for same orientation constraint. This "
                        "constraint can apply to:\n\n"
                        "    * two normals\n"));
                return;
            }
            SS.UndoRemember();

            Entity *nfree = SK.GetEntity(c.entityA);
            Entity *nref  = SK.GetEntity(c.entityB);
            if(nref->group == SS.GW.activeGroup) {
                std::swap(nref, nfree);
            }
            if(nfree->group == SS.GW.activeGroup && nref->group != SS.GW.activeGroup) {
                // nfree is free, and nref is locked (since it came from a
                // previous group); so let's force nfree aligned to nref,
                // and make convergence easy
                Vector ru = nref ->NormalU(), rv = nref ->NormalV();
                Vector fu = nfree->NormalU(), fv = nfree->NormalV();

                if(std::fabs(fu.Dot(ru)) < std::fabs(fu.Dot(rv))) {
                    // There might be an odd*90 degree rotation about the
                    // normal vector; allow that, since the numerical
                    // constraint does
                    std::swap(ru, rv);
                }
                fu = fu.Dot(ru) > 0 ? ru : ru.ScaledBy(-1);
                fv = fv.Dot(rv) > 0 ? rv : rv.ScaledBy(-1);

                nfree->NormalForceTo(Quaternion::From(fu, fv));
            }
            Constraint::AddConstraint(&c, /*rememberForUndo=*/false);
            break;
        }

        case Command::OTHER_ANGLE:
            if(gs.constraints == 1 && gs.n == 0) {
                Constraint *c = SK.GetConstraint(gs.constraint[0]);
                if(c->type == Constraint::Type::ANGLE) {
                    SS.UndoRemember();
                    c->other = !(c->other);
                    c->ModifyToSatisfy();
                    break;
                }
                if(c->type == Constraint::Type::EQUAL_ANGLE) {
                    SS.UndoRemember();
                    c->other = !(c->other);
                    SS.MarkGroupDirty(c->group);
                    break;
                }
            }
            Error(_("Must select an angle constraint."));
            return;

        case Command::REFERENCE:
            if(gs.constraints == 1 && gs.n == 0) {
                Constraint *c = SK.GetConstraint(gs.constraint[0]);
                if(c->HasLabel() && c->type != Constraint::Type::COMMENT) {
                    SS.UndoRemember();
                    (c->reference) = !(c->reference);
                    SS.MarkGroupDirty(c->group, /*onlyThis=*/true);
                    break;
                }
            }
            Error(_("Must select a constraint with associated label."));
            return;

        case Command::ANGLE:
        case Command::REF_ANGLE: {
            if(gs.vectors == 2 && gs.n == 2) {
                c.type = Constraint::Type::ANGLE;
                c.entityA = gs.vector[0];
                c.entityB = gs.vector[1];
                c.valA = 0;
            } else {
                Error(_("Bad selection for angle constraint. This constraint "
                        "can apply to:\n\n"
                        "    * two line segments\n"
                        "    * a line segment and a normal\n"
                        "    * two normals\n"));
                return;
            }

            Entity *ea = SK.GetEntity(c.entityA),
                   *eb = SK.GetEntity(c.entityB);
            if(ea->type == Entity::Type::LINE_SEGMENT &&
               eb->type == Entity::Type::LINE_SEGMENT)
            {
                Vector a0 = SK.GetEntity(ea->point[0])->PointGetNum(),
                       a1 = SK.GetEntity(ea->point[1])->PointGetNum(),
                       b0 = SK.GetEntity(eb->point[0])->PointGetNum(),
                       b1 = SK.GetEntity(eb->point[1])->PointGetNum();
                if(a0.Equals(b0) || a1.Equals(b1)) {
                    // okay, vectors should be drawn in same sense
                } else if(a0.Equals(b1) || a1.Equals(b0)) {
                    // vectors are in opposite sense
                    c.other = true;
                } else {
                    // no shared point; not clear which intersection to draw
                }
            }

            if(id == Command::REF_ANGLE) {
                c.reference = true;
            }

            c.ModifyToSatisfy();
            Constraint::AddConstraint(&c);
            break;
        }

        case Command::PARALLEL:
            if(gs.faces == 2 && gs.n == 2) {
                c.type = Constraint::Type::PARALLEL;
                c.entityA = gs.face[0];
                c.entityB = gs.face[1];
            } else if(gs.vectors == 2 && gs.n == 2) {
                c.type = Constraint::Type::PARALLEL;
                c.entityA = gs.vector[0];
                c.entityB = gs.vector[1];
            } else if(gs.lineSegments == 1 && gs.arcs == 1 && gs.n == 2) {
                Entity *line = SK.GetEntity(gs.entity[0]),
                       *arc  = SK.GetEntity(gs.entity[1]);
                if(line->type == Entity::Type::ARC_OF_CIRCLE) {
                    std::swap(line, arc);
                }
                if(!Constraint::ConstrainArcLineTangent(&c, line, arc)) {
                    return;
                }
                c.type = Constraint::Type::ARC_LINE_TANGENT;
                c.entityA = arc->h;
                c.entityB = line->h;
            } else if(gs.lineSegments == 1 && gs.cubics == 1 && gs.n == 2) {
                Entity *line  = SK.GetEntity(gs.entity[0]),
                       *cubic = SK.GetEntity(gs.entity[1]);
                if(line->type == Entity::Type::CUBIC) {
                    std::swap(line, cubic);
                }
                if(!Constraint::ConstrainCubicLineTangent(&c, line, cubic)) {
                    return;
                }
                c.type = Constraint::Type::CUBIC_LINE_TANGENT;
                c.entityA = cubic->h;
                c.entityB = line->h;
            } else if(gs.cubics + gs.arcs == 2 && gs.n == 2) {
                if(!SS.GW.LockedInWorkplane()) {
                    Error(_("Curve-curve tangency must apply in workplane."));
                    return;
                }
                Entity *eA = SK.GetEntity(gs.entity[0]),
                       *eB = SK.GetEntity(gs.entity[1]);
                if(!Constraint::ConstrainCurveCurveTangent(&c, eA, eB)) {
                    return;
                }
                c.type = Constraint::Type::CURVE_CURVE_TANGENT;
                c.entityA = eA->h;
                c.entityB = eB->h;
            } else {
                Error(_("Bad selection for parallel / tangent constraint. This "
                        "constraint can apply to:\n\n"
                        "    * two faces\n"
                        "    * two line segments (parallel)\n"
                        "    * a line segment and a normal (parallel)\n"
                        "    * two normals (parallel)\n"
                        "    * two line segments, arcs, or beziers, that share "
                              "an endpoint (tangent)\n"));
                return;
            }
            Constraint::AddConstraint(&c);
            break;

        case Command::PERPENDICULAR:
            if(gs.faces == 2 && gs.n == 2) {
                c.type = Constraint::Type::PERPENDICULAR;
                c.entityA = gs.face[0];
                c.entityB = gs.face[1];
            } else if(gs.vectors == 2 && gs.n == 2) {
                c.type = Constraint::Type::PERPENDICULAR;
                c.entityA = gs.vector[0];
                c.entityB = gs.vector[1];
            } else {
                Error(_("Bad selection for perpendicular constraint. This "
                        "constraint can apply to:\n\n"
                        "    * two faces\n"
                        "    * two line segments\n"
                        "    * a line segment and a normal\n"
                        "    * two normals\n"));
                return;
            }
            Constraint::AddConstraint(&c);
            break;

        case Command::WHERE_DRAGGED:
            if(gs.points == 1 && gs.n == 1) {
                c.type = Constraint::Type::WHERE_DRAGGED;
                c.ptA = gs.point[0];
            } else {
                Error(_("Bad selection for lock point where dragged constraint. "
                        "This constraint can apply to:\n\n"
                        "    * a point\n"));
                return;
            }
            Constraint::AddConstraint(&c);
            break;

        case Command::COMMENT:
            if(gs.points == 1 && gs.n == 1) {
                c.type = Constraint::Type::COMMENT;
                c.ptA = gs.point[0];
                c.group       = SS.GW.activeGroup;
                c.workplane   = SS.GW.ActiveWorkplane();
                c.comment     = _("NEW COMMENT -- DOUBLE-CLICK TO EDIT");
                Constraint::AddConstraint(&c);
            } else {
                SS.GW.pending.operation = GraphicsWindow::Pending::COMMAND;
                SS.GW.pending.command = Command::COMMENT;
                SS.GW.pending.description = _("click center of comment text");
                SS.ScheduleShowTW();
            }
            break;

        default: {
				  std::cout << "unexpected menu id: " << uint32_t(id) << std::endl;
				  ssassert(false, "Unexpected menu ID");
				}
    }

    for(const Constraint &cc : SK.constraint) {
        if(c.h != cc.h && c.Equals(cc)) {
            // Oops, we already have this exact constraint. Remove the one we just added.
            SK.constraint.RemoveById(c.h);
            SS.GW.ClearSelection();
            // And now select the old one, to give feedback.
            SS.GW.MakeSelected(cc.h);
            return;
        }
    }

    if(SK.constraint.FindByIdNoOops(c.h)) {
        Constraint *constraint = SK.GetConstraint(c.h);
        if(SS.TestRankForGroup(c.group) == SolveResult::REDUNDANT_OKAY &&
                !SK.GetGroup(SS.GW.activeGroup)->allowRedundant &&
                constraint->HasLabel()) {
            constraint->reference = true;
        }
    }

    if((id == Command::DISTANCE_DIA || id == Command::ANGLE ||
        id == Command::RATIO || id == Command::DIFFERENCE) &&
       SS.immediatelyEditDimension) {
        SS.GW.EditConstraint(c.h);
    }

    SS.GW.ClearSelection();
}

void SolveSpaceUI::MenuGroup(Command id)  {
    MenuGroup(id, Platform::Path());
}

void SolveSpaceUI::MenuGroup(Command id, Platform::Path linkFile) {
    Platform::SettingsRef settings = Platform::GetSettings();

    Group g = {};
    g.visible = true;
    g.color = RGBi(100, 100, 100);
    g.scale = 1;
    g.linkFile = linkFile;

    SS.GW.GroupSelection();
    auto const &gs = SS.GW.gs;

    switch(id) {
        case Command::GROUP_3D:
            g.type = Group::Type::DRAWING_3D;
            g.name = C_("group-name", "sketch-in-3d");
            break;

        case Command::GROUP_WRKPL:
            g.type = Group::Type::DRAWING_WORKPLANE;
            g.name = C_("group-name", "sketch-in-plane");
            if(gs.points == 1 && gs.n == 1) {
                g.subtype = Group::Subtype::WORKPLANE_BY_POINT_ORTHO;

                Vector u = SS.GW.projRight, v = SS.GW.projUp;
                u = u.ClosestOrtho();
                v = v.Minus(u.ScaledBy(v.Dot(u)));
                v = v.ClosestOrtho();

                g.predef.q = Quaternion::From(u, v);
                g.predef.origin = gs.point[0];
            } else if(gs.points == 1 && gs.lineSegments == 2 && gs.n == 3) {
                g.subtype = Group::Subtype::WORKPLANE_BY_LINE_SEGMENTS;

                g.predef.origin = gs.point[0];
                g.predef.entityB = gs.entity[0];
                g.predef.entityC = gs.entity[1];

                Vector ut = SK.GetEntity(g.predef.entityB)->VectorGetNum();
                Vector vt = SK.GetEntity(g.predef.entityC)->VectorGetNum();
                ut = ut.WithMagnitude(1);
                vt = vt.WithMagnitude(1);

                if(std::fabs(SS.GW.projUp.Dot(vt)) < std::fabs(SS.GW.projUp.Dot(ut))) {
                    std::swap(ut, vt);
                    g.predef.swapUV = true;
                }
                if(SS.GW.projRight.Dot(ut) < 0) g.predef.negateU = true;
                if(SS.GW.projUp.   Dot(vt) < 0) g.predef.negateV = true;
            } else if(gs.workplanes == 1 && gs.n == 1) {
                if(gs.entity[0].isFromRequest()) {
                    Entity *wrkpl = SK.GetEntity(gs.entity[0]);
                    Entity *normal = SK.GetEntity(wrkpl->normal);
                    g.subtype = Group::Subtype::WORKPLANE_BY_POINT_ORTHO;
                    g.predef.origin = wrkpl->point[0];
                    g.predef.q = normal->NormalGetNum();
                } else {
                    Group *wrkplg = SK.GetGroup(gs.entity[0].group());
                    g.subtype = wrkplg->subtype;
                    g.predef.origin = wrkplg->predef.origin;
                    if(wrkplg->subtype == Group::Subtype::WORKPLANE_BY_LINE_SEGMENTS) {
                        g.predef.entityB = wrkplg->predef.entityB;
                        g.predef.entityC = wrkplg->predef.entityC;
                        g.predef.swapUV = wrkplg->predef.swapUV;
                        g.predef.negateU = wrkplg->predef.negateU;
                        g.predef.negateV = wrkplg->predef.negateV;
                    } else if(wrkplg->subtype == Group::Subtype::WORKPLANE_BY_POINT_ORTHO) {
                        g.predef.q = wrkplg->predef.q;
                    } else if(wrkplg->subtype == Group::Subtype::WORKPLANE_BY_POINT_NORMAL) {
                        g.predef.q = wrkplg->predef.q;
                        g.predef.entityB = wrkplg->predef.entityB;
                    } else ssassert(false, "Unexpected workplane subtype");
                }
            } else if(gs.anyNormals == 1 && gs.points == 1 && gs.n == 2) {
                g.subtype       = Group::Subtype::WORKPLANE_BY_POINT_NORMAL;
                g.predef.entityB = gs.anyNormal[0];
                g.predef.q      = SK.GetEntity(gs.anyNormal[0])->NormalGetNum();
                g.predef.origin = gs.point[0];
            //} else if(gs.faces == 1 && gs.points == 1 && gs.n == 2) {
            //    g.subtype = Subtype::WORKPLANE_BY_POINT_FACE;
            //    g.predef.q      = SK.GetEntity(gs.face[0])->NormalGetNum();
            //    g.predef.origin = gs.point[0];
            } else {
                Error(_("Bad selection for new sketch in workplane. This "
                        "group can be created with:\n\n"
                        "    * a point (through the point, orthogonal to coordinate axes)\n"
                        "    * a point and two line segments (through the point, "
                        "parallel to the lines)\n"
                        "    * a point and a normal (through the point, "
                        "orthogonal to the normal)\n"
                        /*"    * a point and a face (through the point, "
                        "parallel to the face)\n"*/
                        "    * a workplane (copy of the workplane)\n"));
                return;
            }
            break;

        case Command::GROUP_EXTRUDE:
            if(!SS.GW.LockedInWorkplane()) {
                Error(_("Activate a workplane (Sketch -> In Workplane) before "
                        "extruding. The sketch will be extruded normal to the "
                        "workplane."));
                return;
            }
            g.type = Group::Type::EXTRUDE;
            g.opA = SS.GW.activeGroup;
            g.predef.entityB = SS.GW.ActiveWorkplane();
            g.subtype = Group::Subtype::ONE_SIDED;
            g.name = C_("group-name", "extrude");
            break;

        case Command::GROUP_LATHE:
            if(!SS.GW.LockedInWorkplane()) {
                Error(_("Lathe operation can only be applied to planar sketches."));
                return;
            }
            if(gs.points == 1 && gs.vectors == 1 && gs.n == 2) {
                g.predef.origin = gs.point[0];
                g.predef.entityB = gs.vector[0];
            } else if(gs.lineSegments == 1 && gs.n == 1) {
                g.predef.origin = SK.GetEntity(gs.entity[0])->point[0];
                g.predef.entityB = gs.entity[0];
                // since a line segment is a vector
            } else {
                Error(_("Bad selection for new lathe group. This group can "
                        "be created with:\n\n"
                        "    * a point and a line segment or normal "
                                 "(revolved about an axis parallel to line / "
                                 "normal, through point)\n"
                        "    * a line segment (revolved about line segment)\n"));
                return;
            }
            g.type = Group::Type::LATHE;
            g.opA = SS.GW.activeGroup;
            g.name = C_("group-name", "lathe");
            break;

        case Command::GROUP_REVOLVE:
            if(!SS.GW.LockedInWorkplane()) {
                Error(_("Revolve operation can only be applied to planar sketches."));
                return;
            }
            if(gs.points == 1 && gs.vectors == 1 && gs.n == 2) {
                g.predef.origin  = gs.point[0];
                g.predef.entityB = gs.vector[0];
            } else if(gs.lineSegments == 1 && gs.n == 1) {
                g.predef.origin  = SK.GetEntity(gs.entity[0])->point[0];
                g.predef.entityB = gs.entity[0];
                // since a line segment is a vector
            } else {
                Error(_("Bad selection for new revolve group. This group can "
                        "be created with:\n\n"
                        "    * a point and a line segment or normal "
                                 "(revolved about an axis parallel to line / "
                                 "normal, through point)\n"
                        "    * a line segment (revolved about line segment)\n"));
                return;
            }
            g.type    = Group::Type::REVOLVE;
            g.opA     = SS.GW.activeGroup;
            g.valA    = 2;
            g.subtype = Group::Subtype::ONE_SIDED;
            g.name    = C_("group-name", "revolve");
            break;

        case Command::GROUP_HELIX:
            if(!SS.GW.LockedInWorkplane()) {
                Error(_("Helix operation can only be applied to planar sketches."));
                return;
            }
            if(gs.points == 1 && gs.vectors == 1 && gs.n == 2) {
                g.predef.origin  = gs.point[0];
                g.predef.entityB = gs.vector[0];
            } else if(gs.lineSegments == 1 && gs.n == 1) {
                g.predef.origin  = SK.GetEntity(gs.entity[0])->point[0];
                g.predef.entityB = gs.entity[0];
                // since a line segment is a vector
            } else {
                Error(_("Bad selection for new helix group. This group can "
                        "be created with:\n\n"
                        "    * a point and a line segment or normal "
                                 "(revolved about an axis parallel to line / "
                                 "normal, through point)\n"
                        "    * a line segment (revolved about line segment)\n"));
                return;
            }
            g.type    = Group::Type::HELIX;
            g.opA     = SS.GW.activeGroup;
            g.valA    = 2;
            g.subtype = Group::Subtype::ONE_SIDED;
            g.name    = C_("group-name", "helix");
            break;

        case Command::GROUP_ROT: {
            if(gs.points == 1 && gs.n == 1 && SS.GW.LockedInWorkplane()) {
                g.predef.origin = gs.point[0];
                Entity *w = SK.GetEntity(SS.GW.ActiveWorkplane());
                g.predef.entityB = w->Normal()->h;
                g.activeWorkplane = w->h;
            } else if(gs.points == 1 && gs.vectors == 1 && gs.n == 2) {
                g.predef.origin = gs.point[0];
                g.predef.entityB = gs.vector[0];
            } else {
                Error(_("Bad selection for new rotation. This group can "
                        "be created with:\n\n"
                        "    * a point, while locked in workplane (rotate "
                              "in plane, about that point)\n"
                        "    * a point and a line or a normal (rotate about "
                              "an axis through the point, and parallel to "
                              "line / normal)\n"));
                return;
            }
            g.type = Group::Type::ROTATE;
            g.opA = SS.GW.activeGroup;
            g.valA = 3;
            g.subtype = Group::Subtype::ONE_SIDED;
            g.name = C_("group-name", "rotate");
            break;
        }

        case Command::GROUP_TRANS:
            g.type = Group::Type::TRANSLATE;
            g.opA = SS.GW.activeGroup;
            g.valA = 3;
            g.subtype = Group::Subtype::ONE_SIDED;
            g.predef.entityB = SS.GW.ActiveWorkplane();
            g.activeWorkplane = SS.GW.ActiveWorkplane();
            g.name = C_("group-name", "translate");
            break;

        case Command::GROUP_LINK: {
            g.type = Group::Type::LINKED;
            g.meshCombine = Group::CombineAs::ASSEMBLE;
            if(g.linkFile.IsEmpty()) {
                Platform::FileDialogRef dialog = Platform::CreateOpenFileDialog(SS.GW.window);
                dialog->AddFilters(Platform::SolveSpaceLinkFileFilters);
                dialog->ThawChoices(settings, "LinkSketch");
                if(!dialog->RunModal()) return;
                dialog->FreezeChoices(settings, "LinkSketch");
                g.linkFile = dialog->GetFilename();
            }

            // Assign the default name of the group based on the name of
            // the linked file.
            g.name = g.linkFile.FileStem();
            for(size_t i = 0; i < g.name.length(); i++) {
                if(!(isalnum(g.name[i]) || (unsigned)g.name[i] >= 0x80)) {
                    // convert punctuation to dashes
                    g.name[i] = '-';
                }
            }
            break;
        }

        default: ssassert(false, "Unexpected menu ID");
    }

    // Copy color from the previous mesh-contributing group.
    if(g.IsMeshGroup() && !SK.groupOrder.IsEmpty()) {
        Group *running = SK.GetRunningMeshGroupFor(SS.GW.activeGroup);
        if(running != NULL) {
            g.color = running->color;
        }
    }

    SS.GW.ClearSelection();
    SS.UndoRemember();

    bool afterActive = false;
    for(hGroup hg : SK.groupOrder) {
        Group *gi = SK.GetGroup(hg);
        if(afterActive)
            gi->order += 1;
        if(gi->h == SS.GW.activeGroup) {
            g.order = gi->order + 1;
            afterActive = true;
        }
    }

    SK.group.AddAndAssignId(&g);
    Group *gg = SK.GetGroup(g.h);

    if(gg->type == Group::Type::LINKED) {
        SS.ReloadAllLinked(SS.saveFile);
    }
    gg->clean = false;
    SS.GW.activeGroup = gg->h;
    SS.GenerateAll();
    if(gg->type == Group::Type::DRAWING_WORKPLANE) {
        // Can't set the active workplane for this one until after we've
        // regenerated, because the workplane doesn't exist until then.
        gg->activeWorkplane = gg->h.entity(0);
    }
    gg->Activate();
    TextWindow::ScreenSelectGroup(0, gg->h.v);
    SS.GW.AnimateOntoWorkplane();
}

void SolveSpaceUI::MenuHelp(Command id) {
    switch(id) {
        case Command::WEBSITE:
            Platform::OpenInBrowser("http://solvespace.com/helpmenu");
            break;

        case Command::ABOUT:
            Message(_(
"This is SolveSpace version %s.\n"
"\n"
"For more information, see http://solvespace.com/\n"
"\n"
"SolveSpace is free software: you are free to modify\n"
"and/or redistribute it under the terms of the GNU\n"
"General Public License (GPL) version 3 or later.\n"
"\n"
"There is NO WARRANTY, to the extent permitted by\n"
"law. For details, visit http://gnu.org/licenses/\n"
"\n"
"© 2008-%d Jonathan Westhues and other authors.\n"),
PACKAGE_VERSION, 2022);
            break;

        case Command::GITHUB:
            Platform::OpenInBrowser(GIT_HASH_URL);
            break;

        default: ssassert(false, "Unexpected menu ID");
    }
}
