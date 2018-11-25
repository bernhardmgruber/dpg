#pragma once

namespace global {
	inline bool showHud = false;
	inline bool coords = false;
	inline bool polygonmode = false;
	inline bool normals = false;
	inline bool enableChunkCache = false;
	inline int CAMERA_CHUNK_RADIUS = 2;

	inline float gravity = 1.0f;

	namespace noise {
		inline int octaves = 6;
	}
}
