#pragma once

class Lib {
	private:
		/* data */
		int a, b;
	public:
		Lib(int ax, int bx) : a(ax), b(bx) {}
		~Lib() = default;
		const int getSum() const;
};