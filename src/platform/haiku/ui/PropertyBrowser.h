#ifndef PROPERTYBROWSER_H
#define PROPERTYBROWSER_H

#include "GroupsListView.h"
#include <Window.h>

class PropertyBrowser : public BWindow {
  public:
    PropertyBrowser(void);
    void MessageReceived(BMessage *msg);
    void ShowListOfGroups();

  private:
    GroupsListView *groupList;
};

#endif
