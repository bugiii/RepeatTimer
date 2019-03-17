#include "stdafx.h"
#include <ole2.h>
#include <gdiplus.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cwchar>
#include <cstdio>
#include <cstdarg>
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
	TimerMaxSec timerMaxSec[TM_MAX] = {
		TMS_5, TMS_10, TMS_15, TMS_20, TMS_30, TMS_60, TMS_120, TMS_180, TMS_240, TMS_300
	};

	const int bigScaleDiv[TM_MAX] = {
		5, 10, 3, 4, 6,
		12, 12, 12, 8, 10
	};

	const int smallScaleDiv[TM_MAX] = {
		6, 6, 5, 5, 5,
		5, 10, 5, 6, 10
	};

#define MAKETWOLESS(a) (timerMaxSec[(a)] * (bigScaleDiv[(a)]-2) / bigScaleDiv[(a)])

	int restartDefaultSec[TM_MAX] = {
		MAKETWOLESS(0), MAKETWOLESS(1), MAKETWOLESS(2), MAKETWOLESS(3), MAKETWOLESS(4),
		MAKETWOLESS(5), MAKETWOLESS(6),	MAKETWOLESS(7), MAKETWOLESS(8), MAKETWOLESS(9)
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
	restartSec(restartDefaultSec[maxSecIndex]),
	dialColor(128, 255, 255, 255),
	remainPieColor(128, 255, 0, 0),
	remainPieBegin(0.2f),
	remainPieEnd(0.95f),
	scaleColor(128, 96, 96, 96),
	smallScaleThickness(0.01f),
	smallScaleBegin(0.95f),
	smallScaleEnd(1.0f),
	bigScaleThickness(0.02f),
	bigScaleBegin(0.9f),
	bigScaleEnd(1.0f),
	indexTextPos(0.8f),
	knobColor(128, 64, 64, 64),
	knobEnd(0.15f)
{
	remainSec = restartDefaultSec[maxSecIndex]; // TODO:
	//maxSec_ = timerMaxSec[maxSecIndex];
}

TimerGraphic::~TimerGraphic()
{
}

int TimerGraphic::maxSec()
{
	return timerMaxSec[maxSecIndex];
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

int TimerGraphic::secFromXY(HWND hwnd, int x, int y)
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
	int r = static_cast<int>((M_PI + atan2(static_cast<float>(-x), static_cast<float>(-y))) * 180 / M_PI * maxSec() / 360);
	r = (r + 59) / 60 * 60; // raising to a minute.
	return r;
}

static void fillCircle(Graphics& G, Brush* brush, REAL r)
{
	G.FillEllipse(brush, -r, -r, 2*r, 2*r);
}

static void fillDonut(Graphics& G, Brush* brush, REAL r1, REAL r2, REAL s, REAL e)
{
	GraphicsPath gp(Gdiplus::FillModeAlternate);

	gp.AddPie(-r2, -r2, 2*r2, 2*r2, s, e);
	gp.AddPie(-r1, -r1, 2*r1, 2*r1, s, e);

	G.FillPath(brush, &gp);
}

static void drawString(Graphics& G, Font* font, REAL d, REAL r, StringFormat* format, Brush* brush, const wchar_t* fmt, ...)
{
	int size = 0;
	wchar_t buffer[256] = L"";

	va_list args;
	va_start(args, fmt);
	size = _vsnwprintf_s(buffer, dimof(buffer), _TRUNCATE, fmt, args);

	RectF rect = { -1.0f, -1.0f, 2.0f, 2.0f };
	Matrix mx;
	G.GetTransform(&mx);
	G.TranslateTransform(d, 0.0f);
	G.ScaleTransform(1.0f, -1.0f);
	G.RotateTransform(r);
	G.DrawString(buffer, size, font, rect, format, brush);
	G.SetTransform(&mx);

	va_end(args);
}

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

void TimerGraphic::draw(HDC hdc, int w, int h)
{
	Graphics G(hdc);

	G.SetSmoothingMode(SmoothingModeAntiAlias); // TODO: option
	G.SetTextRenderingHint(TextRenderingHintClearTypeGridFit); // TODO: option
	G.SetTextRenderingHint(TextRenderingHintAntiAlias); // TODO: option

	// screen coordinate to rectangular coordinate
	G.TranslateTransform(w / 2.0f, h / 2.0f);
	// scale (0,0)-(w, h) to (-1, 0)-(1,-1) & flip y coordinate
	G.ScaleTransform(w / 2.0f, -h / 2.0f);
	G.RotateTransform(90.0f); // top is 0 degree
	//GraphicsState gs = G.Save();

	// draw axis, arc
	if (true) {
		Matrix mx;
		G.GetTransform(&mx);
		Pen axisPen(Color::Black, 0.02f);
		axisPen.SetStartCap(LineCapNoAnchor);
		axisPen.SetEndCap(LineCapArrowAnchor);
		G.DrawLine(&axisPen, 0.0f, 0.0f, 0.5f, 0.0f);
		G.DrawArc(&axisPen, -0.2f, -0.2f, 0.4f, 0.4f, 0.0f, 45.0f);
		G.RotateTransform(45.0f);
		G.DrawLine(&axisPen, 0.0f, 0.0f, 0.3f, 0.0f);
		G.SetTransform(&mx);
	}

	// Dial
	// TODO: visible option
	SolidBrush dialBrush(dialColor);
	fillCircle(G, &dialBrush, 1.0f);

	// Scale, Index
	// TODO: visible option (small?)
	Pen smallScalePen(scaleColor, smallScaleThickness);
	Pen bigScalePen(scaleColor, bigScaleThickness);
	SolidBrush indexTextBrush(Color::Black);
	Font indexTextFont(L"Segoe UI", knobEnd / 2);
	RectF indexTextRect(-knobEnd, -bigScaleBegin, 2*knobEnd, knobEnd);
	StringFormat indexTextFormat;
	indexTextFormat.SetAlignment(StringAlignmentCenter);
	indexTextFormat.SetLineAlignment(StringAlignmentCenter);

	REAL accrueDegree = 0.0f;
	for (int bigIndex = 0; bigIndex < bigScaleDiv[maxSecIndex]; ++bigIndex) {
		G.DrawLine(&bigScalePen, 0.0f, bigScaleBegin, 0.0f, bigScaleEnd);

		drawString(G, &indexTextFont, indexTextPos, -accrueDegree + 90, &indexTextFormat, &indexTextBrush,
			L"%d", bigIndex * maxSec() / bigScaleDiv[maxSecIndex] / 60);

		for (int smallIndex = 0; smallIndex < smallScaleDiv[maxSecIndex]; ++smallIndex) {
			REAL degree = 360.0f / (bigScaleDiv[maxSecIndex] * smallScaleDiv[maxSecIndex]);
			accrueDegree += degree;
			G.RotateTransform(-degree);
			if (smallScaleDiv[maxSecIndex] - 1 != smallIndex) {
				G.DrawLine(&smallScalePen, 0.0f, smallScaleBegin, 0.0f, smallScaleEnd);
			}
		}
	}

	// Remain Pie
	SolidBrush pieBrush(remainPieColor);
	REAL remainDegree = -remainSec * 360.0f / maxSec();
	fillDonut(G, &pieBrush, remainPieBegin, remainPieEnd, 0.0f, remainDegree);

	// Knob
	SolidBrush knobBrush(knobColor);
	fillCircle(G, &knobBrush, knobEnd);

	// Remain Text
	SolidBrush remainTextBrush(Color::Black);
	Font remainTextFont(L"Segoe UI", knobEnd/2);
	RectF remainTextRect(-knobEnd, -knobEnd, 2 * knobEnd, 2 * knobEnd);
	StringFormat remainTextFormat;
	remainTextFormat.SetAlignment(StringAlignmentCenter);
	remainTextFormat.SetLineAlignment(StringAlignmentCenter);

	{
		Matrix mx;
		G.GetTransform(&mx);
		G.RotateTransform(-90.0f);
		drawString(G, &remainTextFont, 0.0f, 0.0f, &remainTextFormat, &remainTextBrush, L"%d", remainSec / 60);
		G.SetTransform(&mx);
	}

	G.Flush();
}
