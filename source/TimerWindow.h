#pragma once

#include <string>

class TimerGraphic;

enum TimerRepeatMode {
	TRM_NONE,
	TRM_RESTART,
	TRM_RESTART_SPARE,
	TRM_ON_THE_HOUR
};

class TimerWindow
{
public:
	TimerWindow(const std::string& id);
	HWND hwnd() { return hwnd_; }

private:
	TimerWindow(const TimerWindow&) = delete; // non-copyable
	TimerWindow& operator=(const TimerWindow&) = delete; // non-copyable
	~TimerWindow(); // prohibiting stack-based objects

private:
	static ATOM registerClass();
	HWND createWindow();
	static LRESULT CALLBACK staticWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	uint64_t setupStartTime();
	void processTime();

private:
	void onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void onDestroy(HWND hwnd);
	UINT onDpiChanged(HWND hwnd, int x, int y, LPRECT rect);
	void onLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void onLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
	void onMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
	UINT onNCHitTest(HWND hwnd, int x, int y);
	void onNCRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest);
	void onPaint(HWND hwnd);
	void onRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
	void onTimer(HWND hwnd, UINT id);
	BOOL onWindowPosChanging(HWND hwnd, LPWINDOWPOS lpwpos);

private:
	std::string id_;
	HWND hwnd_;
	TimerGraphic* graph_;
	bool captured_;
	TimerRepeatMode repeatMode_;
	uint64_t startTime_; // must be after repeatMode_
	Zoom zoom_;
};
