/*
 * Copyright 2023, 2024 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include "PropertyBrowser.h"

#include <Application.h>
#include <Bitmap.h>
#include <LayoutBuilder.h>
#include <NodeInfo.h>
#include <PictureButton.h>
#include <Roster.h>
#include <ScrollView.h>
#include <View.h>
#include <Window.h>

#include "HVIFUtil.h"

#include "App.h" // contains message enums
#include "ListView/ThumbListItem.h"

#include <Alert.h> // BAlert
#include <Button.h>
#include <iostream>

#include "solvespace.h"
#include "ssg.h"

void PropertyBrowser::ShowListOfGroups() {
  dbp("%Ft active");
  dbp("%Ft    shown dof group-name%E");
  bool afterActive = false;
  bool backgroundParity = false;
  for (hGroup hg : SK.groupOrder) {
    Group *g = SK.GetGroup(hg);

    std::string s = g->DescriptionString();
    bool active = (g->h == SS.GW.activeGroup);
    bool shown = g->visible;
    bool ok = g->IsSolvedOkay();
    bool warn =
        (g->type == Group::Type::DRAWING_WORKPLANE && g->polyError.how != PolyError::GOOD) ||
        ((g->type == Group::Type::EXTRUDE || g->type == Group::Type::LATHE) &&
         SK.GetGroup(g->opA)->polyError.how != PolyError::GOOD);
    int dof = g->solved.dof;
    char sdof[16] = "ok ";
    if (ok && dof > 0) {
      if (dof > 999) {
        strcpy(sdof, "###");
      } else {
        sprintf(sdof, "%-3d", dof);
      }
    }
    std::string suffix;
    if (g->forceToMesh || g->IsTriangleMeshAssembly()) {
      suffix = " (∆)";
    }

    bool ref = (g->h == Group::HGROUP_REFERENCES);
    dbp(g->name.c_str());
    /*false,
           "%Bp%Fd "
           "%Ft%s%Fb%D%f%Ll%s%E "
           "%Fb%s%D%f%Ll%s%E  "
           "%Fp%D%f%s%Ll%s%E "
           "%Fp%Ll%D%f%s%E%s",
           // Alternate between light and dark backgrounds, for readability
           backgroundParity ? 'd' : 'a',
           // Link that activates the group
           ref ? "   " : "",
           g->h.v, (&TextWindow::ScreenActivateGroup),
           ref ? "" : (active ? radioTrue : radioFalse),
           // Link that hides or shows the group
           afterActive ? " - " : "",
           g->h.v, (&TextWindow::ScreenToggleGroupShown),
           afterActive ? "" : (shown ? checkTrue : checkFalse),
           // Link to the errors, if a problem occurred while solving
           ok ? (warn ? 'm' : (dof > 0 ? 'i' : 's')) : 'x',
           g->h.v, (&TextWindow::ScreenHowGroupSolved),
           ok ? ((warn && SS.checkClosedContour) ? "err" : sdof) : "",
           ok ? "" : "ERR",
           // Link to a screen that gives more details on the group
           g->suppress ? 'g' : 'l',
           g->h.v, (&TextWindow::ScreenSelectGroup), s.c_str(),
           suffix.c_str());
           */

    ThumbListItem *listItem;
    static BBitmap *closedEyeIcon = LoadIconFromResource("closed-eye", 20);

    listItem = new ThumbListItem(closedEyeIcon, g->name.c_str(), 20, 0, FALSE);
    groupList->AddItem(listItem);

    if (active) {
      afterActive = true;
    }
    backgroundParity = !backgroundParity;
  }
}

PropertyBrowser::PropertyBrowser(void)
    : BWindow(BRect(BPoint(740, 35), BSize(300, 150)), "Property Browser", B_FLOATING_WINDOW_LOOK,
              B_FLOATING_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS,
              B_CURRENT_WORKSPACE) {
  BScrollView *scrollView;

  groupList = new GroupsListView(Bounds(), "groups_list", NULL, B_SINGLE_SELECTION_LIST,
                                 B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

  groupList->SetViewColor(255, 220, 220, 0);

  BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
      .Add(scrollView = new BScrollView("scroll_view", groupList, 0, false, true, B_PLAIN_BORDER),
           0.0f)
      .End();
}

void PropertyBrowser::MessageReceived(BMessage *msg) {
  switch (msg->what) {
  case SHOW_LIST_OF_GROUPS: {
    ShowListOfGroups();
    break;
  }
  }
}
