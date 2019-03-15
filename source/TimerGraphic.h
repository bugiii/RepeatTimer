#pragma once

class TimerGraphic
{
public:
	TimerGraphic();
	~TimerGraphic();

public:
	void draw(HWND hwnd, HDC hdc);

private:
	void draw(HDC hdc, int w, int h);
};

