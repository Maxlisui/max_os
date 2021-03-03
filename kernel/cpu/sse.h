#pragma once

namespace SSE {

bool has_xsave();
bool has_avx();
void init_sse();
void load_sse_context(u64* context);
void save_sse_context(u64* context);

}
