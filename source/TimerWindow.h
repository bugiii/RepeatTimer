#pragma once

class TimerGraphic;

class TimerWindow
{
public:
	TimerWindow();

private:
	~TimerWindow(); // prohibiting stack-based objects
	HWND hwnd() { return hwnd_;  }

private:
	static ATOM registerClass();
	HWND createWindow();
	static LRESULT CALLBACK staticWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	LRESULT windowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	void onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void onDestroy(HWND hwnd);
	void onPaint(HWND hwnd);

private:
	HWND hwnd_;
	TimerGraphic* graph_;
};
