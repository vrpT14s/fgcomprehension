#include "eventconsumer.h"

using llvm::object::SymbolRef;

//--------------------Dso--------------------

Dso::Dso(std::string filename) : filename(filename) {
	auto buf_or_err = llvm::MemoryBuffer::getFile(filename);
	if (!buf_or_err)
		return;
	buffer = buf_or_err->release();
	printf("obj = %llx\n", buffer);

	auto obj_or_err = llvm::object::ObjectFile::createObjectFile(
			buffer->getMemBufferRef()
	);
	if (!obj_or_err) {
		llvm::consumeError(obj_or_err.takeError());
		return;
	}

	this->obj = obj_or_err->release();
	printf("obj = %llx\n", obj);
	this->dwarf = DWARFContext::create(*obj).release();
	printf("dwarf = %llx\n", dwarf);
	assert(this->dwarf > (void *)0x100);
	auto size_map = llvm::object::computeSymbolSizes(*obj);
	for (const auto &[sym, size] : size_map) {
		auto addr = sym.getAddress();
		if (!addr) {
			llvm::consumeError(addr.takeError());
			continue;
		}
		//sym_intervals.insert({*addr, size, sym}); 
		sym_intervals[*addr] = std::pair{size, sym};
	}
}

void Dso::fillSymCache(u64 dso_off)
{
	if (sym_cache.contains(dso_off))
		return;

	SymbolData data;

	DILineInfoSpecifier spec(
			DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath,
			DILineInfoSpecifier::FunctionNameKind::ShortName);

	object::SectionedAddress addr;
	addr.Address = dso_off;
	addr.SectionIndex = object::SectionedAddress::UndefSection;

	auto inline_info =
		dwarf->getInliningInfoForAddress(addr, spec);

	if (inline_info.getNumberOfFrames() == 0) {
		sym_cache[dso_off] = std::move(data);
		return;
	}

	auto frame = inline_info.getFrame(0);

	data.demangled_name = frame.FunctionName;
	data.path = frame.FileName;
	data.line = frame.Line;
	data.column = frame.Column;

	sym_cache[dso_off] = std::move(data);
}


std::optional<std::pair<u64, SymbolRef>> Dso::lookupSym(u64 dso_off) {
	if (sym_intervals.size() == 0)
		return {};

	auto it = sym_intervals.upper_bound(dso_off);
	if (it == sym_intervals.begin())
		return {};
	--it;

	auto [start, pair] = *it;
	auto [len, sym] = pair;
	if (start <= dso_off && dso_off < start + len)
		return std::pair{start, sym};
	return {};
}

std::pair<u64, u64> Dso::getSymOff(u64 dso_off) {
	auto maybe = lookupSym(dso_off);
	if (!maybe.has_value())
		return {0, dso_off};
	auto [start, sym] = maybe.value();
	return {start, dso_off - start};
}

//------------------------------EventConsumer--------------------

const MmapInterval *EventConsumer::getMmapInterval(u32 pid, u32 tid, u64 ip) {
	const MmapInterval ip_interval{ pid, tid, ip };
	auto it = std::upper_bound(
		mmap_intervals.begin(), 
		mmap_intervals.end(), 
		ip_interval
	);
	if (it == mmap_intervals.begin())
		return NULL;
	it--;
	if (it->contains(pid, tid, ip))
		return &*it;
	return NULL;
}

//prereq: callchain needs to have current ip at the end
void EventConsumer::addSample(u32 pid, u32 tid, u64 time, std::vector<u64> callchain) { 
	std::vector<std::tuple<const MmapInterval *, u64, u64>> offset_callchain;
	for (u64 ip: callchain) {
		const MmapInterval *interval = getMmapInterval(pid, tid, ip);
		if (!interval) {
			offset_callchain.push_back({NULL, 0, ip});
			continue;
		}
		auto [sym_start, sym_off] = interval->dso->getSymOff(
				interval->relativeAddr(ip)
				);
		offset_callchain.push_back({interval, sym_start, sym_off});
	}
	samples[std::pair{pid, tid}].push_back({ offset_callchain, time });
}
