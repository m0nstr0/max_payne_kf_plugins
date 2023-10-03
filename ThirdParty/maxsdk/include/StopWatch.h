/*==============================================================================
Copyright 2010 Autodesk, Inc.  All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement provided
at the time of installation or download, or which otherwise accompanies this software
in either electronic or hard copy form.   */

/**************************************************************************/
// DESCRIPTION: Classes To Watch Time Consumption For Max's Components
// AUTHOR: Golden Cheng created March 2010
/***************************************************************************/
#pragma once
#include "WindowsDefines.h"
#include "utilexp.h"
#include "MaxHeap.h"

namespace MaxSDK
{
	namespace Util
	{
		//! \brief A basic timer tool
		/*! A StopWatch is a light-weight timer tool that tests the time consumption.This timer tool
		uses high performance counter(QueryPerformanceCounter) to record the time comsumption
		and the time tested is at the milliseconds level.The workflow of StopWatch is like this:
		Start -\> ( Pause -\>(Resume)*n)*n -\> Stop. Between Start and Stop, numbers of Pause -\> (Resume)*n 
		operations can be done. And also Stop operation can be done directly after Pause operation.
		For example:
				\code
		           void theFunctionTested()
				   {
				        StopWatch watch;

						watch.Start();
						.....                          // the part that the user wants to time
						watch.Pause();
						.....                          // the part that the user doesn't want to time
						watch.Resume();
						.....                          // the part that the user wants to time
						watch.Pause();
						.....                          // the part that the user doesn't want to time
						watch.Resume();
						.....                          // the part that the user wants to time
						watch.Pause();
						.....                          // the part that the user doesn't want to time
						watch.Stop();             // Stop operation can be done directly after Pause operation done
						.....
						double ms = watch.GetElapsedTime();  // get the consumption time that the user wants in the loop
				   }
				\endcode
		*/
		class StopWatch: public MaxHeapOperators
		{
		public:
			//! \brief Three timer states are defined 
			enum TimerState
			{
				//! \brief If timer is running, the state is defined as PLAYING and means that the timer is timing now
				PLAYING =0,
				//! \brief If timer is paused, the state is defined as PAUSED and means that the timer pauses to time unless doing Resume
				PAUSED ,
				//! \brief If timer is stopped, the state is defined as STOPPED and means that the timer stops to time unless doing Start
				STOPPED 
			};
			//! \brief Construct the StopWatch Object
			UtilExport StopWatch();
			//! \brief This method lets you start to trace the time and changes the current state to PLAYING
			UtilExport void Start();
			//! \brief This method lets you start in a PAUSED state
			UtilExport void StartPaused();
			//! \brief This method lets you pause to trace the time and changes the current state to PAUSED
			UtilExport void Pause();
			//! \brief This method lets you resume to trace the time and changes the current state to PLAYING
			UtilExport void Resume();
			//! \brief This method lets you stop to trace the time and changes the current state to STOPPED
			UtilExport void Stop();
			//! \brief This method lets you clear the time traced and changes the current state to STOPPED
			UtilExport void Reset();
			//! \brief This method lets you know whether the timer is running
			UtilExport bool IsRunning() const;
			//! \brief This method lets you know whether the timer is paused
			UtilExport bool IsPaused() const;
			//! \brief This method lets you know whether the timer is stopped
			UtilExport bool IsStopped() const;
			//! \brief This method lets you know what the current timer state is
			UtilExport TimerState GetTimerCurrentState() const;
			//! \brief This method lets you get the elapsed time(milliseconds) traced
			/*! Note that each paired call of Start/Stop and Resume/Pause imposes some overhead, mainly the time associated with 2 calls to QueryPerformanceCounter.
			* If client code is making many Resume/Pause calls and/or the code executing between the Start/Stop and Resume/Pause calls runs very fast, the reported
			* elapsed time may be significantly impacted by this overhead. 
			\sa CalculateAverageOverhead(int loopCount)
			 */
			UtilExport double GetElapsedTime() const;
			//! \brief Retrieves the current local time
			UtilExport void GetLocalTime(SYSTEMTIME* time) const;
			//! \brief Calculates and returns the average overhead associated with Start/Stop and Pause/Resume. 
			/*! Each paired call of Start/Stop and Resume/Pause imposes some overhead. The overhead is mainly the time associated with 2 calls to QueryPerformanceCounter.
			On average, the overhead is small (~ 1.6E-5 msec), but this can be significant if Pause/Resume is called many, many times while measuring something that runs very fast.
			The actual time associated with 2 calls to QueryPerformanceCounter is noisy, and it is not practical to correct for this overhead in the StopWatch code. Thus it is the
			responsibility of the client code to handle this if need be.
			Example:
			\code
			size_t PTell_count = 0; 
			MaxSDK::Util::StopWatch PTell_stopwatch; 
			PTell_stopwatch.StartPaused(); 
			while (...)
			{
				++PTell_count;
				PTell_stopwatch.Resume();
				...
				PTell_stopwatch.Pause();
			}
			PTell_stopwatch.Stop();
			double timerOverhead = MaxSDK::Util::StopWatch::CalculateAverageOverhead(100);
			DebugPrint(_T("Tell:  %llu - %lf\n"), PTell_count, (PTell_stopwatch.GetElapsedTime() - PTell_count*timerOverhead));
			\endcode
			\param loopCount The number of loops calling Pause/Resume to average overhead over.
			\return The average overhead of calling Start/Stop and Pause/Resume.
			*/
			UtilExport static double CalculateAverageOverhead(int loopCount);

		private:
			void StartTicking();
			double CalculateCurrentElapsedTime() const;

			LARGE_INTEGER mStartTime;
			LARGE_INTEGER mFreq;
			double mElapsedTime;
			TimerState mTimerState;
		};

	}
}
