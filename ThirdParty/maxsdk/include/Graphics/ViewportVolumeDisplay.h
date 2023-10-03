//
// Copyright 2021 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "GraphicsExport.h"
#include "UpdateDisplayContext.h"
#include "UpdateNodeContext.h"
#include "TextureHandle.h"
#include "../SharedMesh.h"
#include "../geom/Matrix3.h"
#include "../geom/Box3.h"
#include "../geom/AColor.h"

namespace MaxSDK{namespace Graphics{

/**
* \brief Volume Display functions for 3ds Max's viewport
* This is a set of functions for an Object plug-in to support a volume display in the viewport
* Link with MaxGraphics.lib when using this header.
*/

namespace ViewportVolumeDisplay
{
	/** The type of display for the volume.*/
	enum class DisplayType
	{
		BoundingBox,//!< Display the bounding box of the volume.
		Voxels,		//!< Display the voxels' positions as box meshes.
		Volume,		//!< Display the volume grid doing ray marching on the GPU.
		Wireframe,	//!< Display the volume as a wireframe mesh (the mesh used is the one from GridGPUParams::mpWireframeMesh if it is not null).
		Iterations, //!< For debugging: display the number of iterations for each pixel of the volume. We use the value from DisplayParams::mMaxIterations to set the maximum iterations.
					//!< The darker the color is, less iterations are made during the ray marching. A white color means you are at or over DisplayParams::mMaxIterations iterations.
		Gradient,	//!< For debugging: display the gradient (the normal) per pixel.
		Octree,		//!< For debugging: display the subdivisions to see filled or empty voxels, a red color means the voxel is non empty, a green color means it is empty.
		LastValue = Octree,
	};

	/**  The type of grid for this volume, there is one grid displayed per volume.*/
	enum class GridType
	{
		Float16,//!<A 16 bits float (also called a half such as in OpenEXR) data for each voxel of the volume.
		Float32,//!<A 32 bits float data for each voxel of the volume.
		LastValue = Float32,
	};

	/** The type of algorithm used to set color on the volume grid.*/
	enum class GridColorType
	{
		Color,				//!< The grid is displayed using a color which is accumulated during the volume sampling
		GradientMap,		//!< The float value of the grid is converted from a float to an ARGB color using a Texmap, which can theoretically be a 1D image since we only use one line of pixels. 
							//!< If you use 2D Bitmap/Texmap, we use the first line of the image
		BlackBody,			//!< The grid density is converted from float to RGB using the black body formula
		TurboRainbow,		//!< Using a Turbo rainbow gradient
		LastValue = TurboRainbow,
	};

	/** We can normalize the volume grid data when providing a Float16 or Float32 grid.*/
	enum class NormalizationType
	{
		None,		//!< No normalization, we will use the grid data as is, be careful as the display of the volume is expecting values in the [0, 1] range, going over that range may have unexpected effects.
		Automatic,	//!< We compute the min and max of the values and normalize the data.
		ManualRange,//!< You provide the the min and max values we use for normalizing the data into GridParams::mManualNormalizationRange.
		LastValue = ManualRange,
	};

	/** Slice axis is used to display only a part of the volume, a slice of it on a given axis at a given position and thickness.*/
	enum class SliceAxis 
	{
		Off,	//!< Not using it.
		X,		//!< Using the X axis as the slice axis.
		Y,		//!< Using the Y axis as the slice axis.
		Z,		//!< Using the Z axis as the slice axis.
		LastValue = Z,
	};

	/**  GridParams is where you define the parameters of the volume grid, independently from the display.
	* You are going to pass this structure so that the helper function ComputeGridGPUParams will convert it to a GridGPUParams structure which is GPU compliant.
	* The GridParams structure is the in parameters of the helper function ComputeGridGPUParams and the GridGPUParams structure is the out parameters.*/
	struct GridParams
	{
		// Volume grid
		int					mGridWidth						= 0;		//!< Width of the grid.
		int					mGridHeight						= 0;		//!< Height of the grid.
		int					mGridDepth						= 0;		//!< Depth of the grid.
		void*				mpVoxelsGridValues				= nullptr;	//!< For example : is either a float* if GridParams::mGridType == GridType::Float32, or a half* if GridParams::mGridType == GridType::Float16.
		void*				mpBackgroundValue				= nullptr;	//!< A pointer on a background value which is either a float if GridParams::mGridType == GridType::Float32 or a half if GridParams::mGridType == GridType::Float16.
		/** A boolean to tell if this grid is a level set or not. A level set grid is a grid created from a signed distance function (SDF). 
		* It contains negative and positive values that are the distance to the surface.
		* Values for level sets grid go from -background to +background with negative values being the inside of the surface, 0.0 being 
		* the surface and positive values being the outside of the surface.
		*/
		bool				mbIsLevelSetGrid				= false;
		
		/** Type of data for each voxel, a voxel is a volume element. The resulting volume texture on the GPU will use the same format for its data.
		* So using 16 bits will make a smaller GPU texture than using 32 bits voxel data.*/
		GridType			mGridType						= GridType::Float16; 
		
		// Normalization
		NormalizationType	mNormalizationType				= NormalizationType::Automatic;//!< Type of normalization for the grid data when the grid is a float grid. If this grid is a level set, GridParams::mbIsLevelSetGrid is true, then these parameters are ignored, we won't normalize the data.
		float				mManualNormalizationRangeMin	= 0.0f;//!< Only used when GridParams::mNormalizationType = NormalizationType::ManualRange, is the minimum of the range
		float				mManualNormalizationRangeMax	= 1.0f;//!< Only used when GridParams::mNormalizationType = NormalizationType::ManualRange, is the maximum of the range

		/** The size of the array mpVoxelsPositions, is optional if you don't want to use the voxels positions as a display mode. 
		* If you are using openvdb to fill this data, it's : pGrid->activeVoxelCount() */
		size_t				mNumVoxelsPositions				= 0;
		/** Optional if you don't want to use the voxels positions as a display mode. 
		* If provided, you will still own this data, we will reuse your pointer and 
		* copy its value into GridGPUParams::mpVoxelPositions when you call ComputeGridGPUParams.
		* Therefore this data must not be deleted while the volume is still in use.
		* An example on how to provide mpVoxelsPositions data using openvdb : 
		\code
		const auto bboxIndexSpace			= mFloatGrid->evalActiveVoxelBoundingBox();
		const openvdb::Vec3d worldBBoxMin	= mFloatGrid->indexToWorld(bboxIndexSpace.min());
	
		const openvdb::Coord gridDimensions = mFloatGrid->evalActiveVoxelDim();
		const size_t width					= gridDimensions.x();
		const size_t height					= gridDimensions.y();
		const size_t depth					= gridDimensions.z();
		
		Box3 localNodeBBox;
		mMyVolumeObj.GetLocalBoundBox(GetCOREInterface()->GetTime(), nullptr, nullptr, localNodeBBox);
		const Point3 localNodeBBoxSize		= localNodeBBox.Width();
	
		//Store locally voxels positions
		mNumVoxelsPositions					= mFloatGrid->activeVoxelCount();
		mpVoxelsPositions					= new Point3[mNumVoxelsPositions];
		int curVoxelIndex					= 0;
		for (auto iter = mFloatGrid->cbeginValueOn(); iter; ++iter, ++curVoxelIndex){
			auto coord							= iter.getCoord();
			const openvdb::Vec3d worldCoord		= mFloatGrid->indexToWorld(coord);
			mpVoxelsPositions[curVoxelIndex].x	= ((worldCoord.x() - worldBBoxMin.x())/width)	* localNodeBBoxSize.x;
			mpVoxelsPositions[curVoxelIndex].y	= ((worldCoord.y() - worldBBoxMin.y())/height)	* localNodeBBoxSize.y;
			mpVoxelsPositions[curVoxelIndex].z	= ((worldCoord.z() - worldBBoxMin.z())/depth)	* localNodeBBoxSize.z;
			mpVoxelsPositions[curVoxelIndex]	+= localNodeBBox.Min();
		}
		\endcode
		*/
		Point3*				mpVoxelsPositions				= nullptr;	
		
		Point3				mVoxelsSize = Point3(1.f,1.f,1.f);//!< The size of voxels, like the voxel size from openvdb.
		
		Box3				mNodeLocalBoundingBox;//!< Local bounding box of the node, will be copied to GridGPUParams::mNodeLocalBoundingBox.

		/** This will create a mesh to be used in wireframe representation of the volume. The resulting mesh can be found in GridGPUParams::mpWireFrameMesh.
		* In case you want to use your own wireframe representation for the volume, you can, by setting this to false, and 
		* initializing GridGPUParams::mpWireFrameMesh with a custom SharedMesh (we will use your pointer but never delete it).*/
		bool				mbComputeWireframeMesh	= true;	
		/** Used only if mbComputeWireframeMesh is true. It is the number of subdivisions for the bounding box to create the wireframe mesh. 
		* We subdivide the bounding box of the volume by this wireFrameLevel and create a box mesh from each non empty cells which 
		* is concatenated into a single mesh. The smaller the value, the coarser the wireframe mesh.*/
		int					mWireframeLevel			= 10;
	};

	/** This structure contains the GPU data of the volume that lives as the DisplayParams::mGridGPUParams member so we don't recompute all of this each frame.
	 You store locally a DisplayParams instance in your plug-in and use its DisplayParams::mGridGPUParams member to be filled by the call to ComputeGridGPUParams. 
	You should not have to change anything in that structure, you can see it as the GPU compliant volume data.
	It is built using the helper function ComputeGridGPUParams(const GridParams& inGridParams, GridGPUParams& outGridGPUParams);
	*/
	struct GridGPUParams
	{
		~GridGPUParams(){CleanUp();}//!< Destructor.
		TextureHandle	m3DTexture; //!< The volume grid information stored in a 3D texture.
		TextureHandle	mOctreeTexture; //!< The octree (spatial subdivision) stored in a 2D texture.
		Point3*			mpVoxelPositions	= nullptr; //!< The voxels positions, this is the pointer you provided in GridParams::mpVoxelsPositions. This data is used when DisplayParams::mDisplayType = DisplayType::Voxels.
		size_t			mNumVoxelsPositions = 0;//!< The number of voxels positions in mpVoxelPositions, this data is used when DisplayParams::mDisplayType = DisplayType::Voxels.
		Point3			mVoxelsSize			= Point3(1.f,1.f,1.f);//!< The size of voxels, is the same as GridParams::mVoxelsSize.
		/** A Shared Mesh Ptr computed from the grid voxels to be displayed in wireframe mode, it is valid only if GridParams::mbComputeWireframeMesh was true 
		* when calling ComputeGridGPUParams. It will be deleted and null'ed when its ref count reaches 0 automatically.
		* If you want a custom wireframe mesh, you can instantiate a SharedMesh and set it in that member so we will use your wireframe mesh for the display in wireframe.*/
		MaxSDK::SharedMeshPtr	mpWireframeMesh;
		Box3			mNodeLocalBoundingBox;//!< The local bounding box of the node your volume belongs to.
		bool			mbIsLevelSetGrid = false;	//!< Same  value as in GridParams::mbIsLevelSetGrid.
		float			mBackgroundValue = 0.f;//!< Is the same value as GridParams::mpBackgroundValue stored in a 32 bits float.
		
		/** Call this when you have finished using the volume data, when the grid data has changed for example if you are loading an animation or in your plug-in destructor.
		* After calling CleanUp() none of the volume data will be valid.*/
		void CleanUp()
		{
			m3DTexture.Release();
			mOctreeTexture.Release();
			// We don't own mpVoxelPositions so we don't delete it.
			mpVoxelPositions	= nullptr;
			mNumVoxelsPositions = 0;
			mVoxelsSize.x = mVoxelsSize.y = mVoxelsSize.z = 1.f;
			mNodeLocalBoundingBox.Init();
			mbIsLevelSetGrid = false;
			mBackgroundValue = 0.f;
		}
	};

	/** The parameters set for the display of the volume grid in the viewport*/
	struct DisplayParams
	{
		~DisplayParams(){CleanUp();}

		// Node and Object information.
		Matrix3		mNodeTM;   //!< Example : Use INode::GetNodeTM(t)).
		Matrix3		mObjectTM; //!< Example : Use INode::GetObjTMAfterWSM(t)).
		
		GridGPUParams	mGridGPUParams;//!< Grid GPU compliant data computed by ComputeGridGPUParams.

		//Level sets
		float		mLevelSetSurfaceValue = 0.f; //!< The level set surface value in the grid. It is used only when GridParams::mbIsLevelSetGrid is true.

		//Sampling parameters
		/** Used only when mRayMarchingStepType = RayMarchingStepType::Manual. It is the maximum iterations per pixel for sampling the volume during the ray marching,
		* the higher it is, the higher is the quality but the slower is the rendering. This value must be positive but has no upper limit.*/
		int			mRayMarchingMaxIterations		= 256;	
		float       mSamplingThreshold				= 1.f;	//!< In the range 0.001 to 1.0, we will stop the ray marching when the accumulated opacity of the volume will reach that value per pixel.
		/** A multiplier applied per voxel sample on the opacity, the lower it is, the less dense is the volume, 
		* this is to make your volume look more "cloudy".*/
		float       mOpacityMultiplier				= 1.f;	
		
		//Options
		bool        mbViewPointInsideVolume		= true;//!< Set this to true if you want the volume to take care of the camera being inside the volume, set it to false to ignore it (resulting in a faster display).
		DisplayType	mDisplayType				= DisplayType::Volume;//!< The type of display you want for the volume grid.

		/** Optional : This is only used with fog volume grids. It is a 1D texture (a single line of pixels) to map the density of the voxels to the range [0.0, 1.0].
		* You can use a 2D texture but we will only consider the first line. A typical example of texture is an OSL Curve (float) map. 
		* If the texture map is RGB or RGBA, we will use only the R component to do the mapping.
		* Use the function MaxSDK:Graphics::TextureHandle::Initialize(const MSTR& filename) defined in MaxSDK/include/Graphics/TextureHandle.h 
		* to create a TextureHandle from a bitmap fullpath filename and store it in your plugin, as it's a counted ref object.*/
		TextureHandle		mDensityTransferTexture; 
		
	//Display types parameters
		/** Usually there are millions of voxels for a given volume, and displaying a part of them is enough to get an idea of the shape of it.
		* So you can set the maximum voxels displayed. It is used only when DisplayParams::mDisplayType is DisplayType::Voxels.*/
		size_t      mMaxVoxelsDisplayed			= 100000;
		AColor		mVoxelsColor				= AColor(1.f, 1.f, 1.f, 1.f);//!< Used only if DisplayParams::mDisplayType is DisplayType::Voxels, is the color accumulated per voxel sample.
		int			mMaxIterations				= 100; //!< Used only when DisplayParams::mDisplayType is DisplayType::Iterations, see DisplayType::Iterations for more information.

		GridColorType	mGridColorType			= GridColorType::Color;//!< Use a color accumulated per voxel sample to display the grid.
		AColor			mGridColor				= AColor(1.f, 1.f, 1.f, 1.f);//!< Used only if DisplayParams::mGridColorType == GridColorType::Color, color to accumulate per voxel sample.
		/** Used only if DisplayParams::mGridColorType == GridColorType::GradientMap, provide a 1D texture (a single line of pixels) or 
		* a 2D texture (we will only use the first line of pixels) to map from a volume grid value (either fog or level set) to 
		* a RGBA color. If the texture map provided doesn't have an alpha layer, we will use 1.0 for the alpha, if it has an alpha layer we will use it. 
		* If you want to use a separate map to handle the alpha, please see DisplayParams::mGridGradientMapAlpha.
		* Use the function MaxSDK:Graphics::TextureHandle::Initialize(const MSTR& filename) defined in MaxSDK/include/Graphics/TextureHandle.h 
		* to create a TextureHandle from a bitmap fullpath filename.*/
		TextureHandle	mGridGradientMap;
		/** Used only if DisplayParams::mGridColorType == GridColorType::GradientMap and DisplayParams::mGridGradientMap has been provided. This is a 1D texture (a single line of pixels) or 
		* a 2D texture (we will only use the first line of pixels) to map from a volume grid value to 
		* an alpha value in the range [0,0, 1.0]. If the texture map you use is RGB or RGBA, we will use only the R value as the alpha value.
		* This works with fog and level set grids.
		* Use the function MaxSDK:Graphics::TextureHandle::Initialize(const MSTR& filename) defined in MaxSDK/include/Graphics/TextureHandle.h 
		* to create a TextureHandle from a bitmap fullpath filename.*/
		TextureHandle	mGridGradientMapAlpha;

		/** Used only if DisplayParams::mGridColorType == GridColorType::Blackbody and DisplayParams::mBlackbodyNormalize is false, is a factor applied 
		* to the black body temperature conversion, a factor of 1.0 makes it physically correct.*/
		float			mBlackbodyIntensity		= 1.0f;	
		bool			mBlackbodyNormalize		= true; //!< Used only if DisplayParams::mGridColorType == GridColorType::Blackbody, normalize the color with the luminance for the black body conversion.

		SliceAxis		mSliceAxis				= SliceAxis::Off;//!< Slice axis is used to display only a part of the volume, a slice of it on a given axis at a given position and thickness in a coordinates system.
		Matrix3			mSliceCoordinates;		//!< Coordinates system in which the slice takes place, using the position, rotation and scale.
		/** The thickness of the slice, is used only if mSliceAxis is different from SliceAxis::Off. The thickness is between 0.0 and 1.0 as 
		* it's relative to the the bounding box of the volume, a value of 0.5 meaning half of the bounding box size.*/
		float			mSliceAxisThickness		= 0.1f;	

		//! Call this when you have finished using this data, such as when the grid data has changed (displaying another grid) for example or in your plug-in destructor.
		void CleanUp()
		{
			mNodeTM						= Matrix3();
			mObjectTM					= Matrix3();
			mLevelSetSurfaceValue		= 0.f;
			mRayMarchingMaxIterations	= 256;	
			mSamplingThreshold			= 1.f;
			mOpacityMultiplier			= 1.f;	
			mbViewPointInsideVolume		= true;
			mDisplayType				= DisplayType::Volume;
			mMaxVoxelsDisplayed			= 100000;
			mVoxelsColor				= AColor(1.f, 1.f, 1.f, 1.f);
			mMaxIterations				= 100;
			mGridColorType				= GridColorType::Color;
			mGridColor					= AColor(1.f, 1.f, 1.f, 1.f);
			mBlackbodyIntensity			= 1.0f;	
			mBlackbodyNormalize			= true; 
			mSliceAxis					= SliceAxis::Off;
			mSliceCoordinates			= Matrix3();
			mSliceAxisThickness			= 0.1f;

			mGridGPUParams.CleanUp();
			mDensityTransferTexture.Release();
			mGridGradientMap.Release();
			mGridGradientMap.Release();
			mGridGradientMapAlpha.Release();
		}
	};

	/** This function is used to compute the GridGPUParams (GPU compliant data) information from the GridParams used by the display parameters,
	the output GridGPUParams is the DisplayParams::mGridGPUParams member of an instance of the DisplayParams structure stored locally in your plug-in so we don't recompute this data each frame.
	\param inGridParams The grid construction parameters.
	\param outGridGPUParams The Grid GPU parameters ready to be used by 3ds Max's viewport functions.
	\return true if successful or false otherwise.
	*/
	MaxGraphicsObjectAPI bool ComputeGridGPUParams(const GridParams& inGridParams, GridGPUParams& outGridGPUParams);

	/** This function is used to let the volume API do everything for you to display the volume correctly from your plug-in method IObjectDisplay2::UpdatePerNodeItems.
	*  It will fill the targetRenderItemContainer with the suitable RenderItem using the display modes from the viewport (wireframe, shaded, clay etc.) and using the options you have set in 
	*  the volumeDisplayParams parameter.
	\param updateDisplayContext The display context, which contains all the requirements for plugin to generate render items.
	\param nodeContext which contains all information required for adding per node relevant render items.
	\param targetRenderItemContainer the target render item container to which this	plugin object will be added.
	\param volumeDisplayParams The volume display parameters.
	\return true if successful or false otherwise.
	*/
	MaxGraphicsObjectAPI bool UpdatePerNodeItems(
                        const MaxSDK::Graphics::UpdateDisplayContext& updateDisplayContext,
                        MaxSDK::Graphics::UpdateNodeContext& nodeContext,
                        MaxSDK::Graphics::IRenderItemContainer& targetRenderItemContainer, 
						const DisplayParams& volumeDisplayParams);

	/** This function is used to let the volume API do the hit testing of the volume when the user wants to select a volume in a viewport.
	The parameters are matching the parameters from BaseObject::HitTest, except the volumeDisplayParams which is the Volume Display Parameters that we use to do the hit testing.
	\param t The time to perform the hit test.
	\param inode A pointer to the node to test.
	\param type The type of hit testing to perform. See \ref SceneAndNodeHitTestTypes for details.
	\param crossing The state of the crossing setting. If TRUE crossing selection is on.
	\param flags The hit test flags. See \ref SceneAndNodeHitTestFlags for details.
	\param p The screen point to test.
	\param vpt An interface pointer that may be used to call methods associated with the viewports.
	\param volumeDisplayParams The volume display parameters.
	\return  Nonzero if the item was hit; otherwise 0. 
	*/
	MaxGraphicsObjectAPI int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, const DisplayParams& volumeDisplayParams);

};//end of namespace ViewportVolumeDisplay

}};//end of namespace MaxSDK{namespace Graphics{
