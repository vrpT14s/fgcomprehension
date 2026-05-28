#include "perfdata.h"
#include "eventconsumer.h"
#include "layout.h"
#include <iostream>

int main(int argc, char **argv)
{
	assert(argc == 2);
	char *path = argv[1];

	PerfDataFile perf_data{ path };

	EventConsumer consumer;
	perf_data.parse(&consumer);

	//printf("num events: %d\n", perf_data.num_events);
	printf("mmap intervals:\n");
	for (auto &interval: consumer.mmap_intervals) {
		std::cout << interval.dso->filename << ":\t" << (void *)interval.addr << "\n";
	}
	auto rects = Rect::emitRects(consumer.samples);
	for (auto rect: rects) {
		printf("depth - %d\tstart - %f\tend - %f\tmmap dso - %x\tsym - %x\n",
				rect.depth,
				rect.start,
				rect.end,
				rect.mmap,
				rect.sym);
		if (rect.mmap && rect.mmap->dso)
			printf("filename - %s\n", rect.mmap->dso->filename.c_str());

	}
	std::cout << "num samples: " << consumer.samples.size() << '\n';
	std::cout << "num rects: " << rects.size() << '\n';
	return 0;
}
