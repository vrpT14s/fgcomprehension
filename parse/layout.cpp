#include "layout.h"
#include <algorithm>

static void testsamples(std::vector<Sample> samples) {
	for (int i = 1; i < samples.size(); i++) {
		//printf("%lf, %lf\n", (double)samples[i-1].time / 1000, (double)samples[i].time / 1000);
		assert(samples[i - 1].time <= samples[i].time);
	}
}

//split for each pid/tid pair, generate a map from pid/tid to vec rect
std::vector<Rect> Rect::emitRects(
	std::vector<Sample> samples
) {
	std::sort(samples.begin(), samples.end(), [](Sample s1, Sample s2){ return s1.time < s2.time; });
	testsamples(samples);


	std::vector<Rect> rects;
	std::vector<Rect> open_rects;

	for (int curr = 0; curr < samples.size(); curr++) {
		auto sample = samples[curr];
		auto callchain = sample.callchain;
		int match_len = 0;
		for (int i = 0; i < callchain.size() && i < open_rects.size(); i++) {
			auto [mmap, sym, off] = callchain[i];
			if (mmap == open_rects[i].mmap && sym == open_rects[i].sym)
				match_len++;
			else 
				break;
		}
		for (int i = match_len; i < open_rects.size(); i++) {
			open_rects[i].closeAt(sample.time);
			rects.push_back(open_rects[i]);
		}
		open_rects.resize(match_len);
		for (int i = match_len; i < callchain.size(); i++) {
			auto [mmap, sym, off] = callchain[i];
			Rect rect(mmap, sym, i, sample.time);
			open_rects.push_back(rect);
		}
	}
	for (auto rect: open_rects) {
		rect.closeAt(samples.back().time - 1);
		rects.push_back(rect);
	}

	for (auto rect: rects) {
		if (!rect.mmap || !rect.mmap->dso)
			continue;
		rect.mmap->dso->fillSymCache(rect.sym);
	}

	return rects;
}
