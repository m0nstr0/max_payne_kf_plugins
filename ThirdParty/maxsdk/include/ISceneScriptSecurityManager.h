//**************************************************************************/
// Copyright 2020 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise accompanies 
// this software in either electronic or hard copy form.   

#pragma once

#include "iFnPub.h"
#include "export.h"

class Animatable;

namespace MaxSDK
{
class ISecurityMessageManager;

#define ISCENESCRIPTSECURITYMANAGER_INTERFACE Interface_ID(0x29285d06, 0x3c8e5e7b)

	//! \brief Interface class for the Scene Script Security Manager.
	/*! Interface for the Scene Script Security Manager class. This class has only one instance. 
		This singleton allows access to the security settings related to safe scene script execution.
	*/
	class ISceneScriptSecurityManager : public FPStaticInterface
	{
	public:
		enum SettingType
		{
			Current,			//!< The settings of the current 3ds Max session
			Pending,			//!< The settings that will be applied in the next 3ds Max session if they are changed in the current session
			SettingTypeNum	//!< The number of setting types
		};

		//! The features that can be enabled/disabled when executing scene embedded scripts in safe scene script execution mode
		enum ScriptingFeature
		{
			Python,							//!< Python
			DotNet,							//!< Construction of .NET classes / class instances by MAXScript that were not pre-approved for unsafe mode
			MAXScriptSystemCommands,	//!< Potentially unsafe MAXScript system commands
		};

		//! Gets whether the safe scene settings are locked for this 3ds Max session.
		virtual bool AreSettingsLocked() const = 0;

		//! Gets whether safe scene script execution is enabled.
		virtual bool IsSafeSceneScriptExecutionEnabled(SettingType type = SettingType::Current) const = 0;

		//! Gets whether safe script asset execution is enabled.
		virtual bool IsSafeScriptAssetExecutionEnabled(SettingType type = SettingType::Current) const = 0;

		//! Gets whether execution is blocked for a particular scripting language/technology when safe scene script execution is enabled.
		virtual bool IsSceneScriptExecutionBlocked(ScriptingFeature feature, SettingType type = SettingType::Current) const = 0;

		//! Gets whether the Security Messages window will be automatically opened when a scene script is blocked.
		virtual bool IsShowSecurityMessagesWindowOnBlockedCommandsEnabled(SettingType type = SettingType::Current) const = 0;

		//! Gets whether a script editor will be automatically opened when a scene script is blocked.
		// When handling security exceptions, the value returned by this function should be checked before displaying any form of the editor
		// that could be used to remove unsafe commands from a scene script. If the value returned is false, then no editor should be displayed.
		virtual bool IsShowScriptEditorOnBlockedCommandsEnabled(SettingType type = SettingType::Current) const = 0;

		/*! Logs a message in the Security Messages window that notifies the user when a command from a scene script has been blocked.
		\param[in] nameOfBlockedCommand - The name of the command that was blocked.
		For example: "HiddenDOSCommand", "python.Execute", "System.Windows.Forms.ListView", etc.
		This information is displayed by the Security Messages window.
		\param[in] feature - The type of command that was blocked.
		For example, if the command that was blocked was a "ShellExecute" MAXScript command, then ScriptingFeature::MAXScriptSystemCommands should be passed.
		If it was a "python.ExecuteFile" MAXScript command, then ScriptingFeature::Python should be passed.
		If it was a "dotNetClass" MAXScript command, then ScriptingFeature::DotNet should be passed.
		This information is used to compose an appropriate message to be displayed by the Security Messages window.
		*/
		virtual void LogBlockedCommand(const MSTR& nameOfBlockedCommand, ScriptingFeature feature) = 0;

		/*! Gets whether the specified file name is a protected .ini file
		When safe scene script execution is enabled, certain .ini files are to be protected from modification
		via MAXScript methods. This method returns true if the specified file name is one of these protected .ini files.
		\param[in] filename - The file name to be tested.
		\return True if the file name corresponds to a protected .ini file, false otherwise.
		*/
		virtual bool IsProtectedINIFile(const MCHAR* filename) const = 0;

		/*! Gets whether the extension of the specified file name is a protected extension
		When safe scene script execution is enabled, files with certain extensions are to be protected from modification
		via MAXScript methods. This method returns true if the extension of the specified file name is one of these protected extensions.
		\param[in] filename - The file name to be tested.
		\return True if the extension of the specified file name is one of the protected extensions, false otherwise.
		*/
		virtual bool IsProtectedFileBasedOnExtension(const MCHAR* filename) const = 0;

		virtual ISecurityMessageManager* GetSecurityMessageManager() = 0;

		//! \brief Returns the only instance of this manager.
		DllExport static ISceneScriptSecurityManager* GetInstance();

		//! \brief Checks if the provided filename is protected from writing or creation based on its path (extension,
		//! directory or anything that applies) 
		//! \param filename the filename to check for write/create 
		//! \return true if write protected
		virtual bool IsProtectedFromOpeningForWriting(const TCHAR* filename) const = 0;
	};
}