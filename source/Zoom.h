#pragma once

class Zoom
{
public:
	Zoom();
	~Zoom();
	int index() const;
	int index(int value);
	int level() const;
	int level(int value);
	int inout(int dir) { bool ok; return inout(dir, ok); }
	int inout(int dir, bool& ok);
	int in() { bool ok; return in(ok); }
	int in(bool& ok);
	int out() { bool ok; return out(ok); }
	int out(bool& ok);
	int scale(int value) const;
	int dpi() const;
	int dpi(int value);

	int indexLast() const;
	int levelLast() const;

private:
	int index_;
	int dpi_;
};
