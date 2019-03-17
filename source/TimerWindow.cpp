#include "stdafx.h"
#include "resource.h"
#include "TimerWindow.h"
#include "TimerGraphic.h"

namespace bugiii_timer_window {
	bool debug = false;
	LPCWSTR className_ = L"TimerWindow_Class";
	ATOM classAtom_ = 0;
	int windowCount_ = 0;
	const int timerPeriod = debug ? 100 : 1000;
	const int timeDiv = debug ? 100 : 10000;
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
	captured_(false),
	repeatMode_(TRM_ON_THE_HOUR),
	startTime_(setupStartTime())
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

	SetTimer(hwnd, 1, timerPeriod, NULL);

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
		HANDLE_MSG(hwnd, WM_NCRBUTTONDOWN, onNCRButtonDown);
		HANDLE_MSG(hwnd, WM_PAINT, onPaint);
		HANDLE_MSG(hwnd, WM_RBUTTONDOWN, onRButtonDown);
		HANDLE_MSG(hwnd, WM_TIMER, onTimer);
		HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGING, onWindowPosChanging);

	default: return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

///////////////////////////////////////////////////////////////////////////////

uint64_t TimerWindow::setupStartTime()
{
	uint64_t time = currentTime();

	if (TRM_ON_THE_HOUR == repeatMode_) {
		//uint64_t maxTime = graph_->maxSec() * 1000ULL/*ms*/ / timeDiv;
		//time = time / maxTime * maxTime; // nomalized to the max time
		uint64_t maxTime = 60 * 60 * 1000ULL/*ms*/ / timeDiv; // one hour
		time = time / maxTime * maxTime; // nomalized to one hour
	}

	return time;
}

void TimerWindow::processTime()
{
	uint64_t diffSec = (currentTime() - graph_->restartSec) / 1000ULL/*ms*/ / timeDiv;
	uint64_t modSec = diffSec % graph_->maxSec();

	switch (repeatMode_) {
	case TRM_NONE:
		break;
	case TRM_RESTART:
		break;
	case TRM_RESTART_SPARE:
		break;
	case TRM_ON_THE_HOUR:
		graph_->remainSec = graph_->maxSec() - modSec;
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////

void TimerWindow::onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	// TODO: scale
	// TODO: hidpi
	// TODO: UI lock
	// TODO: sticky
	// TODO: multi timer
	// TODO: multi timer sticky

	switch (id)
	{
	case IDM_ABOUT:
		extern INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		DialogBox(defaultInstance(), MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
		break;

	case ID_TIMERMENU_EXIT:
		DestroyWindow(hwnd);
		break;

	case ID_MAX_5:
	case ID_MAX_10:
	case ID_MAX_15:
	case ID_MAX_20:
	case ID_MAX_30:
	case ID_MAX_60:
	case ID_MAX_120:
	case ID_MAX_180:
	case ID_MAX_240:
	case ID_MAX_300:
		int secIndex = id - ID_MAX_5;
		graph_->setMaxSecIndex(static_cast<TimerMax>(secIndex));
		InvalidateRect(hwnd, NULL, 0);
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
		processTime();
		InvalidateRect(hwnd, NULL, 0);
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

void TimerWindow::onNCRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT codeHitTest)
{
	displayMenu(hwnd, x, y);
}

void TimerWindow::onPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	graph_->draw(hwnd, hdc);
	EndPaint(hwnd, &ps);
}

void TimerWindow::onRButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	POINT p = { x, y };
	ClientToScreen(hwnd, &p);
	displayMenu(hwnd, p.x, p.y);
}

void TimerWindow::onTimer(HWND hwnd, UINT id)
{
	if (captured_) {
		return;
	}

	processTime();
	InvalidateRect(hwnd, NULL, 0);
}

BOOL TimerWindow::onWindowPosChanging(HWND hwnd, LPWINDOWPOS lpwpos)
{
	// TODO: 0 x 0 ?
	static bool first = true; // prevent from side effect of locating pos. by mouse cursor xy
	if (first || (0 == lpwpos->x && 0 == lpwpos->y && 0 == lpwpos->cx && 0 == lpwpos->cy)) {
		first = false;
		return TRUE;
	}
	return stickSide(hwnd, lpwpos);
}