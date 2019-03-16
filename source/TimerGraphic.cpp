#include "stdafx.h"
#include <ole2.h>
#include <gdiplus.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include "TimerGraphic.h"

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#ifndef M_PIl
#define M_PIl (3.14159265358979323846264338327950288)
#endif

#pragma comment(lib, "gdiplus")
using namespace Gdiplus;

///////////////////////////////////////////////////////////////////////////////

namespace bugiii_timer_graph {
	TimerMaxSec timerMaxSec[] = {
		TMS_5, TMS_10, TMS_15, TMS_20, TMS_30, TMS_45, TMS_60, TMS_120, TMS_180, TMS_240, TMS_300
	};

	// 5/6
	#define MAKE5_6(a) ((timerMaxSec[a])*5/6)

	int resetDefaultSec[] = {
		MAKE5_6(0), MAKE5_6(1), MAKE5_6(2), MAKE5_6(3), MAKE5_6(4), MAKE5_6(5),
		MAKE5_6(6),
		MAKE5_6(7), MAKE5_6(8), MAKE5_6(9), MAKE5_6(10)
	};

	// big scale
	#define MAKEBIG(a) (360/(a))

	const int bigScaleUnit[] = {
		MAKEBIG(5), MAKEBIG(10), MAKEBIG(3), MAKEBIG(4), MAKEBIG(6), MAKEBIG(9),
		MAKEBIG(12),
		MAKEBIG(12), MAKEBIG(12), MAKEBIG(8), MAKEBIG(10)
	};

	// small scale
	#define MAKESMALL(a,b) (bigScaleUnit[(a)]/(b))

	const int smallScaleUnit[] = {
		MAKESMALL(TM_5,6), MAKESMALL(TM_10,6), MAKESMALL(TM_15,5), MAKESMALL(TM_20,5), MAKESMALL(TM_30,5), MAKESMALL(TM_45,5),
		MAKESMALL(TM_60,5),
		MAKESMALL(TM_120,10), MAKESMALL(TM_180,10), MAKESMALL(TM_240,10), MAKESMALL(TM_300,10)
	};
}

using namespace bugiii_timer_graph;

///////////////////////////////////////////////////////////////////////////////

GdiPlusInit::GdiPlusInit() {
	Gdiplus::GdiplusStartupInput startupInput;
	Gdiplus::GdiplusStartup(&token_, &startupInput, NULL);
}

GdiPlusInit::~GdiPlusInit() {
	if (token_) {
		Gdiplus::GdiplusShutdown(token_);
	}
}

///////////////////////////////////////////////////////////////////////////////

TimerGraphic::TimerGraphic(const std::string& id) :
	remainSec(50 * 60), 
	id_(id),
	maxSecIndex(TM_60),
	resetSec(static_cast<REAL>(resetDefaultSec[maxSecIndex])),
	dialColor(128, 255, 255, 255),
	pieColor(128, 255, 0, 0),
	pieBegin(0.2f),
	pieEnd(0.95f),
	scaleColor(128, 96, 96, 96),
	smallScaleThickness(0.01f),
	smallScaleBegin(0.95f),
	smallScaleEnd(1.0f),
	bigScaleThickness(0.02f),
	bigScaleBegin(0.9f),
	bigScaleEnd(1.0f),
	knobColor(128, 64, 64, 64),
	knobEnd(0.15f)
{
	maxSec_ = timerMaxSec[maxSecIndex];
}

TimerGraphic::~TimerGraphic()
{
}

bool TimerGraphic::inKnob(HWND hwnd, int x, int y)
{
	RECT rect;
	GetWindowRect(hwnd, &rect);

	// screen coordinate to rectangular and client coordinate
	x = x - (rect.right - rect.left) / 2 - rect.left;
	y = (rect.bottom - rect.top) / 2 - y + rect.top;
	float r = knobEnd * (rect.right - rect.left) / 2;

	return 0 <= r * r - (x*x + y * y);
}

int TimerGraphic::remainSecFromXY(HWND hwnd, int x, int y)
{
	RECT rect;
	GetWindowRect(hwnd, &rect);

	// client coordinate to rectangular coordinate
	x = x - (rect.right - rect.left) / 2;
	y = (rect.bottom - rect.top) / 2 - y;

	/*
		MAGIC FORMULA :)

		The result value of atan2(y,x) is as follows according to
		the angle.
			0 <= th < 180 -> 0 <= remain < pi
			180 <= th < 360 -> -pi <= remain < 0

		Since the value changes discontinuously at 180 degrees,
		if the result is negative, it is necessary to add 2 pi.

		However, I would like to be able to eliminate conditional
		statements with a single expression.

		First, if pi is added unconditionally to the result of atan2,
		the result is:
			0 <= th < 180 -> pi <= remain < 2*pi
			180 <= th < 360 -> 0 <= remain < pi

		The resulting value is continuous but the starting angle is
		180 degrees and the point is selected which is symmetrical to
		the mouse position and center.

		Therefore, if the signs of the x and y coordinates are reversed,
		a desired result can be obtained.

		However, another operation is necessary because it requires
		90 degree rotation and clockwise rotation.

		The 90 degree rotation can be solved by changing the x and y
		positions, and the clockwise rotation can be done by changing
		the sign where the pie is displayed.
	*/
	int r = static_cast<int>((M_PI + atan2(static_cast<float>(-x), static_cast<float>(-y))) * 180 / M_PI * maxSec_ / 360);
	remainSec = (r + 59) / 60 * 60; // raising to a minute.
	return remainSec;
}

void fillCircle(Graphics& G, Brush* brush, REAL r)
{
	G.FillEllipse(brush, -r, -r, 2*r, 2*r);
}

void fillDonut(Graphics& G, Brush* brush, REAL r1, REAL r2, REAL s, REAL e)
{
	GraphicsPath gp(Gdiplus::FillModeAlternate);

	gp.AddPie(-r2, -r2, 2*r2, 2*r2, s, e);
	gp.AddPie(-r1, -r1, 2*r1, 2*r1, s, e);

	G.FillPath(brush, &gp);
}

void TimerGraphic::draw(HWND hwnd, HDC hdc)
{
	RECT wrect;
	GetWindowRect(hwnd, &wrect);
	int w = wrect.right - wrect.left;
	int h = wrect.bottom - wrect.top;
	int r = w / 2;

	HDC memdc = CreateCompatibleDC(hdc);
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, w, h);
	HGDIOBJ oldBitmap = SelectObject(memdc, bitmap);

	draw(memdc, w, h);

	POINT dst = { wrect.left, wrect.top };
	POINT src = { 0, };
	SIZE size = { w, h };
	BLENDFUNCTION blend = { 0, };
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;
	UpdateLayeredWindow(hwnd, hdc, &dst, &size, memdc, &src, 0, &blend, ULW_ALPHA);

	SelectObject(memdc, oldBitmap);
	DeleteObject(bitmap);
	DeleteDC(memdc);
}

void TimerGraphic::draw(HDC hdc, int w, int h)
{
	Graphics G(hdc);

	G.SetSmoothingMode(SmoothingMode::SmoothingModeAntiAlias); // TODO: option

	// screen coordinate to rectangular coordinate
	G.TranslateTransform(w / 2.0f, h / 2.0f);
	// scale (w, h) to (-1, 1) & flip y coordinate
	G.ScaleTransform(w / 2.0f, -h / 2.0f);
	GraphicsState gs = G.Save();

	// Dial
	// TODO: visible option
	SolidBrush dialBrush(dialColor);
	fillCircle(G, &dialBrush, 1.0f);

	// Scale
	// TODO: visible option (small?)
	Pen smallScalePen(scaleColor, smallScaleThickness);
	Pen bigScalePen(scaleColor, bigScaleThickness);
	for (int degree = 0; degree < 360; degree += smallScaleUnit[maxSecIndex]) {
		if (0 == degree % bigScaleUnit[maxSecIndex]) {
			G.DrawLine(&bigScalePen, 0.0f, bigScaleBegin, 0.0f, bigScaleEnd);
		}
		else {
			G.DrawLine(&smallScalePen, 0.0f, smallScaleBegin, 0.0f, smallScaleEnd);
		}
		G.RotateTransform(static_cast<REAL>(smallScaleUnit[maxSecIndex]));
	}

	// Pie
	G.Restore(gs);
	gs = G.Save();
	G.RotateTransform(90); // top is 0 degree

	SolidBrush pieBrush(pieColor);
	REAL remainDegree = -remainSec * 360.0f / maxSec_;
	fillDonut(G, &pieBrush, pieBegin, pieEnd, 0.0f, remainDegree);

	// Knob
	SolidBrush knobBrush(knobColor);
	fillCircle(G, &knobBrush, knobEnd);
	  
	G.Flush();
}
