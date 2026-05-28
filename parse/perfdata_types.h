#pragma once

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#include <linux/perf_event.h>

struct perf_file_section {
	uint64_t offset;	/* offset from start of file */
	uint64_t size;		/* size of the section */
};


struct perf_header {
	char magic[8];		/* PERFILE2 */
	uint64_t size;		/* size of the header */
	uint64_t attr_size;	/* size of an attribute in attrs */
	struct perf_file_section attrs;
	struct perf_file_section data;
	struct perf_file_section event_types;
	uint64_t flags;
	uint64_t flags1[3];
};


struct perf_event_mmap2 {
	struct perf_event_header header;
	u32    pid;
	u32    tid;
	u64    addr;
	u64    len;
	u64    pgoff;
	union {
		struct {
			u32    maj;
			u32    min;
			u64    ino;
			u64    ino_generation;
		};
		struct {   /* if PERF_RECORD_MISC_MMAP_BUILD_ID */
			u8     build_id_size;
			u8     __reserved_1;
			u16    __reserved_2;
			u8     build_id[20];
		};
	};
	u32    prot;
	u32    flags;
	char   filename[];
	//struct sample_id sample_id;
};

struct perf_event_comm {
	struct perf_event_header header;
	u32    pid;
	u32    tid;
	char   comm[];
};



//gemini 3 pro
/*
 * Fixed-length "full" sample struct.
 * Variable length arrays are represented as pointers into the original
 * raw buffer immediately following the perf_event_header.
 */
struct perf_sample_parsed {
    uint64_t sample_id;
    uint64_t ip;
    uint32_t pid;
    uint32_t tid;
    uint64_t time;
    uint64_t addr;
    uint64_t id;
    uint64_t stream_id;
    uint32_t cpu;
    uint32_t res;
    uint64_t period;

    void *read_format_data; // Pointer to the variable read_format struct

    uint64_t callchain_nr;
    uint64_t *callchain_ips;

    uint32_t raw_size;
    char *raw_data;

    uint64_t branch_stack_nr;
    struct perf_branch_entry *branch_entries;

    uint64_t user_abi;
    uint64_t *user_regs;

    uint64_t user_stack_size;
    char *user_stack_data;
    uint64_t user_stack_dyn_size;

    uint64_t weight;
    uint64_t data_src;
    uint64_t transaction;

    uint64_t intr_abi;
    uint64_t *intr_regs;

    uint64_t phys_addr;
    uint64_t cgroup;
    uint64_t data_page_size;
    uint64_t code_page_size;

    uint64_t aux_size;
    char *aux_data;
};



/*
PERF_RECORD_SAMPLE
This record indicates a sample.

    struct {
	struct perf_event_header header;
	u64    sample_id;   // if PERF_SAMPLE_IDENTIFIER
	u64    ip;          // if PERF_SAMPLE_IP
	u32    pid, tid;    // if PERF_SAMPLE_TID
	u64    time;        // if PERF_SAMPLE_TIME
	u64    addr;        // if PERF_SAMPLE_ADDR
	u64    id;          // if PERF_SAMPLE_ID
	u64    stream_id;   // if PERF_SAMPLE_STREAM_ID
	u32    cpu, res;    // if PERF_SAMPLE_CPU
	u64    period;      // if PERF_SAMPLE_PERIOD
	struct read_format v;
			    // if PERF_SAMPLE_READ
	u64    nr;          // if PERF_SAMPLE_CALLCHAIN
	u64    ips[nr];     // if PERF_SAMPLE_CALLCHAIN
	u32    size;        // if PERF_SAMPLE_RAW
	char   data[size];  // if PERF_SAMPLE_RAW
	u64    bnr;         // if PERF_SAMPLE_BRANCH_STACK
	struct perf_branch_entry lbr[bnr];
			    // if PERF_SAMPLE_BRANCH_STACK
	u64    abi;         // if PERF_SAMPLE_REGS_USER
	u64    regs[weight(mask)];
			    // if PERF_SAMPLE_REGS_USER
	u64    size;        // if PERF_SAMPLE_STACK_USER
	char   data[size];  // if PERF_SAMPLE_STACK_USER
	u64    dyn_size;    // if PERF_SAMPLE_STACK_USER &&
			       size != 0
	union perf_sample_weight weight;
			    // if PERF_SAMPLE_WEIGHT
			    // || PERF_SAMPLE_WEIGHT_STRUCT
	u64    data_src;    // if PERF_SAMPLE_DATA_SRC
	u64    transaction; // if PERF_SAMPLE_TRANSACTION
	u64    abi;         // if PERF_SAMPLE_REGS_INTR
	u64    regs[weight(mask)];
			    // if PERF_SAMPLE_REGS_INTR
	u64    phys_addr;   // if PERF_SAMPLE_PHYS_ADDR
	u64    cgroup;      // if PERF_SAMPLE_CGROUP
	u64    data_page_size;
			  // if PERF_SAMPLE_DATA_PAGE_SIZE
	u64    code_page_size;
			  // if PERF_SAMPLE_CODE_PAGE_SIZE
	u64    size;        // if PERF_SAMPLE_AUX
	char   data[size];  // if PERF_SAMPLE_AUX
    };

*/


