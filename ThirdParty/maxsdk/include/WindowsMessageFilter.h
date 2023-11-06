//**************************************************************************/
// Copyright (c) 2008 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/
// AUTHOR: Nicolas Desjardins
// DATE: 2008-10-21
//***************************************************************************/

#pragma once
#include "WindowsDefines.h"
#include "coreexp.h"
#include "noncopyable.h"
#include "autoptr.h"

namespace MaxSDK
{

/**
 * \brief Runs a message loop without blocking, allowing only acceptable 
 * Windows messages through to their destination UI controls.
 *
 * WindowsMessageFilter's message loop is a common idiom for Win32 progress dialogs.
 * A progress dialog may call WindowsMessageFilter::RunNonBlockingMessageLoop 
 * periodically to allows paint messages through.  In this way, the UI can 
 * refresh and the application appears to respond instead of entering Windows' 
 * "Not Responding" state.
 *
 * WindowsMessageFilter also solves a common bug with our hosted WPF (.NET 3.5) UI
 * controls.  It appears that the operating system periodically sends a sort of 
 * heartbeat message to WPF controls.  This message is in the user message range,
 * above WM_USER. If this message is filtered out, as was the case for previous
 * message filtering loops, the controls stop responding for the rest of the 
 * session.
 *
 * The WindowsMessageFilter ensures that any messages required by the internal 
 * Qt event system are passed through for the Qt UI. This is automatically done 
 * within the default implementation of IsAcceptableMessage, which calls the 
 * IsQtSystemMessage method. If you provide a custom override for 
 * IsAcceptableMessage make sure it contains a call IsQtSystemMessage - without 
 * that the Qt UI can freeze.
 *
 * The default implementation calls PeekMessage, removing messages from the
 * message queue.  It inspects each message in turn.  If IsAbortMessage 
 * returns true for a message, the loop is aborted.  If IsAcceptableMessage
 * returns true for the message, it is passed to the application's main
 * Translate and Dispatch message function.  If IsAcceptableMessage returns
 * false, and ShouldRepostMessage returns true, the message is reposted to
 * the message queue.  See each function's documentation for a description
 * of its behaviour.
 *
 * The implementation may be customized by creating a derived class and 
 * overriding any of IsAcceptableMessage, IsAbortMessage, or 
 * ShouldRepostMessage.  Alternatively, a derived class may override 
 * RunNonBlockingMessageLoop to specialize the message filtering loop itself.
 *
 * Example:
 * 
 * Generally, a message filtering loop looks like the following.  This could
 * be replaced by WindowsMessageFilter. Using WindowsMessageFilter is recommended
 * over a custom solution, since it allows us to solve bugs like the WPF problem 
 * described above in one place for the entire application.
 *
 * \code
 * // returns false if the current processing should be aborted.
 * // Messages are allowed through for the passed-in HWND and its child controls.
 * bool CheckWindowsMessages(HWND hWnd)
 * {
 *	   MSG msg = {0};
 *    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
 *    {
 *       // Quit message aborts
 *       if(WM_QUIT == msg.message) 
 *       {
 *          PostMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
 *          return false;
 *       }
 *
 *       // Escape key is consumed and aborts processing
 *       if(WM_KEYDOWN == msg.message && VK_ESCAPE == msg.wParam) 
 *       {
 *           return false;
 *       }
 *
 *       // Allow paint messages through, as well as any messages for the 
 *       // unfiltered window.
 *       if( WM_PAINT == msg.message || 
 *          WM_PAINTICON == msg.message ||
 *          msg.hwnd == hWnd || 
 *          IsChild(hWnd, msg.hwnd ) 
 *       {
 *          GetCOREInterface()->TranslateAndDispatchMAXMessage(msg);
 *       }
 *    }
 *
 *    return true; // allow processing to continue
 * }
 * \endcode
 *
 * This message filter loop can be replaced with the WindowsMessageFilter as
 * shown below.
 *
 * \code
 * bool CheckWindowsMessages(HWND hWnd)
 * {
 *	   MaxSDK::WindowsMessageFilter messageFilter;
 *    messageFilter.AddUnfilteredWindow(hWnd);
 *    messageFilter.RunNonBlockingMessageLoop();
 *    return !messageFilter.Aborted();
 * }
 * \endcode
 * 
 */
class WindowsMessageFilter : public MaxSDK::Util::Noncopyable
{
public:
	/** 
	 * \brief Constructor.
	 */
	CoreExport WindowsMessageFilter();
	
	/**
	 * \brief Destructor.
	 */
	CoreExport virtual ~WindowsMessageFilter();

	/**
	 * \brief Run a message loop without blocking, allowing only acceptable 
	 * Windows messages through to their destination UI controls.
	 *
	 * Messages deemed acceptable by IsAcceptableMessage are Translated and
	 * Dispatched.  An Abort message, as determined by IsAbortMessage is 
	 * consumed, sets Aborted to true, and exits the loop.  A message may be
	 * reposted to the end of the message queue if it is not acceptable and if
	 * ShouldRepostMessage returns true.  The loop exits normally when the 
	 * message queue is depleted.
	 *
	 * Note that this call is ignored if not called from the main thread.
	 */
	CoreExport virtual void RunNonBlockingMessageLoop();
	
	/**
	 * \brief Abort the current message loop.
	 * 
	 * Sets Aborted to true immediately, then aborts the currently running 
	 * message loop when control returns.
	 */
	CoreExport virtual void AbortMessageLoop();
	
	/**
	 * \brief May the given message be delivered to its destination control?
	 *
	 * The default implementation considers the following messages acceptable:
	 * \li WM_PAINT
	 * \li WM_PAINTICON
	 * \li Any user message (&gt;= WM_USER)
	 * \li Any message required by the underlying Qt event system. This check
	 * uses IsQtSystemMessage for determining acceptable messages.
	 * \li Any message whose hwnd value is a registered unfiltered window or
	 * a registered unfiltered window's child.
	 *
	 * \param msg Message to evaluate.
	 * \return true if the message is acceptable, false otherwise.
	 * \see IsQtSystemMessage()
	 */
	CoreExport virtual bool IsAcceptableMessage(const MSG& msg) const;
	
	/**
	 * \brief Is the given message an "Abort" message?
	 *
	 * The default implementation considers an ESC key down message as an
	 * "Abort" message.
	 *
	 * \param msg Message to evaluate.
	 * \return true if the message is an Abort message, false otherwise.
	 */
	CoreExport virtual bool IsAbortMessage(const MSG& msg) const;
	
	/**
	 * \brief Should the given message be reposted to the message queue?
	 *
	 * The default implementation only returns true for WM_QUIT.
	 *
	 * \param msg Message to evaluate.
	 * \return true if the message should be reposted, false otherwise.
	 */
	CoreExport virtual bool ShouldRepostMessage(const MSG& msg) const;
	
	/**
	 * \brief Was the last message loop aborted?
	 *
	 * The Aborted value is reset to false at the start of any call to 
	 * RunNonBlockingMessageLoop.  If the loop terminates by depleting the 
	 * message queue, Aborted will remain false.  On the other hand, if the
	 * message loop finds an Abort or Quit message, it will abort, and mark
	 * Aborted as true.  Similarly, if a client calls AbortMessageLoop, 
	 * the message loop will abort and set Aborted to true.
	 *
	 * \return true if the message loop aborted early, false otherwise.
	 */
	CoreExport virtual bool Aborted() const;
	
	/**
	 * \brief Add a window handle to the list of unfiltered windows.
	 *
	 * All messages intended for the given window will be allowed through
	 * the message filter.
	 *
	 * Note that adding duplicate elements will place the same element multiple
	 * times in the list.  If a set of window handles may be added multiple times,
	 * it is better to call clear first.
	 *
	 * \param window Window handle to leave unfiltered.
	 */
	CoreExport void AddUnfilteredWindow(HWND window);
	
	/**
	 * \brief Remove a currently unfiltered window from the list.
	 *
	 * This command is ignored if the window is not present in the list.  If the 
	 * window handle was added multiple times, all instances of it will be 
	 * removed from the list.
	 *
	 * \param window Window handle to remove from the list of unfiltered windows.
	 */
	CoreExport void RemoveUnfilteredWindow(HWND window);
	
	/**
	 * \brief Remove all window handles from the list of unfiltered windows.
	 */
	CoreExport void ClearUnfilteredWindowList();
	
	/**
	 * \brief Is the given window handle in the list of unfiltered windows?
	 *
	 * \param window Window handle for which to look.
	 * \return true if the given window handled is in the unfiltered list, false
	 * otherwise.
	 */
	CoreExport bool IsUnfilteredWindow(HWND window) const;
	
	/**
	 * \brief Is the given window a child of a registered unfiltered window?
	 * 
	 * \param child Window handle to examine.
	 * \return true if this window handle is a child of a registered unfiltered 
	 * window, false otherwise.
	 */
	CoreExport bool IsUnfilteredWindowChild(HWND child) const;

	/**
	 * \brief Returns if the given message is required for the underlying Qt event system
	 * and therefore needs to be accepted by the message filter.
	 * \param msg Message to evaluate.
	 * \return true if the message is required Qt system message, false otherwise.
	 * \see IsAcceptableMessage()
	 */
	CoreExport static bool IsQtSystemMessage(const MSG& msg);

	/**
	 * \brief Sets the message types that will be processed by the message loop
	 * \param msgTypes Can be any combination of the following flags:
	 * - PM_QS_INPUT: Process mouse and keyboard messages
	 * - PM_QS_PAINT: Process paint messages
	 * - PM_QS_POSTMESSAGE: Process all posted messages, including timers and hotkeys
	 * - PM_QS_SENDMESSAGE: Process all sent messages
	 * By default, all message types are processed
	 * Note that you should never pass any of the flags that specify how messages are handled to this method
	 * Those flags are: PM_NOREMOVE, PM_REMOVE and PM_NOYIELD
	 * The PM_REMOVE value is always used so that messages are removed from the message queue after being processed
	 */
	CoreExport void SetMessageTypesToProcess(UINT msgTypes);

private:
	struct Storage;
	MaxSDK::AutoPtr<Storage> mStorage;
	UINT mMessageTypesToProcess;
};


}
