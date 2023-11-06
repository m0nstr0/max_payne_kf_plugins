//
// Copyright 2020 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "IRenderGeometry.h"
#include "DrawContext.h"
#include "../Materials/Mtl.h"
#include "../geom/quat.h"
#include "UpdateDisplayContext.h"
#include "UpdateNodeContext.h"
#include "MaterialRequiredStreams.h"
#include "VertexBufferHandle.h"
#include "IndexBufferHandle.h"
#include "../Noncopyable.h"
#include "MaterialConversionHelper.h"
#include "../export.h"

namespace MaxSDK { namespace Graphics {

//Declaration for the private implementation of this class
class InstanceDisplayGeometryImpl;

/** This API is about having a single INode in the scene, but having many geometries being instantiated at the GPU level in the viewport 
 when the INode is drawn, e.g : a particles system. MaxSDK::Graphics::InstanceDisplayGeometry is a class dealing with instancing geometries 
 in the viewport (only in the viewport, not at render time).
 It is a subclass of MaxSDK::Graphics::IRenderGeometry, and in that class you provide a geometry that is used by Nitrous (the 3ds Max modern viewport) 
 to be displayed in the viewport. That geometry is provided at the GPU level by giving vertex buffers and material stream requirement.
 <b>3ds Max is implementing all the following functions, you don't need to override any of those to make the API work</b>.

 In MaxSDK::Graphics::InstanceDisplayGeometry, you provide the same information for the geometry that you would provide in MaxSDK::Graphics::IRenderGeometry, 
 but you also add some information about instancing that geometry in multiple ways in the viewport, still using the same single INode. 
 It's not related to the instances/references you have for an INode in the user interface.
 This API was developed for dealing with animated instances in mind, so performance is taken into consideration.
 
 This viewport instancing API has 2 modes : 
 1) The creation mode, to create the instances using InstanceDisplayGeometry::CreateInstanceData(const InstanceData& data) where you provide the initial data for instancing an object.
 2) The update mode, to update the instances using InstanceDisplayGeometry::UpdateInstanceData(const InstanceData& data). It may be used to animate the instances from one frame to another 
	  such as update their positions
	  The update mode should be used only if you have at least called InstanceDisplayGeometry::CreateInstanceData(const InstanceData& data) once before so the instance data is created.
	  This mode has a better performance than recreating the whole instance data with InstanceDisplayGeometry::CreateInstanceData(const InstanceData& data) since we are only updating the existing data, 
	  no vertex buffers are recreated. There are restrictions to using it, since you can not change the number of instances or provide new data that was not present initially 
	  nor change the number of vertices from the instantiated geometry.

Note :	When a viewport is in high quality (instead of the standard quality which is the default), 
		the geometry or its instances are only displayed correctly if the following data in the vertex buffers is provided 
		(this is the material stream requirement ):
	1) The vertices' positions
	2) The vertices' normals
	3) A UV map channel (can be any map channel)
	4) A UV map channel (can be any map channel also be the same as the map channel from 3) )
	5) A UV map channel (can be any map channel also be the same as the map channel from 3) )
	6) A UV map channel (can be any map channel also be the same as the map channel from 3) )
	7) The tangents for normal mapping
	8) The bitangents for normal mapping

	Even if you don't use normal mapping or vertex color, you must provide this data in high quality viewports.

	Link with optimesh.lib to use this API.
*/
namespace ViewportInstancing
{

///This function converts from a color r,g,b in bytes (range 0 to 255) to a combined DWORD used internally by 3ds Max for storing Vertex Colors.
inline DWORD ConvertFromR8G8B8ToDWORD(int r, int g, int b)
{
	DbgAssert((r>=0 && r<=255) && (g>=0 && g<=255) && (b>=0 && b<=255) );
	return ((((WORD)((BYTE)(b)) << 8)) | (((DWORD)(BYTE)(g)) << 16) | (((DWORD)(BYTE)(r)) << 24));
}

/** The struct InstanceData is used to pass instance data to different methods of the InstanceDisplayGeometry class.
* It is shared by the creation and the update mode.
It is used in the creation mode with InstanceDisplayGeometry::CreateInstanceData(const InstanceData& data) to create the instance vertex buffer. 
This has to be called once only unless you want to recreate all instance data when the number of instances has changed or the number of vertices has changed in the original instantiated geometry.
It is also used in the update mode with InstanceDisplayGeometry::UpdateInstanceData(const InstanceData& data) to update the instance data that has changed.
*/
struct InstanceData
{
	/**numInstances : this is the number of instances.
	In Creation mode : this parameter should be used with InstanceDisplayGeometry::CreateInstanceData(const InstanceData& data), it should be non zero.
	In update mode : with InstanceDisplayGeometry::UpdateInstanceData(const InstanceData& data), this parameter can be ignored as the number of instances should not have changed.
	*/
	size_t				numInstances					= 0; 

	/**If bTransformationsAreInWorldSpace is true, then all matrices/positions/orientation/scales used in InstanceData are in world space, so moving the node will not move the instances.
	If bTransformationsAreInWorldSpace is false then all matrices/positions/orientation/scales are relative to the node's transfrom matrix, so moving the node will move all instances.
	In creation mode : this parameter should be set.
	In update mode : this parameter should not be used as this should not change in an update.
	*/
	bool				bTransformationsAreInWorldSpace = false;

	/**pMatrices is an array of Matrix3 with InstanceData::numInstances elements which are the transform matrix of each instance. 
	This parameter can be null if you use the pPositions array instead but either pMatrices or pPositions should be non null if you want the instances to be at different positions.
	To set these matrices to world space or relative to the node's transform, please see InstanceData::bTransformationsAreInWorldSpace.
	In creation mode : this parameter can be used.
	In update mode : this parameter can only be used if it was used in the creation mode previously.
	*/
	Matrix3*			pMatrices						= nullptr;

	/**pPositions is an array of Point3 with InstanceData::numInstances elements which are the position of each instance. It can be a null pointer if you use the pMatrices array instead, 
	but at least one of both pointers should be non null.
	To set these matrices to world space or relative to the node's transform, please see InstanceData::bTransformationsAreInWorldSpace.
	Internally, in the instance vertex buffer, this data is converted to a Matrix3 with pos/orientation/scale combined when added in the instance vertex buffer.
	In update mode, with InstanceDisplayGeometry::UpdateInstanceData(const InstanceData& data), if you are not using InstanceData::pMatrices, but rather position/orientation/scale, 
	always provide all three, as a missing value will be replaced by identity or null (for position) as we are combining the 3 into a matrix, even if only one of them 
	is changing for a given frame.
	In creation mode : this parameter can be used.
	In update mode : this parameter can only be used if it was used in the creation mode previously. And please also provide orientation and scale in this mode 
	even if not all of them have changes.
	*/
	Point3*				pPositions						= nullptr;

	/**pOrientationsAsPoint4 is an array of Point4 which are quaternions. This parameter can be null if the orientation of instances is not overriden, 
	it will be an identity orientation in that case. If you prefer to use the Quat class to provide orientations, please see InstanceData::pOrientationsAsQuat.
	To set these orientations to world space or relative to the node's transform, please see InstanceData::bTransformationsAreInWorldSpace.
	Internally in the vertex buffer, this data will be converted to a Matrix3 with pos/rot/scale combined when put in the instance vertex buffer.
	In update mode, with InstanceDisplayGeometry::UpdateInstanceData(const InstanceData& data), if you are not using InstanceData::pMatrices, but rather position/orientation/scale, 
	always provide all three, as a missing value will be replaced by identity or null (for position) as we are combining the 3 into a matrix, even if only one of them is 
	changing for a given frame.
	In creation mode : this parameter can be used.
	In update mode : this parameter can only be used if it was used in the creation mode previously. And please also provide position and scale in this mode 
	even if not all of them have changes.
	*/
	Point4*				pOrientationsAsPoint4			= nullptr;

	/**numOrientationsAsPoint4 is the number of elements in InstanceData::pOrientationsAsPoint4, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	* This means you can only give one orientation if you want all instances to have the same orientation.
	* In creation mode : this parameter can be used.
	In update mode : this parameter can only be used if it was used in the creation mode previously.
	*/
	size_t				numOrientationsAsPoint4			= 0;


	/**pOrientationsAsQuat is an array of Quat which are quaternions. This parameter can be null if the orientation of instances is not overriden, it will be an identity orientation in that case.
	If you prefer to use the Point4 class to provide orientations instead of Quat, please see InstanceData::pOrientationsAsPoint4.
	To set these orientations to world space or relative to the node's transform, please see InstanceData::bTransformationsAreInWorldSpace.
	Internally in the vertex buffer, this data will be converted to a Matrix3 with pos/rot/scale combined when put in the instance vertex buffer.
	In update mode, with InstanceDisplayGeometry::UpdateInstanceData(const InstanceData& data), if you are not using InstanceData::pMatrices, but rather position/orientation/scale, 
	always provide all three, as a missing value will be replaced by identity or null (for position) as we are combining the 3 into a matrix, even if only one of them is 
	changing for a given frame.
	In creation mode : this parameter can be used.
	In update mode : this parameter can only be used if it was used in the creation mode previously. And please also provide position and scale in this mode 
	even if not all of them have changes.
	*/
	Quat*				pOrientationsAsQuat				= nullptr;

	/**numOrientationsAsQuat is the number of elements in InstanceData::pOrientationsAsQuat, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	This means you can only give one orientation if you want all instances to have the same orientation.
	In creation mode : this parameter can be used.
	In update mode : this parameter can only be used if it was used in the creation mode previously.
	*/
	size_t				numOrientationsAsQuat			= 0;

	/**pScales is an array of Point3 which is the scale of each instance. This parameter can be null if the scale of instances is not overriden, it will be an identity scale (1,1,1) used in that case.
	To set these orientations to world space or relative to the node's transform, please see InstanceData::bTransformationsAreInWorldSpace.
	Internally in the vertex buffer, this data will be converted to a Matrix3 with pos/rot/scale combined when put in the instance vertex buffer.
	In update mode, with InstanceDisplayGeometry::UpdateInstanceData(const InstanceData& data), if you are not using InstanceData::pMatrices, but rather position/orientation/scale, 
	always provide all three, as a missing value will be replaced by identity or null (for position) as we are combining the 3 into a matrix, even if only one of them is 
	changing for a given frame.
	In creation mode : this parameter can be used.
	In update mode : this parameter can only be used if it was used in the creation mode previously. And please also provide position and orientation in this mode 
	even if not all of them have changes.
	*/
	Point3*				pScales							= nullptr;

	/**numScales is the number of elements in pScales, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	* In creation mode : this parameter can be used.
	In update mode : this parameter can only be used if it was used in the creation mode previously.
	*/
	size_t				numScales						= 0;

	/**pViewportMaterials is an array of BaseMaterialHandle which is the material of each instance. This parameter can be null if you want to use the original material from the node.
	If you need to convert from a 3ds Max Mtl class to a Nitrous BaseMaterialHandle, please see MaxSDK::Graphics::MaterialConversionHelper::ConvertMaxToNitrousMaterial.
	In creation mode : this parameter can be used.
	In update mode : this parameter should not be used. It is not compatible with the update mode since internally we sort the instances per material to batch render them for 
	performance reasons which involves a reordering of the instance vertex buffer data.
	Internally, we will do a local copy of this array.
	*/
	BaseMaterialHandle*	pViewportMaterials				= nullptr;

	/**numViewportMaterials is the number of elements in pViewportMaterials, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	In creation mode : this parameter can be used.
	In update mode : this parameter should not be used. Changing the number of materials is not possible in this mode. See InstanceData::pViewportMaterials for more info.
	*/
	size_t				numViewportMaterials			= 0;

	/**pUVWMapChannel1 is an array of Point3 which is the UVW value of each instance in map channel 1, we override the UVWs from the whole geometry with a single UVW value per 
	instance (for all vertices of the geometry) on that map channel. That is how override UVWs per instance works on GPU.
	This parameter can be null if the UVWs in map channel 1 of each instance are not overriden.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	Point3*				pUVWMapChannel1					= nullptr;

	/**numUVWMapChannel1 is the number of elements in pUVWMapChannel1, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	size_t				numUVWMapChannel1				= 0;

	/**pUVWMapChannel2 is an array of Point3 which is the UVW value of each instance in map channel 2, we override the UVWs from the whole geometry with a single UVW value per 
	instance (for all vertices of the geometry) on that map channel. That is how override UVWs per instance works on GPU.
	This parameter can be null if the UVWs in the map channel 2 of each instance are not overriden.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	Point3*				pUVWMapChannel2					= nullptr;

	/**numUVWMapChannel2 is the number of elements in pUVWMapChannel2, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	size_t				numUVWMapChannel2				= 0;

	/**pUVWMapChannel3 is an array of Point3 which is the UVW value of each instance in map channel 3, we override the UVWs from the whole geometry with a single UVW value per 
	instance (for all vertices of the geometry) on that map channel. That is how override UVWs per instance works on GPU.
	This parameter can be null if the UVWs in the map channel 3 of each instance are not overriden.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/	
	Point3*				pUVWMapChannel3					= nullptr;

	/**numUVWMapChannel3 is the number of elements in pUVWMapChannel3, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	size_t				numUVWMapChannel3				= 0;

	/**pUVWMapChannel4 is an array of Point3 which is the UVW value of each instance in map channel 4, we override the UVWs from the whole geometry with a single UVW value per 
	instance (for all vertices of the geometry) on that map channel. That is how override UVWs per instance works on GPU.
	This parameter can be null if the UVWs in the map channel 4 of each instance are not overriden.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	Point3*				pUVWMapChannel4					= nullptr;

	/**numUVWMapChannel4 is the number of elements in pUVWMapChannel4, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	size_t				numUVWMapChannel4				= 0;

	/**pUVWMapChannel5 is an array of Point3 which is the UVW value of each instance in map channel 5, we override the UVWs from the whole geometry with a single UVW value per 
	instance (for all vertices of the geometry) on that map channel. That is how override UVWs per instance works on GPU.
	This parameter can be null if the UVWs in the map channel 5 of each instance are not overriden.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	Point3*				pUVWMapChannel5					= nullptr;

	/**numUVWMapChannel5 is the number of elements in pUVWMapChannel5, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	size_t				numUVWMapChannel5				= 0;

	/**pUVWMapChannel6 is an array of Point3 which is the UVW value of each instance in map channel 6, we override the UVWs from the whole geometry with a single UVW value per 
	instance (for all vertices of the geometry) on that map channel. That is how override UVWs per instance works on GPU.
	This parameter can be null if the UVWs in the map channel 6 of each instance are not overriden.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	Point3*				pUVWMapChannel6					= nullptr;

	/**numUVWMapChannel6 is the number of elements in pUVWMapChannel6, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	size_t				numUVWMapChannel6				= 0;

	/**pUVWMapChannel7 is an array of Point3 which is the UVW value of each instance in map channel 7, we override the UVWs from the whole geometry with a single UVW value per 
	instance (for all vertices of the geometry) on that map channel. That is how override UVWs per instance works on GPU.
	This parameter can be null if the UVWs in the map channel 7 of each instance are not overriden.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	Point3*				pUVWMapChannel7					= nullptr;

	/**numUVWMapChannel7 is the number of elements in pUVWMapChannel7, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	size_t				numUVWMapChannel7				= 0;

	/**pUVWMapChannel8 is an array of Point3 which is the UVW value of each instance in map channel 8, we override the UVWs from the whole geometry with a single UVW value per 
	instance (for all vertices of the geometry) on that map channel. That is how override UVWs per instance works on GPU.
	This parameter can be null if the UVWs in the map channel 8 of each instance are not overriden.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	Point3*				pUVWMapChannel8					= nullptr;

	/**numUVWMapChannel8 is the number of elements in pUVWMapChannel8, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	size_t				numUVWMapChannel8				= 0;

	/**pColors is an array of AColor which is the rgba color of each instance. This will result in the viewport as a flat shading color applied on each instance (no lighting effect added).
	AColor has r,g,b,a components with values from 0.0 to 1.0, though we don't do any check on this.
	You can set a non zero alpha value to use transparency.
	This parameter can be null if you don't want to override the color per instance.
	Using a non-null InstanceData::pColors value with a non-null InstanceData::pViewportMaterials is not advised as we will use only either the material or 
	the colors but not both.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	Internally, we will do a local copy of this array.
	*/
	AColor*				pColors							= nullptr;

	/**numColors is the number of elements in pColors, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	size_t				numColors						= 0;

	/**pVertexColorsAsColor is an array of Color, i.e : an rgb color with no alpha. With r,g,b with values from 0.0 to 1.0. There is no alpha on vertex colors in 3ds Max.
	We override the Vertex Colors from the whole geometry with a single Vertex Color value per instance, that is how it works on at the GPU level.
	There is a limitation : Vertex Colors override will work only on map channels which have not been overriden when you show both in the viewport.
	This parameter can be null if the Vertex Colors of each instance are not overriden.
	This data will be converted when written in the vertex buffer to DWORD R8G8B8X8 color as this is what 3ds Max uses internally for Vertex Colors in vertex buffers. 
	If you prefer to use directly DWORD for Vertex Colors as it's faster, please see InstanceData::pVertexColorsAsDWORD.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	Color*				pVertexColorsAsColor					= nullptr;

	/**numVertexColorsAsColor is the number of elements in pVertexColorsAsColor, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	size_t				numVertexColorsAsColor					= 0;

	/**pVertexColorsAsDWORD is an array of 32 bits DWORD R8G8B8X8, i.e : an rgb color with no alpha. With r,g,b with values from 0.0 to 1.0. There is no alpha on vertex colors in 3ds Max.
	We override the Vertex Colors from the whole geometry with a single Vertex Color value per instance, that is how it works at the GPU level.
	At this time, there is a limitation : Vertex Colors override will work only on map channels which have not been overriden when you show both in the viewport.
	This parameter can be null if the Vertex Colors of each instance are not overriden.
	If you prefer to use Color for Vertex Colors, please see pVertexColorsAsColor. This will be slower as DWORD is the format that 3ds Max uses natively for the Vertex Color 
	in vertex buffers so we will convert from Color to DWORD.
	As an helper, to convert from r,g,b being values from 0 to 255 to 32 bits DWORD R8G8B8X8, please use the funcion declared above ConvertFromR8G8B8ToDWORD(int r, int g, int b).
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	DWORD*				pVertexColorsAsDWORD			= nullptr;

	/**numVertexColorsAsDWORD is the number of elements in pVertexColorsAsDWORD, it can be less than numInstances, in that case, we will loop through the data when filling the vertex buffer.
	In creation mode : this parameter can be used.
	In update mode : this parameter can be used.
	*/
	size_t				numVertexColorsAsDWORD			= 0;
};

/** InstanceDisplayGeometry is an extension of IRenderGeometry dealing with instancing. 

	Link with the optimesh.lib library to use this in your plugin.

	You can replace any call using a IRenderGeometry by this class, so the geometry is filled the usual way (vertex buffers, material required stream etc.).
	IRenderGeometry and InstanceDisplayGeometry have identical methods related to the geometry (except SetStreamRequirement where we fixed the typo on SetSteamRequirement).

	And you can instantiate this geometry by using the methods below.
	You can override matrices, positions, orientations, scales, material, UVs map channel (any up to 8), Vertex Colors or a color per instance.
	The number of instances and the number of elements passed from an array to be applied on instances can be different.
	Say, you have 100 instances. You are setting 100 positions for instances and 50 colors for instances, it's possible.
	We will loop through the data that is lower than the number of instances, say if we have 50 colors, instance 51 
	will use color#0, ... Looping.
	
	For instances to be visible, at least an array of matrices or positions must be provided to differentiate the instances positions visually.
	
	About materials, there are different situation where this class can be used :
	- You want to keep the original node's material. In that case don't set the members InstanceData::numColors or InstanceData::pColors and don't set InstanceData::numViewportMaterials 
		or InstanceData::pViewportMaterials and we will use by default the node's material on all instances.
	- You want the instances to use a flat color shading : use the members InstanceData::numColors and InstanceData::pColors and it will display a flat color per instance ignoring the original 
		node's material. Transparency can be applied on this flat colors shading.
	- You want to use a (possibly) different material per instance ignoring the node's original material, then use the members InstanceData::numViewportMaterials 
		and InstanceData::pViewportMaterials.

	Performance considerations :
	- Using InstanceData::pMatrices directly is faster than using InstanceData::pPositions / InstanceData::pOrientationsAsPoint4 / InstanceData::pOrientationsAsQuat / InstanceData::pScales 
		as we will convert them into a Matrix3 in the vertex buffer by combining the position/orientation and scale. Though the conversion is not very expensive.
	- Also please consider using the update mode wherever it can be used, since it's faster than the creation mode where the whole instance vertex buffers is created.
	
	About InstanceData members pointers ownership : 3ds Max never takes the ownership of the arrays provided and immediately builds the instance vertex buffer from that data.
	About local copy of the data, 3ds Max does local copies of the data only for the members InstanceData::pColors and InstanceData::pViewportMaterials.

	For a Sample code that uses these API, please see <b>/MAXSDK/HOWTO/OBJECTS/VIEWPORTINSTANCE/INSTANCEOBJECT.CPP</b>
	*/

class InstanceDisplayGeometry : public IRenderGeometry
{
public:
	DllExport InstanceDisplayGeometry();///Constructor
	DllExport virtual ~InstanceDisplayGeometry();///Destructor

	/** For the creation mode : creates the instances internal data on the GPU.
	* The plugin should call this function with an Instancedata object populated with instance positions, orientations, materials etc... It can be called at any place in your plugin.
	This does a full rebuild of the instance data, erasing any existing data. 
	You should call this again when the number of instances has changed or the original geometry instantiated has changed to rebuild the full internal data.
	\param[in] data : a const reference on InstanceData which is populated with values such as instance positions, orientations, materials etc...
	/return true if it succeeded or false if it failed. The main reason why it could fail is because the GPU memory allocated by the instances would be too big for the GPU. 
			You would see an assert in that case if asserts are enabled in your build.
	*/
	DllExport bool CreateInstanceData(const InstanceData& data);

	/** For the update mode : updates the instances internal data on the GPU.
	This function assumes that InstanceDisplayGeometry::CreateInstanceData(const InstanceData& data) has been already called once at least. As to update the data, it must have been created.
	Each time something changes on instances, say : the positions and colors have changed, you should call this method to update the instances internal data.
	In the data parameter of this method, only what needs to be updated should be non-null pointers and non zero for the number of elements.
	It can be called at any place in your plugin.
	Say you are updating only the positions, you should do :
	\code
	Point3* my_updated_Point3_positions_array = initialized_somewhere_else;
	InstanceData data;	
	data.pPositions   = my_updated_Point3_positions_array;
	data.numPositions = numPositions;//numPositions is the number of elements of my_updated_Point3_positions_array
	MyInstanceDisplayGeometry.UpdateInstanceData(data);//This will update only the positions and leave unchanged the others data set previously in a call to InstanceDisplayGeometry::CreateInstanceData(const InstanceData& data).
	\endcode
	\param[in] data : a const reference on InstanceData
	*/
	DllExport void UpdateInstanceData(const InstanceData& data);
	
	/** Get if the matrices / positions / orientations / scales on instances are in world space or relative to the node's transform matrix.
	\return true if they are in world space, false if they are relative to the node's transform.
	*/
	DllExport bool GetTransformationsAreInWorldSpace(void)const;

	/** Generates the GeometryRenderItemHandle and adds it to the targetRenderItemContainer.
	This is an helper function when you are in the method from IObjectDisplay2::UpdatePerNodeItems(const UpdateDisplayContext& updateDisplayContext, UpdateNodeContext& nodeContext, IRenderItemContainer& targetRenderItemContainer)
	it creates and adds the instance render items to the container.
	\param[in] bWireframe is true to generate the render items for a wireframe display, false means for a solid mesh display.
	\param[in] updateDisplayContext is an UpdateDisplayContext which has to be passed from UpdatePerNodeItems.
	\param[in] nodeContext is UpdateNodeContext which has to be passed from UpdatePerNodeItems.
	\param[in] targetRenderItemContainer is an IRenderItemContainer which has to be passed from UpdatePerNodeItems.
	*/
	DllExport bool GenerateInstances(bool bWireframe, const UpdateDisplayContext& updateDisplayContext, UpdateNodeContext& nodeContext, IRenderItemContainer& targetRenderItemContainer);
	
	/** Get the instance vertex buffer.
		\return the instance vertex buffer handle.
	*/
	DllExport VertexBufferHandle GetInstanceVertexBuffer(void) const;

	/** Get the MaterialRequiredStreams for the instance vertex buffer. MaterialRequiredStreams is the class that describe the GPU vertex buffer data and the order in which it is defined.
		\return the MaterialRequiredStreams for the instance vertex buffer.
	*/
	DllExport const MaterialRequiredStreams& GetInstanceStream(void) const;
	
	/** Get the geometry MaterialRequiredStreams for the geometry vertex buffer. 
	* MaterialRequiredStreams is the class that describes the GPU vertex buffer data and the order in which it is defined. 
	* This has to match the MaterialRequiredStreams instance from the material for the display to be correct in the viewport.
		\return the MaterialRequiredStreams for the geometry vertex buffer.
	*/
	DllExport const MaterialRequiredStreams& GetGeometryStream(void) const;
	
	/** Get the type of primitives in the geometry. This method is implemented by 3ds Max.
	/return the geometry's primitive type
	*/
	DllExport virtual PrimitiveType GetPrimitiveType() override;
		
	/** Sets type of primitives in the geometry. This method is implemented by 3ds Max.
	/param type : the geometry's primitive type
	*/
	DllExport virtual void SetPrimitiveType(PrimitiveType type) override;

	/** Number of primitives the original geometry has. This method is implemented by 3ds Max.
	/return geometry's primitive count
	*/
	DllExport virtual size_t GetPrimitiveCount() override;

	/** Set the number of primitives in the geometry. This method is implemented by 3ds Max.
		\param[in] count the number of primitives
	*/
	DllExport virtual void SetPrimitiveCount(size_t count);

	/** Number of vertices in the geometry. This method is implemented by 3ds Max.
	/return number of vertices in the geometry.
	*/
	DllExport virtual size_t GetVertexCount() override;
		
	/** Get start primitive of this geometry. This method is implemented by 3ds Max.
	\return The index of the start primitive.
	*/
	DllExport virtual int GetStartPrimitive() const override;

	/** Set the start primitive offset for drawing. This method is implemented by 3ds Max.
	\param[in] offset this offset will pass to Display() function
	*/
	DllExport virtual void SetStartPrimitive(int offset);
	
	/** Get the stream requirement of this geometry. This method is implemented by 3ds Max.
	To optimize performance, it's better to create a requirement-geometry mapping. 
	And make the render geometry read-only after created.
	\return the stream requirement which this geometry built with.
	*/
	DllExport virtual MaterialRequiredStreams& GetSteamRequirement()override;

	/** Set the stream requirement of this geometry. This method is implemented by 3ds Max.
		\param[in] streamFormat the stream requirement which this geometry built with.
	*/
	DllExport virtual void SetStreamRequirement(const MaterialRequiredStreams& streamFormat);

	/** Get the vertex streams of this geometry. This method is implemented by 3ds Max.
	\return vertex streams of this geometry
	*/
	DllExport virtual VertexBufferHandleArray& GetVertexBuffers()override;

	/** Get index buffer of this geometry. This method is implemented by 3ds Max.
	\return index buffer of this geometry. Might be invalid if the geometry doesn't need index buffer
	*/
	DllExport virtual IndexBufferHandle& GetIndexBuffer()override;

	/** Set index buffer of this geometry. This method is implemented by 3ds Max.
		\param[in] indexBuffer index buffer of this geometry. 
	*/
	DllExport virtual void SetIndexBuffer(const IndexBufferHandle& indexBuffer);

	/** Add a vertex buffer to this geometry. This method is implemented by 3ds Max.
		\param[in] vertexBuffer : the vertex buffer to be added. 
	*/
	DllExport virtual void AddVertexBuffer(const VertexBufferHandle& vertexBuffer);

	/** Remove the index-th geometry vertex buffer. This method is implemented by 3ds Max.
		\param[in] index the index of the geometry vertex buffer to be removed
	*/
	DllExport virtual void RemoveVertexBuffer(size_t index);

	/** Get the number of geometry vertex buffers. This method is implemented by 3ds Max.
		\return	the number of geometry vertex buffers.
	*/
	DllExport virtual size_t GetVertexBufferCount() const;

	/** Get the index-th vertex buffer from the geometry. This method is implemented by 3ds Max.
		\param[in] index the index of the geoemtry vertex buffer
		\return the index-th geometry vertex buffer.
	*/
	DllExport virtual VertexBufferHandle GetVertexBuffer(size_t index) const;

	/** This method is implemented by 3ds Max.
	* It is used to display the instances in the viewport
	/param drawContext the context for display
	/param start start primitive to render
	/param count primitive count to render
	/param lod current lod value from the adaptive degradation system
	*/
	DllExport virtual void Display(DrawContext& drawContext, int start, int count, int lod) override;

protected:
	friend class InstanceRenderItemHandle;
	/** Retrieve an InterfaceID, is reserved for internal usage only. This method is implemented by 3ds Max.
	*/
	DllExport virtual BaseInterface* GetInterface(Interface_ID id) override;
	
private:
	//Private implementation of this class.
	InstanceDisplayGeometryImpl* mpImpl = nullptr;
};

}}} // namespaces
