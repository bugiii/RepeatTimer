#pragma once

#include <cstdint>

template<size_t SIZE, class T> inline size_t dimof(T(&arr)[SIZE]) {
	return SIZE;
}

int dprintf(const char *format, ...);
void displayMenu(HWND hwnd, int x, int y);
uint64_t currentTime();
BOOL stickSide(HWND hwnd, LPWINDOWPOS lpwpos);
