#pragma once

namespace global {
	inline bool showHud = false;
	inline bool showCoords = false;
	inline bool polygonmode = false;
	inline bool showTriangles = true;
	inline bool showNormals = false;
	inline bool showChunks = false;
	inline bool showVoxels = false;
	inline bool enableChunkCache = false;
	inline bool freeCamera = false;
	inline int CAMERA_CHUNK_RADIUS = 3;

	inline float gravity = 1.0f;

	namespace noise {
		inline int octaves = 6;
	}
}
