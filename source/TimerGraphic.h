#pragma once

#include <ole2.h>
#include <gdiplus.h>
#include <string>

///////////////////////////////////////////////////////////////////////////////

class GdiPlusInit
{
public:
	GdiPlusInit();
	~GdiPlusInit();
	GdiPlusInit(const GdiPlusInit&) = delete;
	GdiPlusInit& operator=(const GdiPlusInit&) = delete;

private:
	ULONG_PTR token_ = 0;
}; //TODO: singleton

///////////////////////////////////////////////////////////////////////////////

enum TimerRepeatMode {
	TRM_NONE,
	TRM_RESTART,
	TRM_RESTART_SPARE,
	TRM_ON_THE_HOUR
};

enum TimerMax
{
	TM_5 = 0,
	TM_10,
	TM_15,
	TM_20,
	TM_30,
	TM_60,
	TM_120,
	TM_180,
	TM_240,
	TM_300,
	TM_MAX
};

enum TimerMaxSec
{
	TMS_5 = 5 * 60,
	TMS_10 = 10 * 60,
	TMS_15 = 15 * 60,
	TMS_20 = 20 * 60,
	TMS_30 = 30 * 60,
	TMS_60 = 60 * 60,
	TMS_120 = 120 * 60,
	TMS_180 = 180 * 60,
	TMS_240 = 240 * 60,
	TMS_300 = 300 * 60
};

///////////////////////////////////////////////////////////////////////////////

class TimerGraphic
{
public:
	TimerGraphic(const std::string& id);
	~TimerGraphic();
	static void init();
	static void fini();

public:
	void draw(HWND hwnd, HDC hdc);
	bool inKnob(HWND hwnd, int x, int y);
	int secFromXY(HWND hwnd, int x, int y);
	int maxSec();
	void setMaxSecIndex(TimerMax value);
	Gdiplus::REAL secToDegree(int sec);

private:
	void draw(HDC hdc, int w, int h);

public:
	const std::string& id_;
	TimerRepeatMode repeatMode;
	TimerMax maxSecIndex;
	int restartSec;
	int remainSec;
	BYTE alpha;
	Gdiplus::Color dialColor;
	Gdiplus::Color remainPieColor, sparePieColor;
	Gdiplus::REAL pieBegin, pieEnd;
	Gdiplus::Color scaleColor;
	Gdiplus::REAL smallScaleThickness, smallScaleBegin, smallScaleEnd;
	Gdiplus::REAL bigScaleThickness, bigScaleBegin, bigScaleEnd;
	Gdiplus::REAL indexTextPos;
	Gdiplus::REAL indexTextFontSize;
	Gdiplus::Color knobColor;
	Gdiplus::REAL knobEnd;
	Gdiplus::REAL fontSize;
	Gdiplus::Color remainFontColor, spareFontColor;
	Gdiplus::REAL faintDiv;

private:
	//TimerMaxSec maxSec_;
};
