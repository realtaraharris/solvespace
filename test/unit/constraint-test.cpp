/*
 * Copyright 2024 Tara Harris <3769985+realtaraharris@users.noreply.github.com>
 * All rights reserved. Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include "harness.h"

TEST_CASE(Constraint__DescriptionString) {
  Constraint c = {};
  c.group = SS.GW.activeGroup;
  c.workplane = SS.GW.ActiveWorkplane();
  c.type = SS.GW.pending.suggestion;
  c.entityA = SS.GW.pending.request.entity(0);

  CHECK_EQ_STR(c.DescriptionString(), "c000-???");
}
