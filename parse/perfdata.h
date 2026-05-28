#pragma once
#include "perfdata_types.h"
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>

#include "eventconsumer.h"

class PerfDataFile {
public:
	PerfDataFile(char *path)
		: path(path)
		, filesize(getFileSize(path))
	{
		int fd = open(path, O_RDONLY);
		assert(fd > 0);
		buf = (char *)mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
		assert(buf);
	}

	void parse(EventConsumer *consumer);
private:
	std::string path;
	size_t filesize;
	char *buf;
	int num_events = 0;
	struct perf_event_attr *attrs = NULL;
	int num_attrs = 0;

	static size_t getFileSize(char *path) {
		struct stat statbuf;
		int ret = stat(path, &statbuf);
		if (ret != 0) {
			perror("stat");
			assert(ret == 0);
		}
		assert(statbuf.st_size > 0);
		return statbuf.st_size;
	}

	void parsePerfDataSection(EventConsumer *consumer, char *buf, size_t len);
	void handlePerfEvent(EventConsumer *consumer, struct perf_event_header *ehdr);
	void parsePerfSample(struct perf_event_header *ehdr, struct perf_sample_parsed *out);
};

