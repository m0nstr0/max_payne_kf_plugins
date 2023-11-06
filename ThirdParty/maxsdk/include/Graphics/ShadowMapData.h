//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "../Noncopyable.h"
#include "../inode.h"
#include "Matrix44.h"

namespace MaxSDK { namespace Graphics {

/** ShadowMapData is used to get the shadow map data associated to an INode which is a light.*/
class ShadowMapData : public MaxSDK::Util::Noncopyable
{
public:
	/** Get an instance of that class.
		\return the unique instance of that class.
	*/
	MaxGraphicsObjectAPI static ShadowMapData& Singleton();

	/** Retrieve the shadow maps data from the light node provided
		\param[in] node - the INode which should be a light node only.
		\param[out] shadowcolor : the color of the shadow for this light.
		\param[out] shadowDepthBias : the depth bias of the shadow for this light.
		\param[out] shadowSlopeBias : the slope bias of the shadow for this light.
		\param[out] shadowDepthRange : the depth range of the shadow map for this light.
		\param[out] shadowMapResolution : the shadow map resolution (is always a square texture).
		\param[out] shadowCubeMatrix : the matrix to convert from light space to Cube space for the shadow map.
		\param[out] viewProjMatricesPerFaceCube : the 6 view and projection matrices combined together for each cubemap face to convert from light space to viewport clip space. If the shadow map 
		is not a cubemap, there is only 1 view projection matrix in the array.
		\param[out] viewMatricesPerFaceCube : the 6 view matrices to convert from world space to light space for each cube map face. If the shadow map is not a cubemap, there is only 1 view matrix in the array.
		\param[out] bShadowMapIsCubemap
		\return false if something went wrong during the call so the returned parameters are not valid, true if everything is ok.
	*/
	MaxGraphicsObjectAPI  bool GetShadowMapData(INode& node, AColor& shadowcolor, float& shadowDepthBias, float& shadowSlopeBias, Point2& shadowDepthRange, int& shadowMapResolution,
												Matrix44& shadowCubeMatrix, Matrix44* viewProjMatricesPerFaceCube, Matrix44* viewMatricesPerFaceCube, bool& bShadowMapIsCubemap);
};

}} // namespace MaxSDK::Graphics
