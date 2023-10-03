//**************************************************************************/
// Copyright (c) 2021 Autodesk, Inc.
// All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
//**************************************************************************/
// DESCRIPTION: Autobackup interface
// AUTHOR: Autodesk Inc.
//**************************************************************************/

#pragma once

#include "iFnPub.h"
#include "GetCOREInterface.h"

#define AutoBackup_INTERFACE Interface_ID(0x4dab68f9, 0x3e4032c6)

//! Interface class for Autobackup.
//! Interface for AutoBackup class. This class has only one instance and is responsible
//! for automatically saving the max scene at periodic time intervals.
class IAutoBackup : public FPStaticInterface
{
public:
	static constexpr Interface_ID ID = AutoBackup_INTERFACE;

	//! Enables or disables AutoBackup.
	//! This change is reflected in preference settings. Changing this can cause UI redraws.
	//! To temporarily disable Autobackup during an operation, use SetTempDisable
	//! \see IAutoBackup::SetTempDisable
	virtual void SetEnable(bool enable) = 0;
	//! \return true when Autobackup feature is enabled
	virtual bool GetEnable() const = 0;

	//! Temporarily disables Autobackup
	//! Use this during operations where Autobackup should not occur (ex: Computations on separate threads, heavy scene manipulations)
	//! Pass true to this function before operation and false when operation is done. 
	//! This can be called many times with 'true'. Autobackup is re-enabled when the same number of 'false' are passed
	//! \see AutoBackupDisableHelper
	virtual void SetTempDisable(bool disable) = 0;
	//! \return true when Autobackup is temporarily disabled
	virtual bool GetTempDisable() const = 0;

	//! Sets the number of Autobackup files.
	//! When that number is reached, Autobackup will overwrite older backup files
	virtual void SetNumFiles(int n) = 0;
	//! \return Number of Autobackup files
	virtual int GetNumFiles() const = 0;

	//! Sets the time interval before backup, in minutes.
	virtual void SetInterval(float minutes) = 0;
	//! \return Autobackup interval in minutes
	virtual float GetInterval() const = 0;

	//! Sets period of time at the end of autobackup interval during which timer can pause if there is user activity.
	//! If set to 0, activity detection is disabled and backup will always proceed when GetSecondsUntilNextAttempt goes down to 0.
	//! \see GetSecondsUntilNextAttempt
	virtual void SetFinalCountdownInterval(float minutes) = 0;
	//! \return Final countdown interval in minutes
	virtual float GetFinalCountdownInterval() const = 0;

	//! Sets the filename portion of Autobackup file, excluding path and extension.
	//! Pass empty string "" to reset to default file name
	//! The full path is determined by system paths, project paths and SetPrependSceneName
	//! \see IPathConfigMgr::GetDir, APP_AUTOBACK_DIR, SetPrependSceneName
	virtual void SetFileName(const TCHAR* fileName) = 0;
	//! \return File name of Autobackup file, excluding path and extension.
	virtual const MSTR& GetFileName() const = 0;

	//! When set to true, current scene name is prepended to autobackup file name. ex: myScene_AutoBackup01.max
	//! In that case, the total number of Autobackup files as determined by SetNumFiles is counted per-scene
	//! \see SetFileName, SetNumFiles
	virtual void SetPrependSceneName(bool prepend) = 0;
	//! \return true if scene name is prepended to autobackup file name
	virtual bool GetPrependSceneName() const = 0;

	//! Turns on and off Bailout during Autobackup.
	//! Allows user to abort backup by pressing escape key
	//! \see  Class MaxSDK::Util::IBailOutBusyProcessManager
	virtual void SetBailoutEnable(bool b) = 0;
	//! \return true when Autobackup bailout is enabled
	virtual bool GetBailoutEnable() const = 0;

	//! Sets whether Autobackup files should be compressed or not.
	virtual void SetCompressOnAutobackup(bool b) = 0;
	//! \return true when Autobackup files will be compressed
	virtual bool GetCompressOnAutobackup() const = 0;

	//! Performs the actual Autobackup operation
	virtual void AutoBackupNow() = 0;
	//! Tells whether an Autobackup operation is currently happening.
	virtual bool IsCurrentlySaving() const = 0;
	//! \deprecated This has been deprecated as of 3ds Max 2023, please use IsCurrentlySaving() instead
	MAX_DEPRECATED virtual bool GetActive() const final { return IsCurrentlySaving(); };

	//! Resets timer to interval. Does nothing when timer is not started
	//! \see GetInterval, IsTimerStarted
	virtual void ResetTimer() = 0;
	//! Resets timer to minutes
	virtual void ResetTimer(float minutes) = 0;

	//! Returns true if Autobackup timer is started
	virtual bool IsTimerStarted() const = 0;
	//! Returns the number of seconds until the next Autobackup attempt.
	//! Returns -1 if Autobackup is disabled or if Autobackup timer is not started
	//! \see IsTimerStarted, GetInterval
	virtual int GetSecondsUntilNextAttempt() const = 0;
	/*! Returns true if all conditions are satisfied for next Autobackup attempt to proceed, based on current state.
		If this is still true when GetSecondsUntilNextAttempt() goes down to 0, Autobackup kicks in.
		\ see GetSecondsUntilNextAttempt
		\code
		// approximate implementation
		{
			return GetEnable() // Autobackup enabled
				&& !GetTempDisable() // Autobackup not temporarily disabled
				&& !IsCurrentlySaving() // Autobackup not in the middle of saving a backup
				&& !GetCOREInterface8()->IsSavingToFile() // 3ds Max not in the middle of saving a scene
				&& IsSaveRequired() // From custcont.h, scene "dirty" flag
				&& IsAutoSaveRequired() // From custcont.h, scene "dirty" flag
				&& GetCapture() == nullptr // from WinUser.h, no window is capturing mouse input
				&& IsWindowEnabled(GetCOREInterface()->GetMAXHWnd()) // from WinUser.h, no modal dialog on top of main 3ds Max window
				// Implementation defined using GetFocus() from WinUser.h. Keyboard focus is not in a text edit field
				&& !IsKeyboardFocusInEditField()
		}
		\endcode
	*/
	virtual bool CanProceed() const = 0;
};
//! \brief Gets the IAutobackup interface pointer.
inline IAutoBackup* GetAutoBackupInterface()
{
	return static_cast<IAutoBackup*>(GetCOREInterface(IAutoBackup::ID));
}

//! Helper struct to temporarily disable and re-enable Autobackup during a computation
//! \see SetTempDisable
struct AutoBackupDisableHelper : public MaxHeapOperators
{
	IAutoBackup* m_autobackupInterface = nullptr;
	AutoBackupDisableHelper() : m_autobackupInterface(GetAutoBackupInterface())
	{
		if (m_autobackupInterface)
			m_autobackupInterface->SetTempDisable(true);
	}
	~AutoBackupDisableHelper()
	{
		if (m_autobackupInterface)
			m_autobackupInterface->SetTempDisable(false);
	}
};
