#include "perfdata.h"

void PerfDataFile::parse(EventConsumer *consumer) {
	assert(filesize >= sizeof(struct perf_header));
	//global hdr
	struct perf_header *ghdr = (struct perf_header *)buf; 
	assert(strncmp(ghdr->magic, "PERFILE2", 8) == 0);
	//printf("global hdr size = %d\n", ghdr->size);
	//printf("attr size = %d\n", ghdr->attr_size);
	this->attrs = (struct perf_event_attr *)(buf + ghdr->attrs.offset);
	num_attrs = ghdr->attrs.size / ghdr->attr_size;
	//printf("num_attrs = %d / %d = %d\n",
	//		ghdr->attrs.size , ghdr->attr_size,
	//		num_attrs);
	for (int i = 0; i < num_attrs; i++) {
		//printf("attrs type = %d (out of hard %d, soft %d, trace %d, hwcache %d, raw %d, breakpoint %d) \n", attrs[i].type,
		//		PERF_TYPE_HARDWARE,
		//		PERF_TYPE_SOFTWARE,
		//		PERF_TYPE_TRACEPOINT,
		//		PERF_TYPE_HW_CACHE,
		//		PERF_TYPE_RAW,
		//		PERF_TYPE_BREAKPOINT
		//);
		//printf("attrs size = %d\n", attrs[i].size);
		//printf("attrs period/freq = %d\n", attrs[i].sample_freq);
		//printf("attrs sample type = %d\n", attrs[i].sample_type);
		//printf("attrs read format = %d\n", attrs[i].read_format);
	}

	//printf("data size: %d\n", ghdr->data.size);

	parsePerfDataSection(consumer, buf + ghdr->data.offset, ghdr->data.size);
}

void PerfDataFile::parsePerfDataSection(EventConsumer *consumer, char *buf, size_t len) {
	num_events = 0;
	for (struct perf_event_header *ehdr = (struct perf_event_header *)buf;
			((char *)ehdr) < buf + len;
			ehdr = (struct perf_event_header *) (((char *)ehdr) + ehdr->size)) {
		num_events++;
		handlePerfEvent(consumer, ehdr);
	}
	//printf("num_events = %d\n", num_events);
}

void PerfDataFile::handlePerfEvent(EventConsumer *consumer, struct perf_event_header *ehdr) {
	switch (ehdr->type) {
	case PERF_RECORD_SAMPLE:
		{
			//puts("perf record sample");
			struct perf_sample_parsed sample;
			parsePerfSample(ehdr, &sample);

			std::vector<u64> callchain;
			for (int i = sample.callchain_nr - 1; i >= 0; i--)
				callchain.push_back(sample.callchain_ips[i]);
			consumer->addSample(sample.pid, sample.tid, sample.time, callchain);
		}
		break;

	case PERF_RECORD_MMAP:
		puts("perf record mmap");
		assert(!"we don't handle non-mmap2 mmaps!");
		break;
	case PERF_RECORD_MMAP2:
		//puts("perf record mmap2");
		{
			if (ehdr->misc & PERF_RECORD_MISC_MMAP_BUILD_ID) {
				puts("build id enabled in mmap");
			}
			//struct perf_event_mmap2 *rec = ((void *)ehdr) + sizeof(*ehdr);
			struct perf_event_mmap2 *rec = (perf_event_mmap2 *)ehdr;
			//printf("pid\t%d\ntid\t%d\naddr\t0x%llx\nlen\t0x%llx\npgoff\t0x%llx\n",
			//		rec->pid,
			//		rec->tid,
			//		rec->addr,
			//		rec->len,
			//		rec->pgoff);
			//printf("filename - %s\n", rec->filename);
			consumer->addMmapRegion(
				rec->pid,
				rec->tid,
				rec->addr,
				rec->len,
				rec->pgoff,
				rec->filename
			);
		}
		break;
	case PERF_RECORD_COMM:
		puts("perf record comm");
		{
			struct perf_event_comm *rec = (struct perf_event_comm *)ehdr;
			//printf("pid\t%d\ntid\t%d\n", rec->pid, rec->tid);
			//printf("comm\t%s\n", rec->comm);
			//EventConsumer::AddComm(rec);
		}
		break;
	case PERF_RECORD_FORK:
		//puts("perf record fork");
		break;
	case PERF_RECORD_EXIT:
		//puts("perf record exit");
		break;
	case PERF_RECORD_TEXT_POKE:
		//puts("perf record text poke");
		break;
	default:
		//printf("unhandled perf record type: %d\n", ehdr->type);
		break;
	}
}

#define CONSUME_FIELD(cursor, type, dest) \
    do { \
        (dest) = *(type *)(cursor); \
        (cursor) += sizeof(type); \
    } while (0)

void
PerfDataFile::parsePerfSample(struct perf_event_header *ehdr, struct perf_sample_parsed *out)
{
	assert(ehdr->type == PERF_RECORD_SAMPLE);

	char *cur = (char *)(ehdr + 1);
	char *end = ((char *)ehdr) + ehdr->size;
	uint64_t sample_type = attrs->sample_type;

	__builtin_memset(out, 0, sizeof(*out));

	if (sample_type & PERF_SAMPLE_IDENTIFIER) {
		CONSUME_FIELD(cur, uint64_t, out->sample_id);
	}
	if (sample_type & PERF_SAMPLE_IP) {
		CONSUME_FIELD(cur, uint64_t, out->ip);
	}
	if (sample_type & PERF_SAMPLE_TID) {
		CONSUME_FIELD(cur, uint32_t, out->pid);
		CONSUME_FIELD(cur, uint32_t, out->tid);
	}
	if (sample_type & PERF_SAMPLE_TIME) {
		CONSUME_FIELD(cur, uint64_t, out->time);
	}
	if (sample_type & PERF_SAMPLE_ADDR) {
		CONSUME_FIELD(cur, uint64_t, out->addr);
	}
	if (sample_type & PERF_SAMPLE_ID) {
		CONSUME_FIELD(cur, uint64_t, out->id);
	}
	if (sample_type & PERF_SAMPLE_STREAM_ID) {
		CONSUME_FIELD(cur, uint64_t, out->stream_id);
	}
	if (sample_type & PERF_SAMPLE_CPU) {
		CONSUME_FIELD(cur, uint32_t, out->cpu);
		CONSUME_FIELD(cur, uint32_t, out->res);
	}
	if (sample_type & PERF_SAMPLE_PERIOD) {
		CONSUME_FIELD(cur, uint64_t, out->period);
	}

	if (sample_type & PERF_SAMPLE_READ) {
		out->read_format_data = cur;
		uint64_t read_format = attrs->read_format;
		uint64_t read_size = 0;

		if (read_format & PERF_FORMAT_GROUP) {
			uint64_t nr = *(uint64_t *)cur;
			read_size += sizeof(uint64_t); // nr
			if (read_format & PERF_FORMAT_TOTAL_TIME_ENABLED) read_size += sizeof(uint64_t);
			if (read_format & PERF_FORMAT_TOTAL_TIME_RUNNING) read_size += sizeof(uint64_t);

			// Each group member has a value, and optionally an ID
			uint64_t member_size = sizeof(uint64_t);
			if (read_format & PERF_FORMAT_ID) member_size += sizeof(uint64_t);
			read_size += nr * member_size;
		} else {
			read_size += sizeof(uint64_t); // value
			if (read_format & PERF_FORMAT_TOTAL_TIME_ENABLED) read_size += sizeof(uint64_t);
			if (read_format & PERF_FORMAT_TOTAL_TIME_RUNNING) read_size += sizeof(uint64_t);
			if (read_format & PERF_FORMAT_ID) read_size += sizeof(uint64_t);
		}

		cur += read_size;
	}

	if (sample_type & PERF_SAMPLE_CALLCHAIN) {
		CONSUME_FIELD(cur, uint64_t, out->callchain_nr);
		out->callchain_ips = (uint64_t *)cur;
		cur += out->callchain_nr * sizeof(uint64_t);
	}
	if (sample_type & PERF_SAMPLE_RAW) {
		CONSUME_FIELD(cur, uint32_t, out->raw_size);
		out->raw_data = cur;
		cur += out->raw_size;
	}
	if (sample_type & PERF_SAMPLE_BRANCH_STACK) {
		CONSUME_FIELD(cur, uint64_t, out->branch_stack_nr);
		out->branch_entries = (struct perf_branch_entry *)cur;
		cur += out->branch_stack_nr * sizeof(struct perf_branch_entry);
	}

	/* Use attr->sample_regs_user to calculate how many registers to skip */
	if (sample_type & PERF_SAMPLE_REGS_USER) {
		CONSUME_FIELD(cur, uint64_t, out->user_abi);
		out->user_regs = (uint64_t *)cur;
		cur += __builtin_popcountll(attrs->sample_regs_user) * sizeof(uint64_t);
	}
	if (sample_type & PERF_SAMPLE_STACK_USER) {
		CONSUME_FIELD(cur, uint64_t, out->user_stack_size);
		out->user_stack_data = cur;
		cur += out->user_stack_size;
		if (out->user_stack_size != 0) {
			CONSUME_FIELD(cur, uint64_t, out->user_stack_dyn_size);
		}
	}
	if (sample_type & PERF_SAMPLE_WEIGHT) {
		CONSUME_FIELD(cur, uint64_t, out->weight);
	}
	if (sample_type & PERF_SAMPLE_DATA_SRC) {
		CONSUME_FIELD(cur, uint64_t, out->data_src);
	}
	if (sample_type & PERF_SAMPLE_TRANSACTION) {
		CONSUME_FIELD(cur, uint64_t, out->transaction);
	}

	/* Use attr->sample_regs_intr to calculate how many registers to skip */
	if (sample_type & PERF_SAMPLE_REGS_INTR) {
		CONSUME_FIELD(cur, uint64_t, out->intr_abi);
		out->intr_regs = (uint64_t *)cur;
		cur += __builtin_popcountll(attrs->sample_regs_intr) * sizeof(uint64_t);
	}
	if (sample_type & PERF_SAMPLE_PHYS_ADDR) {
		CONSUME_FIELD(cur, uint64_t, out->phys_addr);
	}
	if (sample_type & PERF_SAMPLE_CGROUP) {
		CONSUME_FIELD(cur, uint64_t, out->cgroup);
	}
	if (sample_type & PERF_SAMPLE_DATA_PAGE_SIZE) {
		CONSUME_FIELD(cur, uint64_t, out->data_page_size);
	}
	if (sample_type & PERF_SAMPLE_CODE_PAGE_SIZE) {
		CONSUME_FIELD(cur, uint64_t, out->code_page_size);
	}
	if (sample_type & PERF_SAMPLE_AUX) {
		CONSUME_FIELD(cur, uint64_t, out->aux_size);
		out->aux_data = cur;
		cur += out->aux_size;
	}
	assert(cur == end);

	//if (cur > end) {
	//	assert(cur <= end);
	//}
}
