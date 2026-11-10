/**
  Wyrmhold -- resource and command identifiers.
  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 9, 2026
  SPDX-License-Identifier: GPL-2.0-only

  The menus are built in code: their text changes with the language, and they are owner-
  drawn so that they follow the light or dark theme and the accent colour like the rest of
  the window. Ranges of ids (modes, profiles, themes, accents, symbol sets, separators) are
  consecutive, so an item's position in its range is its value.
**/

#ifndef RESOURCE_H
#define RESOURCE_H

#define IDI_APP          101
#define IDR_ICONLAYER    102     /* the icon drawing without its tile colour (make_icon.py) */

#define IDM_GENERATE     301
#define IDM_COPY         302
#define IDM_COPYALL      303
#define IDM_SAVE         304
#define IDM_EXIT         305
#define IDM_LANG_EN      306
#define IDM_LANG_JP      307
#define IDM_LANG_ES      308
#define IDM_HELP         309
#define IDM_ABOUT        310

#define IDM_MODE_CHARS   320     /* + PW_CHARS, PW_WORDS, PW_PIN */
#define IDM_MODE_WORDS   321
#define IDM_MODE_PIN     322
#define IDM_PRESET_0     330     /* + preset index */
#define IDM_THEME_0      345     /* + 0 follow Windows, 1 light, 2 dark */
#define IDM_ACCENT_0     350     /* + accent index (the theme window) */
#define IDM_THEMEDLG     344
#define IDM_SYM_0        365     /* + PW_SYM_* */
#define IDM_SEP_0        370     /* + PW_SEP_* */

#endif
