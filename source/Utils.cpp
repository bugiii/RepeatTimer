#include "stdafx.h"
#include "resource.h"
#include <cstdio>
#include <cstdint>

int dprintf(const char *format, ...)
{
	char str[1024];

	va_list argptr;
	va_start(argptr, format);
	int ret = vsnprintf(str, sizeof(str), format, argptr);
	va_end(argptr);

	OutputDebugStringA(str);

	return ret;
}

void displayMenu(HWND hwnd, int x, int y)
{
	HMENU menu = LoadMenu(NULL, MAKEINTRESOURCE(IDC_TIMER_MENU));

	// TODO: check menu item

	HMENU popup = GetSubMenu(menu, 0);

	TrackPopupMenuEx(popup, TPM_LEFTALIGN, x, y, hwnd, NULL);

	DestroyMenu(menu);
}

uint64_t currentTime()
{
	SYSTEMTIME sysTime;
	GetSystemTime(&sysTime);
	FILETIME fileTime;
	SystemTimeToFileTime(&sysTime, &fileTime);
	return *reinterpret_cast<uint64_t*>(&fileTime);
}

BOOL stickSide(HWND hwnd, LPWINDOWPOS lpwpos)
{
	// using current mouse x y
	POINT p;
	GetCursorPos(&p);
	lpwpos->x = p.x - lpwpos->cx / 2;
	lpwpos->y = p.y - lpwpos->cy / 2;

	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	if (NULL != monitor) {
		MONITORINFOEX mi;
		mi.cbSize = sizeof MONITORINFOEX;
		if (GetMonitorInfo(monitor, &mi)) {
			int xMargin = (mi.rcMonitor.right - mi.rcMonitor.left) * 1 / 100; // width's 1%
			int yMargin = xMargin; // (mi.rcMonitor.bottom - mi.rcMonitor.top) * 1 / 100;

			if (mi.rcWork.left - xMargin < lpwpos->x && lpwpos->x < mi.rcWork.left + xMargin) {
				lpwpos->x = mi.rcWork.left;
			}

			if (mi.rcWork.right - xMargin < lpwpos->x + lpwpos->cx && lpwpos->x + lpwpos->cx < mi.rcWork.right + xMargin) {
				lpwpos->x = mi.rcWork.right - lpwpos->cx;
			}

			if (mi.rcWork.top - yMargin < lpwpos->y && lpwpos->y < mi.rcWork.top + yMargin) {
				lpwpos->y = mi.rcWork.top;
			}

			if (mi.rcWork.bottom - yMargin < lpwpos->y + lpwpos->cy && lpwpos->y + lpwpos->cy < mi.rcWork.bottom + yMargin) {
				lpwpos->y = mi.rcWork.bottom - lpwpos->cy;
			}

			// TODO: xy center
			return FALSE;
		}
	}

	return TRUE;
}
