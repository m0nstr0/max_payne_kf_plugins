//**************************************************************************/
// Copyright (c) 2020 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once
#include "../ifnpub.h"
#include "../Graphics/FragmentGraph/ViewFragment.h"
#include "IInteractiveRenderer.h"
#include "IActiveShadeFragmentManager.h"

namespace MaxSDK {
  
    /** Used to retrieve a pointer to an instance of a global class that implements the
    IActiveShadeFragmentManager2 interface */
    #define IACTIVE_SHADE_VIEWPORT_MANAGER_INTERFACE_2 Interface_ID(0x66c54933, 0x68955573)

    /**
    The IActiveShadeFragmentManager2 class is used to control the ActiveShade fragment (in-viewport ActiveShade) and extends IActiveShadeFragmentManager.
    Get it through 
    * Sample code in C++ :\n
        * \code{.cpp}
        * MaxSDK::IActiveShadeFragmentManager2* pActiveShadeFragmentManager2 = dynamic_cast<MaxSDK::IActiveShadeFragmentManager2*>(GetCOREInterface(IACTIVE_SHADE_VIEWPORT_MANAGER_INTERFACE_2));
        * if (nullptr != pActiveShadeFragmentManager2) {
        * ...
        * }
        \endcode
    */
    class IActiveShadeFragmentManager2 : public IActiveShadeFragmentManager
    {
    public:

        /** Get the ActiveShade fragment resolution multiplier.
        A multiplier of 0.5 will give a half resolution ActiveShade rendering used and upscaled in the viewport. So a better time performance but resulting in a lower image quality
        This multiplier has to be a positive, non-zero float number.
        \return the value of the multiplier.
        */
        virtual float GetActiveViewportResolutionMultiplier(void) const = 0;

        /** Set the ActiveShade fragment resolution multiplier.
        A multiplier of 0.5 will give a half resolution ActiveShade rendering used and upscaled in the viewport. So a better time performance but resulting in a lower image quality
        This multiplier has to be a positive, non-zero float number.
        \param[in] factor the value of the multiplier, if the multipler is negative or 0, we set that value to 1.0f.
        */
        virtual void SetActiveViewportResolutionMultiplier(float factor) = 0;
    };
}//End of namespace MaxSDK