 /*	
  *		WindowStreams.h - MAXScript
  *
  *			Copyright (c) John Wainwright 1996
  *
  */
 
#pragma once
#include "CharStream.h"
#include "../kernel/value.h"

 visible_class_debug_ok(WindowStream)
 
 // Collectable::flags3 - bit 0 set if WindowStream is wrapping a MXS_Scintilla edit control. If not, standard
 // edit control is assumed
 class WindowStream : public CharStream
 {
 public:
 	HWND		window;
 	int			cursor;
 	MSTR		title;
 	Listener*	listener;
 	CharStream*	log;
	MCHAR		wputs_buf[512];			// edit control output buffer
	MCHAR*		wputs_p;
	HWND		echo;
 
	// Constructor / Destructor
 				WindowStream(HWND iwin);
 				WindowStream(const MCHAR* title);		/* for background scripts; window with given title will open if output generated */
 			   ~WindowStream();
 
 				classof_methods (WindowStream, CharStream);
 	void		collect() override;
 	void		gc_trace() override;
 	void		sprin1(CharStream* s) override;
 	Value*		get_property(Value** arg_list, int count) override;
 	Value*		set_property(Value** arg_list, int count) override;
 
 #ifndef DOXYGEN
 #include "../macros/define_implementations.h"
 #endif
 	def_generic(sprint,	  "sprint");
  	use_generic( coerce,  "coerce");
 	use_generic( flush,   "flush"); 
		
 	/* internal MCHAR stream protocol */
 	
 	MaxSDK::Util::Char get_char() override { return MaxSDK::Util::Char(); }
 	void               unget_char(const MaxSDK::Util::Char& c) override { UNUSED_PARAM(c); }
 	MaxSDK::Util::Char peek_char() override {return MaxSDK::Util::Char(); }
 	int			at_eos() override { return TRUE; }
 	void		rewind() override {}
 	void		flush_to_eol() override {}
 
 	MaxSDK::Util::Char	putch(const MaxSDK::Util::Char& c) override;
 	const MCHAR*	puts(const MCHAR* str) override;
 	int		printf(const MCHAR *format, ...) override;
 	void		flush() override;

 	void		ensure_window_open();
 
 	void		log_to(CharStream *pLog) override;
 	void		close_log() override;
 	Listener*	get_listener() override { return listener; }
 	CharStream* get_log() override { return log; }

	// edit control output primitives
	ScripterExport const MCHAR*	wputs(const MCHAR *str);
	ScripterExport void		wflush();
	ScripterExport MaxSDK::Util::Char wputch(const MaxSDK::Util::Char& c);
	ScripterExport int		wprintf(const MCHAR *format, ...);
				   void		set_echo_window(HWND wnd) { echo = wnd; }
				   void		echo_cur_line();
 				   int		get_cur_line(MSTR& line);
	ScripterExport int		vprintf(const MCHAR *format, va_list args);

    // Introduced for [MAXX-68646: Simple Python help() command crashes 3ds Max]
    // to control UpdateWindow() calls based on an update rate. Eespecially for in Python
    // streaming with one large string containing all the lines to output.
    void update_window(int lastWritePos);
 };
 
 
