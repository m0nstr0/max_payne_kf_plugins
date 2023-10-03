 /**********************************************************************
 
	FILE:           INodeBakeProperties.h

	DESCRIPTION:    Public interface for setting and getting a node's
                    texture baking properties.

	CREATED BY:		Kells Elmquist

	HISTORY:		created 15 december 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#pragma once

#include "ifnpub.h"
#include "geom/ipoint2.h"
#include "geom/box2.h"
// forward declarations
class MaxBakeElement;

// The interface ID for class INodeBakeProperties
#define NODE_BAKE_PROPERTIES_INTERFACE Interface_ID(0x3e5d0f38, 0x37e500ac)

// by default use this channel for all baking ops
#define DEFAULT_BAKE_CHANNEL	3
#define DEFAULT_N_DILATIONS		2

//==============================================================================
// class INodeBakeProperties
/*! \par Description:
	This class defines an interface for accessing a node's texture
	baking properties.
	An instance of this interface can be retrieved using the following line of
	code (assuming 'node' is of type INode*):
	\code
		static_cast<INodeBakeProperties*>(pNode->GetInterface(NODE_BAKE_PROPERTIES_INTERFACE))
	\endcode
	Description of the node's texture baking properties:

	\par General Properties:
	Enable:
	-	Texture baking is enabled for this object
	.
	Baking Channel:
	-	Flattening & baking use this uv mapping channel for this object
	.
	List of Baking Render Elements:
	-	each object has a list of render elements for output
*/
//==============================================================================
class INodeBakeProperties : public FPMixinInterface {

public:

	// General properties

	/** Get the flag to identify if the baking is enabled.
	\remark The baker can query the flag to perform the baking or not.
	\returns The flag to identify if the baking is enabled.
	*/
	virtual BOOL GetBakeEnabled() const = 0;

	/** Set the flag to identify if the baking is enabled.
	\remark The client can set the flag to enable or disable the baking.
	\param isEnabled The flag to identify if the baking is enabled.
	*/
	virtual void SetBakeEnabled( BOOL isEnabled ) = 0;

	/** Get the mapping channel to use for baking Object level.
	\remark The baker can query the mapping channel, then apply it for baking.
	\returns The mapping channel for baking.
	*/
	virtual int  GetBakeMapChannel() const = 0;

	/** Set the mapping channel for baking.
	\remark The client can set the mapping channel to bake.
	\param mapChannel The mapping channel for baking.
	*/
	virtual void SetBakeMapChannel( int mapChannel ) = 0;

	/** Get the number of dilations after rendering, affects seaming.
	\remark The baker can query the number of dilations, then apply it for baking.
	\returns The number of dilations for baking.
	*/
	virtual int  GetNDilations() const = 0;

	/** Set the number of dilations for baking.
	\remark The client can set the number of dilations to bake.
	\param nDilations The number of dilations for baking.
	*/
	virtual void SetNDilations( int nDilations ) = 0;

	//! \deprecated This has been deprecated as of 3ds Max 2022, please do not use it.
	/** Get the bake flags for baking.
	\remark The baker can query the bake flags, then apply it for baking.
	\returns The bake flags for baking.
	*/
	MAX_DEPRECATED virtual int GetBakeFlags() const final { return 0; }

	//! \deprecated This has been deprecated as of 3ds Max 2022, please do not use it.
	/** Set the bake flags for baking.
	\remark The client can set the bake flags to bake.
	\param flags The bake flags for baking.
	*/
	MAX_DEPRECATED virtual void SetBakeFlags( int flags ) final { UNREFERENCED_PARAMETER( flags ); }

	/** Get the surface area for baking.
	\remark The baker can query the surface area, then apply it for baking.
	\returns The surface area for baking.
	*/
	virtual float GetSurfaceArea() const = 0;

	/** Set the surface area for baking.
	\remark The client can set the surface area to bake.
	\param area The surface area for baking.
	*/
	virtual void  SetSurfaceArea( float area ) = 0;

	/** Get the area scale for baking.
	\remark The baker can query the area scale, then apply it for baking.
	\returns The area scale for baking.
	*/
	virtual float GetAreaScale() const = 0;

	/** Set the area scale for baking.
	\remark The client can set the area scale to bake.
	\param scale The area scale for baking.
	*/
	virtual void  SetAreaScale( float scale ) = 0;

	/** Get total number of the bake elements.
	\returns The total number of the bake elements.
	*/
	virtual int GetNBakeElements() const = 0;

	/** Get the bake element by the index.
	\param nElement The index of the bake element.
	\returns The pointer to the bake element.
	*/
	virtual MaxBakeElement* GetBakeElement( int nElement ) = 0;

	//! \brief returns a Tab object holding the list of Texture Bake elements assigned to the node. 
	/*! Developers should not try to set BakeElements by altering the contents of this Tab;
		use the methods AddBakeElement() and RemoveBakeElement() instead 
		\returns a Tab object holding the list of Texture Bake elements assigned to the node. */
	virtual Tab<MaxBakeElement*> GetBakeElementArray() = 0;

	/** Add bake element by the pointer.
	\param pEle The pointer to the bake element.
	\returns The flag to identify if the operation is successful.
	*/
	virtual BOOL AddBakeElement( MaxBakeElement* pEle ) = 0;

	/** Remove bake element by the pointer.
	\param pEle The pointer to the bake element.
	\returns The flag to identify if the operation is successful.
	*/
	virtual BOOL RemoveBakeElement( MaxBakeElement* pEle ) = 0;

	/** Remove bake element by name.
	\param name The name of the bake element.
	\returns The flag to identify if the operation is successful.
	*/
	virtual BOOL RemoveBakeElementByName( const MCHAR * name ) = 0;

	/** Remove bake element by index.
	\param index The index of the bake element.
	\returns The flag to identify if the operation is successful.
	*/
	virtual BOOL RemoveBakeElementByIndex( int index ) = 0;

	/** Remove all bake elements hold by this entity.
	*/
	virtual void RemoveAllBakeElements() = 0;

	/** Reset params to default, toss render elements.
	*/
	virtual void ResetBakeProps() = 0;

	/** Get the flag to identify if there's anything effective to bake.
	\remark The result depends on if the node is bake enabled & has some elements & sz not 0.
	\returns The flag to identify if there's anything effective to bake.
	*/
	virtual BOOL GetEffectiveEnable() = 0;

	/** Get the render size for baking.
	\remark It returns the largest size of enabled baking elements.
	\remark The baker can query the render size, then render to target with that size.
	\returns The the render size for baking.
	*/
	virtual IPoint2 GetRenderSize() = 0;

	/** Get the active region for baking.
	\remark The region is a rectangle area.
	\remark The baker can query the active region, then bake everything in that region.
	\returns The active region for baking.
	*/
	virtual FBox2 GetActiveRegion() =0;

	/** Set the active region for baking.
	\remark The region is a rectangle area.
	\remark The client can set the active region to bake everything in that region.
	\param region The active region for baking.
	*/
	virtual void SetActiveRegion(FBox2 region) = 0;

	/** Get the active uv tile for baking.
	\remark In UDIM baking workflow, the tile to bake could be IPoint2(uOffset,vOffset), uOffset/vOffset means the offset in u/v space.
	\remark The default value is IPoint2(0,0), which represents the main/default tile or tile u0v0/1001.
	\remark The baker can query the active uv tile, then bake everything in that uv tile.
	\returns The active uv tile for baking.
	*/
	virtual IPoint2 GetActiveTile() const = 0;

	/** Set the active uv tile for baking.
	\remark In UDIM baking workflow, the tile to bake could be IPoint2(uOffset,vOffset), uOffset/vOffset means the offset in u/v space.
	\remark If no tile is specified, the default value used by baker is IPoint2(0,0), which represents the main/default tile or tile u0v0/1001.
	\remark The client can set the active uv tile to bake the specific tile.
	\param tile The active uv tile for baking.
	*/
	virtual void SetActiveTile( const IPoint2& tile ) = 0;
};


//==============================================================================
// class INodeBakeProjProperties
//
// Properties related to projection mapping with texture baking
//
//==============================================================================

#define NODE_BAKE_PROJ_PROPERTIES_INTERFACE Interface_ID(0x59d919a5, 0x6fb90a85)

class INodeBakeProjProperties : public FPMixinInterface {
public:
	//Enabled
	virtual BOOL	GetEnabled() = 0;
	virtual void	SetEnabled( BOOL b ) = 0;

	//ProjMod - Projection Modifier
	virtual ReferenceTarget* GetProjMod() = 0;
	virtual void	SetProjMod( ReferenceTarget* refTarg ) = 0;

	//ProjModTarg - Projection Modifier Target
	virtual const MCHAR*	GetProjModTarg() = 0;
	virtual void	SetProjModTarg( const MCHAR* s ) = 0;

	//CropAlpha
	virtual BOOL	GetCropAlpha() = 0;
	virtual void	SetCropAlpha( BOOL b ) = 0;

	//ProjSpace: XYZ object space, or UVW texture space
	enum { enumIdProjSpaceXYZ=0, enumIdProjSpaceUVW };
	virtual int		GetProjSpace() = 0;
	virtual void	SetProjSpace( int enum_val ) = 0;

	//NormalSpace: World Space, Screen Space, Local Space or Tangent Space
	enum { enumIdNormalSpaceWorld=0, enumIdNormalSpaceScreen, enumIdNormalSpaceLocal, enumIdNormalSpaceTangent };
	virtual int		GetNormalSpace() = 0;
	virtual void	SetNormalSpace( int enum_val ) = 0;

	//TangentYDir: Y-Up or Y-Down
	enum { enumIdTangentDirYUp=0, enumIdTangentDirYDown};
	virtual int		GetTangentYDir() = 0;
	virtual void	SetTangentYDir( int enum_val ) = 0;

	//TangentXDir: X-Right or X_Left
	enum {enumIdTangentDirXRight=0, enumIdTangentDirXLeft };
	virtual int		GetTangentXDir() = 0;
	virtual void	SetTangentXDir( int enum_val ) = 0;

	//UseCage
	virtual BOOL	GetUseCage() = 0;
	virtual void	SetUseCage( BOOL b ) = 0;

	//RayOffset
	virtual float	GetRayOffset() = 0;
	virtual void	SetRayOffset( float f ) = 0;

	//HitResolveMode
	enum { enumIdHitResolveClosest=0, enumIdHitResolveFurthest };	
	virtual int		GetHitResolveMode() = 0;
	virtual void	SetHitResolveMode( int enum_val ) = 0;

	//HitMatchMtlID
	virtual BOOL	GetHitMatchMtlID() = 0;
	virtual void	SetHitMatchMtlID( BOOL b ) = 0;

	//HitWorkingModel
	virtual BOOL	GetHitWorkingModel() = 0;
	virtual void	SetHitWorkingModel( BOOL b ) = 0;

	//WarnRayMiss
	virtual BOOL	GetWarnRayMiss() = 0;
	virtual void	SetWarnRayMiss( BOOL b ) = 0;

	//RayMissColor
	virtual Color*	GetRayMissColor() = 0;
	virtual void	SetRayMissColor( Color* c ) = 0;

	//HeightMapMin - rays of less than this length are output as black pixels by the Height Map
	virtual float	GetHeightMapMin() = 0;
	virtual void	SetHeightMapMin( float f ) = 0;

	//HeightMapMax - rays of more than this length are output as white pixels by the Height Map
	virtual float	GetHeightMapMax() = 0;
	virtual void	SetHeightMapMax( float f ) = 0;

	//HeightBufMin - the minimum height value encountered during the last rendering
	virtual float	GetHeightBufMin() = 0;
	virtual void	SetHeightBufMin( float f ) = 0;

	//HeightBufMax - the maximum height value encountered during the last rendering
	virtual float	GetHeightBufMax() = 0;
	virtual void	SetHeightBufMax( float f ) = 0;

	//ProportionalOutput
	virtual BOOL	GetProportionalOutput() = 0;
	virtual void	SetProportionalOutput( BOOL b ) = 0;

	// mapping channel to use for baking SubObject level
	virtual int  	GetBakeMapChannel_SO() const = 0;
	virtual void 	SetBakeMapChannel_SO( int mapChannel ) = 0;

	//BakeObjectLevel
	virtual BOOL	GetBakeObjLevel() = 0;
	virtual void	SetBakeObjLevel( BOOL b ) = 0;

	//BakeSubObjLevels
	virtual BOOL	GetBakeSubObjLevels() = 0;
	virtual void	SetBakeSubObjLevels( BOOL b ) = 0;

	//useObjectBakeForMtl
	virtual BOOL	GetObjBakeToMtl() = 0;
	virtual void	SetObjBakeToMtl( BOOL b ) = 0;

};

