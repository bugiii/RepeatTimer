#include "stdafx.h"
#include "resource.h"
#include "TimerWindow.h"
#include "TimerGraphic.h"

namespace bugiii_timer_window {
	LPCWSTR className_ = L"TimerWindow_Class";
	ATOM classAtom_ = 0;
	int windowCount_ = 0;
}

using namespace bugiii_timer_window;

static HINSTANCE defaultInstance()
{
	return (HINSTANCE)GetModuleHandle(nullptr);
}

///////////////////////////////////////////////////////////////////////////////

TimerWindow::TimerWindow(const std::string& id) :
	id_(id),
	hwnd_(0),
	graph_(new TimerGraphic(id)),
	captured_(false)
{
	if (!classAtom_) {
		classAtom_ = registerClass();
	}

	hwnd_ = createWindow();
	if (hwnd_) {
		++windowCount_;
	}
}

TimerWindow::~TimerWindow()
{
	delete graph_;

	if (hwnd_) {
		--windowCount_;
		if (0 == windowCount_) {
			PostQuitMessage(0);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

ATOM TimerWindow::registerClass()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = staticWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(TimerWindow*); // for saving this pointer of each instance
	wcex.hInstance = defaultInstance();
	wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_REPEATTIMER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = NULL; // don't erase backgroud
	wcex.lpszMenuName = NULL; // no menu bar
	wcex.lpszClassName = className_;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

HWND TimerWindow::createWindow()
{
	HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_APPWINDOW | WS_EX_LAYERED,
		MAKEINTATOM(classAtom_), L"", WS_POPUP,
		0, 0, 500, 500, nullptr, nullptr, defaultInstance(), nullptr);

	if (!hwnd) {
		return 0;
	}

	// save this pointer
	SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);
	InvalidateRect(hwnd, NULL, FALSE);

	return hwnd;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK TimerWindow::staticWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// retreive this pointer
	TimerWindow* tw = reinterpret_cast<TimerWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	return tw->windowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK TimerWindow::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		HANDLE_MSG(hwnd, WM_COMMAND, onCommand);
		HANDLE_MSG(hwnd, WM_DESTROY, onDestroy);
		HANDLE_MSG(hwnd, WM_LBUTTONDOWN, onLButtonDown);
		HANDLE_MSG(hwnd, WM_LBUTTONUP, onLButtonUp);
		HANDLE_MSG(hwnd, WM_MOUSEMOVE, onMouseMove);
		HANDLE_MSG(hwnd, WM_NCHITTEST, onNCHitTest);
		HANDLE_MSG(hwnd, WM_PAINT, onPaint);

	default: return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

///////////////////////////////////////////////////////////////////////////////

void TimerWindow::onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case IDM_ABOUT:
		extern INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		DialogBox(defaultInstance(), MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
		break;
	case IDM_EXIT:
		DestroyWindow(hwnd);
		break;
	}
}

void TimerWindow::onDestroy(HWND hwnd)
{
	delete this;
}

void TimerWindow::onLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	SetCapture(hwnd);
	captured_ = true;
	onMouseMove(hwnd, x, y, keyFlags);

	// TODO: snap to big scale on near dial border.
}

void TimerWindow::onLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
	if (captured_) {
		captured_ = false;
		ReleaseCapture();
		onMouseMove(hwnd, x, y, keyFlags);
	}
}

void TimerWindow::onMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	if (captured_) {
		int r = graph_->secFromXY(hwnd, x, y);
		graph_->restartSec = r;
		graph_->remainSec = r;
		InvalidateRect(hwnd, NULL, 0);
	}
}

UINT TimerWindow::onNCHitTest(HWND hwnd, int x, int y)
{
	return graph_->inKnob(hwnd, x, y) ? HTCAPTION : HTCLIENT;
}

void TimerWindow::onPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	graph_->draw(hwnd, hdc);
	EndPaint(hwnd, &ps);
}
