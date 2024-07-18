#pragma once

#include "render_types.hpp"

class CGeometryGenerator
{
public:
    static sMeshData CreateBox(float aWidth, float aHeight, float aDepth, uint32_t aNumSubdivisions);

    static sMeshData CreateSphere(float aRadius, uint32_t aSliceCount, uint32_t aStackCount);

    static sMeshData CreateGrid(float aWidth, float aDepth, uint32_t aRows, uint32_t aCols);

    static sMeshData CreateQuad(float x, float y, float w, float h, float aDepth);

private:
	static void Subdivide(sMeshData& aMeshData);
    static sVertex MidPoint(const sVertex& aFirstVertex, const sVertex& aSecondVertex);
};