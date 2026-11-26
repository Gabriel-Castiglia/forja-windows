/**
  access.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 26, 2026
  SPDX-License-Identifier: GPL-2.0-only

  What Narrator and other screen readers are told about what Forja draws itself. Windows
  describes its own controls; the switches, chips, segments, slider, list rows, status line
  and menus of Forja are drawn by the program, so it tells Windows what they are through
  Dynamic Annotation (oleacc, IAccPropServices): a name, a role (check box, option button,
  slider...) and a state (checked or not, focused, unavailable, hidden). The status line is
  a polite live region, so what it says is read out without moving the focus; the rows of
  the list answer with their password when asked (listProc, layout.c) and the menu items
  carry their text (accNameMenu). Without oleacc everything still works, unread.
**/

#define COBJMACROS
#include "forja.h"
#include <initguid.h>
#include <oleacc.h>

/* UI Automation's LiveSetting property (UIAutomationClient.h, LiveSetting_Property_GUID):
 * 1 = Polite, read out when the reader is idle. */
DEFINE_GUID(FORJA_LiveSetting, 0xc12bcd8e, 0x2a8e, 0x4950, 0x8a, 0xe7, 0x36, 0x25, 0x11, 0x1d, 0x58, 0xeb);

#ifndef EVENT_OBJECT_LIVEREGIONCHANGED
#define EVENT_OBJECT_LIVEREGIONCHANGED 0x8019
#endif

static IAccPropServices* gAps = NULL;
static int gApsTried = 0;

/* The Dynamic Annotation service, created on first use; NULL when it is not available. */
static IAccPropServices* aps(void)
{
	if (!gApsTried)
	{
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		gApsTried = 1;
		if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE)
			if (FAILED(CoCreateInstance(&CLSID_AccPropServices, NULL, CLSCTX_INPROC_SERVER,
			                            &IID_IAccPropServices, (void**)&gAps))) gAps = NULL;
	}
	return gAps;
}

/* A numeric property (role, state, live setting) of a window. */
static void setInt(HWND h, const MSAAPROPID* prop, LONG value)
{
	VARIANT v;

	if (!h || !aps()) return;
	VariantInit(&v);
	v.vt = VT_I4;
	v.lVal = value;
	IAccPropServices_SetHwndProp(gAps, h, OBJID_CLIENT, CHILDID_SELF, *prop, v);
}

void accSetName(HWND h, const wchar_t* name)
{
	if (h && aps()) IAccPropServices_SetHwndPropStr(gAps, h, OBJID_CLIENT, CHILDID_SELF, PROPID_ACC_NAME, name);
}

void accSetValue(HWND h, const wchar_t* value)
{
	if (h && aps()) IAccPropServices_SetHwndPropStr(gAps, h, OBJID_CLIENT, CHILDID_SELF, PROPID_ACC_VALUE, value);
}

void accSetRole(HWND h, int role) { setInt(h, &PROPID_ACC_ROLE, role); }

/* The whole state of a control drawn by Forja: what Windows would say of a window of its
 * own (focusable, focused, unavailable, hidden) plus checked, when it is a choice. */
int accStateOf(HWND h, int checked)
{
	int s = STATE_SYSTEM_FOCUSABLE;

	if (GetFocus() == h)       s |= STATE_SYSTEM_FOCUSED;
	if (!IsWindowEnabled(h))   s |= STATE_SYSTEM_UNAVAILABLE;
	if (!IsWindowVisible(h))   s |= STATE_SYSTEM_INVISIBLE;
	if (checked > 0)           s |= STATE_SYSTEM_CHECKED;
	return s;
}

void accSetState(HWND h, int state) { setInt(h, &PROPID_ACC_STATE, state); }

void accNotify(HWND h, DWORD event) { if (h) NotifyWinEvent(event, h, OBJID_CLIENT, CHILDID_SELF); }

/* ------------------------------------------------------------------ the status line */

void accStatusInit(HWND h)
{
	accSetRole(h, ROLE_SYSTEM_STATICTEXT);
	setInt(h, &FORJA_LiveSetting, 1);
}

void accStatusSays(HWND h, const wchar_t* text)
{
	if (!h) return;
	accSetName(h, text);
	accNotify(h, EVENT_OBJECT_LIVEREGIONCHANGED);
}

/* ------------------------------------------------------------------ menus */

/* Owner-drawn menu items carry no text for Windows to read: each one is given its string as
 * well (without the details after a tab), which Windows reads but never draws, and so are the
 * items of its submenus. */
void accNameMenu(HMENU m)
{
	int i, n = m ? GetMenuItemCount(m) : 0;

	for (i = 0; i < n; i++)
	{
		MENUITEMINFOW mi;
		wchar_t name[160];
		int id;

		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_DATA | MIIM_FTYPE | MIIM_SUBMENU;
		if (!GetMenuItemInfoW(m, (UINT)i, TRUE, &mi)) continue;
		if (mi.hSubMenu) accNameMenu(mi.hSubMenu);
		if ((mi.fType & MFT_SEPARATOR) || mi.dwItemData == MENU_SEP) continue;
		id = MENU_ID(mi.dwItemData);
		if (id < 0 || id >= S_COUNT) continue;
		headCopy(name, 160, T(id));
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.fMask = MIIM_STRING;
		mi.dwTypeData = name;
		SetMenuItemInfoW(m, (UINT)i, TRUE, &mi);
	}
}
