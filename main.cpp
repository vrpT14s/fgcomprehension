#include "demo_app.h"
#include "parse/perfdata.h"
#include "parse/eventconsumer.h"
#include "parse/layout.h"

#include <cmath>
#include "helper.h"

class Flamegraph: public Application {
	float f = 0.0f;
	int i = 0;
	EventConsumer consumer;
	PerfDataFile perf_data;
	std::string path;
	std::vector<Rect> rects;

	int bias = 0;
public:
	Flamegraph(std::string path)
			: path(path)
			, perf_data((char *)path.c_str())
	{
		perf_data.parse(&consumer);
		auto process_samples = consumer.samples.begin()->second;
		rects = Rect::emitRects(process_samples);
		for (auto rect: rects) {
			auto data = rect.getSymData();
			if (!data)
				continue;
			//std::cout << rect.depth << '\t' << data->demangled_name << '\n';
			std::cout << rect.depth << '\t' << data->demangled_name << '\t' << data->path << ':' << data->line << ':' << data->column << '\n';
		}
	}



	void draw() override {
		ImGui::Begin("Flamegraph");
		ImVec2 avail_size = ImGui::GetContentRegionAvail();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImDrawList *drawlist = ImGui::GetWindowDrawList();

		auto samples = consumer.samples.begin()->second;
		auto start_time = samples.front().time;
		auto end_time = samples.back().time;
		int totalx = end_time - start_time;
		int totaly = 40;

		for (auto rect: rects) {
			if (rect.start == rect.end)
				continue;
			//printf("%lf, %lf\n", (double)rect.start / 1000, (double) rect.end / 1000);
			int start = std::round(((double)(rect.start - start_time)) / (double)totalx * (double)avail_size.x);
			int end = std::round(((double)(rect.end - start_time)) / (double)totalx * (double)avail_size.x);
			float depth_start = ((float)(rect.depth)) / totaly * avail_size.y;
			float depth_end = ((float)(rect.depth + 1)) / totaly * avail_size.y;
			drawlist->AddRectFilled(
				{pos.x + start, pos.y + std::round(depth_start)},
				{pos.x + end,   pos.y + std::round(depth_end)},
				//hash((u64)(rect.mmap ? rect.mmap->dso : NULL), bias), 1.0f);
				hash((u64)((rect.mmap ? rect.mmap->dso : NULL) + rect.sym), bias), 1.0f);
			auto data = rect.getSymData();
			if (!data)
				continue;
			drawRectLabel(drawlist, 
				{pos.x + start, pos.y + std::round(depth_start)},
				{pos.x + end,   pos.y + std::round(depth_end)},
				data->demangled_name.c_str()
			);
			//printf("start=%f end=%f depth=%d\n", rect.start, rect.end, rect.depth);
			//printf("(%f, %f), (%f, %f)\n", start, depth_start, end, depth_end);
		}

		ImGui::End();

		ImGui::Begin("Controls");
		ImGui::SliderInt("bias", &bias, 0, 100);
		ImGui::End();
	}
};

int main(int argc, const char **argv) {
	const char *filename = (argc >= 2) 
			? argv[1] 
			: "perf.data";
	Flamegraph app{ filename };
	app.run();
}
