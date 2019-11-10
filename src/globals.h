#pragma once

namespace global {
	inline bool showHud = false;
	inline bool showCoords = true;
	inline bool polygonmode = false;
	inline bool showTriangles = true;
	inline bool showVertexNormals = false;
	inline bool showTriangleNormals = false;
	inline bool showChunks = false;
	inline bool showVoxels = true;
	inline bool enableChunkCache = false;
	inline bool freeCamera = false;
	inline int CAMERA_CHUNK_RADIUS = 0;

	namespace noise {
		inline int octaves = 6;
	}
}
