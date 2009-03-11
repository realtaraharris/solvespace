#include "solvespace.h"

extern int FLAG;

void SSurface::AddExactIntersectionCurve(SBezier *sb, SSurface *srfB,
                            SShell *agnstA, SShell *agnstB, SShell *into)
{
    SCurve sc;
    ZERO(&sc);
    sc.surfA = h;
    sc.surfB = srfB->h;
    sb->MakePwlInto(&(sc.pts));

    // Now split the line where it intersects our existing surfaces
    SCurve split = sc.MakeCopySplitAgainst(agnstA, agnstB, this, srfB);
    sc.Clear();
   
/*
    if(sb->deg == 1) {
        dbp(" ");
        Vector *prev = NULL, *v;
        dbp("split.pts.n =%d", split.pts.n);
        for(v = split.pts.First(); v; v = split.pts.NextAfter(v)) {
            if(prev) {
                SS.nakedEdges.AddEdge(*prev, *v);
            }
            prev = v;
        }
    } */

    split.source = SCurve::FROM_INTERSECTION;
    into->curve.AddAndAssignId(&split);
}

void SSurface::IntersectAgainst(SSurface *b, SShell *agnstA, SShell *agnstB, 
                                SShell *into)
{
    Vector amax, amin, bmax, bmin;
    GetAxisAlignedBounding(&amax, &amin);
    b->GetAxisAlignedBounding(&bmax, &bmin);

    if(Vector::BoundingBoxesDisjoint(amax, amin, bmax, bmin)) {
        // They cannot possibly intersect, no curves to generate
        return;
    }

    if((degm == 1 && degn == 1 && b->IsExtrusion(NULL, NULL)) ||
       (b->degm == 1 && b->degn == 1 && this->IsExtrusion(NULL, NULL)))
    {
        // The intersection between a plane and a surface of extrusion
        SSurface *splane, *sext;
        if(degm == 1 && degn == 1) {
            splane = this;
            sext = b;
        } else {
            splane = b;
            sext = this;
        }

        Vector n = splane->NormalAt(0, 0).WithMagnitude(1), along;
        double d = n.Dot(splane->PointAt(0, 0));
        SBezier bezier;
        (void)sext->IsExtrusion(&bezier, &along);

        if(fabs(n.Dot(along)) < LENGTH_EPS) {
            // Direction of extrusion is parallel to plane; so intersection
            // is zero or more lines. Build a line within the plane, and
            // normal to the direction of extrusion, and intersect that line
            // against the surface; each intersection point corresponds to
            // a line.
            Vector pm, alu, p0, dp;
            // a point halfway along the extrusion
            pm = ((sext->ctrl[0][0]).Plus(sext->ctrl[0][1])).ScaledBy(0.5);
            alu = along.WithMagnitude(1);
            dp = (n.Cross(along)).WithMagnitude(1);
            // n, alu, and dp form an orthogonal csys; set n component to
            // place it on the plane, alu component to lie halfway along
            // extrusion, and dp component doesn't matter so zero
            p0 = n.ScaledBy(d).Plus(alu.ScaledBy(pm.Dot(alu)));

            List<SInter> inters;
            ZERO(&inters);
            sext->AllPointsIntersecting(p0, p0.Plus(dp), &inters, false, false);
    
            SInter *si;
            for(si = inters.First(); si; si = inters.NextAfter(si)) {
                Vector al = along.ScaledBy(0.5);
                SBezier bezier;
                bezier = SBezier::From((si->p).Minus(al), (si->p).Plus(al));

                AddExactIntersectionCurve(&bezier, b, agnstA, agnstB, into);
            }

            inters.Clear();
        } else {
            // Direction of extrusion is not parallel to plane; so
            // intersection is projection of extruded curve into our plane.
            // If both curves are planes, then we could do it either way;
            // so choose the one that generates the shorter curve.
            // XXX TODO

            int i;
            for(i = 0; i <= bezier.deg; i++) {
                Vector p0 = bezier.ctrl[i],
                       p1 = p0.Plus(along);

                bezier.ctrl[i] =
                    Vector::AtIntersectionOfPlaneAndLine(n, d, p0, p1, NULL);
            }

            AddExactIntersectionCurve(&bezier, b, agnstA, agnstB, into);
        }
    }

    // need to implement general numerical surface intersection for tough
    // cases, just giving up for now
}

void SSurface::WeightControlPoints(void) {
    int i, j;
    for(i = 0; i <= degm; i++) {
        for(j = 0; j <= degn; j++) {
            ctrl[i][j] = (ctrl[i][j]).ScaledBy(weight[i][j]);
        }
    }
}
void SSurface::UnWeightControlPoints(void) {
    int i, j;
    for(i = 0; i <= degm; i++) {
        for(j = 0; j <= degn; j++) {
            ctrl[i][j] = (ctrl[i][j]).ScaledBy(1.0/weight[i][j]);
        }
    }
}
void SSurface::CopyRowOrCol(bool row, int this_ij, SSurface *src, int src_ij) {
    if(row) {
        int j;
        for(j = 0; j <= degn; j++) {
            ctrl  [this_ij][j] = src->ctrl  [src_ij][j];
            weight[this_ij][j] = src->weight[src_ij][j];
        }
    } else {
        int i;
        for(i = 0; i <= degm; i++) {
            ctrl  [i][this_ij] = src->ctrl  [i][src_ij];
            weight[i][this_ij] = src->weight[i][src_ij];
        }
    }
}
void SSurface::BlendRowOrCol(bool row, int this_ij, SSurface *a, int a_ij,
                                                    SSurface *b, int b_ij)
{
    if(row) {
        int j;
        for(j = 0; j <= degn; j++) {
            Vector c = (a->ctrl  [a_ij][j]).Plus(b->ctrl  [b_ij][j]);
            double w = (a->weight[a_ij][j]   +   b->weight[b_ij][j]);
            ctrl  [this_ij][j] = c.ScaledBy(0.5);
            weight[this_ij][j] = w / 2;
        }
    } else {
        int i;
        for(i = 0; i <= degm; i++) {
            Vector c = (a->ctrl  [i][a_ij]).Plus(b->ctrl  [i][b_ij]);
            double w = (a->weight[i][a_ij]   +   b->weight[i][b_ij]);
            ctrl  [i][this_ij] = c.ScaledBy(0.5);
            weight[i][this_ij] = w / 2;
        }
    }
}
void SSurface::SplitInHalf(bool byU, SSurface *sa, SSurface *sb) {
    sa->degm = sb->degm = degm;
    sa->degn = sb->degn = degn;

    // by de Casteljau's algorithm in a projective space; so we must work
    // on points (w*x, w*y, w*z, w)
    WeightControlPoints();

    switch(byU ? degm : degn) {
        case 1:
            sa->CopyRowOrCol (byU, 0, this, 0);
            sb->CopyRowOrCol (byU, 1, this, 1);

            sa->BlendRowOrCol(byU, 1, this, 0, this, 1);
            sb->BlendRowOrCol(byU, 0, this, 0, this, 1);
            break;

        case 2:
            sa->CopyRowOrCol (byU, 0, this, 0);
            sb->CopyRowOrCol (byU, 2, this, 2);

            sa->BlendRowOrCol(byU, 1, this, 0, this, 1);
            sb->BlendRowOrCol(byU, 1, this, 1, this, 2);

            sa->BlendRowOrCol(byU, 2, sa,   1, sb,   1);
            sb->BlendRowOrCol(byU, 0, sa,   1, sb,   1);
            break;

        case 3: {
            SSurface st;
            st.degm = degm; st.degn = degn;

            sa->CopyRowOrCol (byU, 0, this, 0);
            sb->CopyRowOrCol (byU, 3, this, 3);

            sa->BlendRowOrCol(byU, 1, this, 0, this, 1);
            sb->BlendRowOrCol(byU, 2, this, 2, this, 3);
            st. BlendRowOrCol(byU, 0, this, 1, this, 2); // scratch var

            sa->BlendRowOrCol(byU, 2, sa,   1, &st,  0);
            sb->BlendRowOrCol(byU, 1, sb,   2, &st,  0);

            sa->BlendRowOrCol(byU, 3, sa,   2, sb,   1);
            sb->BlendRowOrCol(byU, 0, sa,   2, sb,   1);
            break;
        }

        default: oops();
    }

    sa->UnWeightControlPoints();
    sb->UnWeightControlPoints();
    UnWeightControlPoints();
}

//-----------------------------------------------------------------------------
// Find all points where the indicated finite (if segment) or infinite (if not
// segment) line intersects our surface. Report them in uv space in the list.
// We first do a bounding box check; if the line doesn't intersect, then we're
// done. If it does, then we check how small our surface is. If it's big,
// then we subdivide into quarters and recurse. If it's small, then we refine
// by Newton's method and record the point.
//-----------------------------------------------------------------------------
void SSurface::AllPointsIntersectingUntrimmed(Vector a, Vector b,
                                              int *cnt, int *level,
                                              List<Inter> *l, bool segment,
                                              SSurface *sorig)
{
    // Test if the line intersects our axis-aligned bounding box; if no, then
    // no possibility of an intersection
    Vector amax, amin;
    GetAxisAlignedBounding(&amax, &amin);
    if(!Vector::BoundingBoxIntersectsLine(amax, amin, a, b, segment)) {
        // The line segment could fail to intersect the bbox, but lie entirely
        // within it and intersect the surface.
        if(a.OutsideAndNotOn(amax, amin) && b.OutsideAndNotOn(amax, amin)) {
            return;
        }
    } 
   
    if(*cnt > 2000) {
        dbp("!!! too many subdivisions (level=%d)!", *level);
        return;
    }
    (*cnt)++;

    // If we might intersect, and the surface is small, then switch to Newton
    // iterations.
    double h = max(amax.x - amin.x,
               max(amax.y - amin.y,
                   amax.z - amin.z));
    if(fabs(h) < SS.ChordTolMm()) {
        Vector p = (amax.Plus(amin)).ScaledBy(0.5);
        Inter inter;
        sorig->ClosestPointTo(p, &(inter.p.x), &(inter.p.y), false);
        if(sorig->PointIntersectingLine(a, b, &(inter.p.x), &(inter.p.y))) {
            Vector p = sorig->PointAt(inter.p.x, inter.p.y);
            // Debug check, verify that the point lies in both surfaces
            // (which it ought to, since the surfaces should be coincident)
            double u, v;
            ClosestPointTo(p, &u, &v);
            l->Add(&inter);
        } else {
            // Might not converge if line is almost tangent to surface...
        }
        return;
    }

    // But the surface is big, so split it, alternating by u and v
    SSurface surf0, surf1;
    SplitInHalf((*level & 1) == 0, &surf0, &surf1);

    int nextLevel = (*level) + 1;
    (*level) = nextLevel;
    surf0.AllPointsIntersectingUntrimmed(a, b, cnt, level, l, segment, sorig);
    (*level) = nextLevel;
    surf1.AllPointsIntersectingUntrimmed(a, b, cnt, level, l, segment, sorig);
}

//-----------------------------------------------------------------------------
// Find all points where a line through a and b intersects our surface, and
// add them to the list. If seg is true then report only intersections that
// lie within the finite line segment (not including the endpoints); otherwise
// we work along the infinite line.
//-----------------------------------------------------------------------------
void SSurface::AllPointsIntersecting(Vector a, Vector b,
                                     List<SInter> *l, bool seg, bool trimmed)
{
    Vector ba = b.Minus(a);
    double bam = ba.Magnitude();

    List<Inter> inters;
    ZERO(&inters);

    // First, get all the intersections between the infinite ray and the
    // untrimmed surface.
    int cnt = 0, level = 0;
    AllPointsIntersectingUntrimmed(a, b, &cnt, &level, &inters, seg, this);

    // Remove duplicate intersection points
    inters.ClearTags();
    int i, j;
    for(i = 0; i < inters.n; i++) {
        for(j = i + 1; j < inters.n; j++) {
            if(inters.elem[i].p.Equals(inters.elem[j].p)) {
                inters.elem[j].tag = 1;
            }
        }
    }
    inters.RemoveTagged();

    for(i = 0; i < inters.n; i++) {
        Point2d puv = inters.elem[i].p;

        // Make sure the point lies within the finite line segment
        Vector pxyz = PointAt(puv.x, puv.y);
        double t = (pxyz.Minus(a)).DivPivoting(ba);
        if(seg && (t > 1 - LENGTH_EPS/bam || t < LENGTH_EPS/bam)) {
            continue;
        }
       
        // And that it lies inside our trim region
        Point2d dummy = { 0, 0 };
        int c = bsp->ClassifyPoint(puv, dummy);
        if(trimmed && c == SBspUv::OUTSIDE) {
            continue;
        }

        // It does, so generate the intersection
        SInter si;
        si.p = pxyz;
        si.surfNormal = NormalAt(puv.x, puv.y);
        si.hsrf = h;
        si.srf = this;
        si.onEdge = (c != SBspUv::INSIDE);
        l->Add(&si);
    }

    inters.Clear();
}

void SShell::AllPointsIntersecting(Vector a, Vector b,
                                   List<SInter> *il, bool seg, bool trimmed)
{
    SSurface *ss;
    for(ss = surface.First(); ss; ss = surface.NextAfter(ss)) {
        ss->AllPointsIntersecting(a, b, il, seg, trimmed);
    }
}

//-----------------------------------------------------------------------------
// Does the given point lie on our shell? It might be inside or outside, or
// it might be on the surface with pout parallel or anti-parallel to the
// intersecting surface's normal.
//
// To calculate, we intersect a ray through p with our shell, and classify
// using the closest intersection point. If the ray hits a surface on edge,
// then just reattempt in a different random direction.
//-----------------------------------------------------------------------------
int SShell::ClassifyPoint(Vector p, Vector pout) {
    List<SInter> l;
    ZERO(&l);

    srand(0);

    int ret, cnt = 0, edge_inters;
    double edge_dotp[2];

    for(;;) {
        // Cast a ray in a random direction (two-sided so that we test if
        // the point lies on a surface, but use only one side for in/out
        // testing)
        Vector ray = Vector::From(Random(1), Random(1), Random(1));
        AllPointsIntersecting(p.Minus(ray), p.Plus(ray), &l, false, true);
        
        double dmin = VERY_POSITIVE;
        ret = OUTSIDE; // no intersections means it's outside
        bool onEdge = false;
        edge_inters = 0;

        SInter *si;
        for(si = l.First(); si; si = l.NextAfter(si)) {
            double t = ((si->p).Minus(p)).DivPivoting(ray);
            if(t*ray.Magnitude() < -LENGTH_EPS) {
                // wrong side, doesn't count
                continue;
            }

            double d = ((si->p).Minus(p)).Magnitude();

            // Handle edge-on-edge
            if(d < LENGTH_EPS && si->onEdge && edge_inters < 2) {
                edge_dotp[edge_inters] = (si->surfNormal).Dot(pout);
                edge_inters++;
            }

            if(d < dmin) {
                dmin = d;
                if(d < LENGTH_EPS) {
                    // Edge-on-face (unless edge-on-edge above supercedes)
                    if((si->surfNormal).Dot(pout) > 0) {
                        ret = SURF_PARALLEL;
                    } else {
                        ret = SURF_ANTIPARALLEL;
                    }
                } else {
                    // Edge does not lie on surface; either strictly inside
                    // or strictly outside
                    if((si->surfNormal).Dot(ray) > 0) {
                        ret = INSIDE;
                    } else {
                        ret = OUTSIDE;
                    }
                }
                onEdge = si->onEdge;
            }
        }
        l.Clear();

        // If the point being tested lies exactly on an edge of the shell,
        // then our ray always lies on edge, and that's okay. Otherwise
        // try again in a different random direction.
        if((edge_inters == 2) || !onEdge) break;
        if(cnt++ > 20) {
            dbp("can't find a ray that doesn't hit on edge!");
            break;
        }
    }

    if(edge_inters == 2) {
        double tol = 1e-3;

        if(edge_dotp[0] > -tol && edge_dotp[1] > -tol) {
            return EDGE_PARALLEL;
        } else if(edge_dotp[0] < tol && edge_dotp[1] < tol) {
            return EDGE_ANTIPARALLEL;
        } else {
            return EDGE_TANGENT;
        }
    } else {
        return ret;
    }
}

//-----------------------------------------------------------------------------
// Are two surfaces coincident, with the same (or with opposite) normals?
// Currently handles planes only.
//-----------------------------------------------------------------------------
bool SSurface::CoincidentWith(SSurface *ss, bool sameNormal) {
    if(degm != 1 || degn != 1) return false;
    if(ss->degm != 1 || ss->degn != 1) return false;

    Vector p = ctrl[0][0];
    Vector n = NormalAt(0, 0).WithMagnitude(1);
    double d = n.Dot(p);

    if(!ss->CoincidentWithPlane(n, d)) return false;

    Vector n2 = ss->NormalAt(0, 0);
    if(sameNormal) {
        if(n2.Dot(n) < 0) return false;
    } else {
        if(n2.Dot(n) > 0) return false;
    }

    return true;
}

bool SSurface::CoincidentWithPlane(Vector n, double d) {
    if(degm != 1 || degn != 1) return false;
    if(fabs(n.Dot(ctrl[0][0]) - d) > LENGTH_EPS) return false;
    if(fabs(n.Dot(ctrl[0][1]) - d) > LENGTH_EPS) return false;
    if(fabs(n.Dot(ctrl[1][0]) - d) > LENGTH_EPS) return false;
    if(fabs(n.Dot(ctrl[1][1]) - d) > LENGTH_EPS) return false;
    
    return true;
}

//-----------------------------------------------------------------------------
// In our shell, find all surfaces that are coincident with the prototype
// surface (with same or opposite normal, as specified), and copy all of
// their trim polygons into el. The edges are returned in uv coordinates for
// the prototype surface.
//-----------------------------------------------------------------------------
void SShell::MakeCoincidentEdgesInto(SSurface *proto, bool sameNormal,
                                     SEdgeList *el)
{
    SSurface *ss;
    for(ss = surface.First(); ss; ss = surface.NextAfter(ss)) {
        if(proto->CoincidentWith(ss, sameNormal)) {
            ss->MakeEdgesInto(this, el, false);
        }
    }

    SEdge *se;
    for(se = el->l.First(); se; se = el->l.NextAfter(se)) {
        double ua, va, ub, vb;
        proto->ClosestPointTo(se->a, &ua, &va);
        proto->ClosestPointTo(se->b, &ub, &vb);

        if(sameNormal) {
            se->a = Vector::From(ua, va, 0);
            se->b = Vector::From(ub, vb, 0);
        } else {
            // Flip normal, so flip all edge directions
            se->b = Vector::From(ua, va, 0);
            se->a = Vector::From(ub, vb, 0);
        }
    }
}
