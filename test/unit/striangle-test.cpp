/*
 * Copyright 2024 Tara Harris <3769985+realtaraharris@users.noreply.github.com>
 * All rights reserved. Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include "harness.h"

TEST_CASE(striangle_calc_intersection_true) {
	STriMeta meta = {};
	Vector a = Vector(0.0, 1.0, 0.0);
	Vector b = Vector(1.0, -1.0, 0.0);
	Vector c = Vector(-1.0, -1.0, 0.0);
	STriangle s = STriangle(meta, a, b, c);

	Vector rayPoint = Vector(0.1, 0.1, 1.0);
	Vector rayDir = Vector(0.1, 0.1, -1.0);
	SolveSpace::STriangle::Raytrace_ret r = s.Raytrace(rayPoint, rayDir, true);

	CHECK_TRUE(r.hit);
	CHECK_EQ_EPS(r.t, 1.0);
	CHECK_EQ_EPS(r.inters.x, 0.2);
	CHECK_EQ_EPS(r.inters.y, 0.2);
	CHECK_EQ_EPS(r.inters.z, 0.0);
}

TEST_CASE(striangle_calc_intersection_false) {
	STriMeta meta = {};
	Vector a = Vector(0.0, 1.0, 0.0);
	Vector b = Vector(1.0, -1.0, 0.0);
	Vector c = Vector(-1.0, -1.0, 0.0);
	STriangle s = STriangle(meta, a, b, c);

	Vector rayPoint = Vector(0.1, 0.1, 1.0);
	Vector rayDir = Vector(0.1, 0.1, -1.0);
	SolveSpace::STriangle::Raytrace_ret r = s.Raytrace(rayPoint, rayDir, false);

	CHECK_TRUE(r.hit);
	CHECK_EQ_EPS(r.t, 1.0);
	CHECK_EQ_EPS(r.inters.x, 0.0);
	CHECK_EQ_EPS(r.inters.y, 0.0);
	CHECK_EQ_EPS(r.inters.z, 0.0);
}

TEST_CASE(striangle_contains_point) {
	STriMeta meta = {};
	Vector a = Vector(0.0, 1.0, 0.0);
	Vector b = Vector(1.0, -1.0, 0.0);
	Vector c = Vector(-1.0, -1.0, 0.0);

	STriangle s = STriangle(meta, a, b, c);
	Vector pt = Vector(0.0, 0.0, 0.0);

	bool result = s.ContainsPoint(pt);
	CHECK_TRUE(result);
}

TEST_CASE(striangle_contains_point_zero_area_triangle) {
	STriMeta meta = {};
	Vector a = Vector(0.0, -1.0, 0.0);
	Vector b = Vector(1.0, -1.0, 0.0);
	Vector c = Vector(-1.0, -1.0, 0.0);

	STriangle s = STriangle(meta, a, b, c);
	Vector pt = Vector(0.0, 0.0, 0.0);

	bool result = s.ContainsPoint(pt);
	CHECK_FALSE(result);
}

TEST_CASE(striangle_transform) {
	STriMeta meta = {};
	Vector a = Vector(0.0, 1.0, 0.0);
	Vector b = Vector(1.0, -1.0, 0.0);
	Vector c = Vector(-1.0, -1.0, 0.0);

	STriangle s = STriangle(meta, a, b, c);
	Vector u = Vector(1.0, 0.0, 0.0);
	Vector v = Vector(0.0, 0.0, 0.0);
	Vector n = Vector(0.0, 0.0, 0.0);

	STriangle result = s.Transform(u, v, n);
    CHECK_EQ_EPS(result.a.x, 0);
    CHECK_EQ_EPS(result.a.y, 0);
    CHECK_EQ_EPS(result.a.z, 0);
    CHECK_EQ_EPS(result.b.x, 1);
    CHECK_EQ_EPS(result.b.y, 0);
    CHECK_EQ_EPS(result.b.z, 0);
    CHECK_EQ_EPS(result.c.x, -1);
    CHECK_EQ_EPS(result.c.y, 0);
    CHECK_EQ_EPS(result.c.z, 0);
}

TEST_CASE(striangle_signedvolume) {
	STriMeta meta = {};
	Vector a = Vector(0.0, 0.0, 31.0);
	Vector b = Vector(20.0, 2.0, 0.0);
	Vector c = Vector(12.0, -20.0, 0.0);

	STriangle s = STriangle(meta, a, b, c);
    CHECK_EQ_EPS(s.SignedVolume(), -2190.666667);
}

TEST_CASE(striangle_area) {
	STriMeta meta = {};
	Vector a = Vector(0.0, 1.0, 0.0);
	Vector b = Vector(1.0, -1.0, 0.0);
	Vector c = Vector(-1.0, -1.0, 0.0);

	STriangle s = STriangle(meta, a, b, c);

	CHECK_EQ_EPS(s.Area(), 2.0);
}

TEST_CASE(striangle_isdegenerate) {
	STriMeta meta = {};
	Vector a = Vector(0.0, 0.0, 31.0);
	Vector b = Vector(20.0, 2.0, 0.0);
	Vector c = Vector(12.0, -20.0, 0.0);

	STriangle s = STriangle(meta, a, b, c);
    CHECK_FALSE(s.IsDegenerate());
}
