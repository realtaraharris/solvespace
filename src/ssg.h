/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#pragma once

#include "ssui/solvespaceui.h"
#if defined(HAIKU_GUI)
#  include "platform/haiku/HaikuSpaceUI.h"
  extern HaikuSpaceUI SS;
#else
  extern SolveSpaceUI SS;
#endif
