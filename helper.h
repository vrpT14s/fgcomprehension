static inline u32 hash(u64 x, int bias)
{
    x ^= bias + 0x9e3779b97f4a7c15ULL;

    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;

    return (u32)x ^ (u32)(x >> 32);
}

static inline u32 hash(u64 x, float bias)
{
    // hash/mix
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;

    // base value in [0, 1]
    float base = (u32)x / float(0xffffffffu);

    // smoothly shift with bias
    float h = base + bias * 0.05f;
    h = h - std::floor(h); // fract()

    // back to integer
    return (u32)(h * 0xffffffffu);
}

static void drawRectLabel(
		ImDrawList *drawlist,
		ImVec2 p0,
		ImVec2 p1,
		const char *label,
		ImU32 color = IM_COL32(0, 0, 0, 255))
{
	if (!label || !*label)
		return;

	constexpr float padding = 2.0f;

	ImVec2 text_size = ImGui::CalcTextSize(label);

	float rect_width  = p1.x - p0.x;
	float rect_height = p1.y - p0.y;

	if (text_size.x + padding * 2 > rect_width)
		return;

	if (text_size.y + padding * 2 > rect_height)
		return;

	drawlist->PushClipRect(p0, p1, true);
	drawlist->AddText(
			ImVec2(p0.x + padding, p0.y + padding),
			color,
			label);
	drawlist->PopClipRect();
}
