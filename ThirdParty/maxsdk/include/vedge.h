#pragma once
#include "maxheap.h"
#include <WTypes.h>
#include "tab.h"

// Moved from VEDGE.CPP -TH
#define UNDEF 0xFFFFFFFF
#define UNDEF_FACE 0x3FFFFFFF

class Mesh;

class VEdge: public MaxHeapOperators {
	DWORD f[2];
	public:
		void SetFace(int i, DWORD n) { f[i] =(f[i]&0xc0000000)|n;}
		void SetWhichSide(int i, int s) { f[i] =(f[i]&0x3FFFFFFF)|((s&3)<<30); }
		DWORD GetFace(int i){ return f[i]&0x3fffffff; }
		int GetWhichSide(int i) { return (f[i]>>30)&3; }
	};

struct Edge: public MaxHeapOperators {
	Edge(unsigned short fl = 0, DWORD v0 = UNDEF, DWORD v1 = UNDEF, DWORD f0 = UNDEF, DWORD f1 = UNDEF)
	{ flags = fl; v[0] = v0; v[1] = v1; f[0] = f0; f[1]= f1;}
	unsigned short flags;
	DWORD v[2];  /* indices of two vertices defining edge */
	DWORD f[2];  /* indices of two neighboring faces  */
};

// Builds list of all polygons which would be obtained if all Face entities (triangles) in mesh.faces list were joined across
// any hidden edges to obtain higher-degree polygons. The output tables connectedFaceLists and faceListStartIndices implement a
// linked list of such joined polygons, so that we have a total of nPoly = faceListStartIndices.Count() - 1 polygons, with
// polygon p on the range [0, nPoly) being defined by the union of all faces mesh.faces[connectedFaceLists[s + f]], where
// s = faceListStartIndices[p], and f is on the range [0, nFacesPoly), with nFacesPoly = faceListStartIndices[p + 1] - s.
//
// Additionally, we build the linked list of vertices on the bounding curve of each polygon. Specifically,
// polygonVertexListStartIndices is a table of size nPoly + 1, so that for polygon p on the range [0, nPoly), the vertices of
// the bounding curve are given by mesh.verts[polygonVertexLists[s + v]], where s = polygonVertexListStartIndices[p], and v is
// on the range [0, nVerticesPoly), with nVerticesPoly = polygonVertexListStartIndices[p + 1] - s. Note that the vertex list is
// ordered to represent a traversal of the bounding curve, and in the case of a polygon defined by a single triangular face,
// the vertex ordering is preserved.
void BuildHiddenEdgeConnectedPolygons(Tab<int>& connectedFaceLists, Tab<int>& faceListStartIndices,
                                      Tab<int>& polygonVertexLists, Tab<int>& polygonVertexListStartIndices, Mesh& mesh);



