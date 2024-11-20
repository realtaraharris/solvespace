/*
 * Copyright 2024, My Name <my@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef EVENT_HOOKS_H
#define EVENT_HOOKS_H

void ButtonUnclickedEventHook(void);
void ErrorDialog(std::string message, std::string description,
  std::function<void()> onDismiss = std::function<void()>()
);
void WarningDialog(std::string message, std::string description,
  std::function<void()> onDismiss = std::function<void()>()
);

#endif // EVENT_HOOKS_H