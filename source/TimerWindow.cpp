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
	const int timeDiv = debug ? 10 : 10000;
}

using namespace bugiii_timer_window;

static HINSTANCE defaultInstance()
{
	return (HINSTANCE)GetModuleHandle(nullptr);
}

///////////////////////////////////////////////////////////////////////////////

TimerWindow::TimerWindow(const std::wstring& id) :
	id_(id),
	hwnd_(0),
	menu_(LoadMenu(NULL, MAKEINTRESOURCE(IDC_TIMER_MENU))),
	graph_(new TimerGraphic(id)),
	captured_(false),
	startTime_(setupStartTime()),
	zoom_()
{
	if (!classAtom_) {
		classAtom_ = registerClass();
	}

	hwnd_ = createWindow();
	if (hwnd_) {
		++windowCount_;
		zoom_.index(3);
		resizeAsZoom(hwnd_, zoom_);
	}
}

TimerWindow::~TimerWindow()
{
	delete graph_;
	DestroyMenu(menu_);

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
	wcex.cbWndExtra = 0;
	wcex.hInstance = defaultInstance();
	wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_REPEATTIMER));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL; // don't erase backgroud
	wcex.lpszMenuName = nullptr; // no menu bar
	wcex.lpszClassName = className_;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

HWND TimerWindow::createWindow()
{
	HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_APPWINDOW | WS_EX_LAYERED,
		MAKEINTATOM(classAtom_), id_.c_str(), WS_POPUP,
		0, 0, 500, 500, nullptr, nullptr, defaultInstance(),
		this); // pass this pointer to WM_NCCREATE

	if (!hwnd) {
		return 0;
	}

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);
	InvalidateRect(hwnd, nullptr, FALSE);

	SetTimer(hwnd, 1, timerPeriod, nullptr);

	return hwnd;
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK TimerWindow::staticWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (WM_NCCREATE == msg) { // first msg
		// save this pointer of new window
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
	}

	// retreive this pointer
	TimerWindow* tw = reinterpret_cast<TimerWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	LRESULT r = tw ? tw->windowProc(hwnd, msg, wParam, lParam) : 0;

	if (WM_NCDESTROY == msg) { // last msg
		delete tw;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
	}

	return r;
}

LRESULT CALLBACK TimerWindow::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		HANDLE_MSG(hwnd, WM_COMMAND, onCommand);
		HANDLE_MSG(hwnd, WM_DPICHANGED, onDpiChanged);
		HANDLE_MSG(hwnd, WM_LBUTTONDOWN, onLButtonDown);
		HANDLE_MSG(hwnd, WM_LBUTTONUP, onLButtonUp);
		HANDLE_MSG(hwnd, WM_MOUSEMOVE, onMouseMove);
		HANDLE_MSG(hwnd, WM_NCHITTEST, onNCHitTest);
		HANDLE_MSG(hwnd, WM_NCRBUTTONDOWN, onNCRButtonDown);
		HANDLE_MSG(hwnd, WM_PAINT, onPaint);
		HANDLE_MSG(hwnd, WM_RBUTTONDOWN, onRButtonDown);
		HANDLE_MSG(hwnd, WM_TIMER, onTimer);
		HANDLE_MSG(hwnd, WM_WINDOWPOSCHANGING, onWindowPosChanging);
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////

uint64_t TimerWindow::setupStartTime()
{
	uint64_t time = currentTime();

	if (TRM_ON_THE_HOUR == graph_->repeatMode) {
		//uint64_t maxTime = graph_->maxSec() * 1000ULL/*ms*/ / timeDiv;
		//time = time / maxTime * maxTime; // nomalized to the max time
		uint64_t maxTime = 60 * 60 * 1000ULL/*ms*/ / timeDiv; // one hour
		time = time / maxTime * maxTime; // nomalized to one hour
	}

	return time;
}

void TimerWindow::processTime()
{
	uint64_t diffSec = (currentTime() - graph_->restartSec) / 1000ULL/*ms*/ / timeDiv; //??? unit ???
	uint64_t modSec = diffSec % graph_->maxSec();

	switch (graph_->repeatMode) {
	case TRM_NONE:
		break;
	case TRM_RESTART:
		break;
	case TRM_RESTART_SPARE:
		graph_->remainSec = graph_->maxSec() - static_cast<int>(modSec) - (graph_->maxSec() - graph_->restartSec);
		break;
	case TRM_ON_THE_HOUR:
		graph_->remainSec = currentTime() / 1000ULL / timeDiv % graph_->maxSec();
		break;
	}
}

void TimerWindow::displayMenu(HWND hwnd, int x, int y)
{
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	mii.fType = MFT_RADIOCHECK;

	for (int i = 0; i < zoom_.indexLast(); ++i) {
		mii.fState = (i == zoom_.index()) ? MFS_CHECKED : MFS_UNCHECKED;
		SetMenuItemInfo(menu_, i + ID_SIZE_TINY, FALSE, &mii);
	}

	for (int i = 0; i < TM_MAX; ++i) {
		mii.fState = (i == graph_->maxSecIndex) ? MFS_CHECKED : MFS_UNCHECKED;
		SetMenuItemInfo(menu_, i + ID_MAX_5, FALSE, &mii);
	}

	HMENU popup = GetSubMenu(menu_, 0);
	TrackPopupMenuEx(popup, TPM_LEFTALIGN, x, y, hwnd, nullptr);
}

///////////////////////////////////////////////////////////////////////////////

void TimerWindow::onCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	// TODO: scale
	// TODO: hidpi
	// TODO: UI lock
	// TODO: multi timer
	// TODO: multi timer sticky

	int index;

	switch (id)
	{
	case IDM_ABOUT:
		extern INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		DialogBox(defaultInstance(), MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
		break;

	case ID_TIMERMENU_EXIT:
		DestroyWindow(hwnd);
		break;

	case ID_SIZE_TINY:
	case ID_SIZE_SMALL:
	case ID_SIZE_NORMAL:
	case ID_SIZE_BIG:
	case ID_SIZE_HUGE:
		index = id - ID_SIZE_TINY;
		zoom_.index(index);
		resizeAsZoom(hwnd, zoom_);
		InvalidateRect(hwnd, nullptr, FALSE);
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
		index = id - ID_MAX_5;
		graph_->setMaxSecIndex(static_cast<TimerMax>(index));
		InvalidateRect(hwnd, nullptr, FALSE);
		break;
	}
}

UINT TimerWindow::onDpiChanged(HWND hwnd, int x, int y, LPRECT rect)
{
	zoom_.dpi(getDpi(hwnd));
	resizeAsZoom(hwnd, zoom_);
	InvalidateRect(hwnd, nullptr, FALSE);
	return 0;
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
		InvalidateRect(hwnd, nullptr, FALSE);
	}
}

void TimerWindow::onMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	if (captured_) {
		int r = graph_->secFromXY(hwnd, x, y);
		graph_->restartSec = r;
		graph_->remainSec = r;
		InvalidateRect(hwnd, nullptr, FALSE);
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
	InvalidateRect(hwnd, nullptr, FALSE);
}

BOOL TimerWindow::onWindowPosChanging(HWND hwnd, LPWINDOWPOS lpwpos)
{
	// TODO: 0 x 0 ?
	if (0 == lpwpos->x && 0 == lpwpos->y && 0 == lpwpos->cx && 0 == lpwpos->cy) {
		return TRUE;
	}

	return stickSide(hwnd, lpwpos, GetCapture() != NULL);
}
