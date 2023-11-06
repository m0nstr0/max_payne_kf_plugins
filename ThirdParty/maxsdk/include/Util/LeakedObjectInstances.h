//////////////////////////////////////////////////////////////////////////////
//
//  Copyright 2017 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../dbgprint.h" // for DebugPrint
#include <typeinfo> // for typeid
#include <string> // for std::string
#include <atomic> // for std::atomic
#include <unordered_map> // for std::unordered_map
#include "..\strclass.h"

/*! \brief
This class is derived from in order to automatically track existing instances, and at 
shutdown write to Debug Output the creation count for leaked instances. The template class Obj 
should typically be set to the class deriving from this class. For example:
class ParamBlockDesc2 : private LeakedObjectInstances<ParamBlockDesc2>
On shutdown, if leakage is present the output to Debug Output would be:
2022/08/26 14:28:00 [12616] : 55114ms : **Leaked class ParamBlockDesc2 instances: 587, 588
*/
template <class Obj>
class LeakedObjectInstances
{
public:
	LeakedObjectInstances()
	{
		InstanceCreated(this);
	}
	LeakedObjectInstances(const LeakedObjectInstances&)
	{
		InstanceCreated(this);
	}
	~LeakedObjectInstances()
	{
		InstanceDeleted(this);
	}

private:

	//! \brief Prevent Assignments. 
	LeakedObjectInstances& operator=(const LeakedObjectInstances&) = delete;

	class ObjectInstanceContainer
	{
	public:
		ObjectInstanceContainer(const char* className) : mClassName(className) {}
		~ObjectInstanceContainer()
		{
			DebugPrint(_T("**Leakage count** %hs: %Iu\n"), mClassName.c_str(), mInstancesAlive.size());
			if (mInstancesAlive.size() != 0)
			{
				MSTR msg;
				MCHAR buffer[32]; // needs to be at least 20
				bool first = true;
				for (auto& elt : mInstancesAlive)
				{
					if (first)
						msg.printf(_T("**Leaked %hs instances: "), mClassName.c_str());
					else
						msg.append(_T(", "));
					first = false;
					_ui64tot(elt.second, buffer, 10);
					msg.append(buffer);
				}
				msg.append(_T("\n"));
				DebugOutputString(msg);
			}
		}
		void InstanceCreated(void* ptr)
		{
			uint64_t n = ++mInstanceCount;
			mInstancesAlive.insert({ ptr, n });
		}
		void InstanceDeleted(void* ptr)
		{
			mInstancesAlive.erase(ptr);
		}

		std::unordered_map<void*, uint64_t>& GetMap()
		{
			return mInstancesAlive;
		}

	private:
		std::atomic<uint64_t> mInstanceCount{ 0 };
		std::string mClassName;
		std::unordered_map<void*, uint64_t> mInstancesAlive;
	};

	ObjectInstanceContainer& GetContainer()
	{
		static ObjectInstanceContainer mObjectInstanceContainer(typeid(Obj).name());
		return mObjectInstanceContainer;
	}

	void InstanceCreated(void* ptr)
	{
		GetContainer().InstanceCreated(ptr);
	}
	void InstanceDeleted(void* ptr)
	{
		GetContainer().InstanceDeleted(ptr);
	}
};
