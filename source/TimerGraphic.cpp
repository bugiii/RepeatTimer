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
	id_(id),
	maxSecIndex(TM_60),
	restartSec(restartDefaultSec[maxSecIndex]),
	remainSec(restartDefaultSec[maxSecIndex]),
	alpha(220),
	dialColor(alpha, 255, 255, 255),
	remainPieColor(alpha, 255, 0, 0),
	sparePieColor(alpha, 0, 255, 0),
	pieBegin(0.3f),
	pieEnd(0.95f),
	scaleColor(alpha, 96, 96, 96),
	smallScaleThickness(0.01f),
	smallScaleBegin(0.975f),
	smallScaleEnd(1.0f),
	bigScaleThickness(0.02f),
	bigScaleBegin(0.90f),
	bigScaleEnd(1.0f),
	indexTextPos(0.8f),
	indexTextFontSize(0.075f),
	knobColor(alpha, 96, 96, 96),
	knobEnd(0.25f),
	fontSize(0.1f),
	remainFontColor(alpha, 196, 32, 32),
	spareFontColor(alpha, 32, 196, 32),
	faintDiv(4.0f)
{
}

TimerGraphic::~TimerGraphic()
{
}

int TimerGraphic::maxSec()
{
	return timerMaxSec[maxSecIndex];
}

void TimerGraphic::setMaxSecIndex(TimerMax value)
{
	maxSecIndex = value;
	remainSec = restartDefaultSec[maxSecIndex];
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
	wchar_t buffer[256] = L""; // TODO:

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

Color faintColor(const Color& c, REAL div)
{
	// TODO: clamp
	return Color (
		static_cast<BYTE>(c.GetA() / div),
		static_cast<BYTE>(c.GetR() / div),
		static_cast<BYTE>(c.GetG() / div),
		static_cast<BYTE>(c.GetB() / div));
}

Color blendColor(const Color& a, const Color b)
{
	return Color(
		(a.GetA() + b.GetA()) / 2,
		(a.GetR() + b.GetR()) / 2,
		(a.GetG() + b.GetG()) / 2,
		(a.GetB() + b.GetB()) / 2);
}

void TimerGraphic::draw(HDC hdc, int w, int h)
{
	Graphics G(hdc);
	Matrix mx;

	G.SetSmoothingMode(SmoothingModeAntiAlias); // TODO: option
	G.SetTextRenderingHint(TextRenderingHintClearTypeGridFit); // TODO: option
	G.SetTextRenderingHint(TextRenderingHintAntiAlias); // TODO: option
	G.SetInterpolationMode(InterpolationModeHighQuality);

	// screen coordinate to rectangular coordinate
	G.TranslateTransform(w / 2.0f, h / 2.0f);
	// scale (0,0)-(w, h) to (-1, 0)-(1,-1) & flip y coordinate
	G.ScaleTransform(w / 2.0f, -h / 2.0f);
	G.RotateTransform(90.0f); // top is 0 degree

	// draw axis, arc for debug
	if (false) {
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
	// TODO: visible option (small? index?)
	Pen smallScalePen(scaleColor, smallScaleThickness);
	Pen bigScalePen(scaleColor, bigScaleThickness);
	SolidBrush indexTextBrush(Color::Black);
	Font indexTextFont(L"Segoe UI", indexTextFontSize);
	//Gdiplus::Font indexTextFont(L"Arial", fontSize, FontStyleBold, UnitWorld);
	RectF indexTextRect(-knobEnd, -bigScaleBegin, 2*knobEnd, knobEnd);
	StringFormat indexTextFormat;
	indexTextFormat.SetAlignment(StringAlignmentCenter);
	indexTextFormat.SetLineAlignment(StringAlignmentCenter);

	REAL accrueDegree = 0.0f;
	for (int bigIndex = 0; bigIndex < bigScaleDiv[maxSecIndex]; ++bigIndex) {
		G.DrawLine(&bigScalePen, bigScaleBegin, 0.0f, bigScaleEnd, 0.0f);

		drawString(G, &indexTextFont, indexTextPos, -accrueDegree + 90, &indexTextFormat, &indexTextBrush,
			L"%d", bigIndex * maxSec() / bigScaleDiv[maxSecIndex] / 60);

		for (int smallIndex = 0; smallIndex < smallScaleDiv[maxSecIndex]; ++smallIndex) {
			REAL degree = 360.0f / (bigScaleDiv[maxSecIndex] * smallScaleDiv[maxSecIndex]);
			accrueDegree += degree; // for billboard text of index
			G.RotateTransform(-degree); // rotate CW by small scale degree
			if (smallScaleDiv[maxSecIndex] - 1 != smallIndex) { // not big scale
				G.DrawLine(&smallScalePen, smallScaleBegin, 0.0f, smallScaleEnd, 0.0f);
			}
		}
	}

	// Pie
	SolidBrush pieBrush(remainPieColor);
	REAL remainDegree = -remainSec * 360.0f / maxSec();
	fillDonut(G, &pieBrush, pieBegin, pieEnd, 0.0f, remainDegree);

	SolidBrush faintBrush(faintColor(remainPieColor, faintDiv));
	REAL restartDegree = (maxSec() - restartSec) * 360.0f / maxSec();
	REAL diffDegree = (remainSec - restartSec) * 360.0f / maxSec();

	if (remainSec < restartSec) {
		fillDonut(G, &faintBrush, pieBegin, pieEnd, restartDegree, -diffDegree);
	}

	if (restartSec < remainSec) {
		pieBrush.SetColor(sparePieColor);
		REAL spareDegree = (maxSec() - remainSec) * 360.0f / maxSec();
		fillDonut(G, &pieBrush, pieBegin, pieEnd, spareDegree, diffDegree);
	}

	faintBrush.SetColor(faintColor(sparePieColor, faintDiv));
	fillDonut(G, &faintBrush, pieBegin, pieEnd, 0, restartDegree);

	// Restart Line
	Pen restartPen(blendColor(remainPieColor, sparePieColor), 0.01f);
	G.GetTransform(&mx);
	G.RotateTransform(-restartSec * 360.0f / maxSec());
	G.DrawLine(&restartPen, pieBegin, 0.0f, pieEnd, 0.0f);
	G.SetTransform(&mx);

	// Knob
	SolidBrush knobBrush(knobColor);
	fillCircle(G, &knobBrush, knobEnd);

	// Remain Text
	SolidBrush remainTextBrush(Color::Black);
	Gdiplus::Font remainTextFont(L"Arial", fontSize, FontStyleRegular);
	RectF remainTextRect(-knobEnd, -knobEnd, 2 * knobEnd, 2 * knobEnd);
	StringFormat remainTextFormat;
	remainTextFormat.SetAlignment(StringAlignmentCenter);
	remainTextFormat.SetLineAlignment(StringAlignmentCenter);

	int oSec = (remainSec <= restartSec) ? remainSec : (remainSec - restartSec);
	int rHour = oSec / 60 / 60;
	int rMin = oSec / 60 % 60;
	int rSec = oSec % 60;

	std::wstring knobTextString;
	if (0 < rHour) { // hour, min
		knobTextString = formatString(L"%d\x1D34\n%2d\x1D39", rHour, rMin);
	}
	else if (maxSec() / bigScaleDiv[maxSecIndex] / 60 <= rMin) { // min
		knobTextString = formatString(L"%d\x1D39", rMin);
	}
	else if (0 < rMin) { // min, sec
		knobTextString = formatString(L"%d\x1D39\n%02d", rMin, rSec);
	}
	else { // sec
		knobTextString = formatString(L"%d", rSec);
	}

	SolidBrush fontBrush(Color((remainSec <= restartSec) ? remainFontColor : spareFontColor));
	drawString(G, &remainTextFont, -0.02f, 90.0f, &remainTextFormat, &fontBrush,
		L"%s", knobTextString.c_str());

	SYSTEMTIME time;
	GetLocalTime(&time);
	drawString(G, &remainTextFont, -0.5f, 90.0f, &remainTextFormat, &remainTextBrush,
		L"%2d:%02d", time.wHour, time.wMinute);

	G.Flush();
}
