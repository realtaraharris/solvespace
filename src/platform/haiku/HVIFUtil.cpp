/*
 * Copyright 2023-2025 Tara Harris
 * <3769985+realtaraharris@users.noreply.github.com> All rights reserved.
 * Distributed under the terms of the GPLv3 and MIT licenses.
 */

#include <IconUtils.h>
#include <Resources.h>
#include <Bitmap.h>

BBitmap *LoadIconFromResource(const char *name, float iconSize) {
  BResources resources;
  resources.SetToImage(name);

  size_t size = 0;
  const uint8 *iconData = (const uint8 *)resources.LoadResource('VICN', name, &size);

  if (buffer == NULL) {
    return NULL;
  }

  BBitmap *result = new BBitmap(BRect(0, 0, iconSize, iconSize), B_RGBA32);
  status_t status = BIconUtils::GetVectorIcon(iconData, size, result);
  if (status != B_OK) {
    delete result;
    return NULL;
  }
  return result;
}