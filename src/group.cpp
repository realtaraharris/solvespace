//-----------------------------------------------------------------------------
// Implementation of the Group class, which represents a set of entities and
// constraints that are solved together, in some cases followed by another
// operation, like to extrude surfaces from the entities or to step and
// repeat them parametrically.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"

const hParam Param::NO_PARAM = {0};
#define NO_PARAM (Param::NO_PARAM)

const hGroup Group::HGROUP_REFERENCES = {1};

//-----------------------------------------------------------------------------
// The group structure includes pointers to other dynamically-allocated
// memory. This clears and frees them all.
//-----------------------------------------------------------------------------
void Group::Clear () {
  polyLoops.Clear ();
  bezierLoops.Clear ();
  bezierOpens.Clear ();
  thisMesh.Clear ();
  runningMesh.Clear ();
  thisShell.Clear ();
  runningShell.Clear ();
  displayMesh.Clear ();
  displayOutlines.Clear ();
  impMesh.Clear ();
  impShell.Clear ();
  impEntity.Clear ();
  // remap is the only one that doesn't get recreated when we regen
  remap.clear ();
}

void Group::AddParam (IdList<Param, hParam> *param, hParam hp, double v) {
  Param pa = {};
  pa.h     = hp;
  pa.val   = v;

  param->Add (&pa);
}

bool Group::IsVisible () {
  if (!visible)
    return false;
  Group *active = SK.GetGroup (SS.GW.activeGroup);
  if (order > active->order)
    return false;
  return true;
}

size_t Group::GetNumConstraints () {
  return std::count_if (SK.constraint.begin (), SK.constraint.end (),
                        [&] (Constraint const &c) { return c.group == h; });
}

Vector Group::ExtrusionGetVector () {
  return VectorFromH (h.param (0), h.param (1), h.param (2));
}

void Group::ExtrusionForceVectorTo (const Vector &v) {
  SK.GetParam (h.param (0))->val = v.x;
  SK.GetParam (h.param (1))->val = v.y;
  SK.GetParam (h.param (2))->val = v.z;
}

void Group::TransformImportedBy (Vector t, Quaternion q) {
  ssassert (type == Type::LINKED, "Expected a linked group");

  hParam tx, ty, tz, qw, qx, qy, qz;
  tx = h.param (0);
  ty = h.param (1);
  tz = h.param (2);
  qw = h.param (3);
  qx = h.param (4);
  qy = h.param (5);
  qz = h.param (6);

  Quaternion qg = Quaternion::From (qw, qx, qy, qz);
  qg            = q.Times (qg);

  Vector tg = VectorFromH (tx, ty, tz);
  tg        = tg.Plus (t);

  SK.GetParam (tx)->val = tg.x;
  SK.GetParam (ty)->val = tg.y;
  SK.GetParam (tz)->val = tg.z;

  SK.GetParam (qw)->val = qg.w;
  SK.GetParam (qx)->val = qg.vx;
  SK.GetParam (qy)->val = qg.vy;
  SK.GetParam (qz)->val = qg.vz;
}

bool Group::IsForcedToMeshBySource () const {
  const Group *srcg = this;
  if (type == Type::TRANSLATE || type == Type::ROTATE) {
    // A step and repeat gets merged against the group's previous group,
    // not our own previous group.
    srcg = SK.GetGroup (opA);
    if (srcg->forceToMesh)
      return true;
  }
  Group *g = srcg->RunningMeshGroup ();
  if (g == NULL)
    return false;
  return g->forceToMesh || g->IsForcedToMeshBySource ();
}

bool Group::IsForcedToMesh () const {
  return forceToMesh || IsTriangleMeshAssembly () || IsForcedToMeshBySource ();
}

bool Group::IsTriangleMeshAssembly () const {
  return type == Type::LINKED && linkFile.Extension () == "stl";
}

std::string Group::DescriptionString () {
  if (name.empty ()) {
    return ssprintf ("g%03x-%s", h.v, _ ("(unnamed)"));
  } else {
    return ssprintf ("g%03x-%s", h.v, name.c_str ());
  }
}

void Group::Activate () {
  if (type == Type::DRAWING_WORKPLANE || type == Type::DRAWING_3D) {
    SS.GW.showFaces = SS.GW.showFacesDrawing;
  } else {
    SS.GW.showFaces = SS.GW.showFacesNonDrawing;
  }
  SS.ScheduleShowTW ();
}

void Group::Generate (IdList<Entity, hEntity> *entity, IdList<Param, hParam> *param) {
  Vector gn = (SS.GW.projRight).Cross (SS.GW.projUp);
  Vector gp = SS.GW.projRight.Plus (SS.GW.projUp);
  Vector gc = (SS.GW.offset).ScaledBy (-1);
  gn        = gn.WithMagnitude (200 / SS.GW.scale);
  gp        = gp.WithMagnitude (200 / SS.GW.scale);
  int a, i;
  switch (type) {
  case Type::DRAWING_3D: return;

  case Type::DRAWING_WORKPLANE: {
    Quaternion q;
    if (subtype == Subtype::WORKPLANE_BY_LINE_SEGMENTS) {
      Vector u = SK.GetEntity (predef.entityB)->VectorGetNum ();
      Vector v = SK.GetEntity (predef.entityC)->VectorGetNum ();
      u        = u.WithMagnitude (1);
      Vector n = u.Cross (v);
      v        = (n.Cross (u)).WithMagnitude (1);

      if (predef.swapUV)
        std::swap (u, v);
      if (predef.negateU)
        u = u.ScaledBy (-1);
      if (predef.negateV)
        v = v.ScaledBy (-1);
      q = Quaternion::From (u, v);
    } else if (subtype == Subtype::WORKPLANE_BY_POINT_ORTHO) {
      // Already given, numerically.
      q = predef.q;
    } else if (subtype == Subtype::WORKPLANE_BY_POINT_NORMAL) {
      q = SK.GetEntity (predef.entityB)->NormalGetNum ();
    } else
      ssassert (false, "Unexpected workplane subtype");

    Entity normal    = {};
    normal.type      = Entity::Type::NORMAL_N_COPY;
    normal.numNormal = q;

    normal.point[0] = h.entity (2);
    normal.group    = h;
    normal.h        = h.entity (1);
    entity->Add (&normal);

    Entity point       = {};
    point.type         = Entity::Type::POINT_N_COPY;
    point.numPoint     = SK.GetEntity (predef.origin)->PointGetNum ();
    point.construction = true;
    point.group        = h;
    point.h            = h.entity (2);
    entity->Add (&point);

    Entity wp   = {};
    wp.type     = Entity::Type::WORKPLANE;
    wp.normal   = normal.h;
    wp.point[0] = point.h;
    wp.group    = h;
    wp.h        = h.entity (0);
    entity->Add (&wp);
    return;
  }

  case Type::EXTRUDE: {
    AddParam (param, h.param (0), gn.x);
    AddParam (param, h.param (1), gn.y);
    AddParam (param, h.param (2), gn.z);
    int ai, af;
    if (subtype == Subtype::ONE_SIDED) {
      ai = 0;
      af = 2;
    } else if (subtype == Subtype::TWO_SIDED) {
      ai = -1;
      af = 1;
    } else
      ssassert (false, "Unexpected extrusion subtype");

    // Get some arbitrary point in the sketch, that will be used
    // as a reference when defining top and bottom faces.
    hEntity pt = {0};
    // Not using range-for here because we're changing the size of entity in the loop.
    for (i = 0; i < entity->n; i++) {
      Entity *e = &(entity->Get (i));
      if (e->group != opA)
        continue;

      if (e->IsPoint ())
        pt = e->h;

      e->CalculateNumerical (/*forExport=*/false);
      hEntity he = e->h;
      e          = NULL;
      // As soon as I call CopyEntity, e may become invalid! That
      // adds entities, which may cause a realloc.
      CopyEntity (entity, SK.GetEntity (he), ai, REMAP_BOTTOM, h.param (0), h.param (1),
                  h.param (2), NO_PARAM, NO_PARAM, NO_PARAM, NO_PARAM, NO_PARAM, CopyAs::N_TRANS);
      CopyEntity (entity, SK.GetEntity (he), af, REMAP_TOP, h.param (0), h.param (1), h.param (2),
                  NO_PARAM, NO_PARAM, NO_PARAM, NO_PARAM, NO_PARAM, CopyAs::N_TRANS);
      MakeExtrusionLines (entity, he);
    }
    // Remapped versions of that arbitrary point will be used to
    // provide points on the plane faces.
    MakeExtrusionTopBottomFaces (entity, pt);
    return;
  }

  case Type::LATHE: {
    Vector axis_pos = SK.GetEntity (predef.origin)->PointGetNum ();
    Vector axis_dir = SK.GetEntity (predef.entityB)->VectorGetNum ();

    // Not using range-for here because we're changing the size of entity in the loop.
    for (i = 0; i < entity->n; i++) {
      Entity *e = &(entity->Get (i));
      if (e->group != opA)
        continue;

      e->CalculateNumerical (/*forExport=*/false);
      hEntity he = e->h;

      // As soon as I call CopyEntity, e may become invalid! That
      // adds entities, which may cause a realloc.

      // this is the regular copy of all entities
      CopyEntity (entity, SK.GetEntity (he), 0, REMAP_LATHE_START, NO_PARAM, NO_PARAM, NO_PARAM,
                  NO_PARAM, NO_PARAM, NO_PARAM, NO_PARAM, NO_PARAM, CopyAs::NUMERIC);

      e = &(entity->Get (i)); // because we copied.
      if (e->IsPoint ()) {
        // for points this copy is used for the circle centers
        CopyEntity (entity, SK.GetEntity (he), 0, REMAP_LATHE_ARC_CENTER, NO_PARAM, NO_PARAM,
                    NO_PARAM, NO_PARAM, NO_PARAM, NO_PARAM, NO_PARAM, NO_PARAM, CopyAs::NUMERIC);
        MakeLatheCircles (entity, param, he, axis_pos, axis_dir);
      };
      MakeLatheSurfacesSelectable (entity, he, axis_dir);
    }
    return;
  }

  case Type::REVOLVE: {
    // this was borrowed from LATHE and ROTATE
    Vector axis_pos = SK.GetEntity (predef.origin)->PointGetNum ();
    Vector axis_dir = SK.GetEntity (predef.entityB)->VectorGetNum ();

    // The center of rotation
    AddParam (param, h.param (0), axis_pos.x);
    AddParam (param, h.param (1), axis_pos.y);
    AddParam (param, h.param (2), axis_pos.z);
    // The rotation quaternion
    AddParam (param, h.param (3), 30 * PI / 180);
    AddParam (param, h.param (4), axis_dir.x);
    AddParam (param, h.param (5), axis_dir.y);
    AddParam (param, h.param (6), axis_dir.z);

    // Get some arbitrary point in the sketch, that will be used
    // as a reference when defining end faces.
    hEntity pt = {0};

    int ai = 0, af = 2;
    if (subtype == Subtype::TWO_SIDED) {
      ai = -1;
      af = 1;
    }
    // Not using range-for here because we're changing the size of entity in the loop.
    for (i = 0; i < entity->n; i++) {
      Entity *e = &(entity->Get (i));
      if (e->group != opA)
        continue;

      if (e->IsPoint ())
        pt = e->h;

      e->CalculateNumerical (/*forExport=*/false);
      hEntity he = e->h;
      // one copy for each end of the revolved surface
      CopyEntity (entity, e, ai, REMAP_LATHE_START, h.param (0), h.param (1), h.param (2),
                  h.param (3), h.param (4), h.param (5), h.param (6), NO_PARAM, CopyAs::N_ROT_AA);

      e = &(entity->Get (i)); // because we copied.
      CopyEntity (entity, e, af, REMAP_LATHE_END, h.param (0), h.param (1), h.param (2),
                  h.param (3), h.param (4), h.param (5), h.param (6), NO_PARAM, CopyAs::N_ROT_AA);

      // Arcs are not generated for revolve groups, for now, because our current arc
      // entity is not chiral, and dragging a revolve may break the arc by inverting it.
      // MakeLatheCircles(entity, param, he, axis_pos, axis_dir);
      MakeLatheSurfacesSelectable (entity, he, axis_dir);
    }
    MakeRevolveEndFaces (entity, pt, ai, af);
    return;
  }

  case Type::HELIX: {
    Vector axis_pos = SK.GetEntity (predef.origin)->PointGetNum ();
    Vector axis_dir = SK.GetEntity (predef.entityB)->VectorGetNum ();

    // The center of rotation
    AddParam (param, h.param (0), axis_pos.x);
    AddParam (param, h.param (1), axis_pos.y);
    AddParam (param, h.param (2), axis_pos.z);
    // The rotation quaternion
    AddParam (param, h.param (3), 30 * PI / 180);
    AddParam (param, h.param (4), axis_dir.x);
    AddParam (param, h.param (5), axis_dir.y);
    AddParam (param, h.param (6), axis_dir.z);
    // distance to translate along the rotation axis
    AddParam (param, h.param (7), 20);

    // Get some arbitrary point in the sketch, that will be used
    // as a reference when defining end faces.
    hEntity pt = {0};

    int ai = 0, af = 2; // initial and final number of transformations
    if (subtype != Subtype::ONE_SIDED) {
      ai = -1;
      af = 1;
    }

    // Not using range-for here because we're changing the size of entity in the loop.
    for (i = 0; i < entity->n; i++) {
      Entity *e = &(entity->Get (i));
      if ((e->group.v != opA.v) && !(e->h == predef.origin))
        continue;

      if (e->IsPoint ())
        pt = e->h;

      e->CalculateNumerical (/*forExport=*/false);

      // one copy for each end of the helix
      CopyEntity (entity, e, ai, REMAP_LATHE_START, h.param (0), h.param (1), h.param (2),
                  h.param (3), h.param (4), h.param (5), h.param (6), h.param (7),
                  CopyAs::N_ROT_AXIS_TRANS);

      e = &(entity->Get (i)); // because we copied.
      CopyEntity (entity, e, af, REMAP_LATHE_END, h.param (0), h.param (1), h.param (2),
                  h.param (3), h.param (4), h.param (5), h.param (6), h.param (7),
                  CopyAs::N_ROT_AXIS_TRANS);

      // For point entities on the axis, create a construction line
      e = &(entity->Get (i));
      if (e->IsPoint ()) {
        Vector check = e->PointGetNum ().Minus (axis_pos).Cross (axis_dir);
        if (check.Dot (check) < LENGTH_EPS) {
          //! @todo isn't this the same as &(ent[i])?
          Entity *ep = SK.GetEntity (e->h);
          Entity  en = {};
          // A point gets extruded to form a line segment
          en.point[0]     = Remap (ep->h, REMAP_LATHE_START);
          en.point[1]     = Remap (ep->h, REMAP_LATHE_END);
          en.group        = h;
          en.construction = ep->construction;
          en.style        = ep->style;
          en.h            = Remap (ep->h, REMAP_PT_TO_LINE);
          en.type         = Entity::Type::LINE_SEGMENT;
          entity->Add (&en);
        }
      }
    }
    MakeRevolveEndFaces (entity, pt, ai, af);
    return;
  }

  case Type::TRANSLATE: {
    // inherit meshCombine from source group
    Group *srcg = SK.GetGroup (opA);
    meshCombine = srcg->meshCombine;
    // The translation vector
    AddParam (param, h.param (0), gp.x);
    AddParam (param, h.param (1), gp.y);
    AddParam (param, h.param (2), gp.z);

    int n = (int)valA, a0 = 0;
    if (subtype == Subtype::ONE_SIDED && skipFirst) {
      a0++;
      n++;
    }

    for (a = a0; a < n; a++) {
      // Not using range-for here because we're changing the size of entity in the loop.
      for (i = 0; i < entity->n; i++) {
        Entity *e = &(entity->Get (i));
        if (e->group != opA)
          continue;

        e->CalculateNumerical (/*forExport=*/false);
        CopyEntity (entity, e, a * 2 - (subtype == Subtype::ONE_SIDED ? 0 : (n - 1)),
                    (a == (n - 1)) ? REMAP_LAST : a, h.param (0), h.param (1), h.param (2),
                    NO_PARAM, NO_PARAM, NO_PARAM, NO_PARAM, NO_PARAM, CopyAs::N_TRANS);
      }
    }
    return;
  }
  case Type::ROTATE: {
    // inherit meshCombine from source group
    Group *srcg = SK.GetGroup (opA);
    meshCombine = srcg->meshCombine;
    // The center of rotation
    AddParam (param, h.param (0), gc.x);
    AddParam (param, h.param (1), gc.y);
    AddParam (param, h.param (2), gc.z);
    // The rotation quaternion
    AddParam (param, h.param (3), 30 * PI / 180);
    AddParam (param, h.param (4), gn.x);
    AddParam (param, h.param (5), gn.y);
    AddParam (param, h.param (6), gn.z);

    int n = (int)valA, a0 = 0;
    if (subtype == Subtype::ONE_SIDED && skipFirst) {
      a0++;
      n++;
    }

    for (a = a0; a < n; a++) {
      // Not using range-for here because we're changing the size of entity in the loop.
      for (i = 0; i < entity->n; i++) {
        Entity *e = &(entity->Get (i));
        if (e->group != opA)
          continue;

        e->CalculateNumerical (/*forExport=*/false);
        CopyEntity (entity, e, a * 2 - (subtype == Subtype::ONE_SIDED ? 0 : (n - 1)),
                    (a == (n - 1)) ? REMAP_LAST : a, h.param (0), h.param (1), h.param (2),
                    h.param (3), h.param (4), h.param (5), h.param (6), NO_PARAM, CopyAs::N_ROT_AA);
      }
    }
    return;
  }
  case Type::LINKED:
    // The translation vector
    AddParam (param, h.param (0), gp.x);
    AddParam (param, h.param (1), gp.y);
    AddParam (param, h.param (2), gp.z);
    // The rotation quaternion
    AddParam (param, h.param (3), 1);
    AddParam (param, h.param (4), 0);
    AddParam (param, h.param (5), 0);
    AddParam (param, h.param (6), 0);

    // Not using range-for here because we're changing the size of entity in the loop.
    for (i = 0; i < impEntity.n; i++) {
      Entity *ie = &(impEntity[i]);
      CopyEntity (entity, ie, 0, 0, h.param (0), h.param (1), h.param (2), h.param (3), h.param (4),
                  h.param (5), h.param (6), NO_PARAM, CopyAs::N_ROT_TRANS);
    }
    return;
  }
  ssassert (false, "Unexpected group type");
}

bool Group::IsSolvedOkay () {
  return this->solved.how == SolveResult::OKAY ||
         (this->allowRedundant && this->solved.how == SolveResult::REDUNDANT_OKAY);
}

void Group::AddEq (IdList<Equation, hEquation> *l, Expr *expr, int index) {
  Equation eq;
  eq.e = expr;
  eq.h = h.equation (index);
  l->Add (&eq);
}

void Group::GenerateEquations (IdList<Equation, hEquation> *l) {
  if (type == Type::LINKED) {
    // Normalize the quaternion
    ExprQuaternion q = {Expr::From (h.param (3)), Expr::From (h.param (4)),
                        Expr::From (h.param (5)), Expr::From (h.param (6))};
    AddEq (l, (q.Magnitude ())->Minus (Expr::From (1)), 0);
  } else if (type == Type::ROTATE || type == Type::REVOLVE || type == Type::HELIX) {
    // The axis and center of rotation are specified numerically
#define EC(x) (Expr::From (x))
#define EP(x) (Expr::From (h.param (x)))
    ExprVector orig = SK.GetEntity (predef.origin)->PointGetExprs ();
    AddEq (l, (orig.x)->Minus (EP (0)), 0);
    AddEq (l, (orig.y)->Minus (EP (1)), 1);
    AddEq (l, (orig.z)->Minus (EP (2)), 2);
    // param 3 is the angle, which is free
    Vector axis = SK.GetEntity (predef.entityB)->VectorGetNum ();
    axis        = axis.WithMagnitude (1);
    AddEq (l, (EC (axis.x))->Minus (EP (4)), 3);
    AddEq (l, (EC (axis.y))->Minus (EP (5)), 4);
    AddEq (l, (EC (axis.z))->Minus (EP (6)), 5);
#undef EC
#undef EP
    if (type == Type::HELIX) {
      if (valB != 0.0) {
        AddEq (l,
               Expr::From (h.param (7))
                   ->Times (Expr::From (PI))
                   ->Minus (Expr::From (h.param (3))->Times (Expr::From (valB))),
               6);
      }
    }
  } else if (type == Type::EXTRUDE) {
    if (predef.entityB != Entity::FREE_IN_3D) {
      // The extrusion path is locked along a line, normal to the
      // specified workplane.
      Entity    *w        = SK.GetEntity (predef.entityB);
      ExprVector u        = w->Normal ()->NormalExprsU ();
      ExprVector v        = w->Normal ()->NormalExprsV ();
      ExprVector extruden = {Expr::From (h.param (0)), Expr::From (h.param (1)),
                             Expr::From (h.param (2))};

      AddEq (l, u.Dot (extruden), 0);
      AddEq (l, v.Dot (extruden), 1);
    }
  } else if (type == Type::TRANSLATE) {
    if (predef.entityB != Entity::FREE_IN_3D) {
      Entity    *w = SK.GetEntity (predef.entityB);
      ExprVector n = w->Normal ()->NormalExprsN ();
      ExprVector trans;
      trans = ExprVector::From (h.param (0), h.param (1), h.param (2));

      // The translation vector is parallel to the workplane
      AddEq (l, trans.Dot (n), 0);
    }
  }
}

hEntity Group::Remap (hEntity in, int copyNumber) {
  auto it = remap.find ({in, copyNumber});
  if (it == remap.end ()) {
    std::tie (it, std::ignore) = remap.insert ({{in, copyNumber}, {(uint32_t)remap.size () + 1}});
  }
  return h.entity (it->second.v);
}

void Group::MakeExtrusionLines (IdList<Entity, hEntity> *el, hEntity in) {
  Entity *ep = SK.GetEntity (in);

  Entity en = {};
  if (ep->IsPoint ()) {
    // A point gets extruded to form a line segment
    en.point[0]     = Remap (ep->h, REMAP_TOP);
    en.point[1]     = Remap (ep->h, REMAP_BOTTOM);
    en.group        = h;
    en.construction = ep->construction;
    en.style        = ep->style;
    en.h            = Remap (ep->h, REMAP_PT_TO_LINE);
    en.type         = Entity::Type::LINE_SEGMENT;
    el->Add (&en);
  } else if (ep->type == Entity::Type::LINE_SEGMENT) {
    // A line gets extruded to form a plane face; an endpoint of the
    // original line is a point in the plane, and the line is in the plane.
    Vector a  = SK.GetEntity (ep->point[0])->PointGetNum ();
    Vector b  = SK.GetEntity (ep->point[1])->PointGetNum ();
    Vector ab = b.Minus (a);

    en.param[0]  = h.param (0);
    en.param[1]  = h.param (1);
    en.param[2]  = h.param (2);
    en.numPoint  = a;
    en.numNormal = Quaternion::From (0, ab.x, ab.y, ab.z);

    en.group        = h;
    en.construction = ep->construction;
    en.style        = ep->style;
    en.h            = Remap (ep->h, REMAP_LINE_TO_FACE);
    en.type         = Entity::Type::FACE_XPROD;
    el->Add (&en);
  }
}

void Group::MakeLatheCircles (IdList<Entity, hEntity> *el, IdList<Param, hParam> *param, hEntity in,
                              Vector pt, Vector axis) {
  Entity *ep = SK.GetEntity (in);

  Entity en = {};
  if (ep->IsPoint ()) {
    // A point gets revolved to form an arc.
    en.point[0] = Remap (ep->h, REMAP_LATHE_ARC_CENTER);
    en.point[1] = Remap (ep->h, REMAP_LATHE_START);
    en.point[2] = en.point[1]; // Remap(ep->h, REMAP_LATHE_END);

    // Get arc center and point on arc.
    Entity *pc = SK.GetEntity (en.point[0]);
    Entity *pp = SK.GetEntity (en.point[1]);

    // Project arc point to the revolution axis and use it for arc center.
    double k     = pp->numPoint.Minus (pt).Dot (axis) / axis.Dot (axis);
    pc->numPoint = pt.Plus (axis.ScaledBy (k));

    // Create arc entity.
    en.group        = h;
    en.construction = ep->construction;
    en.style        = ep->style;
    en.h            = Remap (ep->h, REMAP_PT_TO_ARC);
    en.type         = Entity::Type::ARC_OF_CIRCLE;

    // Generate a normal.
    Entity n    = {};
    n.workplane = en.workplane;
    n.h         = Remap (ep->h, REMAP_PT_TO_NORMAL);
    n.group     = en.group;
    n.style     = en.style;
    n.type      = Entity::Type::NORMAL_N_COPY;

    // Create basis for the normal.
    Vector nu   = pp->numPoint.Minus (pc->numPoint).WithMagnitude (1.0);
    Vector nv   = nu.Cross (axis).WithMagnitude (1.0);
    n.numNormal = Quaternion::From (nv, nu);

    // The point determines where the normal gets displayed on-screen;
    // it's entirely cosmetic.
    n.point[0] = en.point[0];
    el->Add (&n);
    en.normal = n.h;
    el->Add (&en);
  }
}

void Group::MakeLatheSurfacesSelectable (IdList<Entity, hEntity> *el, hEntity in, Vector axis) {
  Entity *ep = SK.GetEntity (in);

  Entity en = {};
  if (ep->type == Entity::Type::LINE_SEGMENT) {
    // An axis-perpendicular line gets revolved to form a face.
    Vector a = SK.GetEntity (ep->point[0])->PointGetNum ();
    Vector b = SK.GetEntity (ep->point[1])->PointGetNum ();
    Vector u = b.Minus (a).WithMagnitude (1.0);

    // Check for perpendicularity: calculate cosine of the angle
    // between axis and line direction and check that
    // cos(angle) == 0 <-> angle == +-90 deg.
    if (std::fabs (u.Dot (axis) / axis.Magnitude ()) < ANGLE_COS_EPS) {
      en.param[0]  = h.param (0);
      en.param[1]  = h.param (1);
      en.param[2]  = h.param (2);
      Vector v     = axis.Cross (u).WithMagnitude (1.0);
      Vector n     = u.Cross (v);
      en.numNormal = Quaternion::From (0, n.x, n.y, n.z);

      en.group        = h;
      en.construction = ep->construction;
      en.style        = ep->style;
      en.h            = Remap (ep->h, REMAP_LINE_TO_FACE);
      en.type         = Entity::Type::FACE_NORMAL_PT;
      en.point[0]     = ep->point[0];
      el->Add (&en);
    }
  }
}

// For Revolve and Helix groups the end faces are remapped from an arbitrary
// point on the sketch. We reference the transformed point but there is
// no existing normal so we need to define the rotation and timesApplied.
void Group::MakeRevolveEndFaces (IdList<Entity, hEntity> *el, hEntity pt, int ai, int af) {
  if (pt.v == 0)
    return;
  Group *src = SK.GetGroup (opA);
  Vector n   = src->polyLoops.normal;

  // When there is no loop normal (e.g. if the loop is broken), use normal of workplane
  // as fallback, to avoid breaking constraints depending on the faces.
  if (n.Equals (Vector::From (0.0, 0.0, 0.0)) && src->type == Group::Type::DRAWING_WORKPLANE) {
    n = SK.GetEntity (src->h.entity (0))->Normal ()->NormalN ();
  }

  Entity en = {};
  en.type   = Entity::Type::FACE_ROT_NORMAL_PT;
  en.group  = h;
  // The center of rotation
  en.param[0] = h.param (0);
  en.param[1] = h.param (1);
  en.param[2] = h.param (2);
  // The rotation quaternion
  en.param[3] = h.param (3);
  en.param[4] = h.param (4);
  en.param[5] = h.param (5);
  en.param[6] = h.param (6);

  en.numNormal    = Quaternion::From (0, n.x, n.y, n.z);
  en.point[0]     = Remap (pt, REMAP_LATHE_START);
  en.timesApplied = ai;
  en.h            = Remap (Entity::NO_ENTITY, REMAP_LATHE_START);
  el->Add (&en);

  en.point[0]     = Remap (pt, REMAP_LATHE_END);
  en.timesApplied = af;
  en.h            = Remap (Entity::NO_ENTITY, REMAP_LATHE_END);
  el->Add (&en);
}

void Group::MakeExtrusionTopBottomFaces (IdList<Entity, hEntity> *el, hEntity pt) {
  if (pt.v == 0)
    return;
  Group *src = SK.GetGroup (opA);
  Vector n   = src->polyLoops.normal;

  // When there is no loop normal (e.g. if the loop is broken), use normal of workplane
  // as fallback, to avoid breaking constraints depending on the faces.
  if (n.Equals (Vector::From (0.0, 0.0, 0.0)) && src->type == Group::Type::DRAWING_WORKPLANE) {
    n = SK.GetEntity (src->h.entity (0))->Normal ()->NormalN ();
  }

  Entity en = {};
  en.type   = Entity::Type::FACE_NORMAL_PT;
  en.group  = h;

  en.numNormal = Quaternion::From (0, n.x, n.y, n.z);
  en.point[0]  = Remap (pt, REMAP_TOP);
  en.h         = Remap (Entity::NO_ENTITY, REMAP_TOP);
  el->Add (&en);

  en.point[0] = Remap (pt, REMAP_BOTTOM);
  en.h        = Remap (Entity::NO_ENTITY, REMAP_BOTTOM);
  el->Add (&en);
}

void Group::CopyEntity (IdList<Entity, hEntity> *el, Entity *ep, int timesApplied, int remap,
                        hParam dx, hParam dy, hParam dz, hParam qw, hParam qvx, hParam qvy,
                        hParam qvz, hParam dist, CopyAs as) {
  Entity en       = {};
  en.type         = ep->type;
  en.extraPoints  = ep->extraPoints;
  en.h            = Remap (ep->h, remap);
  en.timesApplied = timesApplied;
  en.group        = h;
  en.construction = ep->construction;
  en.style        = ep->style;
  en.str          = ep->str;
  en.font         = ep->font;
  en.file         = ep->file;

  switch (ep->type) {
  case Entity::Type::WORKPLANE:
    // Don't copy these.
    return;

  case Entity::Type::POINT_N_COPY:
  case Entity::Type::POINT_N_TRANS:
  case Entity::Type::POINT_N_ROT_TRANS:
  case Entity::Type::POINT_N_ROT_AA:
  case Entity::Type::POINT_N_ROT_AXIS_TRANS:
  case Entity::Type::POINT_IN_3D:
  case Entity::Type::POINT_IN_2D:
    if (as == CopyAs::N_TRANS) {
      en.type     = Entity::Type::POINT_N_TRANS;
      en.param[0] = dx;
      en.param[1] = dy;
      en.param[2] = dz;
    } else if (as == CopyAs::NUMERIC) {
      en.type = Entity::Type::POINT_N_COPY;
    } else {
      if (as == CopyAs::N_ROT_AA) {
        en.type = Entity::Type::POINT_N_ROT_AA;
      } else if (as == CopyAs::N_ROT_AXIS_TRANS) {
        en.type = Entity::Type::POINT_N_ROT_AXIS_TRANS;
      } else {
        en.type = Entity::Type::POINT_N_ROT_TRANS;
      }
      en.param[0] = dx;
      en.param[1] = dy;
      en.param[2] = dz;
      en.param[3] = qw;
      en.param[4] = qvx;
      en.param[5] = qvy;
      en.param[6] = qvz;
      if (as == CopyAs::N_ROT_AXIS_TRANS) {
        en.param[7] = dist;
      }
    }
    en.numPoint = (ep->actPoint).ScaledBy (scale);
    break;

  case Entity::Type::NORMAL_N_COPY:
  case Entity::Type::NORMAL_N_ROT:
  case Entity::Type::NORMAL_N_ROT_AA:
  case Entity::Type::NORMAL_IN_3D:
  case Entity::Type::NORMAL_IN_2D:
    if (as == CopyAs::N_TRANS || as == CopyAs::NUMERIC) {
      en.type = Entity::Type::NORMAL_N_COPY;
    } else { // N_ROT_AXIS_TRANS probably doesn't warrant a new entity Type
      if (as == CopyAs::N_ROT_AA || as == CopyAs::N_ROT_AXIS_TRANS) {
        en.type = Entity::Type::NORMAL_N_ROT_AA;
      } else {
        en.type = Entity::Type::NORMAL_N_ROT;
      }
      en.param[0] = qw;
      en.param[1] = qvx;
      en.param[2] = qvy;
      en.param[3] = qvz;
    }
    en.numNormal = ep->actNormal;
    if (scale < 0)
      en.numNormal = en.numNormal.Mirror ();

    en.point[0] = Remap (ep->point[0], remap);
    break;

  case Entity::Type::DISTANCE_N_COPY:
  case Entity::Type::DISTANCE:
    en.type        = Entity::Type::DISTANCE_N_COPY;
    en.numDistance = ep->actDistance * std::fabs (scale);
    break;

  case Entity::Type::FACE_NORMAL_PT:
  case Entity::Type::FACE_XPROD:
  case Entity::Type::FACE_N_ROT_TRANS:
  case Entity::Type::FACE_N_TRANS:
  case Entity::Type::FACE_N_ROT_AA:
  case Entity::Type::FACE_ROT_NORMAL_PT:
  case Entity::Type::FACE_N_ROT_AXIS_TRANS:
    if (as == CopyAs::N_TRANS) {
      en.type     = Entity::Type::FACE_N_TRANS;
      en.param[0] = dx;
      en.param[1] = dy;
      en.param[2] = dz;
    } else if (as == CopyAs::NUMERIC) {
      en.type = Entity::Type::FACE_NORMAL_PT;
    } else if (as == CopyAs::N_ROT_AXIS_TRANS) {
      en.type     = Entity::Type::FACE_N_ROT_AXIS_TRANS;
      en.param[0] = dx;
      en.param[1] = dy;
      en.param[2] = dz;
      en.param[3] = qw;
      en.param[4] = qvx;
      en.param[5] = qvy;
      en.param[6] = qvz;
      en.param[7] = dist;
    } else {
      if (as == CopyAs::N_ROT_AA) {
        en.type = Entity::Type::FACE_N_ROT_AA;
      } else {
        en.type = Entity::Type::FACE_N_ROT_TRANS;
      }
      en.param[0] = dx;
      en.param[1] = dy;
      en.param[2] = dz;
      en.param[3] = qw;
      en.param[4] = qvx;
      en.param[5] = qvy;
      en.param[6] = qvz;
    }
    en.numPoint  = (ep->actPoint).ScaledBy (scale);
    en.numNormal = (ep->actNormal).ScaledBy (scale);
    break;

  default: {
    int  i, points;
    bool hasNormal, hasDistance;
    EntReqTable::GetEntityInfo (ep->type, ep->extraPoints, NULL, &points, &hasNormal, &hasDistance);
    for (i = 0; i < points; i++) {
      en.point[i] = Remap (ep->point[i], remap);
    }
    if (hasNormal)
      en.normal = Remap (ep->normal, remap);
    if (hasDistance)
      en.distance = Remap (ep->distance, remap);
    break;
  }
  }

  // If the entity came from an linked file where it was invisible then
  // ep->actiVisble will be false, and we should hide it. Or if the entity
  // came from a copy (e.g. step and repeat) of a force-hidden linked
  // entity, then we also want to hide it.
  en.forceHidden = (!ep->actVisible) || ep->forceHidden;

  el->Add (&en);
}

bool Group::ShouldDrawExploded () const {
  return SS.explode && h == SS.GW.activeGroup && type == Type::DRAWING_WORKPLANE && !SS.exportMode;
}

// formerly groupmesh.cpp
//-----------------------------------------------------------------------------
// Routines to generate our watertight brep shells from the operations
// and entities specified by the user in each group; templated to work either
// on an SShell of ratpoly surfaces or on an SMesh of triangles.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"

void Group::AssembleLoops (bool *allClosed, bool *allCoplanar, bool *allNonZeroLen) {
  SBezierList sbl = {};

  int i;
  for (auto &e : SK.entity) {
    if (e.group != h)
      continue;
    if (e.construction)
      continue;
    if (e.forceHidden)
      continue;

    e.GenerateBezierCurves (&sbl);
  }

  SBezier *sb;
  *allNonZeroLen = true;
  for (sb = sbl.l.First (); sb; sb = sbl.l.NextAfter (sb)) {
    for (i = 1; i <= sb->deg; i++) {
      if (!(sb->ctrl[i]).Equals (sb->ctrl[0])) {
        break;
      }
    }
    if (i > sb->deg) {
      // This is a zero-length edge.
      *allNonZeroLen         = false;
      polyError.errorPointAt = sb->ctrl[0];
      goto done;
    }
  }

  // Try to assemble all these Beziers into loops. The closed loops go into
  // bezierLoops, with the outer loops grouped with their holes. The
  // leftovers, if any, go in bezierOpens.
  bezierLoops.FindOuterFacesFrom (&sbl, &polyLoops, NULL, SS.ChordTolMm (), allClosed,
                                  &(polyError.notClosedAt), allCoplanar, &(polyError.errorPointAt),
                                  &bezierOpens);
done:
  sbl.Clear ();
}

void Group::GenerateLoops () {
  polyLoops.Clear ();
  bezierLoops.Clear ();
  bezierOpens.Clear ();

  if (type == Type::DRAWING_3D || type == Type::DRAWING_WORKPLANE || type == Type::ROTATE ||
      type == Type::TRANSLATE || type == Type::LINKED) {
    bool allClosed = false, allCoplanar = false, allNonZeroLen = false;
    AssembleLoops (&allClosed, &allCoplanar, &allNonZeroLen);
    if (!allNonZeroLen) {
      polyError.how = PolyError::ZERO_LEN_EDGE;
    } else if (!allCoplanar) {
      polyError.how = PolyError::NOT_COPLANAR;
    } else if (!allClosed) {
      polyError.how = PolyError::NOT_CLOSED;
    } else {
      polyError.how = PolyError::GOOD;
      // The self-intersecting check is kind of slow, so don't run it
      // unless requested.
      if (SS.checkClosedContour) {
        if (polyLoops.SelfIntersecting (&(polyError.errorPointAt))) {
          polyError.how = PolyError::SELF_INTERSECTING;
        }
      }
    }
  }
}

void SShell::RemapFaces (Group *g, int remap) {
  for (SSurface &ss : surface) {
    hEntity face = {ss.face};
    if (face == Entity::NO_ENTITY)
      continue;

    face    = g->Remap (face, remap);
    ss.face = face.v;
  }
}

void SMesh::RemapFaces (Group *g, int remap) {
  STriangle *tr;
  for (tr = l.First (); tr; tr = l.NextAfter (tr)) {
    hEntity face = {tr->meta.face};
    if (face == Entity::NO_ENTITY)
      continue;

    face          = g->Remap (face, remap);
    tr->meta.face = face.v;
  }
}

template<class T>
void Group::GenerateForStepAndRepeat (T *steps, T *outs, Group::CombineAs forWhat) {

  int n = (int)valA, a0 = 0;
  if (subtype == Subtype::ONE_SIDED && skipFirst) {
    a0++;
    n++;
  }

  int a;
  // create all the transformed copies
  std::vector<T> transd (n);
  std::vector<T> workA (n);
  workA[0] = {};
  // first generate a shell/mesh with each transformed copy
#pragma omp parallel for
  for (a = a0; a < n; a++) {
    transd[a] = {};
    workA[a]  = {};
    int ap    = a * 2 - (subtype == Subtype::ONE_SIDED ? 0 : (n - 1));

    if (type == Type::TRANSLATE) {
      Vector trans = VectorFromH (h.param (0), h.param (1), h.param (2));
      trans        = trans.ScaledBy (ap);
      transd[a].MakeFromTransformationOf (steps, trans, Quaternion::IDENTITY, 1.0);
    } else {
      Vector     trans = VectorFromH (h.param (0), h.param (1), h.param (2));
      double     theta = ap * SK.GetParam (h.param (3))->val;
      double     c = cos (theta), s = sin (theta);
      Vector     axis = VectorFromH (h.param (4), h.param (5), h.param (6));
      Quaternion q    = Quaternion::From (c, s * axis.x, s * axis.y, s * axis.z);
      // Rotation is centered at t; so A(x - t) + t = Ax + (t - At)
      transd[a].MakeFromTransformationOf (steps, trans.Minus (q.Rotate (trans)), q, 1.0);
    }
  }
  for (a = a0; a < n; a++) {
    // We need to rewrite any plane face entities to the transformed ones.
    int remap = (a == (n - 1)) ? REMAP_LAST : a;
    transd[a].RemapFaces (this, remap);
  }

  std::vector<T> *soFar   = &transd;
  std::vector<T> *scratch = &workA;
  // do the boolean operations on pairs of equal size
  while (n > 1) {
    for (a = 0; a < n; a += 2) {
      scratch->at (a / 2).Clear ();
      // combine a pair of shells
      if ((a == 0) && (a0 == 1)) { // if the first was skipped just copy the 2nd
        scratch->at (a / 2).MakeFromCopyOf (&(soFar->at (a + 1)));
        (soFar->at (a + 1)).Clear ();
        a0 = 0;
      } else if (a == n - 1) { // for an odd number just copy the last one
        scratch->at (a / 2).MakeFromCopyOf (&(soFar->at (a)));
        (soFar->at (a)).Clear ();
      } else if (forWhat == CombineAs::ASSEMBLE) {
        scratch->at (a / 2).MakeFromAssemblyOf (&(soFar->at (a)), &(soFar->at (a + 1)));
        (soFar->at (a)).Clear ();
        (soFar->at (a + 1)).Clear ();
      } else {
        scratch->at (a / 2).MakeFromUnionOf (&(soFar->at (a)), &(soFar->at (a + 1)));
        (soFar->at (a)).Clear ();
        (soFar->at (a + 1)).Clear ();
      }
    }
    std::swap (scratch, soFar);
    n = (n + 1) / 2;
  }
  outs->Clear ();
  *outs = soFar->at (0);
}

template<class T>
void Group::GenerateForBoolean (T *prevs, T *thiss, T *outs, Group::CombineAs how) {
  // If this group contributes no new mesh, then our running mesh is the
  // same as last time, no combining required. Likewise if we have a mesh
  // but it's suppressed.
  if (thiss->IsEmpty () || suppress) {
    outs->MakeFromCopyOf (prevs);
    return;
  }

  // So our group's shell appears in thisShell. Combine this with the
  // previous group's shell, using the requested operation.
  switch (how) {
  case CombineAs::UNION: outs->MakeFromUnionOf (prevs, thiss); break;

  case CombineAs::DIFFERENCE: outs->MakeFromDifferenceOf (prevs, thiss); break;

  case CombineAs::INTERSECTION: outs->MakeFromIntersectionOf (prevs, thiss); break;

  case CombineAs::ASSEMBLE: outs->MakeFromAssemblyOf (prevs, thiss); break;
  }
}

void Group::GenerateShellAndMesh () {
  bool prevBooleanFailed = booleanFailed;
  booleanFailed          = false;

  Group *srcg = this;

  thisShell.Clear ();
  thisMesh.Clear ();
  runningShell.Clear ();
  runningMesh.Clear ();

  // Don't attempt a lathe or extrusion unless the source section is good:
  // planar and not self-intersecting.
  bool haveSrc = true;
  if (type == Type::EXTRUDE || type == Type::LATHE || type == Type::REVOLVE) {
    Group *src = SK.GetGroup (opA);
    if (src->polyError.how != PolyError::GOOD) {
      haveSrc = false;
    }
  }

  if (type == Type::TRANSLATE || type == Type::ROTATE) {
    // A step and repeat gets merged against the group's previous group,
    // not our own previous group.
    srcg = SK.GetGroup (opA);

    if (!srcg->suppress) {
      if (!IsForcedToMesh ()) {
        GenerateForStepAndRepeat<SShell> (&(srcg->thisShell), &thisShell, srcg->meshCombine);
      } else {
        SMesh prevm = {};
        prevm.MakeFromCopyOf (&srcg->thisMesh);
        srcg->thisShell.TriangulateInto (&prevm);
        GenerateForStepAndRepeat<SMesh> (&prevm, &thisMesh, srcg->meshCombine);
      }
    }
  } else if (type == Type::EXTRUDE && haveSrc) {
    Group *src       = SK.GetGroup (opA);
    Vector translate = VectorFromH (h.param (0), h.param (1), h.param (2));

    Vector tbot, ttop;
    if (subtype == Subtype::ONE_SIDED) {
      tbot = Vector::From (0, 0, 0);
      ttop = translate.ScaledBy (2);
    } else {
      tbot = translate.ScaledBy (-1);
      ttop = translate.ScaledBy (1);
    }

    SBezierLoopSetSet *sblss = &(src->bezierLoops);
    SBezierLoopSet    *sbls;
    for (sbls = sblss->l.First (); sbls; sbls = sblss->l.NextAfter (sbls)) {
      int is = thisShell.surface.n;
      // Extrude this outer contour (plus its inner contours, if present)
      thisShell.MakeFromExtrusionOf (sbls, tbot, ttop, color);

      // And for any plane faces, annotate the model with the entity for
      // that face, so that the user can select them with the mouse.
      Vector onOrig = sbls->point;
      int    i;
      // Not using range-for here because we're starting at a different place and using
      // indices for meaning.
      for (i = is; i < thisShell.surface.n; i++) {
        SSurface *ss   = &(thisShell.surface[i]);
        hEntity   face = Entity::NO_ENTITY;

        Vector p = ss->PointAt (0, 0), n = ss->NormalAt (0, 0).WithMagnitude (1);
        double d = n.Dot (p);

        if (i == is || i == (is + 1)) {
          // These are the top and bottom of the shell.
          if (std::fabs ((onOrig.Plus (ttop)).Dot (n) - d) < LENGTH_EPS) {
            face     = Remap (Entity::NO_ENTITY, REMAP_TOP);
            ss->face = face.v;
          }
          if (std::fabs ((onOrig.Plus (tbot)).Dot (n) - d) < LENGTH_EPS) {
            face     = Remap (Entity::NO_ENTITY, REMAP_BOTTOM);
            ss->face = face.v;
          }
          continue;
        }

        // So these are the sides
        if (ss->degm != 1 || ss->degn != 1)
          continue;

        for (Entity &e : SK.entity) {
          if (e.group != opA)
            continue;
          if (e.type != Entity::Type::LINE_SEGMENT)
            continue;

          Vector a = SK.GetEntity (e.point[0])->PointGetNum (),
                 b = SK.GetEntity (e.point[1])->PointGetNum ();
          a        = a.Plus (ttop);
          b        = b.Plus (ttop);
          // Could get taken backwards, so check all cases.
          if ((a.Equals (ss->ctrl[0][0]) && b.Equals (ss->ctrl[1][0])) ||
              (b.Equals (ss->ctrl[0][0]) && a.Equals (ss->ctrl[1][0])) ||
              (a.Equals (ss->ctrl[0][1]) && b.Equals (ss->ctrl[1][1])) ||
              (b.Equals (ss->ctrl[0][1]) && a.Equals (ss->ctrl[1][1]))) {
            face     = Remap (e.h, REMAP_LINE_TO_FACE);
            ss->face = face.v;
            break;
          }
        }
      }
    }
  } else if (type == Type::LATHE && haveSrc) {
    Group *src = SK.GetGroup (opA);

    Vector pt   = SK.GetEntity (predef.origin)->PointGetNum (),
           axis = SK.GetEntity (predef.entityB)->VectorGetNum ();
    axis        = axis.WithMagnitude (1);

    SBezierLoopSetSet *sblss = &(src->bezierLoops);
    SBezierLoopSet    *sbls;
    for (sbls = sblss->l.First (); sbls; sbls = sblss->l.NextAfter (sbls)) {
      thisShell.MakeFromRevolutionOf (sbls, pt, axis, color, this);
    }
  } else if (type == Type::REVOLVE && haveSrc) {
    Group *src    = SK.GetGroup (opA);
    double anglef = SK.GetParam (h.param (3))->val * 4; // why the 4 is needed?
    double dists = 0, distf = 0;
    double angles = 0.0;
    if (subtype != Subtype::ONE_SIDED) {
      anglef *= 0.5;
      angles = -anglef;
    }
    Vector pt   = SK.GetEntity (predef.origin)->PointGetNum (),
           axis = SK.GetEntity (predef.entityB)->VectorGetNum ();
    axis        = axis.WithMagnitude (1);

    SBezierLoopSetSet *sblss = &(src->bezierLoops);
    SBezierLoopSet    *sbls;
    for (sbls = sblss->l.First (); sbls; sbls = sblss->l.NextAfter (sbls)) {
      if (std::fabs (anglef - angles) < 2 * PI) {
        thisShell.MakeFromHelicalRevolutionOf (sbls, pt, axis, color, this, angles, anglef, dists,
                                               distf);
      } else {
        thisShell.MakeFromRevolutionOf (sbls, pt, axis, color, this);
      }
    }
  } else if (type == Type::HELIX && haveSrc) {
    Group *src    = SK.GetGroup (opA);
    double anglef = SK.GetParam (h.param (3))->val * 4; // why the 4 is needed?
    double dists = 0, distf = 0;
    double angles = 0.0;
    distf         = SK.GetParam (h.param (7))->val * 2; // dist is applied twice
    if (subtype != Subtype::ONE_SIDED) {
      anglef *= 0.5;
      angles = -anglef;
      distf *= 0.5;
      dists = -distf;
    }
    Vector pt   = SK.GetEntity (predef.origin)->PointGetNum (),
           axis = SK.GetEntity (predef.entityB)->VectorGetNum ();
    axis        = axis.WithMagnitude (1);

    SBezierLoopSetSet *sblss = &(src->bezierLoops);
    SBezierLoopSet    *sbls;
    for (sbls = sblss->l.First (); sbls; sbls = sblss->l.NextAfter (sbls)) {
      thisShell.MakeFromHelicalRevolutionOf (sbls, pt, axis, color, this, angles, anglef, dists,
                                             distf);
    }
  } else if (type == Type::LINKED) {
    // The imported shell or mesh are copied over, with the appropriate
    // transformation applied. We also must remap the face entities.
    Vector     offset = Vector (SK.GetParam (h.param (0))->val, SK.GetParam (h.param (1))->val,
                                SK.GetParam (h.param (2))->val);
    Quaternion q      = {SK.GetParam (h.param (3))->val, SK.GetParam (h.param (4))->val,
                         SK.GetParam (h.param (5))->val, SK.GetParam (h.param (6))->val};

    thisMesh.MakeFromTransformationOf (&impMesh, offset, q, scale);
    thisMesh.RemapFaces (this, 0);

    thisShell.MakeFromTransformationOf (&impShell, offset, q, scale);
    thisShell.RemapFaces (this, 0);
  }

  if (srcg->meshCombine != CombineAs::ASSEMBLE) {
    thisShell.MergeCoincidentSurfaces ();
  }

  // So now we've got the mesh or shell for this group. Combine it with
  // the previous group's mesh or shell with the requested Boolean, and
  // we're done.

  Group *prevg = srcg->RunningMeshGroup ();

  if (!IsForcedToMesh ()) {
    SShell *prevs = &(prevg->runningShell);
    GenerateForBoolean<SShell> (prevs, &thisShell, &runningShell, srcg->meshCombine);

    if (srcg->meshCombine != CombineAs::ASSEMBLE) {
      runningShell.MergeCoincidentSurfaces ();
    }

    // If the Boolean failed, then we should note that in the text screen
    // for this group.
    booleanFailed = runningShell.booleanFailed;
    if (booleanFailed != prevBooleanFailed) {
      SS.ScheduleShowTW ();
    }
  } else {
    SMesh prevm, thism;
    prevm = {};
    thism = {};

    prevm.MakeFromCopyOf (&(prevg->runningMesh));
    prevg->runningShell.TriangulateInto (&prevm);

    thism.MakeFromCopyOf (&thisMesh);
    thisShell.TriangulateInto (&thism);

    SMesh outm = {};
    GenerateForBoolean<SMesh> (&prevm, &thism, &outm, srcg->meshCombine);

    // Remove degenerate triangles; if we don't, they'll get split in SnapToMesh
    // in every generated group, resulting in polynomial increase in triangle count,
    // and corresponding slowdown.
    outm.RemoveDegenerateTriangles ();

    if (srcg->meshCombine != CombineAs::ASSEMBLE) {
      // And make sure that the output mesh is vertex-to-vertex.
      SKdNode *root = SKdNode::From (&outm);
      root->SnapToMesh (&outm);
      root->MakeMeshInto (&runningMesh);
    } else {
      runningMesh.MakeFromCopyOf (&outm);
    }

    outm.Clear ();
    thism.Clear ();
    prevm.Clear ();
  }

  displayDirty = true;
}

void Group::GenerateDisplayItems () {
  // This is potentially slow (since we've got to triangulate a shell, or
  // to find the emphasized edges for a mesh), so we will run it only
  // if its inputs have changed.
  if (displayDirty) {
    Group *pg = RunningMeshGroup ();
    if (pg && thisMesh.IsEmpty () && thisShell.IsEmpty ()) {
      // We don't contribute any new solid model in this group, so our
      // display items are identical to the previous group's; which means
      // that we can just display those, and stop ourselves from
      // recalculating for those every time we get a change in this group.
      //
      // Note that this can end up recursing multiple times (if multiple
      // groups that contribute no solid model exist in sequence), but
      // that's okay.
      pg->GenerateDisplayItems ();

      displayMesh.Clear ();
      displayMesh.MakeFromCopyOf (&(pg->displayMesh));

      displayOutlines.Clear ();
      if (SS.GW.showEdges || SS.GW.showOutlines) {
        displayOutlines.MakeFromCopyOf (&pg->displayOutlines);
      }
    } else {
      // We do contribute new solid model, so we have to triangulate the
      // shell, and edge-find the mesh.
      displayMesh.Clear ();
      runningShell.TriangulateInto (&displayMesh);
      STriangle *t;
      for (t = runningMesh.l.First (); t; t = runningMesh.l.NextAfter (t)) {
        STriangle trn = *t;
        Vector    n   = trn.Normal ();
        trn.an        = n;
        trn.bn        = n;
        trn.cn        = n;
        displayMesh.AddTriangle (&trn);
      }

      displayOutlines.Clear ();

      if (SS.GW.showEdges || SS.GW.showOutlines) {
        SOutlineList rawOutlines = {};
        if (!runningMesh.l.IsEmpty ()) {
          // Triangle mesh only; no shell or emphasized edges.
          runningMesh.MakeOutlinesInto (&rawOutlines, EdgeKind::EMPHASIZED);
        } else {
          displayMesh.MakeOutlinesInto (&rawOutlines, EdgeKind::SHARP);
        }

        PolylineBuilder builder;
        builder.MakeFromOutlines (rawOutlines);
        builder.GenerateOutlines (&displayOutlines);
        rawOutlines.Clear ();
      }
    }

    // If we render this mesh, we need to know whether it's transparent,
    // and we'll want all transparent triangles last, to make the depth test
    // work correctly.
    displayMesh.PrecomputeTransparency ();

    // Recalculate mass center if needed
    if (SS.centerOfMass.draw && SS.centerOfMass.dirty && h == SS.GW.activeGroup) {
      SS.UpdateCenterOfMass ();
    }
    displayDirty = false;
  }
}

Group *Group::PreviousGroup () const {
  Group *prev = nullptr;
  for (auto const &gh : SK.groupOrder) {
    Group *g = SK.GetGroup (gh);
    if (g->h == h) {
      return prev;
    }
    prev = g;
  }
  return nullptr;
}

Group *Group::RunningMeshGroup () const {
  if (type == Type::TRANSLATE || type == Type::ROTATE) {
    return SK.GetGroup (opA)->RunningMeshGroup ();
  } else {
    return PreviousGroup ();
  }
}

bool Group::IsMeshGroup () {
  switch (type) {
  case Group::Type::EXTRUDE:
  case Group::Type::LATHE:
  case Group::Type::REVOLVE:
  case Group::Type::HELIX:
  case Group::Type::ROTATE:
  case Group::Type::TRANSLATE: return true;

  default: return false;
  }
}

void Group::DrawMesh (DrawMeshAs how, Canvas *canvas) {
  if (!(SS.GW.showShaded || SS.GW.drawOccludedAs != GraphicsWindow::DrawOccludedAs::VISIBLE))
    return;

  switch (how) {
  case DrawMeshAs::DEFAULT: {
    // Force the shade color to something dim to not distract from
    // the sketch.
    Canvas::Fill fillFront = {};
    if (!SS.GW.showShaded) {
      fillFront.layer = Canvas::Layer::DEPTH_ONLY;
    }
    if ((type == Type::DRAWING_3D || type == Type::DRAWING_WORKPLANE) && SS.GW.dimSolidModel) {
      fillFront.color = Style::Color (Style::DIM_SOLID);
    }
    Canvas::hFill hcfFront = canvas->GetFill (fillFront);

    // The back faces are drawn in red; should never seem them, since we
    // draw closed shells, so that's a debugging aid.
    Canvas::hFill hcfBack = {};
    if (SS.drawBackFaces && !displayMesh.isTransparent) {
      Canvas::Fill fillBack = {};
      fillBack.layer        = fillFront.layer;
      fillBack.color        = RgbaColor::FromFloat (1.0f, 0.1f, 0.1f);
      hcfBack               = canvas->GetFill (fillBack);
    } else {
      hcfBack = hcfFront;
    }

    // Draw the shaded solid into the depth buffer for hidden line removal,
    // and if we're actually going to display it, to the color buffer too.
    canvas->DrawMesh (displayMesh, hcfFront, hcfBack);

    // Draw mesh edges, for debugging.
    if (SS.GW.showMesh) {
      Canvas::Stroke strokeTriangle = {};
      strokeTriangle.zIndex         = 1;
      strokeTriangle.color          = RgbaColor::FromFloat (0.0f, 1.0f, 0.0f);
      strokeTriangle.width          = 1;
      strokeTriangle.unit           = Canvas::Unit::PX;
      Canvas::hStroke hcsTriangle   = canvas->GetStroke (strokeTriangle);
      SEdgeList       edges         = {};
      for (const STriangle &t : displayMesh.l) {
        edges.AddEdge (t.a, t.b);
        edges.AddEdge (t.b, t.c);
        edges.AddEdge (t.c, t.a);
      }
      canvas->DrawEdges (edges, hcsTriangle);
      edges.Clear ();
    }
    break;
  }

  case DrawMeshAs::HOVERED: {
    Canvas::Fill fill = {};
    fill.color        = Style::Color (Style::HOVERED);
    fill.pattern      = Canvas::FillPattern::CHECKERED_A;
    fill.zIndex       = 2;
    Canvas::hFill hcf = canvas->GetFill (fill);

    std::vector<uint32_t> faces;
    hEntity               he = SS.GW.hover.entity;
    if (he.v != 0 && SK.GetEntity (he)->IsFace ()) {
      faces.push_back (he.v);
    }
    canvas->DrawFaces (displayMesh, faces, hcf);
    break;
  }

  case DrawMeshAs::SELECTED: {
    Canvas::Fill fill = {};
    fill.color        = Style::Color (Style::SELECTED);
    fill.pattern      = Canvas::FillPattern::CHECKERED_B;
    fill.zIndex       = 1;
    Canvas::hFill hcf = canvas->GetFill (fill);

    std::vector<uint32_t> faces;
    SS.GW.GroupSelection ();
    auto const &gs = SS.GW.gs;
    if (gs.faces > 0)
      faces.push_back (gs.face[0].v);
    if (gs.faces > 1)
      faces.push_back (gs.face[1].v);
    canvas->DrawFaces (displayMesh, faces, hcf);
    break;
  }
  }
}

void Group::Draw (Canvas *canvas) {
  // Everything here gets drawn whether or not the group is hidden; we
  // can control this stuff independently, with show/hide solids, edges,
  // mesh, etc.

  GenerateDisplayItems ();
  DrawMesh (DrawMeshAs::DEFAULT, canvas);

  if (SS.GW.showEdges) {
    Canvas::Stroke strokeEdge = Style::Stroke (Style::SOLID_EDGE);
    strokeEdge.zIndex         = 1;
    Canvas::hStroke hcsEdge   = canvas->GetStroke (strokeEdge);

    canvas->DrawOutlines (displayOutlines, hcsEdge,
                          SS.GW.showOutlines ? Canvas::DrawOutlinesAs::EMPHASIZED_WITHOUT_CONTOUR
                                             : Canvas::DrawOutlinesAs::EMPHASIZED_AND_CONTOUR);

    if (SS.GW.drawOccludedAs != GraphicsWindow::DrawOccludedAs::INVISIBLE) {
      Canvas::Stroke strokeHidden = Style::Stroke (Style::HIDDEN_EDGE);
      if (SS.GW.drawOccludedAs == GraphicsWindow::DrawOccludedAs::VISIBLE) {
        strokeHidden.stipplePattern = StipplePattern::CONTINUOUS;
      }
      strokeHidden.layer        = Canvas::Layer::OCCLUDED;
      Canvas::hStroke hcsHidden = canvas->GetStroke (strokeHidden);

      canvas->DrawOutlines (displayOutlines, hcsHidden,
                            Canvas::DrawOutlinesAs::EMPHASIZED_AND_CONTOUR);
    }
  }

  if (SS.GW.showOutlines) {
    Canvas::Stroke strokeOutline = Style::Stroke (Style::OUTLINE);
    strokeOutline.zIndex         = 1;
    Canvas::hStroke hcsOutline   = canvas->GetStroke (strokeOutline);

    canvas->DrawOutlines (displayOutlines, hcsOutline, Canvas::DrawOutlinesAs::CONTOUR_ONLY);
  }
}

void Group::DrawPolyError (Canvas *canvas) {
  const Camera &camera = canvas->GetCamera ();

  Canvas::Stroke strokeUnclosed = Style::Stroke (Style::DRAW_ERROR);
  strokeUnclosed.color          = strokeUnclosed.color.WithAlpha (50);
  Canvas::hStroke hcsUnclosed   = canvas->GetStroke (strokeUnclosed);

  Canvas::Stroke strokeError = Style::Stroke (Style::DRAW_ERROR);
  strokeError.layer          = Canvas::Layer::FRONT;
  strokeError.width          = 1.0f;
  Canvas::hStroke hcsError   = canvas->GetStroke (strokeError);

  double textHeight = Style::DefaultTextHeight () / camera.scale;

  // And finally show the polygons too, and any errors if it's not possible
  // to assemble the lines into closed polygons.
  if (polyError.how == PolyError::NOT_CLOSED) {
    // Report this error only in sketch-in-workplane groups; otherwise
    // it's just a nuisance.
    if (type == Type::DRAWING_WORKPLANE) {
      canvas->DrawVectorText (_ ("not closed contour, or not all same style!"), textHeight,
                              polyError.notClosedAt.b, camera.projRight, camera.projUp, hcsError);
      canvas->DrawLine (polyError.notClosedAt.a, polyError.notClosedAt.b, hcsUnclosed);
    }
  } else if (polyError.how == PolyError::NOT_COPLANAR ||
             polyError.how == PolyError::SELF_INTERSECTING ||
             polyError.how == PolyError::ZERO_LEN_EDGE) {
    // These errors occur at points, not lines
    if (type == Type::DRAWING_WORKPLANE) {
      const char *msg;
      if (polyError.how == PolyError::NOT_COPLANAR) {
        msg = _ ("points not all coplanar!");
      } else if (polyError.how == PolyError::SELF_INTERSECTING) {
        msg = _ ("contour is self-intersecting!");
      } else {
        msg = _ ("zero-length edge!");
      }
      canvas->DrawVectorText (msg, textHeight, polyError.errorPointAt, camera.projRight,
                              camera.projUp, hcsError);
    }
  } else {
    // The contours will get filled in DrawFilledPaths.
  }
}

void Group::DrawFilledPaths (Canvas *canvas) {
  for (const SBezierLoopSet &sbls : bezierLoops.l) {
    if (sbls.l.IsEmpty () || sbls.l[0].l.IsEmpty ())
      continue;

    // In an assembled loop, all the styles should be the same; so doesn't
    // matter which one we grab.
    const SBezier *sb = &(sbls.l[0].l[0]);
    Style         *s  = Style::Get ({(uint32_t)sb->auxA});

    Canvas::Fill fill = {};
    fill.zIndex       = 1;
    if (s->filled) {
      // This is a filled loop, where the user specified a fill color.
      fill.color = s->fillColor;
    } else if (h == SS.GW.activeGroup && SS.checkClosedContour &&
               polyError.how == PolyError::GOOD) {
      // If this is the active group, and we are supposed to check
      // for closed contours, and we do indeed have a closed and
      // non-intersecting contour, then fill it dimly.
      fill.color = Style::Color (Style::CONTOUR_FILL).WithAlpha (127);
    } else
      continue;
    Canvas::hFill hcf = canvas->GetFill (fill);

    SPolygon sp = {};
    sbls.MakePwlInto (&sp);
    canvas->DrawPolygon (sp, hcf);
    sp.Clear ();
  }
}

void Group::DrawContourAreaLabels (Canvas *canvas) {
  const Camera &camera = canvas->GetCamera ();
  Vector        gr     = camera.projRight.ScaledBy (1 / camera.scale);
  Vector        gu     = camera.projUp.ScaledBy (1 / camera.scale);

  for (SBezierLoopSet &sbls : bezierLoops.l) {
    if (sbls.l.IsEmpty () || sbls.l[0].l.IsEmpty ())
      continue;

    Vector min  = sbls.l[0].l[0].ctrl[0];
    Vector max  = min;
    Vector zero = Vector::From (0.0, 0.0, 0.0);
    sbls.GetBoundingProjd (Vector::From (1.0, 0.0, 0.0), zero, &min.x, &max.x);
    sbls.GetBoundingProjd (Vector::From (0.0, 1.0, 0.0), zero, &min.y, &max.y);
    sbls.GetBoundingProjd (Vector::From (0.0, 0.0, 1.0), zero, &min.z, &max.z);

    Vector mid = min.Plus (max).ScaledBy (0.5);

    hStyle         hs     = {Style::CONSTRAINT};
    Canvas::Stroke stroke = Style::Stroke (hs);
    stroke.layer          = Canvas::Layer::FRONT;

    std::string label      = SS.MmToStringSI (std::fabs (sbls.SignedArea ()), /*dim=*/2);
    double      fontHeight = Style::TextHeight (hs);
    double      textWidth  = VectorFont::Builtin ()->GetWidth (fontHeight, label),
           textHeight      = VectorFont::Builtin ()->GetCapHeight (fontHeight);
    Vector pos = mid.Minus (gr.ScaledBy (textWidth / 2.0)).Minus (gu.ScaledBy (textHeight / 2.0));
    canvas->DrawVectorText (label, fontHeight, pos, gr, gu, canvas->GetStroke (stroke));
  }
}
