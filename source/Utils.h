#pragma once

#include <string>
#include <cstdint>

template<size_t SIZE, class T> inline size_t dimof(T(&arr)[SIZE]) {
	return SIZE;
}

template<class T> inline T if3(T a, T b, T l, T e, T g) {
	return (a < b) ?  l : (a == b) ? e : g;
}

int dprintf(const char *format, ...);
std::wstring formatString(const wchar_t *format, ...);
uint64_t currentTime();
BOOL stickSide(HWND hwnd, LPWINDOWPOS lpwpos, bool moving);
void resizeAsZoom(HWND hwnd, const Zoom& zoom);
UINT getDpi(HWND hwnd);

#define HANDLE_MSG_BREAK(hwnd, message, fn)    \
    case (message): HANDLE_##message((hwnd), (wParam), (lParam), (fn)); break

/* void Cls_OnDpiChanged(HWND hwnd, int x, int y, LPRECT rect) */
#define HANDLE_WM_DPICHANGED(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)((fn)((hwnd), LOWORD(wParam), HIWORD(wParam), (LPRECT)(lParam)))
#define FORWARD_WM_DPICHANGED(hwnd, x, y, rect) \
    (void)(fn)((hwnd), WM_DPICHANGED, MAKEWPARAM(x,y), (LPARAM)(rect))
