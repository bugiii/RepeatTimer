#pragma once

template<size_t SIZE, class T> inline size_t dimof(T(&arr)[SIZE]) {
	return SIZE;
}

int dprintf(const char *format, ...);
