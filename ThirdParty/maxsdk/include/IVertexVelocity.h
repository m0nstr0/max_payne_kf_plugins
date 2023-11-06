//**************************************************************************/
// Copyright (c) 2017 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include "geom/point3.h"
#include "baseinterface.h"
#include "ipipelineclient.h"


#define IVERTEX_VELOCITY_INTERFACE_ID Interface_ID(0x73211fd, 0x3da07521)



	
	/** Use this class to easily retrieve the vertex velocity data from an object.  The data maybe stored in the map channels which makes it 
	    volatile and if the vertex count does not equal the velocity count it will return a nullptr. Use

		 \code
		IVertexVelocity *velocityInterface = (IVertexVelocity*) mesh->GetInterface(IVERTEX_VELOCITY_INTERFACE_ID);
		int numVelocities = 0;
		const Point3* velData = velocityInterface->Velocity(numVelocities);
		\endcode

		to access the interface.

	*/
	class IVertexVelocity : public IPipelineClient
		{
		public:


			///  \brief  If velocity data has been registered for the object this interface was acquired from, this method returns a pointer to the array 
			/// of the velocity in units per frame otherwise it return null pointer
			/// \param numVelocityVerts  returns the number of velocity verts which should equal the number of geometry vertices.		
			/// note since the velocities maybe in a map channel or some editable channel someone could edit these and change the number of vertices
			virtual const Point3* Velocity( int& numVelocityVerts) = 0;


			
		};	
