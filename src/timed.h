#pragma once

#include <chrono>

template <typename F>
auto timed(F f) -> std::chrono::milliseconds {
	const auto start = std::chrono::high_resolution_clock::now();
	f();
	const auto end = std::chrono::high_resolution_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}
