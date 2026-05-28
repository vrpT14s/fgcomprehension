#pragma once
#include "perfdata_types.h"
#include "eventconsumer.h"

class Rect {
public:
	int depth;
	u64 start, end;
	const MmapInterval *mmap;
	u64 sym;

	Rect() = default;

	Rect(const MmapInterval *mmap, u64 sym, int depth, float start)
		: mmap(mmap)
		, sym(sym)
		, depth(depth)
		, start(start) {}

	void closeAt(float end) {
		this->end = end;	
	}

	static std::vector<Rect> emitRects(std::vector<Sample> samples);
};
