//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
#pragma once

#include <WTypes.h>
#include "maxheap.h"
#include "Animatable.h"

// Callback for EnumAnimTree:
//
// Scope values:

#define SCOPE_DOCLOSED 1   		//!< Enumerate "closed" animatables.
#define SCOPE_SUBANIM  2		//!< Enumerate sub-anims 
#define SCOPE_CHILDREN 4 		//!< Enumerate node children
#define SCOPE_OPEN	(SCOPE_SUBANIM|SCOPE_CHILDREN) //!< Enumerate all open animatables
#define SCOPE_ALL	(SCOPE_OPEN|SCOPE_DOCLOSED)     //!< do all animatables

// Return values for AnimEnum procs
#define ANIM_ENUM_PROCEED 1		//!< Do my children and keep going with the rest of the tree
#define ANIM_ENUM_STOP    2		//!< Don't do my children but keep going with the rest of the tree
#define ANIM_ENUM_ABORT   3		//!< Abort enumeration

// R5 and later only
#define ANIM_ENUM_SKIP    4   //!< Do not include this anim in the hierarchy.
#define ANIM_ENUM_SKIP_NODE 5 //!< Do not include this node and its subAnims, but include its children.

/** A callback class for Animatable::EnumAnimTree(). This class automatically 
	keeps track of the depth of the enumeration. 
	\sa class Animatable
*/
class AnimEnum: public MaxHeapOperators {
	protected:
		int depth;
		int scope;  
		DWORD tv;
	public:
		/*!  Constructor. Sets default scope and depth if specified.  */
	 	AnimEnum(int s = SCOPE_OPEN, int deep = 0, DWORD tv=0xffffffff) 
			{scope = s; depth = deep; this->tv = tv;}
		/*!  Destructor. */
		virtual ~AnimEnum() {;}
		/*!  Implemented by the System.\n\n
		Sets the scope. See below for possible values.
		\param s Specifies the scope to set. See below. */
		void SetScope(int s) { scope = s; }
		/*!  Implemented by the System.\n\n
		Returns the scope.
		\return  One or more of the following scope values:\n\n
		<b>SCOPE_DOCLOSED</b>\n\n
		Do closed animatables.\n\n
		<b>SCOPE_SUBANIM</b>\n\n
		Do the sub anims\n\n
		<b>SCOPE_CHILDREN</b>\n\n
		Do the node children\n\n
		<b>SCOPE_OPEN</b>\n\n
		Do all open animatables\n\n
		Equal to <b>(SCOPE_SUBANIM|SCOPE_CHILDREN)</b>\n\n
		<b>SCOPE_ALL</b>\n\n
		Do all animatables.\n\n
		Equal to <b>(SCOPE_OPEN|SCOPE_DOCLOSED)</b> */
		int Scope() { return scope; }
		/*!  Implemented by the System.\n\n
		Increments the depth count. */
		void IncDepth() { depth++; }
		/*!  Implemented by the System.\n\n
		Decrements the depth count. */
		void DecDepth() { depth--; }
		/*!  Implemented by the System.\n\n
		Returns the depth count. */
		int Depth() { return depth; }
		DWORD TVBits() {return tv;}
		//! \brief This is a method called by <b>EnumAnimTree()</b>.
		/*! If anim is an INode and client is not either an INode or the scene root pointer,
		<b>node_subAnim_proc</b> is called instead of this method.
		\param anim The sub anim.
		\param client The client anim. This is the parent with a sub-anim of <b>anim</b>.
		\param subNum The index of the sub-anim that <b>anim</b> is to <b>client</b>. For
		example, if you were to call <b>client-\>SubAnim(subNum)</b> it would
		return <b>anim</b>. The exception to this is the handling of the scene node 
		hierarchy. In this case anim would be an INode and client would be either
		its parent INode or the scene root pointer, and subNum would be the child
		index.
		\return  One of the following values:\n\n
		<b>ANIM_ENUM_PROCEED</b>\n\n
		Continue the enumeration process.\n\n
		<b>ANIM_ENUM_STOP</b>\n\n
		Stop the enumeration process at this level.\n\n
		<b>ANIM_ENUM_ABORT</b>\n\n
		Abort the enumeration processing. */
		virtual int proc(Animatable *anim, Animatable *client, int subNum)=0;

		//! \brief This is a method called by <b>EnumAnimTree()</b>.
		/*! Normally, if anim is an INode then client would be either
		its parent INode or the scene root pointer. However it is possible that an
		Animatable can return an INode as a sub-anim. In the event that client is 
		a sub-anim somewhere under a node, and anim is a parent node of that node, the
		potential for infinite recursion exists since continuing enumeration into
		that parent node will result in returning to this point. 
		Calling <b>proc</b> and then stopping enumeration at this level would result 
		in the method implementation flagging the node as visited. If the method 
		implementation returns ENUM_STOP if it is passed an anim that has been 
		previously visited, and the node is processed again as the node hierarchy is 
		enumerated, ENUM_STOP would be returned resulting in its sub-anims not being 
		visited.
		Due to this issue, if anim is an INode and client is not either an INode or the 
		scene root pointer, this method is called instead of <b>proc</b>
		The default implementation for this method is to just return ANIM_ENUM_STOP.
		An implementation of this method should account for the potential for 
		recursion. A sample scene to test implementations against can be created by
		running the following script.
		\code
			plugin helper testNodeSubAnim
			classID:#(5432156,5432110)
			extends:Point
			(
				parameters main
				(
					thenode type:#node subanim:true
				)
			)
			s = sphere pos:[50,50,0]
			testNode = testNodeSubAnim parent:s
			testNode.thenode = s
		\endcode
		\param anim The sub anim.
		\param client The client anim. This is the parent with a sub-anim of <b>anim</b>.
		\param subNum The index of the sub-anim that <b>anim</b> is to <b>client</b>. For
		example, if you were to call <b>client-\>SubAnim(subNum)</b> it would
		return <b>anim</b>. 
		\return  One of the following values:\n\n
		<b>ANIM_ENUM_PROCEED</b>\n\n
		Continue the enumeration process.\n\n
		<b>ANIM_ENUM_STOP</b>\n\n
		Stop the enumeration process at this level.\n\n
		<b>ANIM_ENUM_ABORT</b>\n\n
		Abort the enumeration processing. */
		virtual int node_subAnim_proc(Animatable* anim, Animatable* client, int subNum)
		{
			UNUSED_PARAM(anim);
			UNUSED_PARAM(client);
			UNUSED_PARAM(subNum);
			return ANIM_ENUM_STOP;
		}
	};

/** An animatable enumerator for clearing flags. */
class ClearAnimFlagEnumProc : public AnimEnum
{
	DWORD flag;
public:
	//! Constructor
	ClearAnimFlagEnumProc(DWORD f)
		: flag(f)
	{
		// empty
	}
	int proc(Animatable* anim, Animatable* client, int subNum)
	{
		UNUSED_PARAM(client);
		UNUSED_PARAM(subNum);
		anim->ClearAFlag(flag);
		return ANIM_ENUM_PROCEED;
	}
};