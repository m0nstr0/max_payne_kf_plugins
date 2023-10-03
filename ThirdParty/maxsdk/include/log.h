//-----------------------------------------------------------------------------
// ----------------
// File ....: log.h
// ----------------
// Author...: Gus Grubba
// Date ....: November 1996
//
// History .: Nov, 27 1996 - Started
//
//-----------------------------------------------------------------------------
		
#pragma once
#include <WTypes.h>
#include "strbasic.h"
#include "maxheap.h"

#define NO_DIALOG		FALSE
#define DISPLAY_DIALOG	TRUE

/*! \defgroup systemErrorLogMessageTypes System Error Log Message Types
There are four types of log entries. In the preference dialog, the user can select what
types of log entries they want to be generated. This is how users control the
verbosity of the log file in some meaningful way. Developers are encouraged to be very
verbose about information and debug messages -- the Log() functions should be used to
record any events out of the ordinary. As the user can elect to ignore these messages they
are perfect for troubleshooting. The values can be OR'd together.

SYSLOG_DEBUG messages are also always sent to the debugger output using DebugPrint()

\sa Class LogSys */
//!@{
/*! An error message. An example of this type is a fatal error.
	\note Do NOT pass in a string longer than 8192 characters when passing in this type (Otherwise you will crash). */
#define	SYSLOG_ERROR		0x00000001	
/*! An example of this type is a message telling the user the MAX file just loaded is
obsolete and needs to be resaved. This option may not be selected by the MAX user via the
UI but it is available for use (it's used internally often). */
#define	SYSLOG_WARN			0x00000002	//!< A warning message.
/*! An example of this is a message indicating a new MAX file has been loaded. */
#define	SYSLOG_INFO			0x00000004	//!< An information message. 
/*! This message type is for anything you think might help trace problems that the user has with your code.
Message will also be sent to the debugger output using DebugPrint(). */
#define	SYSLOG_DEBUG		0x00000008	//!< A debug message. 
#define	SYSLOG_STARTBATCH	0x00000010	/*!<	The next three defines are used when you want to 
												send a large number of messages to the logger to prevent it 
												from continually opening/closing the log for performance reason.
												The first message's type should be OR'd with SYSLOG_STARTBATCH
												The following messages should be OR's with SYSLOG_BATCH
												And when you are done sending messages the last one should be OR's with SYSLOG_ENDBATCH

												logger->LogEntry(SYSLOG_INFO|SYSLOG_STARTBATCH,...);
												logger->LogEntry(SYSLOG_INFO|SYSLOG_BATCH,...);
												logger->LogEntry(SYSLOG_INFO|SYSLOG_BATCH,...);
												logger->LogEntry(SYSLOG_INFO|SYSLOG_BATCH,...);
												logger->LogEntry(SYSLOG_INFO|SYSLOG_ENDBATCH,...);

												*/
#define	SYSLOG_BATCH		0x00000020	//!< Used to mark messages that are sent in batch
#define	SYSLOG_ENDBATCH		0x00000040	//!< Used to mark the end an batch message

#define SYSLOG_BROADCAST	0x00010000	//!< 
#define SYSLOG_MR			0x00020000	//!< 
#define SYSLOG_IGNORE_VERBOSITY 0x00040000 //!< Used with SYSLOG_BROADCAST to force the display of important messages that are not errors or warnings
//!@}

#define	SYSLOG_LIFE_EVER	0
#define	SYSLOG_LIFE_DAYS	1
#define	SYSLOG_LIFE_SIZE	2

/*! \brief Class for writing out log information to a file.
\remarks MAX maintains a log file that contains the text of error / warning /
information / debug messages generated by the system and plug-ins. This class
is used to work with the log and send messages to it. The log file is placed in
the Network directory and is called Max.Log. To access this
facility from anywhere in MAX use the pointer returned from the method
Interface::Log(). All methods of this class are implemented by the
system.  
\sa  Class Interface. */
class LogSys : public MaxHeapOperators
{
	protected:
		DWORD	valTypes;
		int		logLife;
		DWORD	logDays;
		DWORD	logSize;

	 public:
		 virtual ~LogSys() {;}

		//-- Maintenance methods -----------------------------------------------
		//
		//	 Methods used internally

		/*! \remarks This method is used to find out what log types are enabled.
		See \ref systemErrorLogMessageTypes. The type values are ORed together to create the value returned. */
		virtual		DWORD	LogTypes ( ) { return valTypes; }

		/*! \remarks This method is used to set the log types that are enabled.
		\param types - Sets the type of log messages
		See \ref systemErrorLogMessageTypes. */
		virtual		void	SetLogTypes ( DWORD types ) { valTypes = types; }

		/*! \remarks This method is used to log the error.
		\param type - Defines the type of log entry defined in this header file. See \ref systemErrorLogMessageTypes. Note that since
		the System Error Log Message Types are specified as bit flag values, multiple types can be specified (although normally 
		only a single type is specified). If multiple types are specified, the highest severity type that matches with the enabled log 
		types is used. The types in level of severity are error, warn, info, debug.
		\param dialogue - One of the following values:
			\li NO_DIALOG - If this entry is just some information you don't want a dialogue for,
				or if you are handling the dialogue yourself use this value.
			\li DISPLAY_DIALOG - Use this value if you want the message to be displayed in a dialogue.
				The system will determine if displaying a dialogue is appropriate based
				on network rendering mode.
		\param title - This title string is optional. If non NULL, it will be used to define the module.
		\param format - This parameter (and any other additional arguments that follow) make up
			the format specification. The format matches the standard C printf() function.
		\code
			TheManager-\>Max()-\>Log()-\>LogEntry(SYSLOG_ERROR, NO_DIALOG, _M("Warning Title") ,_M("%s - %s\n"), ShortDesc(), errText);
		\endcode */
		virtual		void	LogEntry		( DWORD type, BOOL dialogue, const MCHAR* title, const MCHAR* format, ... ) = 0;

		/*! \brief Enables or disables 'quiet' mode. 
		\remarks When set to quiet mode, the LogSys::LogEntry(...) method 
		will not bring up any dialog boxes. Thus it will act as it does in 
		network rendering mode. That is the error will only be written to the 
		log file. 
		\note After setting quiet mode, do not forget to clear it when you are done, since
		the user will not see any error messages from the renderer while quiet
		mode is enabled.
		\param quiet - true to enable, false to disable. */
		virtual		void	SetQuietMode( bool quiet ) = 0;

		/*! \brief Gets the Quiet Mode.
		\return true if 'quiet' mode is enabled or false if it's disabled. */
		virtual		bool	GetQuietMode( ) = 0;

		/*! \brief Enables or disables 'enabled' mode.
		\remarks When set to not enabled, the LogSys::LogEntry(...) method
		will not log the message nor bring up any dialog boxes. 
		\note After setting to not enabled, do not forget to re-enable it when you are done, since
		no logging of information will be performed.
		\param enabled - true to enable, false to disable. */
		virtual		void	SetEnabledMode(bool enabled) = 0;

		/*! \brief Gets the 'enabled' mode.
		\return true if 'enabled' mode is enabled or false if it's disabled. */
		virtual		bool	GetEnabledMode() = 0;

		//! \brief Sets additional output log file on session basis
		/*! This method defines additional log file in specified location.
		Unlike default max.log file, the output is based on session. 
		If the given file exists, its contents are destroyed.
		\param [in] logName - output file name. If an empty (but non-null) string is specified,
		the additional output log file is disabled. If the logName is not a valid file name,
		the current additional output log file (if any) will continue to be used.*/
		virtual     void	SetSessionLogName	( const MCHAR* logName ) = 0;
		
		//! \brief Retrieves the additional output log file name
		/*! \return - the output file name set in SetSessionLogName() 
			call or NULL if no file specified. */
		virtual     const MCHAR*	GetSessionLogName	( )= 0;

		//! \brief Retrieves the output log file name
		/*! \return - the output log file name */
		virtual     const   MCHAR*	NetLogName() = 0;

		//! \name Log File Longevity
//!@{
		/*! \brief Returns the conditions under which the log is deleted.
		\return  One of the following values:
		\li SYSLOG_LIFE_EVER - The log is never deleted.
		\li SYSLOG_LIFE_DAYS - This log is maintained for this number of days.
		\li SYSLOG_LIFE_SIZE - The log is maintained until it reaches this many kilobytes (KB). */
		virtual		int		Longevity		( )				{ return logLife; }
		
		/*! \brief Sets the conditions under which the log is deleted.
		\param type - One of the following values:
		\li SYSLOG_LIFE_EVER - The log is never deleted.
		\li SYSLOG_LIFE_DAYS - This log is maintained for this number of days.
		\li SYSLOG_LIFE_SIZE - The log is maintained until it reaches this many kilobytes (KB). */
		virtual		void	SetLongevity	( int type )	{ logLife = type; }

		/*! \brief Returns the conditions under which the log is cleared.
		\return  One of the following values:
		\li SYSLOG_LIFE_EVER - The log is never deleted.
		\li SYSLOG_LIFE_DAYS - This log is maintained for this number of days.
		\li SYSLOG_LIFE_SIZE - The log is maintained until it reaches this many kilobytes (KB). */
		virtual		DWORD	LogDays			( )				{ return logDays; }
		
		/*! \brief Returns the size of the current log file in kilobytes	(KB). */
		virtual		DWORD	LogSize			( )				{ return logSize; }
		
		/*! \brief Set the number of days the log is maintained.
		\param days - The number of days to maintain the log. After this many days after
		creation the log is deleted. */
		virtual		void	SetLogDays		( DWORD days ) 	{ logDays = days; }

		/*! \brief Set the maximum size in kilobytes (KB) of the log file.
		After this size is reached the log file is deleted.
		\param size - The maximum size in kilobytes (KB) of the log file. */
		virtual		void	SetLogSize		( DWORD size ) 	{ logSize = size; }
//!@}

//! \name State
	//!@{
		/*! \remarks This method is used internally. */
		virtual		void	SaveState		( void ) = 0;
		/*! \remarks This method is used internally. */
		virtual		void	LoadState		( void ) = 0;
	//!@}
};

