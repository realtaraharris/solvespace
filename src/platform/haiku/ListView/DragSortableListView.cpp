// this entire file is copied from WonderBrush
// DragSortableListView.cpp

#include <malloc.h>
#include <stdio.h>

#include <Bitmap.h>
#include <Cursor.h>
#include <Entry.h>
#include <MessageRunner.h>
#include <Messenger.h>
#include <ScrollBar.h>
#include <ScrollView.h>
#include <String.h>
#include <Window.h>

#include "DragSortableListView.h"
#include "ThumbListItem.h"

#include "solvespace.h"

#define MAX_DRAG_HEIGHT 200.0
#define ALPHA 170

enum {
  MSG_TICK = 'tick',
};

DragSortableListView::DragSortableListView(BRect frame, const char *name, list_view_type type,
                                           uint32 resizingMode, uint32 flags)
    : BListView(frame, name, type, resizingMode, flags), fDropRect(0.0, 0.0, -1.0, -1.0),
      fMouseWheelFilter(NULL), fScrollPulse(NULL), fDropIndex(-1), fLastClickedItem(NULL),
      fScrollView(NULL), fDragCommand(B_SIMPLE_DATA), fFocusedIndex(-1) {
  SetViewColor(B_TRANSPARENT_32_BIT);
}

DragSortableListView::~DragSortableListView() {
  // delete fMouseWheelFilter;
  delete fScrollPulse;
}

// AttachedToWindow
void DragSortableListView::AttachedToWindow() {
  if (!fMouseWheelFilter) {
    fMouseWheelFilter = new MouseWheelFilter(this);
  }
  Window()->AddCommonFilter(fMouseWheelFilter);

  BListView::AttachedToWindow();

  // work around a bug in BListView
  BRect bounds = Bounds();
  BListView::FrameResized(bounds.Width(), bounds.Height());
}

void DragSortableListView::DetachedFromWindow() {
  // Window()->RemoveCommonFilter(fMouseWheelFilter);
}

void DragSortableListView::FrameResized(float width, float height) {
  BListView::FrameResized(width, height);
}

void DragSortableListView::Draw(BRect updateRect) {
  int32 firstIndex = IndexOf(updateRect.LeftTop());
  int32 lastIndex = IndexOf(updateRect.RightBottom());
  if (firstIndex >= 0) {
    if (lastIndex < firstIndex)
      lastIndex = CountItems() - 1;
    // update rect contains items
    BRect r = updateRect;
    for (int32 i = firstIndex; i <= lastIndex; i++) {
      r = ItemFrame(i);
      DrawListItem(this, i, r);
    }
    updateRect.top = r.bottom + 1.0;
    if (updateRect.IsValid()) {
      SetLowColor(255, 255, 255, 255);
      FillRect(updateRect, B_SOLID_LOW);
    }
  } else {
    SetLowColor(255, 255, 255, 255);
    FillRect(updateRect, B_SOLID_LOW);
  }

  // drop anticipation indication
  if (fDropRect.IsValid()) {
    SetHighColor(ui_color(B_NAVIGATION_BASE_COLOR));
    StrokeRect(fDropRect);
  }

  /*
  // focus indication
  if (IsFocus()) {
      SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
      StrokeRect(Bounds());
  }
  */
}

void DragSortableListView::ScrollTo(BPoint where) {
  uint32 buttons;
  BPoint point;
  GetMouse(&point, &buttons, false);
  uint32 transit = Bounds().Contains(point) ? B_INSIDE_VIEW : B_OUTSIDE_VIEW;
  MouseMoved(point, transit, &fDragMessageCopy);
  BListView::ScrollTo(where);
}

void DragSortableListView::TargetedByScrollView(BScrollView *scrollView) {
  fScrollView = scrollView;
  BListView::TargetedByScrollView(scrollView);
}

bool DragSortableListView::InitiateDrag(BPoint point, int32 index, bool) {
  // supress drag&drop while an item is focused
  if (fFocusedIndex >= 0) {
    return false;
  }

  bool success = false;
  BListItem *item = ItemAt(CurrentSelection(0));
  if (!item) {
    // workaround a timing problem
    Select(index);
    item = ItemAt(index);
  }
  if (item) {
    // create drag message
    BMessage msg(fDragCommand);
    MakeDragMessage(&msg);
    // figure out drag rect
    float width = Bounds().Width();
    BRect dragRect(0.0, 0.0, width, -1.0);
    // figure out, how many items fit into our bitmap
    int32 numItems;
    bool fade = false;
    for (numItems = 0; BListItem *item = ItemAt(CurrentSelection(numItems)); numItems++) {
      dragRect.bottom += ceilf(item->Height()) + 1.0;
      if (dragRect.Height() > MAX_DRAG_HEIGHT) {
        fade = true;
        dragRect.bottom = MAX_DRAG_HEIGHT;
        numItems++;
        break;
      }
    }
    BBitmap *dragBitmap = new BBitmap(dragRect, B_RGB32, true);
    if (dragBitmap && dragBitmap->IsValid()) {
      if (BView *v = new BView(dragBitmap->Bounds(), "helper", B_FOLLOW_NONE, B_WILL_DRAW)) {
        dragBitmap->AddChild(v);
        dragBitmap->Lock();
        BRect itemBounds(dragRect);
        itemBounds.bottom = 0.0;
        // let all selected items, that fit into our drag_bitmap, draw
        for (int32 i = 0; i < numItems; i++) {
          int32 index = CurrentSelection(i);
          BListItem *item = ItemAt(index);
          itemBounds.bottom = itemBounds.top + ceilf(item->Height());
          if (itemBounds.bottom > dragRect.bottom)
            itemBounds.bottom = dragRect.bottom;
          DrawListItem(v, index, itemBounds);
          itemBounds.top = itemBounds.bottom + 1.0;
        }
        // make a frame around the edge
        v->SetHighColor(ui_color(B_CONTROL_BORDER_COLOR));
        v->StrokeRect(v->Bounds());
        v->Sync();

        uint8 *bits = (uint8 *)dragBitmap->Bits();
        int32 height = (int32)dragBitmap->Bounds().Height() + 1;
        int32 width = (int32)dragBitmap->Bounds().Width() + 1;
        int32 bpr = dragBitmap->BytesPerRow();

        if (fade) {
          for (int32 y = 0; y < height - ALPHA / 2; y++, bits += bpr) {
            uint8 *line = bits + 3;
            for (uint8 *end = line + 4 * width; line < end; line += 4)
              *line = ALPHA;
          }
          for (int32 y = height - ALPHA / 2; y < height; y++, bits += bpr) {
            uint8 *line = bits + 3;
            for (uint8 *end = line + 4 * width; line < end; line += 4)
              *line = (height - y) << 1;
          }
        } else {
          for (int32 y = 0; y < height; y++, bits += bpr) {
            uint8 *line = bits + 3;
            for (uint8 *end = line + 4 * width; line < end; line += 4)
              *line = ALPHA;
          }
        }
        dragBitmap->Unlock();
      }
    } else {
      delete dragBitmap;
      dragBitmap = NULL;
    }
    if (dragBitmap) {
      BRect itemFrame = ItemFrame(index);
      BPoint offset = BPoint(point.x - itemFrame.left, point.y - itemFrame.top);
      DragMessage(&msg, dragBitmap, B_OP_ALPHA, offset);
    } else {
      DragMessage(&msg, dragRect.OffsetToCopy(point), this);
    }

    _SetDragMessage(&msg);
    success = true;
  }
  return success;
}

void DragSortableListView::WindowActivated(bool active) {
  // workaround for buggy focus indication of BScrollView
  if (BView *view = Parent()) {
    view->Invalidate();
  }
}

void DragSortableListView::MessageReceived(BMessage *message) {
  if (message->what == fDragCommand) {
    DragSortableListView *list = NULL;
    if (message->FindPointer("list", (void **)&list) == B_OK && list == this) {
      int32 count = CountItems();
      if (fDropIndex < 0 || fDropIndex > count)
        fDropIndex = count;
      BList items;
      int32 index;
      for (int32 i = 0; message->FindInt32("index", i, &index) == B_OK; i++) {
        if (BListItem *item = ItemAt(index)) {
          items.AddItem((void *)item);
        }
      }
      if (items.CountItems() > 0) {
        if (modifiers() & B_SHIFT_KEY) {
          CopyItems(items, fDropIndex);
        } else {
          MoveItems(items, fDropIndex);
        }
      }
      fDropIndex = -1;
    }
  } else {
    switch (message->what) {
    case MSG_TICK: {
      float scrollV = 0.0;
      BRect rect(Bounds());
      BPoint point;
      uint32 buttons;
      GetMouse(&point, &buttons, false);
      if (rect.Contains(point)) {
        // calculate the vertical scrolling offset
        float hotDist = rect.Height() * SCROLL_AREA;
        if (point.y > rect.bottom - hotDist) {
          scrollV = hotDist - (rect.bottom - point.y);
        } else if (point.y < rect.top + hotDist) {
          scrollV = (point.y - rect.top) - hotDist;
        }
      }
      // scroll
      if (scrollV != 0.0 && fScrollView) {
        if (BScrollBar *scrollBar = fScrollView->ScrollBar(B_VERTICAL)) {
          float value = scrollBar->Value();
          scrollBar->SetValue(scrollBar->Value() + scrollV);
          if (scrollBar->Value() != value) {
            // update mouse position
            uint32 buttons;
            BPoint point;
            GetMouse(&point, &buttons, false);
            uint32 transit = Bounds().Contains(point) ? B_INSIDE_VIEW : B_OUTSIDE_VIEW;
            MouseMoved(point, transit, &fDragMessageCopy);
          }
        }
      }
      break;
    }
    // case B_MODIFIERS_CHANGED:
    // ModifiersChanged();
    // break;
    case B_MOUSE_WHEEL_CHANGED: {
      BListView::MessageReceived(message);
      BPoint point;
      uint32 buttons;
      GetMouse(&point, &buttons, false);
      uint32 transit = Bounds().Contains(point) ? B_INSIDE_VIEW : B_OUTSIDE_VIEW;
      MouseMoved(point, transit, &fDragMessageCopy);
      break;
    }
    default: BListView::MessageReceived(message); break;
    }
  }
}

void DragSortableListView::KeyDown(const char *bytes, int32 numBytes) {
  if (numBytes < 1)
    return;

  if ((bytes[0] == B_BACKSPACE) || (bytes[0] == B_DELETE))
    RemoveSelected();

  BListView::KeyDown(bytes, numBytes);
}

void DragSortableListView::MouseDown(BPoint where) {
  int32 clicks = 1;
  uint32 buttons = 0;
  Window()->CurrentMessage()->FindInt32("clicks", &clicks);
  Window()->CurrentMessage()->FindInt32("buttons", (int32 *)&buttons);
  int32 clickedIndex = -1;
  for (int32 i = 0; BListItem *item = ItemAt(i); i++) {
    if (ItemFrame(i).Contains(where)) {
      if (where.x <= TEXT_OFFSET) {
        dbp("hide/show %d", i); // TODO pick up here tomorrow. hide/show group from here?
      }
      if (clicks == 2) {
        // only do something if user clicked the same item twice
        if (fLastClickedItem == item)
          DoubleClicked(i);
      } else {
        // remember last clicked item
        fLastClickedItem = item;
      }
      clickedIndex = i;
      break;
    }
  }
  if (clickedIndex == -1)
    fLastClickedItem = NULL;

  BListItem *item = ItemAt(clickedIndex);
  if (ListType() == B_MULTIPLE_SELECTION_LIST && item && (buttons & B_SECONDARY_MOUSE_BUTTON)) {
    if (item->IsSelected())
      Deselect(clickedIndex);
    else
      Select(clickedIndex, true);
  } else {
    BListView::MouseDown(where);
  }
}

void DragSortableListView::MouseMoved(BPoint where, uint32 transit, const BMessage *msg) {
  if (msg && AcceptDragMessage(msg)) {
    switch (transit) {
    case B_ENTERED_VIEW:
    case B_INSIDE_VIEW: {
      // remember drag message
      // this is needed to react on modifier changes
      _SetDragMessage(msg);
      // set drop target through virtual function
      SetDropTargetRect(msg, where);
      // go into autoscrolling mode
      BRect r = Bounds();
      r.InsetBy(0.0, r.Height() * SCROLL_AREA);
      SetAutoScrolling(!r.Contains(where));
      break;
    }
    case B_EXITED_VIEW:
      // forget drag message
      _SetDragMessage(NULL);
      SetAutoScrolling(false);
      // fall through
    case B_OUTSIDE_VIEW: _RemoveDropAnticipationRect(); break;
    }
  } else {
    _RemoveDropAnticipationRect();
    BListView::MouseMoved(where, transit, msg);
    _SetDragMessage(NULL);
    SetAutoScrolling(false);

    BCursor cursor(B_HAND_CURSOR);
    SetViewCursor(&cursor, true);
  }
  fLastMousePos = where;
}

void DragSortableListView::MouseUp(BPoint where) {
  // remove drop mark
  _SetDropAnticipationRect(BRect(0.0, 0.0, -1.0, -1.0));
  SetAutoScrolling(false);
  // be sure to forget drag message
  _SetDragMessage(NULL);
  BListView::MouseUp(where);

  BCursor cursor(B_HAND_CURSOR);
  SetViewCursor(&cursor, true);
}

void DragSortableListView::DrawItem(BListItem *item, BRect itemFrame, bool complete) {
  DrawListItem(this, IndexOf(item), itemFrame);
  /*
      if (IsFocus()) {
          SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
          StrokeRect(Bounds());
      }
  */
}

bool DragSortableListView::MouseWheelChanged(float x, float y) {
  BPoint where;
  uint32 buttons;
  GetMouse(&where, &buttons, false);
  if (Bounds().Contains(where))
    return true;
  else
    return false;
}

void DragSortableListView::SetDragCommand(uint32 command) {
  fDragCommand = command;
}

void DragSortableListView::ModifiersChanged() {
  SetDropTargetRect(&fDragMessageCopy, fLastMousePos);
}

void DragSortableListView::SetItemFocused(int32 index) {
  InvalidateItem(fFocusedIndex);
  InvalidateItem(index);
  fFocusedIndex = index;
}

bool DragSortableListView::AcceptDragMessage(const BMessage *message) const {
  return message->what == fDragCommand;
}

void DragSortableListView::SetDropTargetRect(const BMessage *message, BPoint where) {
  if (AcceptDragMessage(message)) {
    bool copy = modifiers() & B_SHIFT_KEY;
    bool replaceAll = !message->HasPointer("list") && !copy;
    BRect r = Bounds();
    if (replaceAll) {
      r.bottom--; // compensate for scrollbar offset
      _SetDropAnticipationRect(r);
      fDropIndex = -1;
    } else {
      // offset where by half of item height
      r = ItemFrame(0);
      where.y += r.Height() / 2.0;

      int32 index = IndexOf(where);
      if (index < 0)
        index = CountItems();
      _SetDropIndex(index);

      //			const uchar* cursorData = copy ? kCopyCursor :
      // B_HAND_CURSOR;
      BCursor cursor(B_HAND_CURSOR);
      SetViewCursor(&cursor, true);
    }
  }
}

void DragSortableListView::SetAutoScrolling(bool enable) {
  if (fScrollPulse && enable)
    return;
  if (enable) {
    BMessenger messenger(this, Window());
    BMessage message(MSG_TICK);
    fScrollPulse = new BMessageRunner(messenger, &message, 40000LL);
  } else {
    delete fScrollPulse;
    fScrollPulse = NULL;
  }
}

bool DragSortableListView::DoesAutoScrolling() const {
  return fScrollPulse;
}

void DragSortableListView::ScrollTo(int32 index) {
  if (index < 0)
    index = 0;
  if (index >= CountItems())
    index = CountItems() - 1;

  if (BListItem *item = ItemAt(index)) {
    BRect itemFrame = ItemFrame(index);
    BRect bounds = Bounds();
    if (itemFrame.top < bounds.top) {
      ScrollTo(itemFrame.LeftTop());
    } else if (itemFrame.bottom > bounds.bottom) {
      ScrollTo(BPoint(0.0, itemFrame.bottom - bounds.Height()));
    }
  }
}

void DragSortableListView::MoveItems(BList &items, int32 index) {
  DeselectAll();
  // we remove the items while we look at them, the insertion index is
  // decreased when the items index is lower, so that we insert at the right
  // spot after removal
  BList removedItems;
  int32 count = items.CountItems();
  for (int32 i = 0; i < count; i++) {
    BListItem *item = (BListItem *)items.ItemAt(i);
    int32 removeIndex = IndexOf(item);
    if (RemoveItem(item) && removedItems.AddItem((void *)item)) {
      if (removeIndex < index)
        index--;
    }
    // else ??? -> blow up
  }
  for (int32 i = 0; BListItem *item = (BListItem *)removedItems.ItemAt(i); i++) {
    if (AddItem(item, index)) {
      // after we're done, the newly inserted items will be selected
      Select(index, true);
      // next items will be inserted after this one
      index++;
    } else
      delete item;
  }
}

void DragSortableListView::CopyItems(BList &items, int32 index) {
  DeselectAll();
  // by inserting the items after we copied all items first, we avoid
  // cloning an item we already inserted and messing everything up
  // in other words, don't touch the list before we know which items
  // need to be cloned
  BList clonedItems;
  int32 count = items.CountItems();
  for (int32 i = 0; i < count; i++) {
    BListItem *item = CloneItem(IndexOf((BListItem *)items.ItemAt(i)));
    if (item && !clonedItems.AddItem((void *)item))
      delete item;
  }
  for (int32 i = 0; BListItem *item = (BListItem *)clonedItems.ItemAt(i); i++) {
    if (AddItem(item, index)) {
      // after we're done, the newly inserted items will be selected
      Select(index, true);
      // next items will be inserted after this one
      index++;
    } else
      delete item;
  }
}

void DragSortableListView::RemoveItemList(BList &items) {
  int32 count = items.CountItems();
  for (int32 i = 0; i < count; i++) {
    BListItem *item = (BListItem *)items.ItemAt(i);
    if (RemoveItem(item))
      delete item;
  }
}

void DragSortableListView::RemoveSelected() {
  //	if (fFocusedIndex >= 0)
  //		return;

  BList items;
  for (int32 i = 0; BListItem *item = ItemAt(CurrentSelection(i)); i++)
    items.AddItem((void *)item);
  RemoveItemList(items);
}

int32 DragSortableListView::CountSelectedItems() const {
  int32 count = 0;
  while (CurrentSelection(count) >= 0)
    count++;
  return count;
}

void DragSortableListView::SelectAll() {
  Select(0, CountItems() - 1);
}

bool DragSortableListView::DeleteItem(int32 index) {
  BListItem *item = ItemAt(index);
  if (item && RemoveItem(item)) {
    delete item;
    return true;
  }
  return false;
}

void DragSortableListView::_SetDropAnticipationRect(BRect r) {
  if (fDropRect != r) {
    if (fDropRect.IsValid())
      Invalidate(fDropRect);
    fDropRect = r;
    if (fDropRect.IsValid())
      Invalidate(fDropRect);
  }
}

void DragSortableListView::_SetDropIndex(int32 index) {
  if (fDropIndex != index) {
    fDropIndex = index;
    if (fDropIndex >= 0) {
      int32 count = CountItems();
      if (fDropIndex == count) {
        BRect r;
        if (BListItem *item = ItemAt(count - 1)) {
          r = ItemFrame(count - 1);
          r.top = r.bottom;
          r.bottom = r.top + 1.0;
        } else {
          r = Bounds();
          r.bottom--; // compensate for scrollbars moved slightly out
                      // of window
        }
        _SetDropAnticipationRect(r);
      } else {
        BRect r = ItemFrame(fDropIndex);
        r.top--;
        r.bottom = r.top + 1.0;
        _SetDropAnticipationRect(r);
      }
    }
  }
}

void DragSortableListView::_RemoveDropAnticipationRect() {
  _SetDropAnticipationRect(BRect(0.0, 0.0, -1.0, -1.0));
  //	_SetDropIndex(-1);
}

void DragSortableListView::_SetDragMessage(const BMessage *message) {
  if (message)
    fDragMessageCopy = *message;
  else
    fDragMessageCopy.what = 0;
}

SimpleListView::SimpleListView(BRect frame, BMessage *selectionChangeMessage)
    : DragSortableListView(frame, "playlist listview", B_MULTIPLE_SELECTION_LIST, B_FOLLOW_ALL,
                           B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE),
      fSelectionChangeMessage(selectionChangeMessage) {}

SimpleListView::SimpleListView(BRect frame, const char *name, BMessage *selectionChangeMessage,
                               list_view_type type, uint32 resizingMode, uint32 flags)
    : DragSortableListView(frame, name, type, resizingMode, flags),
      fSelectionChangeMessage(selectionChangeMessage) {}

SimpleListView::~SimpleListView() {
  delete fSelectionChangeMessage;
}

/*
minimax SimpleListView::layoutprefs() {
    mpm.mini.x = 30.0;
    mpm.maxi.x = 10000.0;
    mpm.mini.y = 50.0;
    mpm.maxi.y = 10000.0;
    mpm.weight = 1.0;
    return mpm;
}

BRect SimpleListView::layout(BRect frame) {
    MoveTo(frame.LeftTop());
    ResizeTo(frame.Width(), frame.Height());
    return Frame();
}
*/

void SimpleListView::MessageReceived(BMessage *message) {
  switch (message->what) {
  default: DragSortableListView::MessageReceived(message); break;
  }
}

void SimpleListView::SelectionChanged() {
  BLooper *looper = Looper();
  if (fSelectionChangeMessage && looper) {
    BMessage message(*fSelectionChangeMessage);
    looper->PostMessage(&message);
  }
}

BListItem *SimpleListView::CloneItem(int32 atIndex) const {
  BListItem *clone = NULL;
  if (ThumbListItem *item = dynamic_cast<ThumbListItem *>(ItemAt(atIndex))) {
    clone = item->Clone();
  }

  return clone;
}

void SimpleListView::DrawListItem(BView *owner, int32 index, BRect frame) const {
  if (ThumbListItem *item = dynamic_cast<ThumbListItem *>(ItemAt(index))) {
    uint32 flags = FLAGS_NONE;
    if (index == fFocusedIndex)
      flags |= FLAGS_FOCUSED;
    if (index % 2)
      flags |= FLAGS_TINTED_LINE;
    item->DrawItem(owner, frame, flags);
  }
}

void SimpleListView::MakeDragMessage(BMessage *message) const {
  if (message) {
    message->AddPointer("list", (void *)dynamic_cast<const DragSortableListView *>(this));
    int32 index;
    for (int32 i = 0; (index = CurrentSelection(i)) >= 0; i++)
      message->AddInt32("index", index);
  }
}

void SimpleListView::MoveItems(BList &items, int32 toIndex) {
  DragSortableListView::MoveItems(items, toIndex);
}
