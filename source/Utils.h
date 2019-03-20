#pragma once

#include <string>
#include <cstdint>

template<size_t SIZE, class T> inline size_t dimof(T(&arr)[SIZE]) {
	return SIZE;
}

int dprintf(const char *format, ...);
std::wstring formatString(const wchar_t *format, ...);
void displayMenu(HWND hwnd, int x, int y);
uint64_t currentTime();
BOOL stickSide(HWND hwnd, LPWINDOWPOS lpwpos, bool moving);
void resizeAsZoom(HWND hwnd, const Zoom& zoom);
UINT getDpi(HWND hwnd);

/* void Cls_OnDpiChanged(HWND hwnd, int x, int y, LPRECT rect) */
#define HANDLE_WM_DPICHANGED(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)((fn)((hwnd), LOWORD(wParam), HIWORD(wParam), (LPRECT)(lParam)))
#define FORWARD_WM_DPICHANGED(hwnd, x, y, rect) \
    (void)(fn)((hwnd), WM_DPICHANGED, MAKEWPARAM(x,y), (LPARAM)(rect))
