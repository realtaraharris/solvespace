/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#pragma once

#include "ListView/DragSortableListView.h"

class GroupsListView : public SimpleListView {
  public:
  GroupsListView (BRect frame, BMessage *selectionChangeMessage = NULL);
  GroupsListView (BRect frame, const char *name, BMessage *selectionChangeMessage = NULL,
                  list_view_type type         = B_MULTIPLE_SELECTION_LIST,
                  uint32         resizingMode = B_FOLLOW_ALL_SIDES,
                  uint32         flags        = B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS |
                                 B_FULL_UPDATE_ON_RESIZE);

  ~GroupsListView ();

  virtual void MoveItems (BList &items, int32 toIndex);
};
