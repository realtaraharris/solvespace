#include "solvespace.h"

BBox BBox::From(const Vector &p0, const Vector &p1) {
    BBox bbox;
    bbox.minp.x = min(p0.x, p1.x);
    bbox.minp.y = min(p0.y, p1.y);
    bbox.minp.z = min(p0.z, p1.z);

    bbox.maxp.x = max(p0.x, p1.x);
    bbox.maxp.y = max(p0.y, p1.y);
    bbox.maxp.z = max(p0.z, p1.z);
    return bbox;
}

Vector BBox::GetOrigin() const { return minp.Plus(maxp.Minus(minp).ScaledBy(0.5)); }
Vector BBox::GetExtents() const { return maxp.Minus(minp).ScaledBy(0.5); }

void BBox::Include(const Vector &v, double r) {
    minp.x = min(minp.x, v.x - r);
    minp.y = min(minp.y, v.y - r);
    minp.z = min(minp.z, v.z - r);

    maxp.x = max(maxp.x, v.x + r);
    maxp.y = max(maxp.y, v.y + r);
    maxp.z = max(maxp.z, v.z + r);
}

bool BBox::Overlaps(const BBox &b1) const {
    Vector t = b1.GetOrigin().Minus(GetOrigin());
    Vector e = b1.GetExtents().Plus(GetExtents());

    return fabs(t.x) < e.x && fabs(t.y) < e.y && fabs(t.z) < e.z;
}

bool BBox::Contains(const Point2d &p, double r) const {
    return p.x >= (minp.x - r) &&
           p.y >= (minp.y - r) &&
           p.x <= (maxp.x + r) &&
           p.y <= (maxp.y + r);
}
