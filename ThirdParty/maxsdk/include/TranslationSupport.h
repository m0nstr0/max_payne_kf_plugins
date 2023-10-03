//
// Copyright 2020 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//
//

#pragma once

// Includes to make this file compile
#include "CoreExport.h"
#include "strbasic.h"
#include "tab.h"

namespace MaxSDK
{
	/*! MAXScript scripts are typically written in English, but when MAXScript accesses names from TrackView nodes only localized names are available. 
	By specifying localized and English string pairs using the RegisterTranslation method, MAXScript is able to retrieve the English name corresponding to a 
	localized name to see if it matches the property name or SubAnim name specified by a script.
	Code that operates on TrackView nodes via ITrackViewNode::AddNode or ITrackViewNode::AddController should register via RegisterTranslation the English 
	name corresponding to the localized name specified in these methods.
	TrackView nodes persist their names to the scene file, so in order to ensure that the translations are present when loading the scene file, the translations
	should be registered when the dll is loaded.

	For example, if have the following code: 

	\code
	node->AddNode(vp, GetString(IDS_RB_VIDEOPOST), VIDEO_POST_TVNODE_CLASS_ID);
	vp->AddController(cont, GetString(IDS_RB_FLOAT), cont->ClassID());
	\endcode

	The following would be specified:

	\code
	void RegisterTranslations()
	{
		MaxSDK::TranslationSupport::RegisterTranslation(MaxSDK::TranslationSupport::kTrackViewNodes, 
			GetString(IDS_RB_VIDEOPOST), _T("Video Post"));
		MaxSDK::TranslationSupport::RegisterTranslation(MaxSDK::TranslationSupport::kTrackViewNodes, 
			GetString(IDS_RB_FLOAT), _T("Float"));
		}
	\endcode

	And then RegisterTranslations() would be called from the DllMain function handling of DLL_PROCESS_ATTACH. For example:

	\code
	BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
	{
		switch( fdwReason )
		{
			case DLL_PROCESS_ATTACH:
			MaxSDK::Util::UseLanguagePackLocale();
			hInstance = hinstDLL;
			DisableThreadLibraryCalls(hInstance);
			RegisterTranslations();
			break;
		}
		return (TRUE);
	}
	\endcode

	*/
	namespace TranslationSupport
	{
		//! \brief The property name resolution context where localized \ English string translation lookup is occurring
		enum ResolutionContext
		{
			kTrackViewNodes,			//< Resolving Trackview Node \ Controller names
			kCount						//< the number of contexts
		};
		//! \brief Specifies translation between a localized and non-localized (English) string pair for when MAXScript is performing property resolution
		/*! \param[in] context - The property name resolution context where this translation will be used
		\param[in] localizedString - the localized string
		\param[in] nonLocalizedString - the non-localized (English) string
		\return - always returns true. A return value is specified so that this method can be used as a static variable initializer, registering translation once. 
		/note The strings are stored in lower case as name resolution in MAXScript is case insensitive when dealing with 3dsMax objects.
		/note If the localized and non-localized strings are case-insensitive equivalent, the strings are not registered.
		/note Unique localized \ non-localized string pairs will only be registered once.*/
		CoreExport bool RegisterTranslation(ResolutionContext context, const MCHAR* localizedString, const MCHAR* nonLocalizedString);

		//! \brief Retrieves localized translations for a non-localized (English) string.
		/*! \param[in] context - The property name resolution context 
		\param[in] nonLocalizedString - the non-localized (English) string
		\param[out] localizedStrings - an array of the localized strings
		\return - returns true if localized strings were found, false otherwise.
		/note This method would not normally be used by 3rd party developers. */
		CoreExport bool GetLocalizedStrings(ResolutionContext context, const MCHAR* nonLocalizedString, Tab<const MCHAR*>& localizedStrings);

		//! \brief Retrieves non-localized (English) translations for a localized string.
		/*! \param[in] context - The property name resolution context 
		\param[in] localizedString - the localized string
		\param[out] nonLocalizedStrings - an array of the non-localized (English) strings
		\return - returns true if non-localized strings were found, false otherwise.
		\return - returns true if localized strings were found, false otherwise.
		/note This method would not normally be used by 3rd party developers. */
		CoreExport bool GetNonLocalizedStrings(ResolutionContext context, const MCHAR* localizedString, Tab<const MCHAR*>& nonLocalizedStrings);
	}
}
