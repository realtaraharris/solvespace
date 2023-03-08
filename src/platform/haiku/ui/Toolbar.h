#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <Window.h>

class AppToolbar : public BWindow {
	public:
		AppToolbar(void);
		void AddButton(const BRect &frame, BBitmap *icon, BMessage *message);
		void MessageReceived(BMessage *msg);
	private:
};

#endif
