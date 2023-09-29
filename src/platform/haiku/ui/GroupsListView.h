#ifndef GROUPS_LIST_VIEW_H
#define GROUPS_LIST_VIEW_H

#include "ListView/DragSortableListView.h"

class GroupsListView : public SimpleListView {
  public:
    GroupsListView(BRect frame, BMessage *selectionChangeMessage = NULL);
    GroupsListView(BRect frame, const char *name,
                   BMessage *selectionChangeMessage = NULL,
                   list_view_type type = B_MULTIPLE_SELECTION_LIST,
                   uint32 resizingMode = B_FOLLOW_ALL_SIDES,
                   uint32 flags = B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS |
                                  B_FULL_UPDATE_ON_RESIZE);

    ~GroupsListView();

    virtual void MoveItems(BList &items, int32 toIndex);
};

#endif // GROUPS_LIST_VIEW_H
