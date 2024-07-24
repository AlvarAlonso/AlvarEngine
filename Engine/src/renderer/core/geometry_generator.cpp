#include "geometry_generator.hpp"


sMeshData CGeometryGenerator::CreateBox(float aWidth, float aHeight, float aDepth, uint32_t aNumSubdivisions)
{
    sMeshData MeshData;

    sVertex Vertices[24];

    float w2 = 0.5f * aWidth;
    float h2 = 0.5f * aHeight;
    float d2 = 0.5f * aDepth;
    
    // Fill in the front face vertex data.
    Vertices[0] = sVertex(glm::vec3(-w2, -h2, -d2), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f));
    Vertices[1] = sVertex(glm::vec3(-w2, +h2, -d2), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    Vertices[2] = sVertex(glm::vec3(+w2, +h2, -d2), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
    Vertices[3] = sVertex(glm::vec3(+w2, -h2, -d2), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f));

    // Fill in the back face vertex data.
    Vertices[4] = sVertex(glm::vec3(-w2, -h2, +d2), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f));
    Vertices[5] = sVertex(glm::vec3(+w2, -h2, +d2), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f));
    Vertices[6] = sVertex(glm::vec3(+w2, +h2, +d2), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    Vertices[7] = sVertex(glm::vec3(-w2, +h2, +d2), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));

    // Fill in the top face vertex data.
    Vertices[8]  = sVertex(glm::vec3(-w2, +h2, -d2), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f));
    Vertices[9]  = sVertex(glm::vec3(-w2, +h2, +d2), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    Vertices[10] = sVertex(glm::vec3(+w2, +h2, +d2), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
    Vertices[11] = sVertex(glm::vec3(+w2, +h2, -d2), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f));

    // Fill in the bottom face vertex data.
    Vertices[12] = sVertex(glm::vec3(-w2, -h2, -d2), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f));
    Vertices[13] = sVertex(glm::vec3(+w2, -h2, -d2), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f));
    Vertices[14] = sVertex(glm::vec3(+w2, -h2, +d2), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    Vertices[15] = sVertex(glm::vec3(-w2, -h2, +d2), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));

    // Fill in the left face vertex data.
    Vertices[16] = sVertex(glm::vec3(-w2, -h2, +d2), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f));
    Vertices[17] = sVertex(glm::vec3(-w2, +h2, +d2), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    Vertices[18] = sVertex(glm::vec3(-w2, +h2, -d2), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
    Vertices[19] = sVertex(glm::vec3(-w2, -h2, -d2), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f));

    // Fill in the right face vertex data.
    Vertices[20] = sVertex(glm::vec3(+w2, -h2, -d2), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f));
    Vertices[21] = sVertex(glm::vec3(+w2, +h2, -d2), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    Vertices[22] = sVertex(glm::vec3(+w2, +h2, +d2), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
    Vertices[23] = sVertex(glm::vec3(+w2, -h2, +d2), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f));

    MeshData.Vertices.assign(&Vertices[0], &Vertices[24]);

    //
    // Create the indices.
    //

    uint32_t i[36];

    // Fill in the front face index data
    i[0] = 0; i[1] = 1; i[2] = 2;
    i[3] = 0; i[4] = 2; i[5] = 3;

    // Fill in the back face index data
    i[6] = 4; i[7]  = 5; i[8]  = 6;
    i[9] = 4; i[10] = 6; i[11] = 7;

    // Fill in the top face index data
    i[12] = 8; i[13] =  9; i[14] = 10;
    i[15] = 8; i[16] = 10; i[17] = 11;

    // Fill in the bottom face index data
    i[18] = 12; i[19] = 13; i[20] = 14;
    i[21] = 12; i[22] = 14; i[23] = 15;

    // Fill in the left face index data
    i[24] = 16; i[25] = 17; i[26] = 18;
    i[27] = 16; i[28] = 18; i[29] = 19;

    // Fill in the right face index data
    i[30] = 20; i[31] = 21; i[32] = 22;
    i[33] = 20; i[34] = 22; i[35] = 23;

    MeshData.Indices32.assign(&i[0], &i[36]);

    // Put a cap on the number of subdivisions.
    aNumSubdivisions = std::min<uint32_t>(aNumSubdivisions, 6u);

    for(uint32_t i = 0; i < aNumSubdivisions; ++i)
    {
        Subdivide(MeshData);
    }

    return MeshData;
}

sMeshData CGeometryGenerator::CreateSphere(float aRadius, uint32_t aSliceCount, uint32_t aStackCount)
{
    sMeshData MeshData;

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	sVertex TopVertex(glm::vec3(0.0f, +aRadius, 0.0f), glm::vec3(0.0f, +1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
	sVertex BottomVertex(glm::vec3(0.0f, -aRadius, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f));

	MeshData.Vertices.push_back(TopVertex);

	const float PhiStep = glm::pi<float>() / aStackCount;
	const float ThetaStep = 2.0f * glm::pi<float>() / aSliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for(uint32_t i = 1; i <= aStackCount - 1; ++i)
	{
		float phi = i*PhiStep;

		// Vertices of ring.
        for(uint32_t j = 0; j <= aSliceCount; ++j)
		{
			float Theta = j * ThetaStep;

			sVertex Vertex;

			// spherical to cartesian
			Vertex.Position.x = aRadius*sinf(phi)*cosf(Theta);
			Vertex.Position.y = aRadius*cosf(phi);
			Vertex.Position.z = aRadius*sinf(phi)*sinf(Theta);

			Vertex.Normal = glm::vec3(glm::normalize(Vertex.Position));

			Vertex.UV.x = Theta / glm::pi<float>() * 2.0f;
			Vertex.UV.y = phi / glm::pi<float>();

			MeshData.Vertices.push_back(Vertex);
		}
	}

	MeshData.Vertices.push_back(BottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

    for(uint32_t i = 1; i <= aSliceCount; ++i)
	{
		MeshData.Indices32.push_back(0);
		MeshData.Indices32.push_back(i + 1);
		MeshData.Indices32.push_back(i);
	}
	
	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
    uint32_t BaseIndex = 1;
    uint32_t RingVertexCount = aSliceCount + 1;
	for(uint32_t i = 0; i < aStackCount - 2; ++i)
	{
		for(uint32_t j = 0; j < aSliceCount; ++j)
		{
			MeshData.Indices32.push_back(BaseIndex + i * RingVertexCount + j);
			MeshData.Indices32.push_back(BaseIndex + i * RingVertexCount + j + 1);
			MeshData.Indices32.push_back(BaseIndex + (i + 1) * RingVertexCount + j);

			MeshData.Indices32.push_back(BaseIndex + (i + 1) * RingVertexCount + j);
			MeshData.Indices32.push_back(BaseIndex + i * RingVertexCount + j + 1);
			MeshData.Indices32.push_back(BaseIndex + (i + 1) * RingVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	uint32_t SouthPoleIndex = (uint32_t)MeshData.Vertices.size()-1;

	// Offset the indices to the index of the first vertex in the last ring.
	BaseIndex = SouthPoleIndex - RingVertexCount;
	
	for(uint32_t i = 0; i < aSliceCount; ++i)
	{
		MeshData.Indices32.push_back(SouthPoleIndex);
		MeshData.Indices32.push_back(BaseIndex+i);
		MeshData.Indices32.push_back(BaseIndex+i+1);
	}

    return MeshData;
}

sMeshData CGeometryGenerator::CreateGrid(float aWidth, float aDepth, uint32_t aRows, uint32_t aCols)
{
    sMeshData MeshData;

	const uint32_t VertexCount = aRows * aCols;
	const uint32_t FaceCount   = (aRows - 1) * (aCols - 1) * 2;

	//
	// Create the vertices.
	//

	const float HalfWidth = 0.5f * aWidth;
	const float HalfDepth = 0.5f * aDepth;

	const float dx = aWidth / (aCols - 1);
	const float dz = aDepth / (aRows - 1);

	const float du = 1.0f / (aCols - 1);
	const float dv = 1.0f / (aRows - 1);

	MeshData.Vertices.resize(VertexCount);
	for(uint32_t i = 0; i < aRows; ++i)
	{
		float z = HalfDepth - i * dz;
		for(uint32_t j = 0; j < aCols; ++j)
		{
			float x = -HalfWidth + j * dx;

			MeshData.Vertices[i * aCols + j].Position = glm::vec3(x, 0.0f, z);
			MeshData.Vertices[i * aCols + j].Normal = glm::vec3(0.0f, 1.0f, 0.0f);

			// Stretch texture over grid.
			MeshData.Vertices[i * aCols + j].UV.x = j * du;
			MeshData.Vertices[i * aCols + j].UV.y = i * dv;
		}
	}
 
    //
	// Create the indices.
	//

	MeshData.Indices32.resize(FaceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	uint32_t k = 0;
	for(uint32_t i = 0; i < aRows - 1; ++i)
	{
		for(uint32_t j = 0; j < aCols - 1; ++j)
		{
			MeshData.Indices32[k]   = i * aCols + j;
			MeshData.Indices32[k + 1] = i * aCols + j + 1;
			MeshData.Indices32[k + 2] = (i + 1) * aCols + j;

			MeshData.Indices32[k + 3] = (i + 1) * aCols +j;
			MeshData.Indices32[k + 4] = i * aCols + j + 1;
			MeshData.Indices32[k + 5] = (i + 1) * aCols + j + 1;

			k += 6; // next quad
		}
	}

    return MeshData;
}

sMeshData CGeometryGenerator::CreateQuad(float x, float y, float w, float h, float aDepth)
{
    sMeshData MeshData;

	MeshData.Vertices.resize(4);
	MeshData.Indices32.resize(6);

	// Position coordinates specified in NDC space.
	MeshData.Vertices[0] = sVertex(
        glm::vec3(x, y - h, aDepth),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec2(0.0f, 1.0f));

	MeshData.Vertices[1] = sVertex(
		glm::vec3(x, y, aDepth),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec2(0.0f, 0.0f));

	MeshData.Vertices[2] = sVertex(
		glm::vec3(x+w, y, aDepth),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec2(1.0f, 0.0f));

	MeshData.Vertices[3] = sVertex(
		glm::vec3(x+w, y-h, aDepth),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec2(1.0f, 1.0f));

	MeshData.Indices32[0] = 0;
	MeshData.Indices32[1] = 1;
	MeshData.Indices32[2] = 2;

	MeshData.Indices32[3] = 0;
	MeshData.Indices32[4] = 2;
	MeshData.Indices32[5] = 3;

    return MeshData;
}

void CGeometryGenerator::Subdivide(sMeshData& aMeshData)
{
    // Save a copy of the input geometry.
	sMeshData InputCopy = aMeshData;


	aMeshData.Vertices.resize(0);
	aMeshData.Indices32.resize(0);

	//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

	const uint32_t NumTris = (uint32_t)InputCopy.Indices32.size() / 3;
	for(uint32_t i = 0; i < NumTris; ++i)
	{
		const sVertex v0 = InputCopy.Vertices[ InputCopy.Indices32[i*3+0] ];
		const sVertex v1 = InputCopy.Vertices[ InputCopy.Indices32[i*3+1] ];
		const sVertex v2 = InputCopy.Vertices[ InputCopy.Indices32[i*3+2] ];

		//
		// Generate the midpoints.
		//

        const sVertex m0 = MidPoint(v0, v1);
        const sVertex m1 = MidPoint(v1, v2);
        const sVertex m2 = MidPoint(v0, v2);

		//
		// Add new geometry.
		//

		aMeshData.Vertices.push_back(v0); // 0
		aMeshData.Vertices.push_back(v1); // 1
		aMeshData.Vertices.push_back(v2); // 2
		aMeshData.Vertices.push_back(m0); // 3
		aMeshData.Vertices.push_back(m1); // 4
		aMeshData.Vertices.push_back(m2); // 5
 
		aMeshData.Indices32.push_back(i*6+0);
		aMeshData.Indices32.push_back(i*6+3);
		aMeshData.Indices32.push_back(i*6+5);

		aMeshData.Indices32.push_back(i*6+3);
		aMeshData.Indices32.push_back(i*6+4);
		aMeshData.Indices32.push_back(i*6+5);

		aMeshData.Indices32.push_back(i*6+5);
		aMeshData.Indices32.push_back(i*6+4);
		aMeshData.Indices32.push_back(i*6+2);

		aMeshData.Indices32.push_back(i*6+3);
		aMeshData.Indices32.push_back(i*6+1);
		aMeshData.Indices32.push_back(i*6+4);
	}
}

sVertex CGeometryGenerator::MidPoint(const sVertex& aFirstVertex, const sVertex& aSecondVertex)
{
    const glm::vec3 p0 = aFirstVertex.Position;
    const glm::vec3 p1 = aSecondVertex.Position;

    const glm::vec2 UV0 = aFirstVertex.UV;
    const glm::vec2 UV1 = aSecondVertex.UV;

    // Compute the midpoints of all the attributes.  Vectors need to be normalized
    // since linear interpolating can make them not unit length.  
    const glm::vec3 pos = 0.5f*(p0 + p1);
    const glm::vec2 tex = 0.5f*(UV0 + UV1);

    sVertex Vertex;
    Vertex.Position = pos;
    Vertex.UV = tex;

    return Vertex;
}
