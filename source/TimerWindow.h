#pragma once

#include <string>

class TimerGraphic;

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

private:
	void onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void onDestroy(HWND hwnd);
	UINT onNCHitTest(HWND hwnd, int x, int y);
	void onPaint(HWND hwnd);

private:
	std::string id_;
	HWND hwnd_;
	TimerGraphic* graph_;
};
