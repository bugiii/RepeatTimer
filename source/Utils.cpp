#include "stdafx.h"
#include "resource.h"
#include <shellscalingapi.h>
#include <cwchar>
#include <cstdio>
#include <cstdarg>
#include <string>

#pragma comment(lib, "shcore")

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

std::wstring formatString(const wchar_t *format, ...)
{
	wchar_t buffer[256]; // TODO:

	va_list args;
	va_start(args, format);
	int size = _vsnwprintf_s(buffer, dimof(buffer), _TRUNCATE, format, args);
	std::wstring result(buffer, size);
	va_end(args);

	return result;
}

RECT getRect(HWND hwnd)
{
	RECT rect;
	GetWindowRect(hwnd, &rect);
	return rect;
}

uint64_t currentTime()
{
	SYSTEMTIME sysTime;
	GetSystemTime(&sysTime);
	FILETIME fileTime;
	SystemTimeToFileTime(&sysTime, &fileTime);
	return *reinterpret_cast<uint64_t*>(&fileTime);
}

BOOL stickSide(HWND hwnd, LPWINDOWPOS lpwpos, bool moving)
{
	if (moving) {
		// using current mouse x y
		POINT p;
		GetCursorPos(&p);
		lpwpos->x = p.x - lpwpos->cx / 2;
		lpwpos->y = p.y - lpwpos->cy / 2;
	}

	HMONITOR monitor = MonitorFromWindow(hwnd, NULL);
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

void resizeAsZoom(HWND hwnd, const Zoom& zoom) // TODO: invalidate param
{
	RECT wrect = getRect(hwnd);

	HMONITOR monitor = MonitorFromWindow(hwnd, NULL);
	if (NULL != monitor) {
		MONITORINFOEX mi;
		mi.cbSize = sizeof MONITORINFOEX;
		if (GetMonitorInfo(monitor, &mi)) {
			struct Attached {
				bool left, top, right, bottom;
			} attached;
			attached.left = wrect.left == mi.rcWork.left;
			attached.top = wrect.top == mi.rcWork.top;
			attached.right = wrect.right == mi.rcWork.right;
			attached.bottom = wrect.bottom == mi.rcWork.bottom;

			if (attached.right) {
				wrect.left = mi.rcWork.right - zoom.level();
			}
			else if (!attached.left) {
				wrect.left = (wrect.left + (wrect.right - wrect.left) / 2) - zoom.level() / 2;
			}

			if (attached.bottom) {
				wrect.top = mi.rcWork.bottom - zoom.level();
			}
			else if (!attached.top) {
				wrect.top = (wrect.top + (wrect.bottom - wrect.top) / 2) - zoom.level() / 2;
			}

			wrect.right = wrect.left + zoom.level();
			wrect.bottom = wrect.top + zoom.level();

			SetWindowPos(hwnd, 0, wrect.left, wrect.top, wrect.right - wrect.left, wrect.bottom - wrect.top, SWP_SHOWWINDOW);
		}
	}
}

UINT getDpi(HWND hwnd)
{
	HMONITOR monitor = MonitorFromWindow(hwnd, NULL);
	if (NULL != monitor) {
		MONITORINFOEX mi;
		mi.cbSize = sizeof MONITORINFOEX;
		if (GetMonitorInfo(monitor, &mi)) {
			UINT dpiX, dpiY;
			GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
			return dpiX;
		}
	}

	return 96;
}
