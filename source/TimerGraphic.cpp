#include "stdafx.h"
#include "TimerGraphic.h"

#pragma comment(lib, "gdiplus")
using namespace Gdiplus;

TimerGraphic::TimerGraphic()
{
}


TimerGraphic::~TimerGraphic()
{
}

void fillCircle(Graphics& G, Brush* brush, REAL r)
{
	G.FillEllipse(brush, -r, r, r, -r);
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

	G.TranslateTransform(w / 2.0f, h / 2.0f);
	G.ScaleTransform(w / 2.0f, -h / 2.0f);

	Color red(128, 255, 0, 0);
	SolidBrush redBrush(red);
	fillCircle(G, &redBrush, 1.0f);

	G.Flush();
}
