//-----------------------------------------------------------------------------
// Implementation of the Constraint menu, to create new constraints in
// the sketch.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"
#include "ssg.h"

std::string Constraint::DescriptionString() const {
  std::string s;
  switch (type) {
  case Type::POINTS_COINCIDENT: s = C_("constr-name", "pts-coincident"); break;
  case Type::PT_PT_DISTANCE: s = C_("constr-name", "pt-pt-distance"); break;
  case Type::PT_LINE_DISTANCE: s = C_("constr-name", "pt-line-distance"); break;
  case Type::PT_PLANE_DISTANCE: s = C_("constr-name", "pt-plane-distance"); break;
  case Type::PT_FACE_DISTANCE: s = C_("constr-name", "pt-face-distance"); break;
  case Type::PROJ_PT_DISTANCE: s = C_("constr-name", "proj-pt-pt-distance"); break;
  case Type::PT_IN_PLANE: s = C_("constr-name", "pt-in-plane"); break;
  case Type::PT_ON_LINE: s = C_("constr-name", "pt-on-line"); break;
  case Type::PT_ON_FACE: s = C_("constr-name", "pt-on-face"); break;
  case Type::EQUAL_LENGTH_LINES: s = C_("constr-name", "eq-length"); break;
  case Type::EQ_LEN_PT_LINE_D: s = C_("constr-name", "eq-length-and-pt-ln-dist"); break;
  case Type::EQ_PT_LN_DISTANCES: s = C_("constr-name", "eq-pt-line-distances"); break;
  case Type::LENGTH_RATIO: s = C_("constr-name", "length-ratio"); break;
  case Type::ARC_ARC_LEN_RATIO: s = C_("constr-name", "arc-arc-length-ratio"); break;
  case Type::ARC_LINE_LEN_RATIO: s = C_("constr-name", "arc-line-length-ratio"); break;
  case Type::LENGTH_DIFFERENCE: s = C_("constr-name", "length-difference"); break;
  case Type::ARC_ARC_DIFFERENCE: s = C_("constr-name", "arc-arc-len-difference"); break;
  case Type::ARC_LINE_DIFFERENCE: s = C_("constr-name", "arc-line-len-difference"); break;
  case Type::SYMMETRIC: s = C_("constr-name", "symmetric"); break;
  case Type::SYMMETRIC_HORIZ: s = C_("constr-name", "symmetric-h"); break;
  case Type::SYMMETRIC_VERT: s = C_("constr-name", "symmetric-v"); break;
  case Type::SYMMETRIC_LINE: s = C_("constr-name", "symmetric-line"); break;
  case Type::AT_MIDPOINT: s = C_("constr-name", "at-midpoint"); break;
  case Type::HORIZONTAL: s = C_("constr-name", "horizontal"); break;
  case Type::VERTICAL: s = C_("constr-name", "vertical"); break;
  case Type::DIAMETER: s = C_("constr-name", "diameter"); break;
  case Type::PT_ON_CIRCLE: s = C_("constr-name", "pt-on-circle"); break;
  case Type::SAME_ORIENTATION: s = C_("constr-name", "same-orientation"); break;
  case Type::ANGLE: s = C_("constr-name", "angle"); break;
  case Type::PARALLEL: s = C_("constr-name", "parallel"); break;
  case Type::ARC_LINE_TANGENT: s = C_("constr-name", "arc-line-tangent"); break;
  case Type::CUBIC_LINE_TANGENT: s = C_("constr-name", "cubic-line-tangent"); break;
  case Type::CURVE_CURVE_TANGENT: s = C_("constr-name", "curve-curve-tangent"); break;
  case Type::PERPENDICULAR: s = C_("constr-name", "perpendicular"); break;
  case Type::EQUAL_RADIUS: s = C_("constr-name", "eq-radius"); break;
  case Type::EQUAL_ANGLE: s = C_("constr-name", "eq-angle"); break;
  case Type::EQUAL_LINE_ARC_LEN: s = C_("constr-name", "eq-line-len-arc-len"); break;
  case Type::WHERE_DRAGGED: s = C_("constr-name", "lock-where-dragged"); break;
  case Type::COMMENT: s = C_("constr-name", "comment"); break;
  default: s = "???"; break;
  }

  return ssprintf("c%03x-%s", h.v, s.c_str());
}

//-----------------------------------------------------------------------------
// Delete all constraints with the specified type, entityA, ptA. We use this
// when auto-removing constraints that would become redundant.
//-----------------------------------------------------------------------------
void Constraint::DeleteAllConstraintsFor(Constraint::Type type, hEntity entityA, hEntity ptA) {
  SK.constraint.ClearTags();
  for (auto &constraint : SK.constraint) {
    Constraint *ct = &constraint;
    if (ct->type != type)
      continue;

    if (ct->entityA != entityA)
      continue;
    if (ct->ptA != ptA)
      continue;
    ct->tag = 1;
  }
  SK.constraint.RemoveTagged();
  // And no need to do anything special, since nothing
  // ever depends on a constraint. But do clear the
  // hover, in case the just-deleted constraint was
  // hovered.
  SS.GW.hover.Clear();
}

hConstraint Constraint::AddConstraint(Constraint *c, bool rememberForUndo) {
  if (rememberForUndo)
    SS.UndoRemember();

  hConstraint hc = SK.constraint.AddAndAssignId(c);
  SK.GetConstraint(hc)->Generate(&SK.param);

  SS.MarkGroupDirty(c->group);
  SK.GetGroup(c->group)->dofCheckOk = false;
  return c->h;
}

hConstraint Constraint::Constrain(Constraint::Type type, hEntity ptA, hEntity ptB, hEntity entityA,
                                  hEntity entityB, bool other, bool other2) {
  Constraint c = {};
  c.group = SS.GW.activeGroup;
  c.workplane = SS.GW.ActiveWorkplane();
  c.type = type;
  c.ptA = ptA;
  c.ptB = ptB;
  c.entityA = entityA;
  c.entityB = entityB;
  c.other = other;
  c.other2 = other2;
  return AddConstraint(&c, /*rememberForUndo=*/false);
}

hConstraint Constraint::TryConstrain(Constraint::Type type, hEntity ptA, hEntity ptB,
                                     hEntity entityA, hEntity entityB, bool other, bool other2) {
  int rankBefore, rankAfter;
  SolveResult howBefore = SS.TestRankForGroup(SS.GW.activeGroup, &rankBefore);
  hConstraint hc = Constrain(type, ptA, ptB, entityA, entityB, other, other2);
  SolveResult howAfter = SS.TestRankForGroup(SS.GW.activeGroup, &rankAfter);
  // There are two cases where the constraint is clearly redundant:
  //   * If the group wasn't overconstrained and now it is;
  //   * If the group was overconstrained, and adding the constraint doesn't change rank at all.
  if ((howBefore == SolveResult::OKAY && howAfter == SolveResult::REDUNDANT_OKAY) ||
      (howBefore == SolveResult::REDUNDANT_OKAY && howAfter == SolveResult::REDUNDANT_OKAY &&
       rankBefore == rankAfter)) {
    SK.constraint.RemoveById(hc);
    hc = {};
  }
  return hc;
}

hConstraint Constraint::ConstrainCoincident(hEntity ptA, hEntity ptB) {
  return Constrain(Type::POINTS_COINCIDENT, ptA, ptB, Entity::NO_ENTITY, Entity::NO_ENTITY,
                   /*other=*/false, /*other2=*/false);
}

bool Constraint::ConstrainArcLineTangent(Constraint *c, Entity *line, Entity *arc) {
  Vector l0 = SK.GetEntity(line->point[0])->PointGetNum(),
         l1 = SK.GetEntity(line->point[1])->PointGetNum();
  Vector a1 = SK.GetEntity(arc->point[1])->PointGetNum(),
         a2 = SK.GetEntity(arc->point[2])->PointGetNum();
  if (l0.Equals(a1) || l1.Equals(a1)) {
    c->other = false;
  } else if (l0.Equals(a2) || l1.Equals(a2)) {
    c->other = true;
  } else {
    Error(_("The tangent arc and line segment must share an "
            "endpoint. Constrain them with Constrain -> "
            "On Point before constraining tangent."));
    return false;
  }
  return true;
}

bool Constraint::ConstrainCubicLineTangent(Constraint *c, Entity *line, Entity *cubic) {
  Vector l0 = SK.GetEntity(line->point[0])->PointGetNum(),
         l1 = SK.GetEntity(line->point[1])->PointGetNum();
  Vector as = cubic->CubicGetStartNum(), af = cubic->CubicGetFinishNum();

  if (l0.Equals(as) || l1.Equals(as)) {
    c->other = false;
  } else if (l0.Equals(af) || l1.Equals(af)) {
    c->other = true;
  } else {
    Error(_("The tangent cubic and line segment must share an "
            "endpoint. Constrain them with Constrain -> "
            "On Point before constraining tangent."));
    return false;
  }
  return true;
}

bool Constraint::ConstrainCurveCurveTangent(Constraint *c, Entity *eA, Entity *eB) {
  Vector as = eA->EndpointStart(), af = eA->EndpointFinish(), bs = eB->EndpointStart(),
         bf = eB->EndpointFinish();
  if (as.Equals(bs)) {
    c->other = false;
    c->other2 = false;
  } else if (as.Equals(bf)) {
    c->other = false;
    c->other2 = true;
  } else if (af.Equals(bs)) {
    c->other = true;
    c->other2 = false;
  } else if (af.Equals(bf)) {
    c->other = true;
    c->other2 = true;
  } else {
    Error(_("The curves must share an endpoint. Constrain them "
            "with Constrain -> On Point before constraining "
            "tangent."));
    return false;
  }
  return true;
}
