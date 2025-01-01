//-----------------------------------------------------------------------------
// The implementation of our entities in the symbolic algebra system, methods
// to return a symbolic representation of the entity (line by its endpoints,
// circle by center and radius, etc.).
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"
#include "ssg.h"
#include "bandedmatrix.h"

const hEntity Entity::FREE_IN_3D = {0};
const hEntity Entity::NO_ENTITY  = {0};

bool Entity::HasVector() const {
  switch (type) {
  case Type::LINE_SEGMENT:
  case Type::NORMAL_IN_3D:
  case Type::NORMAL_IN_2D:
  case Type::NORMAL_N_COPY:
  case Type::NORMAL_N_ROT:
  case Type::NORMAL_N_ROT_AA: return true;

  default: return false;
  }
}

ExprVector Entity::VectorGetExprsInWorkplane(hEntity wrkpl) const {
  if (IsFace()) {
    return FaceGetNormalExprs();
  }
  switch (type) {
  case Type::LINE_SEGMENT:
    return (SK.GetEntity(point[0])->PointGetExprsInWorkplane(wrkpl))
        .Minus(SK.GetEntity(point[1])->PointGetExprsInWorkplane(wrkpl));

  case Type::NORMAL_IN_3D:
  case Type::NORMAL_IN_2D:
  case Type::NORMAL_N_COPY:
  case Type::NORMAL_N_ROT:
  case Type::NORMAL_N_ROT_AA: {
    ExprVector ev = NormalExprsN();
    if (wrkpl == Entity::FREE_IN_3D) {
      return ev;
    }
    // Get the offset and basis vectors for this weird exotic csys.
    Entity    *w  = SK.GetEntity(wrkpl);
    ExprVector wu = w->Normal()->NormalExprsU();
    ExprVector wv = w->Normal()->NormalExprsV();

    // Get our coordinates in three-space, and project them into that
    // coordinate system.
    ExprVector result;
    result.x = ev.Dot(wu);
    result.y = ev.Dot(wv);
    result.z = Expr::From(0.0);
    return result;
  }
  default: ssassert(false, "Unexpected entity type");
  }
}

ExprVector Entity::VectorGetExprs() const {
  return VectorGetExprsInWorkplane(Entity::FREE_IN_3D);
}

Vector Entity::VectorGetNum() const {
  if (IsFace()) {
    return FaceGetNormalNum();
  }
  switch (type) {
  case Type::LINE_SEGMENT:
    return (SK.GetEntity(point[0])->PointGetNum()).Minus(SK.GetEntity(point[1])->PointGetNum());

  case Type::NORMAL_IN_3D:
  case Type::NORMAL_IN_2D:
  case Type::NORMAL_N_COPY:
  case Type::NORMAL_N_ROT:
  case Type::NORMAL_N_ROT_AA: return NormalN();

  default: ssassert(false, "Unexpected entity type");
  }
}

Vector Entity::VectorGetRefPoint() const {
  if (IsFace()) {
    return FaceGetPointNum();
  }
  switch (type) {
  case Type::LINE_SEGMENT:
    return ((SK.GetEntity(point[0])->PointGetNum()).Plus(SK.GetEntity(point[1])->PointGetNum()))
        .ScaledBy(0.5);

  case Type::NORMAL_IN_3D:
  case Type::NORMAL_IN_2D:
  case Type::NORMAL_N_COPY:
  case Type::NORMAL_N_ROT:
  case Type::NORMAL_N_ROT_AA: return SK.GetEntity(point[0])->PointGetNum();

  default: ssassert(false, "Unexpected entity type");
  }
}

Vector Entity::VectorGetStartPoint() const {
  switch (type) {
  case Type::LINE_SEGMENT: return SK.GetEntity(point[1])->PointGetNum();

  case Type::NORMAL_IN_3D:
  case Type::NORMAL_IN_2D:
  case Type::NORMAL_N_COPY:
  case Type::NORMAL_N_ROT:
  case Type::NORMAL_N_ROT_AA: return SK.GetEntity(point[0])->PointGetNum();

  default: ssassert(false, "Unexpected entity type");
  }
}

bool Entity::IsCircle() const {
  return (type == Type::CIRCLE) || (type == Type::ARC_OF_CIRCLE);
}

Expr *Entity::CircleGetRadiusExpr() const {
  if (type == Type::CIRCLE) {
    return SK.GetEntity(distance)->DistanceGetExpr();
  } else if (type == Type::ARC_OF_CIRCLE) {
    return Constraint::Distance(workplane, point[0], point[1]);
  } else
    ssassert(false, "Unexpected entity type");
}

double Entity::CircleGetRadiusNum() const {
  if (type == Type::CIRCLE) {
    return SK.GetEntity(distance)->DistanceGetNum();
  } else if (type == Type::ARC_OF_CIRCLE) {
    Vector c  = SK.GetEntity(point[0])->PointGetNum();
    Vector pa = SK.GetEntity(point[1])->PointGetNum();
    return (pa.Minus(c)).Magnitude();
  } else
    ssassert(false, "Unexpected entity type");
}

void Entity::ArcGetAngles(double *thetaa, double *thetab, double *dtheta) const {
  ssassert(type == Type::ARC_OF_CIRCLE, "Unexpected entity type");

  Quaternion q = Normal()->NormalGetNum();
  Vector     u = q.RotationU(), v = q.RotationV();

  Vector c  = SK.GetEntity(point[0])->PointGetNum();
  Vector pa = SK.GetEntity(point[1])->PointGetNum();
  Vector pb = SK.GetEntity(point[2])->PointGetNum();

  Point2d c2  = c.Project2d(u, v);
  Point2d pa2 = (pa.Project2d(u, v)).Minus(c2);
  Point2d pb2 = (pb.Project2d(u, v)).Minus(c2);

  *thetaa = atan2(pa2.y, pa2.x);
  *thetab = atan2(pb2.y, pb2.x);
  *dtheta = *thetab - *thetaa;
  // If the endpoints are coincident, call it a full arc, not a zero arc;
  // useful concept to have when splitting
  while (*dtheta < 1e-6)
    *dtheta += 2 * PI;
  while (*dtheta > (2 * PI))
    *dtheta -= 2 * PI;
}

Vector Entity::CubicGetStartNum() const {
  return SK.GetEntity(point[0])->PointGetNum();
}
Vector Entity::CubicGetFinishNum() const {
  return SK.GetEntity(point[3 + extraPoints])->PointGetNum();
}
ExprVector Entity::CubicGetStartTangentExprs() const {
  ExprVector pon  = SK.GetEntity(point[0])->PointGetExprs(),
             poff = SK.GetEntity(point[1])->PointGetExprs();
  return (pon.Minus(poff));
}
ExprVector Entity::CubicGetFinishTangentExprs() const {
  ExprVector pon  = SK.GetEntity(point[3 + extraPoints])->PointGetExprs(),
             poff = SK.GetEntity(point[2 + extraPoints])->PointGetExprs();
  return (pon.Minus(poff));
}
Vector Entity::CubicGetStartTangentNum() const {
  Vector pon = SK.GetEntity(point[0])->PointGetNum(), poff = SK.GetEntity(point[1])->PointGetNum();
  return (pon.Minus(poff));
}
Vector Entity::CubicGetFinishTangentNum() const {
  Vector pon  = SK.GetEntity(point[3 + extraPoints])->PointGetNum(),
         poff = SK.GetEntity(point[2 + extraPoints])->PointGetNum();
  return (pon.Minus(poff));
}

bool Entity::IsWorkplane() const {
  return (type == Type::WORKPLANE);
}

ExprVector Entity::WorkplaneGetOffsetExprs() const {
  return SK.GetEntity(point[0])->PointGetExprs();
}

Vector Entity::WorkplaneGetOffset() const {
  return SK.GetEntity(point[0])->PointGetNum();
}

void Entity::WorkplaneGetPlaneExprs(ExprVector *n, Expr **dn) const {
  if (type == Type::WORKPLANE) {
    *n = Normal()->NormalExprsN();

    ExprVector p0 = SK.GetEntity(point[0])->PointGetExprs();
    // The plane is n dot (p - p0) = 0, or
    //              n dot p - n dot p0 = 0
    // so dn = n dot p0
    *dn = p0.Dot(*n);
  } else
    ssassert(false, "Unexpected entity type");
}

bool Entity::IsDistance() const {
  return (type == Type::DISTANCE) || (type == Type::DISTANCE_N_COPY);
}
double Entity::DistanceGetNum() const {
  if (type == Type::DISTANCE) {
    return SK.GetParam(param[0])->val;
  } else if (type == Type::DISTANCE_N_COPY) {
    return numDistance;
  } else
    ssassert(false, "Unexpected entity type");
}
Expr *Entity::DistanceGetExpr() const {
  if (type == Type::DISTANCE) {
    return Expr::From(param[0]);
  } else if (type == Type::DISTANCE_N_COPY) {
    return Expr::From(numDistance);
  } else
    ssassert(false, "Unexpected entity type");
}
void Entity::DistanceForceTo(double v) {
  if (type == Type::DISTANCE) {
    (SK.GetParam(param[0]))->val = v;
  } else if (type == Type::DISTANCE_N_COPY) {
    // do nothing, it's locked
  } else
    ssassert(false, "Unexpected entity type");
}

Entity *Entity::Normal() const {
  return SK.GetEntity(normal);
}

bool Entity::IsPoint() const {
  switch (type) {
  case Type::POINT_IN_3D:
  case Type::POINT_IN_2D:
  case Type::POINT_N_COPY:
  case Type::POINT_N_TRANS:
  case Type::POINT_N_ROT_TRANS:
  case Type::POINT_N_ROT_AA:
  case Type::POINT_N_ROT_AXIS_TRANS: return true;

  default: return false;
  }
}

bool Entity::IsNormal() const {
  switch (type) {
  case Type::NORMAL_IN_3D:
  case Type::NORMAL_IN_2D:
  case Type::NORMAL_N_COPY:
  case Type::NORMAL_N_ROT:
  case Type::NORMAL_N_ROT_AA: return true;

  default: return false;
  }
}

Quaternion Entity::NormalGetNum() const {
  Quaternion q;
  switch (type) {
  case Type::NORMAL_IN_3D: q = Quaternion::From(param[0], param[1], param[2], param[3]); break;

  case Type::NORMAL_IN_2D: {
    Entity *wrkpl = SK.GetEntity(workplane);
    Entity *norm  = SK.GetEntity(wrkpl->normal);
    q             = norm->NormalGetNum();
    break;
  }
  case Type::NORMAL_N_COPY: q = numNormal; break;

  case Type::NORMAL_N_ROT:
    q = Quaternion::From(param[0], param[1], param[2], param[3]);
    q = q.Times(numNormal);
    break;

  case Type::NORMAL_N_ROT_AA: {
    q = GetAxisAngleQuaternion(0);
    q = q.Times(numNormal);
    break;
  }

  default: ssassert(false, "Unexpected entity type");
  }
  return q;
}

void Entity::NormalForceTo(Quaternion q) {
  switch (type) {
  case Type::NORMAL_IN_3D:
    SK.GetParam(param[0])->val = q.w;
    SK.GetParam(param[1])->val = q.vx;
    SK.GetParam(param[2])->val = q.vy;
    SK.GetParam(param[3])->val = q.vz;
    break;

  case Type::NORMAL_IN_2D:
  case Type::NORMAL_N_COPY:
    // There's absolutely nothing to do; these are locked.
    break;
  case Type::NORMAL_N_ROT: {
    Quaternion qp = q.Times(numNormal.Inverse());

    SK.GetParam(param[0])->val = qp.w;
    SK.GetParam(param[1])->val = qp.vx;
    SK.GetParam(param[2])->val = qp.vy;
    SK.GetParam(param[3])->val = qp.vz;
    break;
  }

  case Type::NORMAL_N_ROT_AA:
    // Not sure if I'll bother implementing this one
    break;

  default: ssassert(false, "Unexpected entity type");
  }
}

Vector Entity::NormalU() const {
  return NormalGetNum().RotationU();
}
Vector Entity::NormalV() const {
  return NormalGetNum().RotationV();
}
Vector Entity::NormalN() const {
  return NormalGetNum().RotationN();
}

ExprVector Entity::NormalExprsU() const {
  return NormalGetExprs().RotationU();
}
ExprVector Entity::NormalExprsV() const {
  return NormalGetExprs().RotationV();
}
ExprVector Entity::NormalExprsN() const {
  return NormalGetExprs().RotationN();
}

ExprQuaternion Entity::NormalGetExprs() const {
  ExprQuaternion q;
  switch (type) {
  case Type::NORMAL_IN_3D: q = ExprQuaternion::From(param[0], param[1], param[2], param[3]); break;

  case Type::NORMAL_IN_2D: {
    Entity *wrkpl = SK.GetEntity(workplane);
    Entity *norm  = SK.GetEntity(wrkpl->normal);
    q             = norm->NormalGetExprs();
    break;
  }
  case Type::NORMAL_N_COPY: q = ExprQuaternion::From(numNormal); break;

  case Type::NORMAL_N_ROT: {
    ExprQuaternion orig = ExprQuaternion::From(numNormal);
    q                   = ExprQuaternion::From(param[0], param[1], param[2], param[3]);

    q = q.Times(orig);
    break;
  }

  case Type::NORMAL_N_ROT_AA: {
    ExprQuaternion orig = ExprQuaternion::From(numNormal);
    q                   = GetAxisAngleQuaternionExprs(0);
    q                   = q.Times(orig);
    break;
  }

  default: ssassert(false, "Unexpected entity type");
  }
  return q;
}

void Entity::PointForceParamTo(Vector p) {
  switch (type) {
  case Type::POINT_IN_3D:
    SK.GetParam(param[0])->val = p.x;
    SK.GetParam(param[1])->val = p.y;
    SK.GetParam(param[2])->val = p.z;
    break;

  case Type::POINT_IN_2D:
    SK.GetParam(param[0])->val = p.x;
    SK.GetParam(param[1])->val = p.y;
    break;

  default: ssassert(false, "Unexpected entity type");
  }
}

void Entity::PointForceTo(Vector p) {
  switch (type) {
  case Type::POINT_IN_3D:
    SK.GetParam(param[0])->val = p.x;
    SK.GetParam(param[1])->val = p.y;
    SK.GetParam(param[2])->val = p.z;
    break;

  case Type::POINT_IN_2D: {
    Entity *c                  = SK.GetEntity(workplane);
    p                          = p.Minus(c->WorkplaneGetOffset());
    SK.GetParam(param[0])->val = p.Dot(c->Normal()->NormalU());
    SK.GetParam(param[1])->val = p.Dot(c->Normal()->NormalV());
    break;
  }

  case Type::POINT_N_TRANS: {
    if (timesApplied == 0)
      break;
    Vector trans               = (p.Minus(numPoint)).ScaledBy(1.0 / timesApplied);
    SK.GetParam(param[0])->val = trans.x;
    SK.GetParam(param[1])->val = trans.y;
    SK.GetParam(param[2])->val = trans.z;
    break;
  }

  case Type::POINT_N_ROT_TRANS: {
    // Force only the translation; leave the rotation unchanged. But
    // remember that we're working with respect to the rotated
    // point.
    Vector trans               = p.Minus(PointGetQuaternion().Rotate(numPoint));
    SK.GetParam(param[0])->val = trans.x;
    SK.GetParam(param[1])->val = trans.y;
    SK.GetParam(param[2])->val = trans.z;
    break;
  }

  case Type::POINT_N_ROT_AA: {
    // Force only the angle; the axis and center of rotation stay
    Vector offset = VectorFromH(param[0], param[1], param[2]);
    Vector normal = VectorFromH(param[4], param[5], param[6]);
    Vector u = normal.Normal(0), v = normal.Normal(1);
    Vector po = p.Minus(offset), numo = numPoint.Minus(offset);
    double thetap = atan2(v.Dot(po), u.Dot(po));
    double thetan = atan2(v.Dot(numo), u.Dot(numo));
    double thetaf = (thetap - thetan);
    double thetai = (SK.GetParam(param[3])->val) * timesApplied * 2;
    double dtheta = thetaf - thetai;
    // Take the smallest possible change in the actual step angle,
    // in order to avoid jumps when you cross from +pi to -pi
    while (dtheta < -PI)
      dtheta += 2 * PI;
    while (dtheta > PI)
      dtheta -= 2 * PI;
    // this extra *2 explains the mystery *4
    SK.GetParam(param[3])->val = (thetai + dtheta) / (timesApplied * 2);
    break;
  }

  case Type::POINT_N_ROT_AXIS_TRANS: {
    if (timesApplied == 0)
      break;
    // is the point on the rotation axis?
    Vector offset = VectorFromH(param[0], param[1], param[2]);
    Vector normal = VectorFromH(param[4], param[5], param[6]).WithMagnitude(1.0);
    Vector check  = numPoint.Minus(offset).Cross(normal);
    if (check.Dot(check) < LENGTH_EPS) { // if so, do extrusion style drag
      Vector trans               = (p.Minus(numPoint));
      SK.GetParam(param[7])->val = trans.Dot(normal) / timesApplied;
    } else { // otherwise do rotation style
      Vector u = normal.Normal(0), v = normal.Normal(1);
      Vector po = p.Minus(offset), numo = numPoint.Minus(offset);
      double thetap = atan2(v.Dot(po), u.Dot(po));
      double thetan = atan2(v.Dot(numo), u.Dot(numo));
      double thetaf = (thetap - thetan);
      double thetai = (SK.GetParam(param[3])->val) * timesApplied * 2;
      double dtheta = thetaf - thetai;
      // Take the smallest possible change in the actual step angle,
      // in order to avoid jumps when you cross from +pi to -pi
      while (dtheta < -PI)
        dtheta += 2 * PI;
      while (dtheta > PI)
        dtheta -= 2 * PI;
      // this extra *2 explains the mystery *4
      SK.GetParam(param[3])->val = (thetai + dtheta) / (timesApplied * 2);
    }
    break;
  }

  case Type::POINT_N_COPY:
    // Nothing to do; it's a static copy
    break;

  default: ssassert(false, "Unexpected entity type");
  }
}

Vector Entity::PointGetNum() const {
  Vector p;
  switch (type) {
  case Type::POINT_IN_3D: p = VectorFromH(param[0], param[1], param[2]); break;

  case Type::POINT_IN_2D: {
    Entity *c = SK.GetEntity(workplane);
    Vector  u = c->Normal()->NormalU();
    Vector  v = c->Normal()->NormalV();
    p         = u.ScaledBy(SK.GetParam(param[0])->val);
    p         = p.Plus(v.ScaledBy(SK.GetParam(param[1])->val));
    p         = p.Plus(c->WorkplaneGetOffset());
    break;
  }

  case Type::POINT_N_TRANS: {
    Vector trans = VectorFromH(param[0], param[1], param[2]);
    p            = numPoint.Plus(trans.ScaledBy(timesApplied));
    break;
  }

  case Type::POINT_N_ROT_TRANS: {
    Vector     offset = VectorFromH(param[0], param[1], param[2]);
    Quaternion q      = PointGetQuaternion();
    p                 = q.Rotate(numPoint);
    p                 = p.Plus(offset);
    break;
  }

  case Type::POINT_N_ROT_AA: {
    Vector     offset = VectorFromH(param[0], param[1], param[2]);
    Quaternion q      = PointGetQuaternion();
    p                 = numPoint.Minus(offset);
    p                 = q.Rotate(p);
    p                 = p.Plus(offset);
    break;
  }

  case Type::POINT_N_ROT_AXIS_TRANS: {
    Vector offset   = VectorFromH(param[0], param[1], param[2]);
    Vector displace = VectorFromH(param[4], param[5], param[6])
                          .WithMagnitude(SK.GetParam(param[7])->val)
                          .ScaledBy(timesApplied);
    Quaternion q = PointGetQuaternion();
    p            = numPoint.Minus(offset);
    p            = q.Rotate(p);
    p            = p.Plus(offset).Plus(displace);
    break;
  }

  case Type::POINT_N_COPY: p = numPoint; break;

  default: ssassert(false, "Unexpected entity type");
  }
  return p;
}

ExprVector Entity::PointGetExprs() const {
  ExprVector r;
  switch (type) {
  case Type::POINT_IN_3D: r = ExprVector::From(param[0], param[1], param[2]); break;

  case Type::POINT_IN_2D: {
    Entity    *c = SK.GetEntity(workplane);
    ExprVector u = c->Normal()->NormalExprsU();
    ExprVector v = c->Normal()->NormalExprsV();
    r            = c->WorkplaneGetOffsetExprs();
    r            = r.Plus(u.ScaledBy(Expr::From(param[0])));
    r            = r.Plus(v.ScaledBy(Expr::From(param[1])));
    break;
  }
  case Type::POINT_N_TRANS: {
    ExprVector orig  = ExprVector::From(numPoint);
    ExprVector trans = ExprVector::From(param[0], param[1], param[2]);
    r                = orig.Plus(trans.ScaledBy(Expr::From(timesApplied)));
    break;
  }
  case Type::POINT_N_ROT_TRANS: {
    ExprVector     orig  = ExprVector::From(numPoint);
    ExprVector     trans = ExprVector::From(param[0], param[1], param[2]);
    ExprQuaternion q     = ExprQuaternion::From(param[3], param[4], param[5], param[6]);
    orig                 = q.Rotate(orig);
    r                    = orig.Plus(trans);
    break;
  }
  case Type::POINT_N_ROT_AA: {
    ExprVector     orig  = ExprVector::From(numPoint);
    ExprVector     trans = ExprVector::From(param[0], param[1], param[2]);
    ExprQuaternion q     = GetAxisAngleQuaternionExprs(3);
    orig                 = orig.Minus(trans);
    orig                 = q.Rotate(orig);
    r                    = orig.Plus(trans);
    break;
  }
  case Type::POINT_N_ROT_AXIS_TRANS: {
    ExprVector orig     = ExprVector::From(numPoint);
    ExprVector trans    = ExprVector::From(param[0], param[1], param[2]);
    ExprVector displace = ExprVector::From(param[4], param[5], param[6])
                              .WithMagnitude(Expr::From(1.0))
                              .ScaledBy(Expr::From(timesApplied))
                              .ScaledBy(Expr::From(param[7]));

    ExprQuaternion q = GetAxisAngleQuaternionExprs(3);
    orig             = orig.Minus(trans);
    orig             = q.Rotate(orig);
    r                = orig.Plus(trans).Plus(displace);
    break;
  }
  case Type::POINT_N_COPY: r = ExprVector::From(numPoint); break;

  default: ssassert(false, "Unexpected entity type");
  }
  return r;
}

void Entity::PointGetExprsInWorkplane(hEntity wrkpl, Expr **u, Expr **v) const {
  if (type == Type::POINT_IN_2D && workplane == wrkpl) {
    // They want our coordinates in the form that we've written them,
    // very nice.
    *u = Expr::From(param[0]);
    *v = Expr::From(param[1]);
  } else {
    // Get the offset and basis vectors for this weird exotic csys.
    Entity    *w  = SK.GetEntity(wrkpl);
    ExprVector wp = w->WorkplaneGetOffsetExprs();
    ExprVector wu = w->Normal()->NormalExprsU();
    ExprVector wv = w->Normal()->NormalExprsV();

    // Get our coordinates in three-space, and project them into that
    // coordinate system.
    ExprVector ev = PointGetExprs();
    ev            = ev.Minus(wp);
    *u            = ev.Dot(wu);
    *v            = ev.Dot(wv);
  }
}

ExprVector Entity::PointGetExprsInWorkplane(hEntity wrkpl) const {
  if (wrkpl == Entity::FREE_IN_3D) {
    return PointGetExprs();
  }

  ExprVector r;
  PointGetExprsInWorkplane(wrkpl, &r.x, &r.y);
  r.z = Expr::From(0.0);
  return r;
}

void Entity::PointForceQuaternionTo(Quaternion q) {
  ssassert(type == Type::POINT_N_ROT_TRANS, "Unexpected entity type");

  SK.GetParam(param[3])->val = q.w;
  SK.GetParam(param[4])->val = q.vx;
  SK.GetParam(param[5])->val = q.vy;
  SK.GetParam(param[6])->val = q.vz;
}

Quaternion Entity::GetAxisAngleQuaternion(int param0) const {
  Quaternion q;
  double     theta = timesApplied * SK.GetParam(param[param0 + 0])->val;
  double     s = sin(theta), c = cos(theta);
  q.w  = c;
  q.vx = s * SK.GetParam(param[param0 + 1])->val;
  q.vy = s * SK.GetParam(param[param0 + 2])->val;
  q.vz = s * SK.GetParam(param[param0 + 3])->val;
  return q;
}

ExprQuaternion Entity::GetAxisAngleQuaternionExprs(int param0) const {
  ExprQuaternion q;

  Expr *theta = Expr::From(timesApplied)->Times(Expr::From(param[param0 + 0]));
  Expr *c = theta->Cos(), *s = theta->Sin();
  q.w  = c;
  q.vx = s->Times(Expr::From(param[param0 + 1]));
  q.vy = s->Times(Expr::From(param[param0 + 2]));
  q.vz = s->Times(Expr::From(param[param0 + 3]));
  return q;
}

Quaternion Entity::PointGetQuaternion() const {
  Quaternion q;

  if (type == Type::POINT_N_ROT_AA || type == Type::POINT_N_ROT_AXIS_TRANS) {
    q = GetAxisAngleQuaternion(3);
  } else if (type == Type::POINT_N_ROT_TRANS) {
    q = Quaternion::From(param[3], param[4], param[5], param[6]);
  } else
    ssassert(false, "Unexpected entity type");

  return q;
}

bool Entity::IsFace() const {
  switch (type) {
  case Type::FACE_NORMAL_PT:
  case Type::FACE_XPROD:
  case Type::FACE_N_ROT_TRANS:
  case Type::FACE_N_TRANS:
  case Type::FACE_N_ROT_AA:
  case Type::FACE_ROT_NORMAL_PT:
  case Type::FACE_N_ROT_AXIS_TRANS: return true;
  default: return false;
  }
}

ExprVector Entity::FaceGetNormalExprs() const {
  ExprVector r;
  if (type == Type::FACE_NORMAL_PT) {
    Vector v = Vector::From(numNormal.vx, numNormal.vy, numNormal.vz);
    r        = ExprVector::From(v.WithMagnitude(1));
  } else if (type == Type::FACE_XPROD) {
    ExprVector vc = ExprVector::From(param[0], param[1], param[2]);
    ExprVector vn = ExprVector::From(numNormal.vx, numNormal.vy, numNormal.vz);
    r             = vc.Cross(vn);
    r             = r.WithMagnitude(Expr::From(1.0));
  } else if (type == Type::FACE_N_ROT_TRANS) {
    // The numerical normal vector gets the rotation; the numerical
    // normal has magnitude one, and the rotation doesn't change that,
    // so there's no need to fix it up.
    r                = ExprVector::From(numNormal.vx, numNormal.vy, numNormal.vz);
    ExprQuaternion q = ExprQuaternion::From(param[3], param[4], param[5], param[6]);
    r                = q.Rotate(r);
  } else if (type == Type::FACE_N_TRANS) {
    r = ExprVector::From(numNormal.vx, numNormal.vy, numNormal.vz);
  } else if ((type == Type::FACE_N_ROT_AA) || (type == Type::FACE_ROT_NORMAL_PT)) {
    r                = ExprVector::From(numNormal.vx, numNormal.vy, numNormal.vz);
    ExprQuaternion q = GetAxisAngleQuaternionExprs(3);
    r                = q.Rotate(r);
  } else
    ssassert(false, "Unexpected entity type");
  return r;
}

Vector Entity::FaceGetNormalNum() const {
  Vector r;
  if (type == Type::FACE_NORMAL_PT) {
    r = Vector::From(numNormal.vx, numNormal.vy, numNormal.vz);
  } else if (type == Type::FACE_XPROD) {
    Vector vc = VectorFromH(param[0], param[1], param[2]);
    Vector vn = Vector(numNormal.vx, numNormal.vy, numNormal.vz);
    r         = vc.Cross(vn);
  } else if (type == Type::FACE_N_ROT_TRANS) {
    // The numerical normal vector gets the rotation
    r            = Vector::From(numNormal.vx, numNormal.vy, numNormal.vz);
    Quaternion q = Quaternion::From(param[3], param[4], param[5], param[6]);
    r            = q.Rotate(r);
  } else if (type == Type::FACE_N_TRANS) {
    r = Vector::From(numNormal.vx, numNormal.vy, numNormal.vz);
  } else if ((type == Type::FACE_N_ROT_AA) || (type == Type::FACE_ROT_NORMAL_PT)) {
    r            = Vector::From(numNormal.vx, numNormal.vy, numNormal.vz);
    Quaternion q = GetAxisAngleQuaternion(3);
    r            = q.Rotate(r);
  } else
    ssassert(false, "Unexpected entity type");
  return r.WithMagnitude(1);
}

ExprVector Entity::FaceGetPointExprs() const {
  ExprVector r;
  if ((type == Type::FACE_NORMAL_PT) || (type == Type::FACE_ROT_NORMAL_PT)) {
    r = SK.GetEntity(point[0])->PointGetExprs();
  } else if (type == Type::FACE_XPROD) {
    r = ExprVector::From(numPoint);
  } else if (type == Type::FACE_N_ROT_TRANS) {
    // The numerical point gets the rotation and translation.
    ExprVector     trans = ExprVector::From(param[0], param[1], param[2]);
    ExprQuaternion q     = ExprQuaternion::From(param[3], param[4], param[5], param[6]);
    r                    = ExprVector::From(numPoint);
    r                    = q.Rotate(r);
    r                    = r.Plus(trans);
  } else if (type == Type::FACE_N_ROT_AXIS_TRANS) {
    ExprVector orig     = ExprVector::From(numPoint);
    ExprVector trans    = ExprVector::From(param[0], param[1], param[2]);
    ExprVector displace = ExprVector::From(param[4], param[5], param[6])
                              .WithMagnitude(Expr::From(param[7]))
                              .ScaledBy(Expr::From(timesApplied));
    ExprQuaternion q = GetAxisAngleQuaternionExprs(3);
    orig             = orig.Minus(trans);
    orig             = q.Rotate(orig);
    r                = orig.Plus(trans).Plus(displace);
  } else if (type == Type::FACE_N_TRANS) {
    ExprVector trans = ExprVector::From(param[0], param[1], param[2]);
    r                = ExprVector::From(numPoint);
    r                = r.Plus(trans.ScaledBy(Expr::From(timesApplied)));
  } else if (type == Type::FACE_N_ROT_AA) {
    ExprVector     trans = ExprVector::From(param[0], param[1], param[2]);
    ExprQuaternion q     = GetAxisAngleQuaternionExprs(3);
    r                    = ExprVector::From(numPoint);
    r                    = r.Minus(trans);
    r                    = q.Rotate(r);
    r                    = r.Plus(trans);
  } else
    ssassert(false, "Unexpected entity type");
  return r;
}

Vector Entity::FaceGetPointNum() const {
  Vector r;
  if ((type == Type::FACE_NORMAL_PT) || (type == Type::FACE_ROT_NORMAL_PT)) {
    r = SK.GetEntity(point[0])->PointGetNum();
  } else if (type == Type::FACE_XPROD) {
    r = numPoint;
  } else if (type == Type::FACE_N_ROT_TRANS) {
    // The numerical point gets the rotation and translation.
    Vector     trans = VectorFromH(param[0], param[1], param[2]);
    Quaternion q     = Quaternion::From(param[3], param[4], param[5], param[6]);
    r                = q.Rotate(numPoint);
    r                = r.Plus(trans);
  } else if (type == Type::FACE_N_ROT_AXIS_TRANS) {
    Vector offset   = VectorFromH(param[0], param[1], param[2]);
    Vector displace = VectorFromH(param[4], param[5], param[6])
                          .WithMagnitude(SK.GetParam(param[7])->val)
                          .ScaledBy(timesApplied);
    Quaternion q = PointGetQuaternion();
    r            = numPoint.Minus(offset);
    r            = q.Rotate(r);
    r            = r.Plus(offset).Plus(displace);
  } else if (type == Type::FACE_N_TRANS) {
    Vector trans = VectorFromH(param[0], param[1], param[2]);
    r            = numPoint.Plus(trans.ScaledBy(timesApplied));
  } else if (type == Type::FACE_N_ROT_AA) {
    Vector     trans = VectorFromH(param[0], param[1], param[2]);
    Quaternion q     = GetAxisAngleQuaternion(3);
    r                = numPoint.Minus(trans);
    r                = q.Rotate(r);
    r                = r.Plus(trans);
  } else
    ssassert(false, "Unexpected entity type");
  return r;
}

bool Entity::HasEndpoints() const {
  return (type == Type::LINE_SEGMENT) || (type == Type::CUBIC) || (type == Type::ARC_OF_CIRCLE);
}
Vector Entity::EndpointStart() const {
  if (type == Type::LINE_SEGMENT) {
    return SK.GetEntity(point[0])->PointGetNum();
  } else if (type == Type::CUBIC) {
    return CubicGetStartNum();
  } else if (type == Type::ARC_OF_CIRCLE) {
    return SK.GetEntity(point[1])->PointGetNum();
  } else
    ssassert(false, "Unexpected entity type");
}
Vector Entity::EndpointFinish() const {
  if (type == Type::LINE_SEGMENT) {
    return SK.GetEntity(point[1])->PointGetNum();
  } else if (type == Type::CUBIC) {
    return CubicGetFinishNum();
  } else if (type == Type::ARC_OF_CIRCLE) {
    return SK.GetEntity(point[2])->PointGetNum();
  } else
    ssassert(false, "Unexpected entity type");
}
static bool PointInPlane(hEntity h, Vector norm, double distance) {
  Vector p = SK.GetEntity(h)->PointGetNum();
  return (std::fabs(norm.Dot(p) - distance) < LENGTH_EPS);
}
bool Entity::IsInPlane(Vector norm, double distance) const {
  switch (type) {
  case Type::LINE_SEGMENT: {
    return PointInPlane(point[0], norm, distance) && PointInPlane(point[1], norm, distance);
  }
  case Type::CUBIC:
  case Type::CUBIC_PERIODIC: {
    bool periodic = type == Type::CUBIC_PERIODIC;
    int  n        = periodic ? 3 + extraPoints : extraPoints;
    int  i;
    for (i = 0; i < n; i++) {
      if (!PointInPlane(point[i], norm, distance))
        return false;
    }
    return true;
  }

  case Type::CIRCLE:
  case Type::ARC_OF_CIRCLE: {
    // If it is an (arc of) a circle, check whether the normals
    // are parallel and the mid point is in the plane.
    Vector n = Normal()->NormalN();
    if (!norm.Equals(n) && !norm.Equals(n.Negated()))
      return false;
    return PointInPlane(point[0], norm, distance);
  }

  case Type::TTF_TEXT: {
    Vector n = Normal()->NormalN();
    if (!norm.Equals(n) && !norm.Equals(n.Negated()))
      return false;
    return PointInPlane(point[0], norm, distance) && PointInPlane(point[1], norm, distance);
  }

  default: return false;
  }
}

void Entity::RectGetPointsExprs(ExprVector *eb, ExprVector *ec) const {
  ssassert(type == Type::TTF_TEXT || type == Type::IMAGE, "Unexpected entity type");

  Entity *a = SK.GetEntity(point[0]);
  Entity *o = SK.GetEntity(point[1]);

  // Write equations for each point in the current workplane.
  // This reduces the complexity of resulting equations.
  ExprVector ea = a->PointGetExprsInWorkplane(workplane);
  ExprVector eo = o->PointGetExprsInWorkplane(workplane);

  // Take perpendicular vector and scale it by aspect ratio.
  ExprVector eu = ea.Minus(eo);
  ExprVector ev = ExprVector::From(eu.y, eu.x->Negate(), eu.z).ScaledBy(Expr::From(aspectRatio));

  *eb = eo.Plus(ev);
  *ec = eo.Plus(eu).Plus(ev);
}

void Entity::AddEq(IdList<Equation, hEquation> *l, Expr *expr, int index) const {
  Equation eq;
  eq.e = expr;
  eq.h = h.equation(index);
  l->Add(&eq);
}

void Entity::GenerateEquations(IdList<Equation, hEquation> *l) const {
  switch (type) {
  case Type::NORMAL_IN_3D: {
    ExprQuaternion q = NormalGetExprs();
    AddEq(l, (q.Magnitude())->Minus(Expr::From(1)), 0);
    break;
  }

  case Type::ARC_OF_CIRCLE: {
    // If this is a copied entity, with its point already fixed
    // with respect to each other, then we don't want to generate
    // the distance constraint!
    if (SK.GetEntity(point[0])->type != Type::POINT_IN_2D)
      break;

    // If the two endpoints of the arc are constrained coincident
    // (to make a complete circle), then our distance constraint
    // would be redundant and therefore overconstrain things.
    auto it = std::find_if(SK.constraint.begin(), SK.constraint.end(), [&](Constraint const &con) {
      return (con.group == group) && (con.type == Constraint::Type::POINTS_COINCIDENT) &&
             ((con.ptA == point[1] && con.ptB == point[2]) ||
              (con.ptA == point[2] && con.ptB == point[1]));
    });
    if (it != SK.constraint.end()) {
      break;
    }

    Expr *ra = Constraint::Distance(workplane, point[0], point[1]);
    Expr *rb = Constraint::Distance(workplane, point[0], point[2]);
    AddEq(l, ra->Minus(rb), 0);
    break;
  }

  case Type::IMAGE:
  case Type::TTF_TEXT: {
    if (SK.GetEntity(point[0])->type != Type::POINT_IN_2D)
      break;
    Entity    *b  = SK.GetEntity(point[2]);
    Entity    *c  = SK.GetEntity(point[3]);
    ExprVector eb = b->PointGetExprsInWorkplane(workplane);
    ExprVector ec = c->PointGetExprsInWorkplane(workplane);

    ExprVector ebp, ecp;
    RectGetPointsExprs(&ebp, &ecp);

    ExprVector beq = eb.Minus(ebp);
    AddEq(l, beq.x, 0);
    AddEq(l, beq.y, 1);
    ExprVector ceq = ec.Minus(ecp);
    AddEq(l, ceq.x, 2);
    AddEq(l, ceq.y, 3);
    break;
  }

  default: // Most entities do not generate equations.
    break;
  }
}

//-----------------------------------------------------------------------------
// Draw a representation of an entity on-screen, in the case of curves up
// to our chord tolerance, or return the distance from the user's mouse pointer
// to the entity for selection.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------

std::string Entity::DescriptionString() const {
  if (h.isFromRequest()) {
    Request *r = SK.GetRequest(h.request());
    return r->DescriptionString();
  } else {
    Group *g = SK.GetGroup(h.group());
    return g->DescriptionString();
  }
}

void Entity::GenerateEdges(SEdgeList *el) {
  SBezierList *sbl = GetOrGenerateBezierCurves();

  for (int i = 0; i < sbl->l.n; i++) {
    SBezier *sb = &(sbl->l[i]);

    List<Vector> lv = {};
    sb->MakePwlInto(&lv);
    for (int j = 1; j < lv.n; j++) {
      el->AddEdge(lv[j - 1], lv[j], Style::ForEntity(h).v, i);
    }
    lv.Clear();
  }
}

SBezierList *Entity::GetOrGenerateBezierCurves() {
  if (beziers.l.IsEmpty())
    GenerateBezierCurves(&beziers);
  return &beziers;
}

SEdgeList *Entity::GetOrGenerateEdges() {
  if (!edges.l.IsEmpty()) {
    if (EXACT(edgesChordTol == SS.ChordTolMm()))
      return &edges;
    edges.l.Clear();
  }
  if (edges.l.IsEmpty())
    GenerateEdges(&edges);
  edgesChordTol = SS.ChordTolMm();
  return &edges;
}

BBox Entity::GetOrGenerateScreenBBox(bool *hasBBox) {
  SBezierList *sbl = GetOrGenerateBezierCurves();

  // We don't bother with bounding boxes for workplanes, etc.
  *hasBBox = (IsPoint() || IsNormal() || !sbl->l.IsEmpty());
  if (!*hasBBox)
    return {};

  if (screenBBoxValid)
    return screenBBox;

  if (IsPoint()) {
    Vector proj = SS.GW.ProjectPoint3(PointGetNum());
    screenBBox  = BBox::From(proj, proj);
  } else if (IsNormal()) {
    Vector proj = SS.GW.ProjectPoint3(SK.GetEntity(point[0])->PointGetNum());
    screenBBox  = BBox::From(proj, proj);
  } else if (!sbl->l.IsEmpty()) {
    Vector first = SS.GW.ProjectPoint3(sbl->l[0].ctrl[0]);
    screenBBox   = BBox::From(first, first);
    for (auto &sb : sbl->l) {
      for (int i = 0; i <= sb.deg; ++i) {
        screenBBox.Include(SS.GW.ProjectPoint3(sb.ctrl[i]));
      }
    }
  } else
    ssassert(false, "Expected entity to be a point or have beziers");

  screenBBoxValid = true;
  return screenBBox;
}

void Entity::GetReferencePoints(std::vector<Vector> *refs) {
  switch (type) {
  case Type::POINT_N_COPY:
  case Type::POINT_N_TRANS:
  case Type::POINT_N_ROT_TRANS:
  case Type::POINT_N_ROT_AA:
  case Type::POINT_N_ROT_AXIS_TRANS:
  case Type::POINT_IN_3D:
  case Type::POINT_IN_2D: refs->push_back(PointGetDrawNum()); break;

  case Type::NORMAL_N_COPY:
  case Type::NORMAL_N_ROT:
  case Type::NORMAL_N_ROT_AA:
  case Type::NORMAL_IN_3D:
  case Type::NORMAL_IN_2D:
  case Type::WORKPLANE:
  case Type::CIRCLE:
  case Type::ARC_OF_CIRCLE:
  case Type::CUBIC:
  case Type::CUBIC_PERIODIC:
  case Type::TTF_TEXT:
  case Type::IMAGE: refs->push_back(SK.GetEntity(point[0])->PointGetDrawNum()); break;

  case Type::LINE_SEGMENT: {
    Vector a = SK.GetEntity(point[0])->PointGetDrawNum(),
           b = SK.GetEntity(point[1])->PointGetDrawNum();
    refs->push_back(b.Plus(a.Minus(b).ScaledBy(0.5)));
    break;
  }

  case Type::DISTANCE:
  case Type::DISTANCE_N_COPY:
  case Type::FACE_NORMAL_PT:
  case Type::FACE_XPROD:
  case Type::FACE_N_ROT_TRANS:
  case Type::FACE_N_TRANS:
  case Type::FACE_N_ROT_AA:
  case Type::FACE_ROT_NORMAL_PT:
  case Type::FACE_N_ROT_AXIS_TRANS: break;
  }
}

int Entity::GetPositionOfPoint(const Camera &camera, Point2d p) {
  int position;

  ObjectPicker canvas = {};
  canvas.camera       = camera;
  canvas.point        = p;
  canvas.minDistance  = 1e12;
  Draw(DrawAs::DEFAULT, &canvas);
  position = canvas.position;
  canvas.Clear();

  return position;
}

bool Entity::IsStylable() const {
  if (IsPoint())
    return false;
  if (IsWorkplane())
    return false;
  if (IsNormal())
    return false;
  return true;
}

bool Entity::IsVisible() const {
  Group *g = SK.GetGroup(group);

  if (g->h == Group::HGROUP_REFERENCES && IsNormal()) {
    // The reference normals are always shown
    return true;
  }
  if (!(g->IsVisible()))
    return false;

  if (IsPoint() && !SS.GW.showPoints)
    return false;
  if (IsNormal() && !SS.GW.showNormals)
    return false;
  if (construction && !SS.GW.showConstruction)
    return false;

  if (!SS.GW.showWorkplanes) {
    if (IsWorkplane() && !h.isFromRequest()) {
      if (g->h != SS.GW.activeGroup) {
        // The group-associated workplanes are hidden outside
        // their group.
        return false;
      }
    }
  }

  if (style.v) {
    Style *s = Style::Get(style);
    if (!s->visible)
      return false;
  }

  if (forceHidden)
    return false;

  return true;
}

// entities that were created via some copy types will not be
// draggable with the mouse. We identify the undraggables here
bool Entity::CanBeDragged() const {
  // a numeric copy can not move
  if (type == Entity::Type::POINT_N_COPY)
    return false;
  // these transforms applied zero times can not be moved
  if (((type == Entity::Type::POINT_N_TRANS) || (type == Entity::Type::POINT_N_ROT_AA) ||
       (type == Entity::Type::POINT_N_ROT_AXIS_TRANS)) &&
      (timesApplied == 0))
    return false;
  // for these types of entities the first point will indicate draggability
  if (HasEndpoints() || type == Entity::Type::CIRCLE) {
    return SK.GetEntity(point[0])->CanBeDragged();
  }
  // if we're not certain it can't be dragged then default to true
  return true;
}

void Entity::CalculateNumerical(bool forExport) {
  if (IsPoint())
    actPoint = PointGetNum();
  if (IsNormal())
    actNormal = NormalGetNum();
  if (type == Type::DISTANCE || type == Type::DISTANCE_N_COPY) {
    actDistance = DistanceGetNum();
  }
  if (IsFace()) {
    actPoint  = FaceGetPointNum();
    Vector n  = FaceGetNormalNum();
    actNormal = Quaternion::From(0, n.x, n.y, n.z);
  }
  if (forExport) {
    // Visibility in copied linked entities follows source file
    actVisible = IsVisible();
  } else {
    // Copied entities within a file are always visible
    actVisible = true;
  }
}

//-----------------------------------------------------------------------------
// Compute a cubic, second derivative continuous, interpolating spline. Same
// routine for periodic splines (in a loop) or open splines (with specified
// end tangents).
//-----------------------------------------------------------------------------
void Entity::ComputeInterpolatingSpline(SBezierList *sbl, bool periodic) const {
  static const int MAX_N = BandedMatrix::MAX_UNKNOWNS;
  int              ep    = extraPoints;

  // The number of unknowns to solve for.
  int n = periodic ? 3 + ep : ep;
  ssassert(n < MAX_N, "Too many unknowns");
  // The number of on-curve points, one more than the number of segments.
  int pts = periodic ? 4 + ep : 2 + ep;

  int i, j, a;

  // The starting and finishing control points that define our end tangents
  // (if the spline isn't periodic), and the on-curve points.
  Vector ctrl_s = Vector::From(0, 0, 0);
  Vector ctrl_f = Vector::From(0, 0, 0);
  Vector pt[MAX_N + 4];
  if (periodic) {
    for (i = 0; i < ep + 3; i++) {
      pt[i] = SK.GetEntity(point[i])->PointGetNum();
    }
    pt[i++] = SK.GetEntity(point[0])->PointGetNum();
  } else {
    ctrl_s  = SK.GetEntity(point[1])->PointGetNum();
    ctrl_f  = SK.GetEntity(point[ep + 2])->PointGetNum();
    j       = 0;
    pt[j++] = SK.GetEntity(point[0])->PointGetNum();
    for (i = 2; i <= ep + 1; i++) {
      pt[j++] = SK.GetEntity(point[i])->PointGetNum();
    }
    pt[j++] = SK.GetEntity(point[ep + 3])->PointGetNum();
  }

  // The unknowns that we will be solving for, a set for each coordinate.
  double Xx[MAX_N], Xy[MAX_N], Xz[MAX_N];
  // For a cubic Bezier section f(t) as t goes from 0 to 1,
  //    f' (0) = 3*(P1 - P0)
  //    f' (1) = 3*(P3 - P2)
  //    f''(0) = 6*(P0 - 2*P1 + P2)
  //    f''(1) = 6*(P3 - 2*P2 + P1)
  for (a = 0; a < 3; a++) {
    BandedMatrix bm = {};
    bm.n            = n;

    for (i = 0; i < n; i++) {
      int im, it, ip;
      if (periodic) {
        im = WRAP(i - 1, n);
        it = i;
        ip = WRAP(i + 1, n);
      } else {
        im = i;
        it = i + 1;
        ip = i + 2;
      }
      // All of these are expressed in terms of a constant part, and
      // of X[i-1], X[i], and X[i+1]; so let these be the four
      // components of that vector;
      Vector4 A, B, C, D, E;
      // The on-curve interpolated point
      C = Vector4::From((pt[it]).Element(a), 0, 0, 0);
      // control point one back, C - X[i]
      B = C.Plus(Vector4::From(0, 0, -1, 0));
      // control point one forward, C + X[i]
      D = C.Plus(Vector4::From(0, 0, 1, 0));
      // control point two back
      if (i == 0 && !periodic) {
        A = Vector4::From(ctrl_s.Element(a), 0, 0, 0);
      } else {
        // pt[im] + X[i-1]
        A = Vector4::From(pt[im].Element(a), 1, 0, 0);
      }
      // control point two forward
      if (i == (n - 1) && !periodic) {
        E = Vector4::From(ctrl_f.Element(a), 0, 0, 0);
      } else {
        // pt[ip] - X[i+1]
        E = Vector4::From((pt[ip]).Element(a), 0, 0, -1);
      }
      // Write the second derivatives of each segment, dropping constant
      Vector4 fprev_pp = (C.Minus(B.ScaledBy(2))).Plus(A),
              fnext_pp = (C.Minus(D.ScaledBy(2))).Plus(E), eq = fprev_pp.Minus(fnext_pp);

      bm.B[i] = -eq.w;
      if (periodic) {
        bm.A[i][WRAP(i - 2, n)] = eq.x;
        bm.A[i][WRAP(i - 1, n)] = eq.y;
        bm.A[i][i]              = eq.z;
      } else {
        // The wrapping would work, except when n = 1 and everything
        // wraps to zero...
        if (i > 0) {
          bm.A[i][i - 1] = eq.x;
        }
        bm.A[i][i] = eq.y;
        if (i < (n - 1)) {
          bm.A[i][i + 1] = eq.z;
        }
      }
    }
    bm.Solve();
    double *X = (a == 0) ? Xx : (a == 1) ? Xy : Xz;
    memcpy(X, bm.X, n * sizeof(double));
  }

  for (i = 0; i < pts - 1; i++) {
    Vector p0, p1, p2, p3;
    if (periodic) {
      p0     = pt[i];
      int iw = WRAP(i - 1, n);
      p1     = p0.Plus(Vector::From(Xx[iw], Xy[iw], Xz[iw]));
    } else if (i == 0) {
      p0 = pt[0];
      p1 = ctrl_s;
    } else {
      p0 = pt[i];
      p1 = p0.Plus(Vector::From(Xx[i - 1], Xy[i - 1], Xz[i - 1]));
    }
    if (periodic) {
      p3     = pt[i + 1];
      int iw = WRAP(i, n);
      p2     = p3.Minus(Vector::From(Xx[iw], Xy[iw], Xz[iw]));
    } else if (i == (pts - 2)) {
      p3 = pt[pts - 1];
      p2 = ctrl_f;
    } else {
      p3 = pt[i + 1];
      p2 = p3.Minus(Vector::From(Xx[i], Xy[i], Xz[i]));
    }
    SBezier sb = SBezier::From(p0, p1, p2, p3);
    sbl->l.Add(&sb);
  }
}

void Entity::GenerateBezierCurves(SBezierList *sbl) const {
  SBezier sb;

  int i = sbl->l.n;

  switch (type) {
  case Type::LINE_SEGMENT: {
    Vector a  = SK.GetEntity(point[0])->PointGetNum();
    Vector b  = SK.GetEntity(point[1])->PointGetNum();
    sb        = SBezier::From(a, b);
    sb.entity = h.v;
    sbl->l.Add(&sb);
    break;
  }
  case Type::CUBIC: ComputeInterpolatingSpline(sbl, /*periodic=*/false); break;

  case Type::CUBIC_PERIODIC: ComputeInterpolatingSpline(sbl, /*periodic=*/true); break;

  case Type::CIRCLE:
  case Type::ARC_OF_CIRCLE: {
    Vector     center = SK.GetEntity(point[0])->PointGetNum();
    Quaternion q      = SK.GetEntity(normal)->NormalGetNum();
    Vector     u = q.RotationU(), v = q.RotationV();
    double     r = CircleGetRadiusNum();
    double     thetaa, thetab, dtheta;

    if (r < LENGTH_EPS) {
      // If a circle or an arc gets dragged through zero radius,
      // then we just don't generate anything.
      break;
    }

    if (type == Type::CIRCLE) {
      thetaa = 0;
      thetab = 2 * PI;
      dtheta = 2 * PI;
    } else {
      ArcGetAngles(&thetaa, &thetab, &dtheta);
    }
    int i, n;
    if (dtheta > (3 * PI / 2 + 0.01)) {
      n = 4;
    } else if (dtheta > (PI + 0.01)) {
      n = 3;
    } else if (dtheta > (PI / 2 + 0.01)) {
      n = 2;
    } else {
      n = 1;
    }
    dtheta /= n;

    for (i = 0; i < n; i++) {
      double s, c;

      c = cos(thetaa);
      s = sin(thetaa);
      // The start point of the curve, and the tangent vector at
      // that start point.
      Vector p0 = center.Plus(u.ScaledBy(r * c)).Plus(v.ScaledBy(r * s)),
             t0 = u.ScaledBy(-r * s).Plus(v.ScaledBy(r * c));

      thetaa += dtheta;

      c         = cos(thetaa);
      s         = sin(thetaa);
      Vector p2 = center.Plus(u.ScaledBy(r * c)).Plus(v.ScaledBy(r * s)),
             t2 = u.ScaledBy(-r * s).Plus(v.ScaledBy(r * c));

      // The control point must lie on both tangents.
      VectorAtIntersectionOfLines_ret eeep =
          VectorAtIntersectionOfLines(p0, p0.Plus(t0), p2, p2.Plus(t2), NULL);

      SBezier sb   = SBezier::From(p0, eeep.intersectionPoint, p2);
      sb.weight[1] = cos(dtheta / 2);
      sbl->l.Add(&sb);
    }
    break;
  }

  case Type::TTF_TEXT: {
    Vector topLeft = SK.GetEntity(point[0])->PointGetNum();
    Vector botLeft = SK.GetEntity(point[1])->PointGetNum();
    Vector n       = Normal()->NormalN();
    Vector v       = topLeft.Minus(botLeft);
    Vector u       = (v.Cross(n)).WithMagnitude(v.Magnitude());

    SS.fonts.PlotString(font, str, sbl, botLeft, u, v);
    break;
  }

  default:
    // Not a problem, points and normals and such don't generate curves
    break;
  }

  // Record our style for all of the Beziers that we just created.
  for (; i < sbl->l.n; i++) {
    sbl->l[i].auxA = Style::ForEntity(h).v;
  }
}

bool Entity::ShouldDrawExploded() const {
  return SK.GetGroup(group)->ShouldDrawExploded();
}

Vector Entity::ExplodeOffset() const {
  if (ShouldDrawExploded() && workplane.v != 0) {
    int    requestIdx = SK.GetRequest(h.request())->groupRequestIndex;
    double offset     = SS.explodeDistance * (requestIdx + 1);
    return SK.GetEntity(workplane)->Normal()->NormalN().ScaledBy(offset);
  } else {
    return Vector::From(0, 0, 0);
  }
}

Vector Entity::PointGetDrawNum() const {
  // As per Entity::PointGetNum but specifically for when drawing/rendering the point
  // (and not when solving), so we can potentially draw it somewhere different
  return PointGetNum().Plus(ExplodeOffset());
}

void Entity::Draw(DrawAs how, Canvas *canvas) {
  if (!(how == DrawAs::HOVERED || how == DrawAs::SELECTED) && !IsVisible())
    return;

  int zIndex;
  if (IsPoint()) {
    zIndex = 6;
  } else if (how == DrawAs::HIDDEN) {
    zIndex = 2;
  } else if (group != SS.GW.activeGroup) {
    zIndex = 3;
  } else {
    zIndex = 5;
  }

  hStyle hs;
  if (IsPoint()) {
    hs.v = Style::DATUM;
  } else if (IsNormal() || type == Type::WORKPLANE) {
    hs.v = Style::NORMALS;
  } else {
    hs = Style::ForEntity(h);
    if (hs.v == Style::CONSTRUCTION) {
      zIndex = 4;
    }
  }

  Canvas::Stroke stroke = Style::Stroke(hs);
  switch (how) {
  case DrawAs::DEFAULT: stroke.layer = Canvas::Layer::NORMAL; break;

  case DrawAs::OVERLAY: stroke.layer = Canvas::Layer::FRONT; break;

  case DrawAs::HIDDEN:
    stroke.layer          = Canvas::Layer::OCCLUDED;
    stroke.stipplePattern = Style::PatternType({Style::HIDDEN_EDGE});
    stroke.stippleScale   = Style::Get({Style::HIDDEN_EDGE})->stippleScale;
    break;

  case DrawAs::HOVERED:
    stroke.layer = Canvas::Layer::FRONT;
    stroke.color = Style::Color(Style::HOVERED);
    break;

  case DrawAs::SELECTED:
    stroke.layer = Canvas::Layer::FRONT;
    stroke.color = Style::Color(Style::SELECTED);
    break;
  }
  stroke.zIndex       = zIndex;
  Canvas::hStroke hcs = canvas->GetStroke(stroke);

  switch (type) {
  case Type::POINT_N_COPY:
  case Type::POINT_N_TRANS:
  case Type::POINT_N_ROT_TRANS:
  case Type::POINT_N_ROT_AA:
  case Type::POINT_N_ROT_AXIS_TRANS:
  case Type::POINT_IN_3D:
  case Type::POINT_IN_2D: {
    if (how == DrawAs::HIDDEN)
      return;

    // If we're analyzing the sketch to show the degrees of freedom,
    // then we draw big colored squares over the points that are
    // free to move.
    bool free = false;
    if (type == Type::POINT_IN_3D) {
      Param *px = SK.GetParam(param[0]), *py = SK.GetParam(param[1]), *pz = SK.GetParam(param[2]);

      free = px->free || py->free || pz->free;
    } else if (type == Type::POINT_IN_2D) {
      Param *pu = SK.GetParam(param[0]), *pv = SK.GetParam(param[1]);

      free = pu->free || pv->free;
    }

    Canvas::Stroke pointStroke = {};
    pointStroke.layer          = (free) ? Canvas::Layer::FRONT : stroke.layer;
    pointStroke.zIndex         = stroke.zIndex;
    pointStroke.color          = stroke.color;
    pointStroke.width          = 7.0;
    pointStroke.unit           = Canvas::Unit::PX;
    Canvas::hStroke hcsPoint   = canvas->GetStroke(pointStroke);

    Vector p = PointGetDrawNum();
    if (free) {
      Canvas::Stroke analyzeStroke = Style::Stroke(Style::ANALYZE);
      analyzeStroke.width          = 14.0;
      analyzeStroke.layer          = Canvas::Layer::FRONT;
      Canvas::hStroke hcsAnalyze   = canvas->GetStroke(analyzeStroke);

      canvas->DrawPoint(p, hcsAnalyze);
    }

    canvas->DrawPoint(p, hcsPoint);
    return;
  }

  case Type::NORMAL_N_COPY:
  case Type::NORMAL_N_ROT:
  case Type::NORMAL_N_ROT_AA:
  case Type::NORMAL_IN_3D:
  case Type::NORMAL_IN_2D: {
    const Camera &camera = canvas->GetCamera();

    if (how == DrawAs::HIDDEN)
      return;

    for (int i = 0; i < 2; i++) {
      bool asReference = (i == 1);
      if (asReference) {
        if (!h.request().IsFromReferences())
          continue;
      } else {
        if (!SK.GetGroup(group)->IsVisible() || !SS.GW.showNormals)
          continue;
      }

      stroke.layer = (asReference) ? Canvas::Layer::FRONT : Canvas::Layer::NORMAL;
      if (how != DrawAs::HOVERED && how != DrawAs::SELECTED) {
        // Always draw the x, y, and z axes in red, green, and blue;
        // brighter for the ones at the bottom left of the screen,
        // dimmer for the ones at the model origin.
        hRequest hr   = h.request();
        uint8_t  luma = (asReference) ? 255 : 100;
        if (hr == Request::HREQUEST_REFERENCE_XY) {
          stroke.color = RgbaColor::From(0, 0, luma);
        } else if (hr == Request::HREQUEST_REFERENCE_YZ) {
          stroke.color = RgbaColor::From(luma, 0, 0);
        } else if (hr == Request::HREQUEST_REFERENCE_ZX) {
          stroke.color = RgbaColor::From(0, luma, 0);
        }
      }

      Quaternion q = NormalGetNum();
      Vector     tail;
      if (asReference) {
        // Draw an extra copy of the x, y, and z axes, that's
        // always in the corner of the view and at the front.
        // So those are always available, perhaps useful.
        stroke.width = 2;
        double s     = camera.scale;
        double h     = 60 - camera.height / 2.0;
        double w     = 60 - camera.width / 2.0;

        tail = camera.projRight.ScaledBy(w / s)
                   .Plus(camera.projUp.ScaledBy(h / s))
                   .Minus(camera.offset);
      } else {
        tail = SK.GetEntity(point[0])->PointGetDrawNum();
      }
      tail = camera.AlignToPixelGrid(tail);

      hcs        = canvas->GetStroke(stroke);
      Vector v   = (q.RotationN()).WithMagnitude(50.0 / camera.scale);
      Vector tip = tail.Plus(v);
      canvas->DrawLine(tail, tip, hcs);

      v           = v.WithMagnitude(12.0 / camera.scale);
      Vector axis = q.RotationV();
      canvas->DrawLine(tip, tip.Minus(v.RotatedAbout(axis, 0.6)), hcs);
      canvas->DrawLine(tip, tip.Minus(v.RotatedAbout(axis, -0.6)), hcs);

      if (type == Type::NORMAL_IN_3D) {
        Param *nw = SK.GetParam(param[0]), *nx = SK.GetParam(param[1]), *ny = SK.GetParam(param[2]),
              *nz = SK.GetParam(param[3]);

        if (nw->free || nx->free || ny->free || nz->free) {
          Canvas::Stroke analyzeStroke = Style::Stroke(Style::ANALYZE);
          analyzeStroke.layer          = Canvas::Layer::FRONT;
          Canvas::hStroke hcsAnalyze   = canvas->GetStroke(analyzeStroke);
          canvas->DrawLine(tail, tip, hcsAnalyze);
        }
      }
    }
    return;
  }

  case Type::DISTANCE:
  case Type::DISTANCE_N_COPY:
    // These are used only as data structures, nothing to display.
    return;

  case Type::WORKPLANE: {
    const Camera &camera = canvas->GetCamera();

    Vector p = SK.GetEntity(point[0])->PointGetNum();
    p        = camera.AlignToPixelGrid(p);

    Vector u = Normal()->NormalU();
    Vector v = Normal()->NormalV();

    double s = (std::min(camera.width, camera.height)) * 0.45 / camera.scale;

    Vector us = u.ScaledBy(s);
    Vector vs = v.ScaledBy(s);

    Vector pp = p.Plus(us).Plus(vs);
    Vector pm = p.Plus(us).Minus(vs);
    Vector mm = p.Minus(us).Minus(vs), mm2 = mm;
    Vector mp = p.Minus(us).Plus(vs);

    Canvas::Stroke strokeBorder = stroke;
    strokeBorder.zIndex -= 3;
    strokeBorder.stipplePattern = StipplePattern::SHORT_DASH;
    strokeBorder.stippleScale   = 8.0;
    Canvas::hStroke hcsBorder   = canvas->GetStroke(strokeBorder);

    double textHeight = Style::TextHeight(hs) / camera.scale;

    if (!h.isFromRequest()) {
      mm  = mm.Plus(v.ScaledBy(textHeight * 4.7));
      mm2 = mm2.Plus(u.ScaledBy(textHeight * 4.7));
      canvas->DrawLine(mm2, mm, hcsBorder);
    }
    canvas->DrawLine(pp, pm, hcsBorder);
    canvas->DrawLine(mm2, pm, hcsBorder);
    canvas->DrawLine(mm, mp, hcsBorder);
    canvas->DrawLine(pp, mp, hcsBorder);

    Vector      o = mm2.Plus(u.ScaledBy(3.0 / camera.scale)).Plus(v.ScaledBy(3.0 / camera.scale));
    std::string shortDesc = DescriptionString().substr(5);
    canvas->DrawVectorText(shortDesc, textHeight, o, u, v, hcs);
    return;
  }

  case Type::LINE_SEGMENT:
  case Type::CIRCLE:
  case Type::ARC_OF_CIRCLE:
  case Type::CUBIC:
  case Type::CUBIC_PERIODIC:
  case Type::TTF_TEXT: {
    // Generate the rational polynomial curves, then piecewise linearize
    // them, and display those.
    // Calculating the draw offset, if necessary.
    const bool   shouldExplode = ShouldDrawExploded();
    Vector       explodeOffset;
    SBezierList  offsetBeziers = {};
    SBezierList *beziers       = GetOrGenerateBezierCurves();
    if (shouldExplode) {
      explodeOffset = ExplodeOffset();
      for (const SBezier &b : beziers->l) {
        SBezier offset = b.TransformedBy(explodeOffset, Quaternion::IDENTITY, 1.0);
        offsetBeziers.l.Add(&offset);
      }
      beziers = &offsetBeziers;
    }

    SEdgeList *edges       = nullptr;
    SEdgeList  offsetEdges = {};

    if (!canvas->DrawBeziers(*beziers, hcs)) {
      edges = GetOrGenerateEdges();
      if (shouldExplode) {
        for (const SEdge &e : edges->l) {
          offsetEdges.AddEdge(e.a.Plus(explodeOffset), e.b.Plus(explodeOffset), e.auxA, e.auxB,
                              e.tag);
        }
        edges = &offsetEdges;
      }
      canvas->DrawEdges(*edges, hcs);
    }
    if (type == Type::CIRCLE) {
      Entity *dist = SK.GetEntity(distance);
      if (dist->type == Type::DISTANCE) {
        Param *p = SK.GetParam(dist->param[0]);
        if (p->free) {
          Canvas::Stroke analyzeStroke = Style::Stroke(Style::ANALYZE);
          analyzeStroke.layer          = Canvas::Layer::FRONT;
          Canvas::hStroke hcsAnalyze   = canvas->GetStroke(analyzeStroke);
          if (!canvas->DrawBeziers(*beziers, hcsAnalyze)) {
            canvas->DrawEdges(*edges, hcsAnalyze);
          }
        }
      }
    }
    offsetBeziers.Clear();
    offsetEdges.Clear();
    return;
  }
  case Type::IMAGE: {
    Canvas::Fill            fill = {};
    std::shared_ptr<Pixmap> pixmap;
    switch (how) {
    case DrawAs::HIDDEN: return;

    case DrawAs::HOVERED: {
      fill.color   = Style::Color(Style::HOVERED).WithAlpha(180);
      fill.pattern = Canvas::FillPattern::CHECKERED_A;
      fill.zIndex  = 2;
      break;
    }

    case DrawAs::SELECTED: {
      fill.color   = Style::Color(Style::SELECTED).WithAlpha(180);
      fill.pattern = Canvas::FillPattern::CHECKERED_B;
      fill.zIndex  = 1;
      break;
    }

    default:
      fill.color = RgbaColor::FromFloat(1.0f, 1.0f, 1.0f);
      pixmap     = SS.images[file];
      break;
    }

    Canvas::hFill hf   = canvas->GetFill(fill);
    Vector        v[4] = {Vector(0, 0, 0), Vector(0, 0, 0), Vector(0, 0, 0), Vector(0, 0, 0)};
    for (int i = 0; i < 4; i++) {
      v[i] = SK.GetEntity(point[i])->PointGetDrawNum();
    }
    Vector iu = v[3].Minus(v[0]);
    Vector iv = v[1].Minus(v[0]);

    if (how == DrawAs::DEFAULT && pixmap == NULL) {
      Canvas::Stroke stroke = Style::Stroke(Style::DRAW_ERROR);
      stroke.color          = stroke.color.WithAlpha(50);
      Canvas::hStroke hs    = canvas->GetStroke(stroke);
      canvas->DrawLine(v[0], v[2], hs);
      canvas->DrawLine(v[1], v[3], hs);
      for (int i = 0; i < 4; i++) {
        canvas->DrawLine(v[i], v[(i + 1) % 4], hs);
      }
    } else {
      canvas->DrawPixmap(pixmap, v[0], iu, iv, Point2d::From(0.0, 0.0), Point2d::From(1.0, 1.0),
                         hf);
    }
  }

  case Type::FACE_NORMAL_PT:
  case Type::FACE_XPROD:
  case Type::FACE_N_ROT_TRANS:
  case Type::FACE_N_TRANS:
  case Type::FACE_N_ROT_AA:
  case Type::FACE_ROT_NORMAL_PT:
  case Type::FACE_N_ROT_AXIS_TRANS:
    // Do nothing; these are drawn with the triangle mesh
    return;
  }
  ssassert(false, "Unexpected entity type");
}
