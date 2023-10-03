//**************************************************************************/
// Copyright (c) 2022 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//**************************************************************************/
// DESCRIPTION: Helper function to render various objects
// AUTHOR: Autodesk Inc.
//**************************************************************************/

#pragma once

#include "../maxapi.h" // ViewExp
#include "../object.h" // USE_DAMAGE_RECT

class GraphicsWindow;
class Material;

// Helper function to deal with USE_DAMAGE_RECT flag when calling render() function.
// Use this to avoid "error C2102: '&' requires l-value".
// Old code was written as ... &pVpt->GetDammageRect()..., which is incorrect because GetDammageRect returns an rvalue.
template <class MeshType>
inline void MeshRenderHelper(MeshType& mesh, GraphicsWindow* pGW, Material* mtls, int flags, ViewExp* pVpt, int compFlags, int numMat = 1)
{
	const bool useRect = (flags & USE_DAMAGE_RECT);
	Rect damageRect = (useRect && pVpt) ? pVpt->GetDammageRect() : Rect();
	mesh.render(pGW, mtls, useRect ? &damageRect : nullptr, compFlags, numMat);
}
