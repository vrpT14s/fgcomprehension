#pragma once

#include <ios>
#include <sstream>
#include <map>
#include <algorithm>
#include <vector>
#include <string>
#include <set>
#include <fstream>
#include <iostream>

#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/SymbolSize.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/DebugInfo/Symbolize/Symbolize.h>
#include <llvm/Demangle/Demangle.h>


#include <llvm/DebugInfo/DIContext.h>
#include <llvm/DebugInfo/Symbolize/SymbolizableModule.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/SymbolSize.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/Path.h>
#include <llvm/DebugInfo/DWARF/DWARFContext.h>

#include "perfdata_types.h"

using llvm::object::SymbolRef;
using namespace llvm;
using namespace llvm::object;

class SymbolData {
public:
	std::string demangled_name;
	std::string path;
	u32 line;
	u32 column;
};

class Dso {
public:
	std::string filename;
	std::map<u64, std::pair<u64, SymbolRef>> sym_intervals;
	std::map<u64, SymbolData> sym_cache;

	MemoryBuffer *buffer = nullptr;
	ObjectFile *obj = nullptr;
	DIContext *dwarf = nullptr;
	
public:
	Dso() : filename("/bullshit") {}
	Dso(std::string filename);
	std::optional<std::pair<u64, SymbolRef>> lookupSym(u64 dso_off);
	std::pair<u64, u64> getSymOff(u64 dso_off);
	void fillSymCache(u64 dso_off);
};

//purpose - be held in a set so we can find the interval of an ip
class MmapInterval {
public:
	u32 pid;
	u32 tid;
	u64 addr;
	u64 len = 0;
	u64 pgoff = 0;
	Dso *dso;
public:
	MmapInterval(
		u32 pid,
		u32 tid,
		u64 addr,
		u64 len,
		u64 pgoff
	) : pid(pid)
	  , tid(tid)
	  , addr(addr)
	  , len(len)
	  , pgoff(pgoff) {}

	MmapInterval(
		u32 pid,
		u32 tid,
		u64 addr
	) : pid(pid)
	  , tid(tid)
	  , addr(addr) {}

	bool operator<(const MmapInterval& other) const {
		if (pid != other.pid) {
			return pid < other.pid;
		}
		if (tid != other.tid) {
			return tid < other.tid;
		}
		return addr < other.addr;
	}

	bool contains(u32 pid, u32 tid, u64 ip) const {
		return
		   this->pid == pid
		&& this->tid == tid
		&& this->addr <= ip
		&& ip < this->addr + this->len;
	}

	u64 relativeAddr(u64 ip) const {
		assert(ip >= addr && ip < addr + len);
		auto rel_addr = ip - addr + pgoff;
		return rel_addr;
	}
};

using Callchain = std::vector<std::tuple<const MmapInterval *, u64, u64>>;

struct Sample {
	Callchain callchain;
	u64 time;
};

class EventConsumer {
public:
	std::map<std::string /*filename*/, Dso> dsos;
	std::set<MmapInterval> mmap_intervals;
	std::map<std::pair<u32, u32>, 
		std::vector<Sample>> samples;
public:
	EventConsumer() = default;
	
	void addMmapRegion(
		u32 pid,
		u32 tid,
		u64 addr,
		u64 len,
		u64 pgoff,
		std::string filename
	) {
		MmapInterval interval{pid, tid, addr, len, pgoff};

		if (!dsos.contains(filename)) {
			Dso dso(filename);
			dsos[filename] = dso;
		}
		interval.dso = &dsos[filename];

		mmap_intervals.insert(interval);
	}
	const MmapInterval *getMmapInterval(u32 pid, u32 tid, u64 ip);
	void addSample(u32 pid, u32 tid, u64 time, std::vector<u64> callchain);
};
