/* Copyright (c) 2008 NVIDIA CORPORATION

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef MX_MESH_H
#define MX_MESH_H

#include "MxObjects.h"

#include "MxPluginData.h"

class MxTriangleMesh : public NxTriangleMeshDesc
{
public:
	MxTriangleMesh() : m_nxTriangleMesh(NULL) {}
	virtual ~MxTriangleMesh() 
	{
		assert(m_nxTriangleMesh == NULL); //removed by MxShape before deleting this object
	}
	NxTriangleMesh* m_nxTriangleMesh;
};


class MxConvexMesh : public NxConvexMeshDesc
{
public:
	MxConvexMesh() : m_nxConvexMesh(NULL) {}
	virtual ~MxConvexMesh() 
	{
		assert(m_nxConvexMesh == NULL); //removed by MxShape before deleting this object
	}
	NxConvexMesh* m_nxConvexMesh;
};

enum MxMeshType {
	MX_MESHTYPE_TRIMESH = 0,
	MX_MESHTYPE_CLOTH = 1,
	MX_MESHTYPE_SOFTBODY = 2,
	MX_MESHTYPE_CONVEX = 3,
	MX_MESHTYPE_TRIMESH_SW = 4
};

class MxMesh : public MxObject {
public:
	virtual MxMesh* isMesh() { return this; }
	virtual void* isType(MxObjectType type) { if (type == MX_OBJECTTYPE_MESH) return this; return NULL; }

	MxMeshType getType() { return m_type; }

	bool isInitialized() { return m_triMesh != NULL || m_convexMesh != NULL; }
	bool canInstantiate(INode* node, MxMeshType type);

	NxTriangleMesh* getNxTriangleMesh() { 
		if (m_type == MX_MESHTYPE_TRIMESH && m_triMesh != NULL) return m_triMesh->m_nxTriangleMesh; return NULL;
	}
	NxConvexMesh* getNxConvexMesh() {
		if (m_type == MX_MESHTYPE_CONVEX && m_convexMesh != NULL) return m_convexMesh->m_nxConvexMesh; return NULL;
	}
protected:
	friend class MxPluginData;
	MxMesh(const char* name, INode* node, MxMeshType type);
	virtual ~MxMesh();

	void initTriMesh();
	void initConvexMesh();

	MxMeshType m_type;

//remove?
	MxTriangleMesh* m_triMesh;
	MxConvexMesh* m_convexMesh;
	Object* m_object;
};

#endif //MX_MESH_H