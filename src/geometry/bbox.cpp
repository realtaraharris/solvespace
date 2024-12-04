#include "solvespace.h"

BBox BBox::From (const Vector &p0, const Vector &p1) {
  BBox bbox;
  bbox.minp.x = std::min (p0.x, p1.x);
  bbox.minp.y = std::min (p0.y, p1.y);
  bbox.minp.z = std::min (p0.z, p1.z);

  bbox.maxp.x = std::max (p0.x, p1.x);
  bbox.maxp.y = std::max (p0.y, p1.y);
  bbox.maxp.z = std::max (p0.z, p1.z);
  return bbox;
}

Vector BBox::GetOrigin () const {
  return minp.Plus (maxp.Minus (minp).ScaledBy (0.5));
}
Vector BBox::GetExtents () const {
  return maxp.Minus (minp).ScaledBy (0.5);
}

void BBox::Include (const Vector &v, double r) {
  minp.x = std::min (minp.x, v.x - r);
  minp.y = std::min (minp.y, v.y - r);
  minp.z = std::min (minp.z, v.z - r);

  maxp.x = std::max (maxp.x, v.x + r);
  maxp.y = std::max (maxp.y, v.y + r);
  maxp.z = std::max (maxp.z, v.z + r);
}

bool BBox::Overlaps (const BBox &b1) const {
  Vector t = b1.GetOrigin ().Minus (GetOrigin ());
  Vector e = b1.GetExtents ().Plus (GetExtents ());

  return std::fabs (t.x) < e.x && std::fabs (t.y) < e.y && std::fabs (t.z) < e.z;
}

bool BBox::Contains (const Point2d &p, double r) const {
  return p.x >= (minp.x - r) && p.y >= (minp.y - r) && p.x <= (maxp.x + r) && p.y <= (maxp.y + r);
}
