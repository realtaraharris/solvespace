//-----------------------------------------------------------------------------
// Top-level implementation of the program's main window, in which a graphical
// representation of the model is drawn and edited by the user.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"
#include "ssg.h"
#include "platform/EventHooks.h"

void GraphicsWindow::Init(double width, double height, int devicePixelRatio, bool headless) {
  this->width = width;
  this->height = height;
  this->devicePixelRatio = devicePixelRatio;
  this->headless = headless;
  scale = 5;
  offset = Vector::From(0, 0, 0);
  projRight = Vector::From(1, 0, 0);
  projUp = Vector::From(0, 1, 0);

  // Make sure those are valid; could get a mouse move without a mouse
  // down if someone depresses the button, then drags into our window.
  orig.projRight = projRight;
  orig.projUp = projUp;

  // And with the last group active
  ssassert(!SK.groupOrder.IsEmpty(),
           "Group order can't be empty since we will activate the last group.");
  activeGroup = *SK.groupOrder.Last();
  SK.GetGroup(activeGroup)->Activate();

  showWorkplanes = false;
  showNormals = true;
  showPoints = true;
  showConstruction = true;
  showConstraints = true;
  showShaded = true;
  showEdges = true;
  showMesh = false;
  showOutlines = false;
  showFacesDrawing = false;
  showFacesNonDrawing = true;
  drawOccludedAs = DrawOccludedAs::INVISIBLE;

  showTextWindow = true;

  showSnapGrid = false;
  dimSolidModel = true;
  context.active = false;

  canvas = CreateRenderer();
  if (canvas) {
    persistentCanvas = canvas->CreateBatch();
    persistentDirty = true;
  }
}

void GraphicsWindow::AnimateOntoWorkplane() {
  if (!LockedInWorkplane())
    return;

  Entity *w = SK.GetEntity(ActiveWorkplane());
  Quaternion quatf = w->Normal()->NormalGetNum();

  // Get Z pointing vertical, if we're on turntable nav mode:
  if (SS.turntableNav) {
    Vector normalRight = quatf.RotationU();
    Vector normalUp = quatf.RotationV();
    Vector normal = normalRight.Cross(normalUp);
    if (normalRight.z != 0) {
      double theta = atan2(normalUp.z, normalRight.z);
      theta -= atan2(1, 0);
      normalRight = normalRight.RotatedAbout(normal, theta);
      normalUp = normalUp.RotatedAbout(normal, theta);
      quatf = Quaternion::From(normalRight, normalUp);
    }
  }

  Vector offsetf = (SK.GetEntity(w->point[0])->PointGetNum()).ScaledBy(-1);

  // If the view screen is open, then we need to refresh it.
  SS.ScheduleShowTW();

  AnimateOnto(quatf, offsetf);
}

void GraphicsWindow::AnimateOnto(Quaternion quatf, Vector offsetf) {
  // TODO: add animation once the Haiku timer code is added; just skip the animation for now
  projRight = quatf.RotationU();
  projUp = quatf.RotationV();
  offset = offsetf;
  //    window->Invalidate();
  return;

  // Get our initial orientation and translation.
  Quaternion quat0 = Quaternion::From(projRight, projUp);
  Vector offset0 = offset;

  // Make sure we take the shorter of the two possible paths.
  double mp = (quatf.Minus(quat0)).Magnitude();
  double mm = (quatf.Plus(quat0)).Magnitude();
  if (mp > mm) {
    quatf = quatf.ScaledBy(-1);
    mp = mm;
  }
  double mo = (offset0.Minus(offsetf)).Magnitude() * scale;

  // Animate transition, unless it's a tiny move.
  int64_t t0 = GetMilliseconds();
  int32_t dt = (mp < 0.01 && mo < 10) ? (-20) : (int32_t)(100 + 1000 * mp + 0.4 * mo);
  // Don't ever animate for longer than 2000 ms; we can get absurdly
  // long translations (as measured in pixels) if the user zooms out, moves,
  // and then zooms in again.
  if (dt > 2000)
    dt = 2000;
  Quaternion dq = quatf.Times(quat0.Inverse());

  if (!animateTimer) {
    animateTimer = Platform::CreateTimer();
  }
  animateTimer->onTimeout = [=] {
    int64_t tn = GetMilliseconds();
    if ((tn - t0) < dt) {
      animateTimer->RunAfterNextFrame();

      double s = (tn - t0) / ((double)dt);
      offset = (offset0.ScaledBy(1 - s)).Plus(offsetf.ScaledBy(s));
      Quaternion quat = (dq.ToThe(s)).Times(quat0).WithMagnitude(1);

      projRight = quat.RotationU();
      projUp = quat.RotationV();
    } else {
      projRight = quatf.RotationU();
      projUp = quatf.RotationV();
      offset = offsetf;
    }
//    window->Invalidate();
  };
  animateTimer->RunAfterNextFrame();
}

void GraphicsWindow::HandlePointForZoomToFit(Vector p, Point2d *pmax, Point2d *pmin, double *wmin,
                                             bool usePerspective, const Camera &camera) {
  double w;
  Vector pp = camera.ProjectPoint4(p, &w);
  // If usePerspective is true, then we calculate a perspective projection of the point.
  // If not, then we do a parallel projection regardless of the current
  // scale factor.
  if (usePerspective) {
    pp = pp.ScaledBy(1.0 / w);
  }

  pmax->x = std::max(pmax->x, pp.x);
  pmax->y = std::max(pmax->y, pp.y);
  pmin->x = std::min(pmin->x, pp.x);
  pmin->y = std::min(pmin->y, pp.y);
  *wmin = std::min(*wmin, w);
}
void GraphicsWindow::LoopOverPoints(const std::vector<Entity *> &entities,
                                    const std::vector<Constraint *> &constraints,
                                    const std::vector<hEntity> &faces, Point2d *pmax, Point2d *pmin,
                                    double *wmin, bool usePerspective, bool includeMesh,
                                    const Camera &camera) {

  for (Entity *e : entities) {
    if (e->IsPoint()) {
      HandlePointForZoomToFit(e->PointGetNum(), pmax, pmin, wmin, usePerspective, camera);
    } else if (e->type == Entity::Type::CIRCLE) {
      // Lots of entities can extend outside the bbox of their points,
      // but circles are particularly bad. We want to get things halfway
      // reasonable without the mesh, because a zoom to fit is used to
      // set the zoom level to set the chord tol.
      double r = e->CircleGetRadiusNum();
      Vector c = SK.GetEntity(e->point[0])->PointGetNum();
      Quaternion q = SK.GetEntity(e->normal)->NormalGetNum();
      for (int j = 0; j < 4; j++) {
        Vector p = (j == 0)   ? (c.Plus(q.RotationU().ScaledBy(r)))
                   : (j == 1) ? (c.Plus(q.RotationU().ScaledBy(-r)))
                   : (j == 2) ? (c.Plus(q.RotationV().ScaledBy(r)))
                              : (c.Plus(q.RotationV().ScaledBy(-r)));
        HandlePointForZoomToFit(p, pmax, pmin, wmin, usePerspective, camera);
      }
    } else {
      // We have to iterate children points, because we can select entities without points
      for (int i = 0; i < MAX_POINTS_IN_ENTITY; i++) {
        if (e->point[i].v == 0)
          break;
        Vector p = SK.GetEntity(e->point[i])->PointGetNum();
        HandlePointForZoomToFit(p, pmax, pmin, wmin, usePerspective, camera);
      }
    }
  }

  for (Constraint *c : constraints) {
    std::vector<Vector> refs;
    c->GetReferencePoints(camera, &refs);
    for (Vector p : refs) {
      HandlePointForZoomToFit(p, pmax, pmin, wmin, usePerspective, camera);
    }
  }

  if (!includeMesh && faces.empty())
    return;

  Group *g = SK.GetGroup(activeGroup);
  g->GenerateDisplayItems();
  for (int i = 0; i < g->displayMesh.l.n; i++) {
    STriangle *tr = &(g->displayMesh.l[i]);
    if (!includeMesh) {
      bool found = false;
      for (const hEntity &face : faces) {
        if (face.v != tr->meta.face)
          continue;
        found = true;
        break;
      }
      if (!found)
        continue;
    }
    HandlePointForZoomToFit(tr->a, pmax, pmin, wmin, usePerspective, camera);
    HandlePointForZoomToFit(tr->b, pmax, pmin, wmin, usePerspective, camera);
    HandlePointForZoomToFit(tr->c, pmax, pmin, wmin, usePerspective, camera);
  }
  if (!includeMesh)
    return;
  for (int i = 0; i < g->polyLoops.l.n; i++) {
    SContour *sc = &(g->polyLoops.l[i]);
    for (int j = 0; j < sc->l.n; j++) {
      HandlePointForZoomToFit(sc->l[j].p, pmax, pmin, wmin, usePerspective, camera);
    }
  }
}

void GraphicsWindow::ZoomToFit(bool includingInvisibles, bool useSelection) {
  scale = ZoomToFit(GetCamera(), includingInvisibles, useSelection);
}

double GraphicsWindow::ZoomToFit(const Camera &camera, bool includingInvisibles,
                                 bool useSelection) {
  std::vector<Entity *> entities;
  std::vector<Constraint *> constraints;
  std::vector<hEntity> faces;

  if (useSelection) {
    for (int i = 0; i < selection.n; i++) {
      Selection *s = &selection[i];
      if (s->entity.v != 0) {
        Entity *e = SK.entity.FindById(s->entity);
        if (e->IsFace()) {
          faces.push_back(e->h);
          continue;
        }
        entities.push_back(e);
      }
      if (s->constraint.v != 0) {
        Constraint *c = SK.constraint.FindById(s->constraint);
        constraints.push_back(c);
      }
    }
  }

  bool selectionUsed = !entities.empty() || !constraints.empty() || !faces.empty();

  if (!selectionUsed) {
    for (Entity &e : SK.entity) {
      // we don't want to handle separate points, because we will iterate them inside entities.
      if (e.IsPoint())
        continue;
      if (!includingInvisibles && !e.IsVisible())
        continue;
      entities.push_back(&e);
    }

    for (Constraint &c : SK.constraint) {
      if (!c.IsVisible())
        continue;
      constraints.push_back(&c);
    }
  }

  // On the first run, ignore perspective.
  Point2d pmax = {-1e12, -1e12}, pmin = {1e12, 1e12};
  double wmin = 1;
  LoopOverPoints(entities, constraints, faces, &pmax, &pmin, &wmin,
                 /*usePerspective=*/false, /*includeMesh=*/!selectionUsed, camera);

  double xm = (pmax.x + pmin.x) / 2, ym = (pmax.y + pmin.y) / 2;
  double dx = pmax.x - pmin.x, dy = pmax.y - pmin.y;

  offset = offset.Plus(projRight.ScaledBy(-xm)).Plus(projUp.ScaledBy(-ym));

  // And based on this, we calculate the scale and offset
  double scale;
  if (EXACT(dx == 0 && dy == 0)) {
    scale = 5;
  } else {
    double scalex = 1e12, scaley = 1e12;
    if (EXACT(dx != 0))
      scalex = 0.9 * camera.width / dx;
    if (EXACT(dy != 0))
      scaley = 0.9 * camera.height / dy;
    scale = std::min(scalex, scaley);

    scale = std::min(300.0, scale);
    scale = std::max(0.003, scale);
  }

  // Then do another run, considering the perspective.
  pmax.x = -1e12;
  pmax.y = -1e12;
  pmin.x = 1e12;
  pmin.y = 1e12;
  wmin = 1;
  LoopOverPoints(entities, constraints, faces, &pmax, &pmin, &wmin,
                 /*usePerspective=*/true, /*includeMesh=*/!selectionUsed, camera);

  // Adjust the scale so that no points are behind the camera
  if (wmin < 0.1) {
    double k = camera.tangent;
    // w = 1+k*scale*z
    double zmin = (wmin - 1) / (k * scale);
    // 0.1 = 1 + k*scale*zmin
    // (0.1 - 1)/(k*zmin) = scale
    scale = std::min(scale, (0.1 - 1) / (k * zmin));
  }

  return scale;
}

void GraphicsWindow::ZoomToMouse(double zoomMultiplyer) {
  double offsetRight = offset.Dot(projRight);
  double offsetUp = offset.Dot(projUp);

  double righti = currentMousePosition.x / scale - offsetRight;
  double upi = currentMousePosition.y / scale - offsetUp;

  // zoomMultiplyer of 1 gives a default zoom factor of 1.2x: zoomMultiplyer * 1.2
  // zoom = adjusted zoom negative zoomMultiplyer will zoom out, positive will zoom in
  //

  scale *= exp(0.1823216 * zoomMultiplyer); // ln(1.2) = 0.1823216

  double rightf = currentMousePosition.x / scale - offsetRight;
  double upf = currentMousePosition.y / scale - offsetUp;

  offset = offset.Plus(projRight.ScaledBy(rightf - righti));
  offset = offset.Plus(projUp.ScaledBy(upf - upi));

  if (SS.TW.shown.screen == TextWindow::Screen::EDIT_VIEW) {
    if (havePainted) {
      SS.ScheduleShowTW();
    }
  }
  havePainted = false;
  Invalidate();
}

void GraphicsWindow::EnsureValidActives() {
  bool change = false;
  // The active group must exist, and not be the references.
  Group *g = SK.group.FindByIdNoOops(activeGroup);
  if ((!g) || (g->h == Group::HGROUP_REFERENCES)) {
    // Not using range-for because this is used to find an index.
    int i;
    for (i = 0; i < SK.groupOrder.n; i++) {
      if (SK.groupOrder[i] != Group::HGROUP_REFERENCES) {
        break;
      }
    }
    if (i >= SK.groupOrder.n) {
      // This can happen if the user deletes all the groups in the
      // sketch. It's difficult to prevent that, because the last
      // group might have been deleted automatically, because it failed
      // a dependency. There needs to be something, so create a plane
      // drawing group and activate that. They should never be able
      // to delete the references, though.
      activeGroup = SS.CreateDefaultDrawingGroup();
      // We've created the default group, but not the workplane entity;
      // do it now so that drawing mode isn't switched to "Free in 3d".
      SS.GenerateAll(SolveSpaceUI::Generate::ALL);
    } else {
      activeGroup = SK.groupOrder[i];
    }
    SK.GetGroup(activeGroup)->Activate();
    change = true;
  }

  // The active coordinate system must also exist.
  if (LockedInWorkplane()) {
    Entity *e = SK.entity.FindByIdNoOops(ActiveWorkplane());
    if (e) {
      hGroup hgw = e->group;
      if (hgw != activeGroup && SS.GroupsInOrder(activeGroup, hgw)) {
        // The active workplane is in a group that comes after the
        // active group; so any request or constraint will fail.
        SetWorkplaneFreeIn3d();
        change = true;
      }
    } else {
      SetWorkplaneFreeIn3d();
      change = true;
    }
  }

  // And update the checked state for various menus
  // bool locked = LockedInWorkplane();
  //    in3dMenuItem->SetActive(!locked);
  //    inWorkplaneMenuItem->SetActive(locked);

  SS.UndoEnableMenus();

  switch (SS.viewUnits) {
  case Unit::MM:
  case Unit::METERS:
  case Unit::INCHES:
  case Unit::FEET_INCHES: break;
  default: SS.viewUnits = Unit::MM; break;
  }

  if (change)
    SS.ScheduleShowTW();
}

void GraphicsWindow::SetWorkplaneFreeIn3d() {
  SK.GetGroup(activeGroup)->activeWorkplane = Entity::FREE_IN_3D;
}
hEntity GraphicsWindow::ActiveWorkplane() {
  Group *g = SK.group.FindByIdNoOops(activeGroup);
  if (g) {
    return g->activeWorkplane;
  } else {
    return Entity::FREE_IN_3D;
  }
}
bool GraphicsWindow::LockedInWorkplane() {
  return (SS.GW.ActiveWorkplane() != Entity::FREE_IN_3D);
}

void GraphicsWindow::ForceTextWindowShown() {}

void GraphicsWindow::DeleteTaggedRequests() {
  // Delete any requests that were affected by this deletion.
  for (Request &r : SK.request) {
    if (r.workplane == Entity::FREE_IN_3D)
      continue;
    if (!r.workplane.isFromRequest())
      continue;
    Request *wrkpl = SK.GetRequest(r.workplane.request());
    if (wrkpl->tag)
      r.tag = 1;
  }
  // Rewrite any point-coincident constraints that were affected by this
  // deletion.
  for (Request &r : SK.request) {
    if (!r.tag)
      continue;
    FixConstraintsForRequestBeingDeleted(r.h);
  }
  // and then delete the tagged requests.
  SK.request.RemoveTagged();

  // An edit might be in progress for the just-deleted item. So
  // now it's not.
  //    window->HideEditor();
  //    SS.TW.HideEditControl();
  // And clear out the selection, which could contain that item.
  ClearSuper(6);
  // And regenerate to get rid of what it generates, plus anything
  // that references it (since the regen code checks for that).
  SS.GenerateAll(SolveSpaceUI::Generate::ALL);
  EnsureValidActives();
  SS.ScheduleShowTW();
}

Vector GraphicsWindow::SnapToGrid(Vector p) {
  if (!LockedInWorkplane())
    return p;

  Entity *wrkpl = SK.GetEntity(ActiveWorkplane()), *norm = wrkpl->Normal();
  Vector wo = SK.GetEntity(wrkpl->point[0])->PointGetNum(), wu = norm->NormalU(),
         wv = norm->NormalV(), wn = norm->NormalN();

  Vector pp = (p.Minus(wo)).DotInToCsys(wu, wv, wn);
  pp.x = floor((pp.x / SS.gridSpacing) + 0.5) * SS.gridSpacing;
  pp.y = floor((pp.y / SS.gridSpacing) + 0.5) * SS.gridSpacing;
  pp.z = 0;

  return pp.ScaleOutOfCsys(wu, wv, wn).Plus(wo);
}

void GraphicsWindow::ClearSuper(int which) {
  ClearPending(0);
  ClearSelection();
  hover.Clear();
  EnsureValidActives();
}

void GraphicsWindow::ToggleBool(bool *v) {
  *v = !*v;

  // The faces are shown as special stippling on the shaded triangle mesh,
  // so not meaningful to show them and hide the shaded.
  if (!showShaded)
    showFaces = false;

  // If the mesh or edges were previously hidden, they haven't been generated,
  // and if we are going to show them, we need to generate them first.
  Group *g = SK.GetGroup(SS.GW.activeGroup);
  if (*v && (g->displayOutlines.l.IsEmpty() && (v == &showEdges || v == &showOutlines))) {
    SS.GenerateAll(SolveSpaceUI::Generate::UNTIL_ACTIVE);
  }

  if (v == &showFaces) {
    if (g->type == Group::Type::DRAWING_WORKPLANE || g->type == Group::Type::DRAWING_3D) {
      showFacesDrawing = showFaces;
    } else {
      showFacesNonDrawing = showFaces;
    }
  }

  Invalidate(/*clearPersistent=*/true);
  SS.ScheduleShowTW();
}

bool GraphicsWindow::SuggestLineConstraint(hRequest request, Constraint::Type *type) {
  if (!(LockedInWorkplane() && SS.automaticLineConstraints))
    return false;

  Entity *ptA = SK.GetEntity(request.entity(1)), *ptB = SK.GetEntity(request.entity(2));

  Expr *au, *av, *bu, *bv;

  ptA->PointGetExprsInWorkplane(ActiveWorkplane(), &au, &av);
  ptB->PointGetExprsInWorkplane(ActiveWorkplane(), &bu, &bv);

  double du = au->Minus(bu)->Eval();
  double dv = av->Minus(bv)->Eval();

  const double TOLERANCE_RATIO = 0.02;
  if (fabs(dv) > LENGTH_EPS && fabs(du / dv) < TOLERANCE_RATIO) {
    *type = Constraint::Type::VERTICAL;
    return true;
  } else if (fabs(du) > LENGTH_EPS && fabs(dv / du) < TOLERANCE_RATIO) {
    *type = Constraint::Type::HORIZONTAL;
    return true;
  }
  return false;
}
