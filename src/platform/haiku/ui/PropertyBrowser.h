#ifndef PROPERTYBROWSER_H
#define PROPERTYBROWSER_H

#include "ListView/DragSortableListView.h"
#include "ListView/ThumbListItem.h"
#include <Window.h>

class PropertyBrowser : public BWindow {
  public:
    PropertyBrowser(void);
    void MessageReceived(BMessage *msg);
    void ShowListOfGroups();

  private:
    SimpleListView *groupList;
};

#endif
