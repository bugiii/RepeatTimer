#include "stdafx.h"
#include <iterator>
#include <algorithm>
#include "Zoom.h"

static int zoomTable[] = { 64, 128, 256, 512, 1024 };
static const int zoomTableLast = static_cast<int>(std::size(zoomTable));

Zoom::Zoom()
{
	level(128);
	dpi(96);
}

Zoom::~Zoom()
{
}

int Zoom::index() const
{
	return index_;
}

int Zoom::index(int value)
{
	if (0 <= value && value < zoomTableLast) {
		index_ = value;;
	}

	return index_;
}

int Zoom::level() const
{
	return scale(zoomTable[index_]);
}

int Zoom::level(int value)
{
	int pos = static_cast<int>(std::distance(zoomTable, std::lower_bound(&zoomTable[0], &zoomTable[zoomTableLast], value)));

	index_ = pos;
	return level();
}

int Zoom::inout(int dir, bool & ok)
{
	if (dir < 0) {
		return out(ok);
	}
	else if (0 < dir) {
		return in(ok);
	}
	ok = false;

	return level();
}

int Zoom::in(bool& ok)
{
	if (index_ < zoomTableLast - 1) {
		++index_;
		ok = true;
	}
	ok = false;

	return level();
}

int Zoom::out(bool& ok)
{
	if (0 < index_) {
		--index_;
		ok = true;
	}
	ok = false;

	return level();
}

int Zoom::indexLast() const
{
	return zoomTableLast;
}

int Zoom::levelLast() const
{
	return scale(zoomTable[indexLast() - 1]);
}

int Zoom::scale(int value) const
{
	return value * dpi() / 96;
}

int Zoom::dpi() const
{
	return dpi_;
}

int Zoom::dpi(int value)
{
	dpi_ = value;
	return dpi();
}
