//
// Copyright 2021 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "maxtypes.h"

class INode;

namespace MaxSDK { namespace Graphics {
//! \brief The interface for filtering per viewport.
/*! The clients can access this interface to query if a node is filtered by a viewport.
All methods of this class are implemented by the system.
\see View3D
*/
class IPerViewportFilter
{
public:
    virtual ~IPerViewportFilter() {}

    /*  Return true if a given node is filtered for the viewport, false otherwise.
        This method is implemented by the system, it performs various checks to see if the node is filtered for the viewport.
        Note: To get a better performance, call IsNodeFiltered(const SClass_ID&, const Class_ID&, bool, INode*) instead when the node information is known.

        @param node : The pointer of the node.
    */
    virtual bool IsNodeFiltered(INode* node) const = 0;

    /*  Return true if a given node is filtered for the viewport, false otherwise.
        This method is implemented by the system, it performs various checks to see if the node is filtered for the viewport.
        Note: Call IsNodeFiltered(INode*) instead when any node information (SuperClassID, ClassID or Renderable) is not known.
        To get the node information, the evaluation of the node's pipeline is required. 
        If the node information to pass doesn't match the node, an incorrect result could be returned.

        Usage:
        \code
            ObjectState os = node->EvalWorldState(GetCurrentTime());
            Class_ID nodeClassID = (os.obj) ? os.obj->ClassID() : Class_ID(0, 0);
            SClass_ID nodeSuperClassID = (os.obj) ? os.obj->SuperClassID() : 0;
            bool objRenderable = (os.obj) ? os.obj->IsRenderable() : false;
            bool nodeRenderable = objRenderable && node->Renderable();

            bool filtered = IsNodeFiltered(nodeSuperClassID, nodeClassID, nodeRenderable, node);
        \endcode

        @param nodeSuperClassID: The Super Class ID of the evaluated object from the node.
        @param nodeClassID: The Class ID of the evaluated object from the node.
        @param nodeRenderable: The flag to indicate if the evaluated object from the node is renderable or not.
        @param node : The pointer of the node.
    */
    virtual bool IsNodeFiltered(const SClass_ID& nodeSuperClassID, const Class_ID& nodeClassID, bool nodeRenderable, INode* node) const = 0;
};
}}

