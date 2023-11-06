//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//
// FILE:        RemediateNonManifoldGeometry.h
// DESCRIPTION: Header information for the class responsible for detecting
//              and remediating non-manifold edges and vertices.
// AUTHOR:      Lee Betchen
//
// HISTORY:	    May 2020 Creation
//**************************************************************************/

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "MVExport.h"
#include <cassert>
#include <set>
#include <vector>
#include "../geom/geom_span.hpp"

namespace MaxSDK
{

    // Class RemediateNonManifoldGeometry detects and remediates non-manifold edges and vertices for a mesh with nFaces faces and
    // nVertices vertices, where nFaces = (beginFaceVertices.size()) - 1, with each face f defined by the counterclockwise list of
    // nVerticesFace = beginFaceVertices[f + 1] - beginFaceVertices[f] vertices in the array &(faceVertices[beginFaceVertices[f]]).
    // A non-manifold edges is detected when a multiple faces use the same edge in the same sense, while a non-manifold vertex is
    // identified when multiple closed or open sequences of edge-connected faces are present at the vertex. Note that faces for
    // which nVerticesFace = 0 are ignored as inactive; for any other face, we presume that the condition nVerticesFace >= 3 is
    // satisfied.
    //
    // Non-manifold edges are remediated by splitting the edge, generating one pair of duplicate vertices for each face edge at the
    // non-manifold junction, and then re-merging vertices split from the same source where possible to eliminate overlapping
    // boundary edges and non-manifold vertex connections introduced by the splitting. Non-manifold vertices are remediated by
    // forming a unique duplicate vertex for each face sequence.
    //
    // Note that, prior to non-manifold edge and vertex detection, the mesh defined by faceVertices and beginFaceVertices is
    // preprocessed to remediate any "null" face edges, having the same vertex at either endpoint, as the non-manifold
    // detection does not support a mesh containing such degeneracies. When doEliminateNullEdges is true, any null edges will
    // actually be excised from the face bounding loops in which they occur, altering the number of face vertices. If the
    // remaining number of vertices for a particular face after excision of null edges is found to be less than three, the face
    // is judged to be collapsed, and its vertex count is set to zero. If a such a change in face degree is undesirable,
    // doEliminateNullEdges may be set to false; in this case, null edges will be remediated by introducing a duplicates of the
    // repeated vertices, transforming the null edges into topologically valid edges having zero length.
    class RemediateNonManifoldGeometry
    {

    // Public types:
    public:

        // Enumeration indexing entries of the returned edge data array edgeData = GetEdgeDataforManifoldEdge(e) for a particular
        // edge e; note that with f on the range [0, 2), face edge fe = edgeData[EDGE_IN_FIRST_FACE + f] is defined, with
        // vertices = &(manifoldFaceVertices[beginManifoldFaceVertices[face]] and face = edgeData[FIRST_FACE_AT_EDGE + f], to lie
        // between vertices vertices[fe] and vertices[(fe + 1) % nVerticesFace], where the number of face vertices is given by
        // nVerticesFace = beginManifoldFaceVertices[face + 1] - beginManifoldFaceVertices[face], and that entry
        // edgeData[FIRST_FACE_AT_EDGE] should alway be valid, but that for boundary edges, edgeData[SECOND_FACE_AT_EDGE] < 0.
        enum EdgeDataType
        {
            FIRST_FACE_AT_EDGE  = 0,
            SECOND_FACE_AT_EDGE = 1,
            EDGE_IN_FIRST_FACE  = 2,
            EDGE_IN_SECOND_FACE = 3,
            N_EDGE_DATA         = 4
        };

    // Public member functions:
    public:

        // Constructor.
        //
        // faceVertices, beginFaceVertices: face vertex lists defining the mesh upon which to operate, as detailed in the class
        //                                  description; input
        // nVertices:                       total number of mesh vertices, so that all face vertices are presumed to be on the
        //                                  range [0, nVertices); input
        // doEliminateNullEdges:            when true, "null" edges are excised from the mesh, as discussed in the class
        //                                  description, otherwise, the are remediated via vertex duplication; input
        MVEXPORT RemediateNonManifoldGeometry(geo::span<const int> faceVertices, geo::span<const int> beginFaceVertices, int nVertices,
                                              bool doEliminateNullEdges = false);

        // Default constructor; hidden to force specification of required parameters.
        RemediateNonManifoldGeometry() = delete;

        // Destructor.
        MVEXPORT ~RemediateNonManifoldGeometry() = default;

        // In cases where the faces passed to the constructor in faceVertices and beginFaceVertices represent a mapped-space
        // topology, when performing non-manifold edge splitting in RemediateNonManifoldEdgesAndVertices, we may wish to use
        // the physical-space connectivity information, so that for connected physical face edges, we can attempt to maintain
        // connections between the corresponding map face edges. Accordingly, when physical-space connectivity information is
        // supplied via this function, we employ it in subsequent calls to RemediateNonManifoldEdgesAndVertices, on the
        // assumption that faceVertices and beginFaceVertices represent mapped-space topology.
        //
        // physicalFaceVertices: physical face f, defined by the bounding vertex loop physicalFaceVertices[f' + v] for v on the
        //                       range [0, nVerticesFace), corresponds to map face f, defined by the bounding vertex loop
        //                       faceVertices[f' + v], where f' = beginFaceVertices[f], and where the number of face vertices
        //                       nVerticesFace = beginFaceVertices[f + 1] - f'; input
        // nPhysicalVertices:    total number of physical-space vertices, so that all physical face vertices are presumed to be on
        //                       the range [0, nPhysicalVertices); input
        MVEXPORT void SetPhysicalSpaceFaces(geo::span<const int> physicalFaceVertices, int nPhysicalVertices);

        // Detects non-manifold geometry, but does not remediate. Since the non-manifold vertex check requires no non-manifold
        // edges, if nNonManifoldEdges != 0, we will reflexively return nNonManifoldVertex = 0. Should not be called after
        // RemediateNonManifoldEdgesAndVertices. To fetch the detected nonmanifold vertex indices, call function
        // GetDetectedNonManifoldVertexIndices.
        //
        // nNonManifoldVertices:  The detected non-manifold vertex index count, or zero if non-manifold edges are present; output
        // nNonManifoldEdges:    number of non-manifold face edges detected; output
        MVEXPORT void FindNonManifoldEdgesAndVertices(size_t& nNonManifoldVertices, size_t& nNonManifoldEdges);

        // Detects and remediates non-manifold geometry, populating member structures so that the public retrieval functions
        // defined below are available. Should only be called once for an instance of the class.
        // To fetch the detected nonmanifold vertex indices, call GetDetectedNonManifoldVertexIndices().
        //
        // nNonManifoldVertices: number of non-manifold vertices detected and remediated; output
        // nNonManifoldEdges:    number of non-manifold face edges detected and remediated; output
        MVEXPORT void RemediateNonManifoldEdgesAndVertices(size_t& nNonManifoldVertices, size_t& nNonManifoldEdges);

        // Fetch the detected nonmanifold vertex indices. Should be fetched after calling FindNonManifoldEdgesAndVertices
        // or RemediateNonManifoldEdgesAndVertices;
        MVEXPORT geo::span<const int> GetDetectedNonManifoldVertexIndices() const;

        // Return a flag indicating whether the manifold mesh topology determined by this class differs from the input
        // topology. The return value will be true if any non-manifold edges or vertices were split, and/or if any null edges
        // were remediated during preprocessing; otherwise, a value of false will be returned. Must be preceded by a call to
        // RemediateNonManifoldEdgesAndVertices.
        MVEXPORT bool IsTopologyAltered() const;

        // Returns the number of vertices to be used in the mesh after remediation. Must be preceded by a call to
        // RemediateNonManifoldEdgesAndVertices.
        MVEXPORT int GetNumberOfManifoldVertices() const;

        // Returns the vertex from which vertex indexVertex was split if indexVertex >= nVertices. If indexVertex < nVertices,
        // indexVertex = GetSourceVertex(indexVertex) by definition. Must be preceded by a call to
        // RemediateNonManifoldEdgesAndVertices.
        //
        // indexVertex: index in mesh, after remediation, of vertex of interest; input
        MVEXPORT int GetSourceVertex(size_t indexVertex) const;

        // Returns the number of vertices in the bounding loop of face indexFace after remediation. Note that the face vertex
        // counts may be altered by this class only when excising "null" edges, so that if doEliminateNullEdges = false was
        // specified on construction of this instance, or if face indexFace did not initially contain null edges, then
        // GetNumberOfManifoldVerticesForFace(indexFace) = beginFaceVertices[indexFace + 1] - beginFaceVertices[indexFace].
        // Must be preceded by a call to RemediateNonManifoldEdgesAndVertices.
        //
        // indexFace: index in mesh of face of interest; input
        MVEXPORT size_t GetNumberOfManifoldVerticesForFace(size_t indexFace) const;

        // Returns the list of nVerticesFace = GetNumberOfManifoldVerticesForFace(indexFace) vertices defining mesh face
        // indexFace after remediation, in counterclockwise order. If the face is inactive, so that nVerticesFace = 0, a
        // null pointer is returned. Must be preceded by a call to RemediateNonManifoldEdgesAndVertices.
        //
        // indexFace: index in mesh of face of interest; input
        MVEXPORT const int* GetManifoldVerticesForFace(size_t indexFace) const;

        // Returns the list of nEdgesFace = GetNumberOfManifoldVerticesForFace(indexFace) edges defining mesh face indexFace
        // after remediation, in counterclockwise order. Note that face edge e is presumed to connect face vertices e and
        // (e + 1) % nEdgesFace. Edge indices are on the range [0, GetNumberOfManifoldEdges()). Information for a particular
        // edge may be retrieved via GetEdgeDataforManifoldEdge. If the face is inactive, so that nEdgesFace = 0, a null
        // pointer is returned. Must be preceded by a call to RemediateNonManifoldEdgesAndVertices.
        //
        // indexFace: index in mesh of face of interest; input
        MVEXPORT const int* GetManifoldEdgesForFace(int indexFace) const;

        // Returns the number of one- or two-sided, manifold edges required to represent the mesh after remediation. Must be
        // preceded by a call to RemediateNonManifoldEdgesAndVertices.
        MVEXPORT size_t GetNumberOfManifoldEdges() const;

        // Returns the data for the specified mesh edge, as referenced by the face edge lists returned by GetManifoldEdgesForFace,
        // in the format detailed in the description of the EdgeDataType enumeration. Must be preceded by a call to
        // RemediateNonManifoldEdgesAndVertices.
        //
        // indexEdge: index in mesh, after remediation, of edge of interest; input
        MVEXPORT const int* GetEdgeDataforManifoldEdge(int indexEdge) const;

        // Returns the sequence of edge-connected mesh faces about the specified vertex, in counterclockwise order. Must be
        // preceded by a call to RemediateNonManifoldEdgesAndVertices.
        //
        // indexVertex: index in mesh, after remediation, of vertex of interest; input
        MVEXPORT geo::span<const int> GetFacesAtVertex(int indexVertex) const;

        // Returns the sequence of edge-connected mesh face corners about the specified vertex, in counterclockwise order. Must be
        // preceded by a call to RemediateNonManifoldEdgesAndVertices.
        //
        // indexVertex: index in mesh, after remediation, of vertex of interest; input
        MVEXPORT geo::span<const int> GetFaceCornersAtVertex(int indexVertex) const;

    // Private types:
    private:

        // Enumeration indexing vertices along an edge.
        enum EdgeVertexType
        {
            FIRST_VERTEX    = 0,
            SECOND_VERTEX   = 1,
            N_VERTICES_EDGE = 2
        };

        // Enumeration indexing face edges at a vertex.
        enum EdgesAtVertexType
        {
            INCOMING_EDGE       = 0,
            OUTGOING_EDGE       = 1,
            N_EDGE_TYPES_VERTEX = 2
        };

        // Enumeration indexing faces at a manifold edge.
        enum FacesAtEdgeType
        {
            FIRST_FACE = 0,
            SECOND_FACE = 1,
            N_FACES_EDGE = 2
        };

        // Enumeration indexing data values defining edges outgoing from some particular vertex, so that if edgeData is the array
        // defining some particular outgoing edge, then the edge is edge edgeData[INDEX_FACE_EDGE] in the bounding edge loop for
        // face edgeData[INDEX_FACE], and vertex edgeData[INDEX_END_VERTEX] is the global mesh index of the vertex of face
        // edgeData[INDEX_FACE] to which the edge is incoming.
        enum VertexFaceEdgeDataType
        {
            INDEX_END_VERTEX   = 0,
            INDEX_FACE         = 1,
            INDEX_FACE_EDGE    = 2,
            N_VERTEX_EDGE_DATA = 3
        };

        // Enumeration indexing edges to be merged; note that during a merge operation, the target edeg, and its vertices, are
        // retained.
        enum MergedEdgeType
        {
            SOURCE_EDGE    = 0,
            TARGET_EDGE    = 1,
            N_EDGES_MERGED = 2
        };

    // Private variables:
    private:

        // Invalid index or count value.
        static constexpr int m_invalidIndex = -1;

        // Total number of vertices in the input mesh.
        size_t m_nVertices;

        // Total number of vertices in the input mesh, after preprocessing to remediate, and possibly excise, any "null" edges.
        size_t m_nVerticesPreprocessed;

        // Total number of physical vertices, if the input mesh represents a mapped-space topology, or
        // m_nPhysicalVertices = m_invalidIndex, otherwise.
        int m_nPhysicalVertices;

        // Total number of physical vertices after preprocessing to remediate, and possibly excise, any "null" edges, if the
        // input mesh represents a mapped-space topology, or m_nPhysicalVerticesPreprocessed = m_invalidIndex, otherwise.
        int m_nPhysicalVerticesPreprocessed;

        // Entry m_faceVertices[m_beginFaceVertices[f] + v] is the index of the vth vertex of face f of the mesh, for v on the
        // range [0, m_beginFaceVertices[f + 1] - m_beginFaceVertices[f]). Furthermore, vertex v of face f, after remediation,
        // is (a duplicate of) original face vertex m_faceVerticesRetained[m_beginFaceVertices[f] + v].
        std::vector<int> m_faceVertices;
        std::vector<int> m_faceVerticesRetained;
        std::vector<int> m_beginFaceVertices;

        // Copy of the unmodified beginFaceVertices array, as passed to the constructor, consistent with face vertex counts
        // prior to any "null" edge excision.
        std::vector<int> m_beginFaceVerticesInit;

        // When nPhysicalFaceVerticesTot = nFaceVerticesTot, where nPhysicalFaceVerticesTot = m_physicalFaceVertices.size() and
        // nFaceVerticesTot = m_faceVertices.size(), the faces defined by m_faceVertices and m_beginFaceVertices are presumed to
        // be mapped-space faces, with physical face f, defined by the bounding vertex loop m_physicalFaceVertices[f' + v] for v on
        // the range [0, nVerticesFace), corresponding to map face f, defined by the bounding vertex loop m_faceVertices[f' + v],
        // where nVerticesFace = m_beginFaceVertices[f + 1] - f', and f' = m_beginFaceVertices[f]; otherwise, empty arrays.
        std::vector<int> m_physicalFaceVertices;

        // Entry m_manifoldFaceEdges[m_beginFaceVertices[f] + e] is the index of the eth edge of f of the mesh after remediation,
        // for e on the range [0, m_beginFaceVertices[f + 1] - m_beginFaceVertices[f]); the edge is defined by the edge data array
        // &(m_manifoldEdgeData[(m_manifoldFaceEdges[m_beginFaceVertices[f] + e]) * N_EDGE_DATA]), which is indexed as detailed in
        // the description of the EdgeDataType enumeration.
        std::vector<int> m_manifoldFaceEdges;

        // Definitions of each edge required to represent the remediated mesh, with edge e defined by the edge data array
        // &(m_manifoldEdgeData[e * N_EDGE_DATA]), which is indexed as detailed in the description of the EdgeDataType enumeration.
        std::vector<int> m_manifoldEdgeData;

        // If vertex v was generated by a splitting operation performed to remediate a "null" edge, a non-manifold edge, or a
        // non-manifold vertex, that is, for v >= nVertices = m_nVertices, entry m_sourceVertices[v] is the index of the vertex,
        // on the range [0, nVertices), from which vertex v was split; otherwise, m_sourceVertices[v] = v.
        std::vector<int> m_sourceVertices;

        // If vertex v was generated by a splitting operation performed to remediate a non-manifold edge or vertex, that is,
        // for v >= m_nVerticesPreprocessed, entry m_sourceVerticesPreprocessed[v] is the index of the vertex, on the range
        // [0, m_nVerticesPreprocessed), from which vertex v was split; otherwise, m_sourceVerticesPreprocessed[v] = v.
        std::vector<int> m_sourceVerticesPreprocessed;

        // After remediation mesh vertex v is surrounded by a single open or closed counterclockwise sequence of edge-connected
        // faces given by m_facesAtVertex[v], with vertex v at the corners of the faces listed in m_faceCornersVertex.
        std::vector<std::vector<int>> m_facesAtVertex;
        std::vector<std::vector<int>> m_faceCornersAtVertex;

        // Provide caller access to detected nonmanifold vertex indices after calling FindNonManifoldEdgesAndVertices or
        // RemediateNonManifoldEdgesAndVertices.
        std::vector<int> m_detectedNonManifoldVertexIndices;

    // Private member functions:
    private:

        // Helper functions to simplify ProcessNonManifoldEdgesAndVertices.
        void AdjustPreviousEdgeOnThisFace(const int* edgeDataCurr, int nVerticesFace, std::vector<std::vector<int>>& vertexOutgoingEdges,
            int* faceVertices, int v, std::vector<std::set<size_t>>& vertexNonManifoldEdges, int splitVertexNewVertex);
        void ExciseSubsequentEdgeOnThisFace(std::vector<std::vector<int>>& vertexOutgoingEdges, std::vector<int>& splitVertexEdges,
            const int* edgeDataCurr, int indexOutgoingEndVertexEdgeInFace, std::vector<std::set<size_t>>& vertexNonManifoldEdges);
        int IdentifyNonManifoldEdges(std::vector<std::set<size_t>>& vertexNonManifoldEdges, std::vector<std::set<size_t>>& vertexEdgesSplitFrom,
            const std::vector<std::vector<int>>& vertexOutgoingEdges);
        void GrowSourceVertices(int nVerticesTot);
        int ComputeAdditionalSplitEdges(const std::vector<std::set<size_t>>& vertexEdgesSplitFrom, const std::vector<std::vector<int>>& vertexOutgoingEdges,
            std::vector<std::set<size_t>>& vertexNonManifoldEdges);
        void SplitVertexOnNonManifoldEdge(size_t& manifoldRangeBegin, std::set<size_t>::const_iterator& iterNonManifoldEdges, const size_t& nEdgesVertex,
            std::vector<int>& manifoldEdgesAtVertex, std::vector<int>& vertexEdges, int v, int& indexVertexCurr,
            std::vector<std::vector<int>>& vertexOutgoingEdges, std::vector<std::set<size_t>>& vertexNonManifoldEdges);
        int BuildManifoldEdgeData(const std::vector<std::vector<int>>& vertexOutgoingEdges, std::vector<int>& unorderedEdgeData, int nFaces);
        void OrderEdges(int nEdges, int nFaceEdgesTot, std::vector<int>& orderedEdges, const int& edgeAllocationFactor, std::vector<int>& unorderedEdgeData);
        void AllocateWorkArrays(int nFaceEdgesTot, std::vector<int>& orderedEdges, std::vector<int>& unorderedEdgeData);

        // Builds the m_faceVertices, m_faceVerticesRetained, and m_beginFaceVertices arrays to reflect the mesh topology
        // defined by faceVertices, with any "null" edges excised from the face bounding loops, as discussed in the class
        // description.
        //
        // faceVertices: entry faceVertices[begin + v], for v on the range [0, m_beginFaceVerticesInit[f + 1] - begin), is the
        //               index of vertex v along the bounding vertex loop of face f of the mesh of interest, prior to excision
        //               of any null edges, where begin = m_beginFaceVerticesInit[f]; input
        void EliminateNullEdges(geo::span<const int> faceVertices);

        // Preprocesses the mesh topology defined by the faceVerticesInit and m_beginFaceVerticesInit arrays in order to
        // remediate any degeneracies which may hinder processing of the mesh by this class, returning the adjusted topology in
        // the faceVerticesPreprocessed array. Presently, the only degeneracy considered is the case of a "null" edge, in which
        // a particular edge e of some face f has the same vertex at either endpoint, that is, where:
        //
        //     verticesFaceInit[e'] = verticesFaceInit[e]
        //     e' = (e + 1) % nVerticesFace
        //     
        // with verticesFaceInit[v] = faceVerticesInit[m_beginFaceVerticesInit[f] + m_faceVerticesRetained[begin + v]],
        // nVerticesFace = m_beginFaceVertices[f + 1] - begin, and begin = m_beginFaceVertices[f]. To remediate such a
        // degeneracy, for the nth null edge encountered during processing, a split vertex v_split = nVerticesInit + n is
        // formed, with sourceVertices[v_split] = verticesFaceInit[e], is formed, and employed for vertex e' along the
        // preprocessed bounding vertex loop verticesFace = &(faceVerticesPreprocessed[begin]).
        //
        // faceVerticesPreprocessed: array &(faceVerticesPreprocess[m_beginFaceVertices[f]]) specifies the bounding vertex loop
        //                           for face f of the mesh topology of interest, after preprocessing; output
        // sourceVertices:           for any vertex v' = verticesFace[v] of the preprocessed mesh topology, where
        //                           verticesFace = &(faceVerticesPreprocessed[m_beginFaceVertices[f]]), if
        //                           v' >= nVerticesInit, then vertex v' is a duplicate of input mesh vertex sourceVertices[v']
        //                           formed during preprocessing, otherwise v' = sourceVertices[v'] is an input mesh vertex;
        //                           output
        // faceVerticesInit:         array &(faceVerticesInit[m_beginFaceVerticesInit[f]]) specifies the input bounding vertex
        //                           loop for face f of the mesh topology of interest; input
        // nVerticesInit:            total number of vertices for the mesh topology of interest, so that all vertex indices
        //                           specified in the faceVerticesInit array are on the range [0, nVerticesInit); input
        void PreprocessMeshTopology(std::vector<int>& faceVerticesPreprocessed, std::vector<int>& sourceVertices, geo::span<const int> faceVerticesInit, size_t nVerticesInit);

        // Detects and, if requested, remediates non-manifold geometry, populating member structures. After being called with
        // doRemediate set to true, should not be called again for this instance.
        //
        // nonManifoldVertices: list of which vertices that wer detected to be non-manifold. If requested, remediated, or zero if doRemediate is
        //                       false and non-manifold edges are present; output
        // nNonManifoldEdges:    number of non-manifold face edges detected and, if requested, remediated; output
        // doRemediate:          flag controlling whether remediation is performed, or only detection; input
        void ProcessNonManifoldEdgesAndVertices(std::vector<int>& nonManifoldVertices, size_t& nNonManifoldEdges, bool doRemediate);

        // Performs some topological cleanup on edges split from originally non-manifold mesh edges, in order to avoid as much as
        // possible collinear boundary edges, and undesirable vertex connectivity.
        //
        // edgesAtVertex:               entry edgesAtVertex[et][v] is the list of edges, defined by the m_manifoldEdgeData array,
        //                              with sense et; modified
        // vertexOutgoingPhysicalEdges: if vertexOutgoingPhysicalEdges is not null, then it is assumed that the input mesh
        //                              represents a mapped-space topology, and that physical-space connectivity information is to
        //                              be employed, as detailed in the description of the function SetPhysicalSpaceFaces, so that
        //                              the array &((*vertexOutgoingPhysicalEdges)[v][e * N_VERTEX_EDGE_DATA]), indexed according
        //                              to the VertexFaceEdgeDataType enumeration, defines the eth outgoing physical edge at
        //                              physical vertex v; input
        void RemediateSplitTopology(std::vector<std::set<int>> edgesAtVertex[N_EDGE_TYPES_VERTEX], const std::vector<std::vector<int>>* vertexOutgoingPhysicalEdges = nullptr);

        // Returns an edge neighbor face indexFaceNB across the specified edge of the passed mesh topology, and the position of the
        // specified edge in face indexFaceNB's bounding edge loop. If the specified edge is manifold, indexFaceNB will be the sole
        // face that uses the specified edge in the sense opposite to that used by face indexFace, while if the edge is non-manifold,
        // non-manifold, it will simply be the first such face encountered. 
        //
        // indexFaceNB, indexEdgeInFaceNB:  if edge indexEdgeInFace of the bounding edge loop of face indexFace is an interior
        //                                  edge, then face indexFaceNB is a neighbor of face indexFace across this edge, and the
        //                                  edge is edge indexEdgeInFaceNB of face indexFaceNB's bounding edge loop, while in the
        //                                  case of a boundary edge, we return indexFaceNB = m_invalidIndex = indexEdgeInFaceNB;
        //                                  output
        // indexFace, indexEdgeInFace:      we are to determine the edge neighbor of face indexFace across edge indexEdgeInFace of
        //                                  its bounding edge loop; input
        // vertexOutgoingEdges:             array &((*vertexOutgoingEdges)[v][e * N_VERTEX_EDGE_DATA]), indexed according to the
        //                                  VertexFaceEdgeDataType enumeration, defines the eth outgoing edge at vertex v; input
        // faceVertices, beginFaceVertices: face f is defined by the bounding vertex loop faceVertices[f' + v] for v on the range
        //                                  [0, nVerticesFace), where f' = beginFaceVertices[f], and the number of face vertices
        //                                  nVerticesFace = beginFaceVertices[f + 1] - f'; input
        void GetConnectedFaceEdge(int& indexFaceNB, int& indexEdgeInFaceNB, int indexFace, int indexEdgeInFace,
            const std::vector<std::vector<int>>& vertexOutgoingEdges, const std::vector<int>& faceVertices, const std::vector<int>& beginFaceVertices);

        // Computes the edge-connected element of the mesh to which each face belongs. Returns the total number of elements
        // identified.
        //
        // faceElement: entry faceElement[f] is the index of the edge-connected mesh element to which face f belongs; output
        int BuildElements(std::vector<int>& faceElement);

        // Returns a flag indicating whether merging of the specified edges, which are presumed to be one-sided edges connecting
        // vertices with the same sources in opposite senses, is a valid operation. A merge is judged to be valid if it would not
        // reintroduce any non-manifold edges.
        //
        // mergedEdges:   indices of edges, defined in the m_manifoldEdgeData array, to be merged; input
        // edgesAtVertex: entry edgesAtVertex[et][v] is the list of edges, defined by the m_manifoldEdgeData array, with sense et;
        //                input
        bool CheckMergeValid(const int mergedEdges[N_EDGES_MERGED], const std::vector<std::set<int>> edgesAtVertex[N_EDGE_TYPES_VERTEX]);

        // Merges a specified pair of matching one-sided edges; the specified edges are presumed to have opposite sense, so that
        // vertices on opposite ends of the source and target edges have the same source vertex, and these matching vertices are
        // merged if not already identical, with the vertex on the target edge retained, and that on the source edge reduced to an
        // isolated vertex.
        //
        // mergedEdges:   indices of edges, defined in the m_manifoldEdgeData array, to be merged; input
        // edgesAtVertex: entry edgesAtVertex[et][v] is the list of edges, defined by the m_manifoldEdgeData array, with sense et;
        //                modified
        void MergeEdges(const int mergedEdges[N_EDGES_MERGED], std::vector<std::set<int>> edgesAtVertex[N_EDGE_TYPES_VERTEX]);

        // Build  all counterclockwise sequences of consecutive edge-connectedfaces about the specified vertex, noting that for
        // non-manifold vertices, we shall obtain multiple unrelated sequences.
        //
        // vertexFaces,             array sequenceFaces = &(vertexFaces[vertexFaceSequenceBegin[seq]]) list the seqth sequence of
        // vertexFaceCorners,       edge-connected faces about vertex indexVertex, consisting of nFaces faces, where
        // vertexFaceSequenceBegin: nFaces = vertexFaceSequenceBegin[seq + 1] - vertexFaceSequenceBegin[seq], with
        //                          sequenceCorners[f] denoting the corner of face sequenceFaces[f] at the vertex, where
        //                          sequenceCorners = &(vertexFaceCorners[vertexFaceSequenceBegin[seq]]); output
        // edgesAtVertex:           on entry, *(vertexEdges[et]) is the list of edges, defined by the m_manifoldEdgeData array,
        //                          with sense et, on exit, *(vertexEdges[et]) is cleared; modified
        // indexVertex:             index of the vertex to operate upon; input
        void BuildFaceSequencesAtVertex(std::vector<int>& vertexFaces, std::vector<int>& vertexFaceCorners,
            std::vector<size_t>& vertexFaceSequenceBegin, std::set<int>* vertexEdges[N_EDGE_TYPES_VERTEX],
            int indexVertex);
    };
};
