/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include "GroupsListView.h"
#include "ListView/DragSortableListView.h"

#include "solvespace.h"

GroupsListView::GroupsListView(BRect frame, BMessage *selectionChangeMessage)
    : SimpleListView(frame, selectionChangeMessage) {}

GroupsListView::GroupsListView(BRect frame, const char *name, BMessage *selectionChangeMessage,
                               list_view_type type, uint32 resizingMode, uint32 flags)
    : SimpleListView(frame, name, selectionChangeMessage, type, resizingMode, flags) {}

GroupsListView::~GroupsListView() {}

void GroupsListView::MoveItems(BList &items, int32 toIndex) {
  dbp("hi hi hi");

  for (hGroup hg : SK.groupOrder) {
    Group *gi = SK.GetGroup(hg);

    if (gi->order == 0) {
      gi->order += 1;
    }
  }

  SS.GenerateAll(SolveSpaceUI::Generate::ALL);

  int32 count = items.CountItems();
  for (int32 i = 0; i < count; i++) {
    BListItem *item      = (BListItem *)items.ItemAt(i);
    int32      fromIndex = IndexOf(item);

    dbp("moving fromIndex: %d, toIndex: %d\n", fromIndex, toIndex);
  }

  // call the base MoveItems method last because it mutates the item indices
  SimpleListView::MoveItems(items, toIndex);
}