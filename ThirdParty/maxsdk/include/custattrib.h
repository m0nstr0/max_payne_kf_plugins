/**********************************************************************
 *<
	FILE:  CustAttrib.h

	DESCRIPTION:  Defines CustAttrib class

	CREATED BY: Nikolai Sander

	HISTORY: created 5/25/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxtypes.h"
#include "plugapi.h"
#include "ref.h"
#include "ifnpub.h"

class ICustAttribContainer;
class IGraphObjectManager;
class IMtlParams;
class ParamDlg;

/*! \sa  Class ICustAttribContainer,  Class ReferenceTarget,  Class ParamDlg, Class Animatable\n\n
\par Description:
This class represents the Custom Attributes.\n\n
A sample on how to use this class is located in
<b>/MAXSDK/SAMPLES/HOWTO/CUSTATTRIBUTIL</b>  */

class CustAttrib: public ReferenceTarget
{
public:
	virtual SClass_ID SuperClassID() { return CUST_ATTRIB_CLASS_ID; }
	CustAttrib() {}
	//! \note This method has been deprecated in terms of implementation as of 3ds Max 2022. Plugin developers should implement GetName(bool localized) const instead.
	//!       This method can no longer be overriden and calls to it are now forwarded to the function that replaced it with a "bool localized" value of true.
	//!       This is done so that plugin developers who do not localize their plugins don't have to update all the places where they call this method.
	//!       Plugin developers who do localize their plugins should analyze the places where they call this method to decide what value to pass it for the "bool localized" parameter.
	//! \sa CustAttrib::GetName(bool localized) const
#pragma warning(push)
#pragma warning(disable : 4481)
	MAX_DEPRECATED virtual const MCHAR* GetName() MAX_SEALED { return GetName(true); }
#pragma warning(pop)

	/*! \remarks A CustAttrib plugin can implement this method in order to provide the name
	that gets displayed in the TrackView and the name that is used internally.
	\par Parameters:
	<b>bool localized</b>\n\n
	If true, then the name returned should be localized in the language 3ds Max is currently using. Otherwise it should be the name in English.
	If a plugin does not provide localized string resources, it can disregard this parameter and always return the name in English.\n\n
	\par Default Implementation:
	<b>{ return "Custom Attribute";}</b> */
	virtual const MCHAR* GetName(bool localized)
	{
		UNUSED_PARAM(localized);
		return _M("Custom Attribute");
	}

	/*! \remarks This method gets called when the material or texture is to be displayed in
	the material editor parameters area. The plug-in should allocate a new
	instance of a class derived from ParamDlg to manage the user interface.
	\par Parameters:
	<b>HWND hwMtlEdit</b>\n\n
	The window handle of the materials editor.\n\n
	<b>IMtlParams *imp</b>\n\n
	The interface pointer for calling methods in 3ds Max.
	\return  A pointer to the created instance of a class derived from
	<b>ParamDlg</b>.
	\par Default Implementation:
	<b>{return NULL;}</b> */
	#pragma warning(push)
	#pragma warning(disable:4100)
	virtual ParamDlg *CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp){return NULL;}
	/*! \remarks This method will check if it possible to copy the current custom attributes
	to the specified custom attributes container.
	\par Parameters:
	<b>ICustAttribContainer *to</b>\n\n
	A pointer to the custom attributes container you wish to check for possible
	reception of the custom attributes..
	\return  TRUE if it is possible to copy, otherwise FALSE.
	\par Default Implementation:
	<b>{ return true; }</b> */
	
	virtual bool CheckCopyAttribTo(ICustAttribContainer *to) { return true; }
	virtual SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
	{
		return SvStdTraverseAnimGraph(gom, owner, id, flags); 
	}
	#pragma warning(pop)
};


/*! \brief Structure used with NOTIFY_CUSTOM_ATTRIBUTES_ADDED and NOTIFY_CUSTOM_ATTRIBUTES_REMOVED notifications
*/
struct NotifyCustomAttribute : public MaxHeapOperators 
{
	NotifyCustomAttribute(Animatable* owner = NULL, Animatable* custAttr = NULL): 
		m_owner(owner), m_customAttribute(custAttr) {};
		/*! \brief The owner of the custom attribute that was added or is about to be removed */
		Animatable*	m_owner; 
		/*! \brief The custom attribuet that was added or is about to be removed */
		Animatable*	m_customAttribute;
};

// published custattrib interface ID
#define IID_PUBLISHED_CUSTATTRIB Interface_ID(0x0619714b, 0xdd58c508)

//! \brief Any CustAttrib which wants to support being published should implement this interface.
/*! When users xref objects with published custom attribute, users can visit and edit those CustAttrib
	values and saved with the XRef main scene and later if the value of source scene was changed,
	the main scene will also be updated accordingly(merge the value from source scene and the value
	saved in the main scene)
*/
class IPublishedCustAttrib : public BaseInterface, public MaxSDK::Util::Noncopyable
{
public:
	//! \brief Return the name of the published custom attribute.
	//! \return the name of the published custom attribute
	virtual const MCHAR* GetName() const= 0;
	
	//! \brief Return the custom attribute this interface publishes.
	//! \return the CustAttrib the interface publishes
	virtual CustAttrib* GetPublishedCustAttrib()  const = 0;

	//! \brief Return how many published parameters there are in this custom attribute
	//! \return the count of published parameters.
	virtual int PublishedParameterCount()  const = 0;

	//! \brief Retrieve the name of the i-th published parameter.
	/*! \param index - index of the published parameter whose name we want to get.
	\return - the name of the i-th published parameter.
	*/
	virtual MSTR PublishedParameterName(int index) const = 0;

	//! \brief Retrieve the controller of the i-th published parameter.
	/*! \param index - index of the published parameter whose controller we want to get.
	\return - the controller of the i-th published parameter.
	*/
	virtual Control* GetPublishedParameterControl(int index) const = 0;

	//! \brief Set the controller of the i-th published parameter.
	/*! \param index - index of the published parameter whose controller we want to set.
		\param pControl - the controller to be set to the i-th parameter.
	\return - if the operation is succeeded or not
	*/
	virtual bool SetPublishedParameterControl(int index, Control* pControl) = 0;

	//! \brief Get the controller type of the i-th published parameter.
	/*! \param index - index of the published parameter whose controller type we want to get.
	\return - the controller type of the i-th parameter, return 0 if not applicable.
	*/
	virtual SClass_ID GetParameterControlType(int index) const = 0;
};

// Interface obtained from here must be released using ReleaseInterface.
inline IPublishedCustAttrib* GetPublishedCustAttribInterface(InterfaceServer& iServer)	
{
	return static_cast<IPublishedCustAttrib*>(iServer.GetInterface(IID_PUBLISHED_CUSTATTRIB));
}
