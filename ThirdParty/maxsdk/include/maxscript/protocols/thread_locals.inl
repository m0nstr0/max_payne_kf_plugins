/*    
 *        thread_locals.h - thread locals for each interpreter thread in MAXScript
 *
 *            Copyright (c) John Wainwright 1996
 *
 */

/* thread locals and initial values */
#pragma warning(push)
#pragma warning(disable:4100)

// This file describes the thread local variables. This file is included in various contexts where the definition of 'def_thread_local'
// is context specific. For example, in one place type, 'def_thread_local' is used to declare the thread local variable, in another it
// is used to initialize the thread local variable, in another it is used during garbage collection to mark Value* derived values
// as being in use.

// Column 1 is the type of the thread local variable
// Column 2 is the thread local variable's name
// Column 3 is whether to re-initialize the thread local variable on a reset_thread_locals
// Column 4 is the value used to initialize the thread local variable

    def_thread_local( BOOL,                          seh_set,                        FALSE,  FALSE);           // whether the structured exception handler has been set for the thread
    def_thread_local( NewCollectableLinkedListItem*, last_new_collectable,           FALSE,  new NewCollectableLinkedListItem);    // linked list of Collectables created via new before initialized as a Value
    def_thread_local( CharStream*,                   current_stdout,                 TRUE,   new (GC_IN_HEAP) WindowStream(_M("Script Output")));
    def_thread_local( String*,                       undo_label,                     TRUE,   new (GC_IN_HEAP) String(_M("MAXScript"))); // current undo label
    def_thread_local( BOOL,                          force_listener_open,            TRUE,   TRUE);            // whether to force listener open on output to it

    def_thread_local( Value**,                       current_frame,                  TRUE,   NULL);            // current interpreter frame (for thunk evals)
    def_thread_local( Value**,                       current_scan_frame,             TRUE,   NULL);            // current interpreter frame (for gc scanner) 
    def_thread_local( Value**,                       current_locals_frame,           TRUE,   NULL);            // C++ local frame
    def_thread_local( Value*,                        current_result,                 TRUE,   NULL);            // C++ current Value* function result
    def_thread_local( Value*,                        last_new_heap_value,            TRUE,   NULL);            // last non-GC_PERMANENT heap value created
    def_thread_local( long,                          stack_limit,                    TRUE,   ALLOCATOR_STACK_SIZE);    // max stack size to catch recurse loops, 1Mb to start, 
                                                                                                                       // minus buffer to handle calls made during error handling
    def_thread_local( LONG_PTR,                      stack_base,                     TRUE,   (LONG_PTR)_alloca(sizeof(int)));    // current stack base
    def_thread_local( MSPlugin*,                     current_plugin,                 TRUE,   NULL);            // current scripted plugin (for plugin thunk evals)
    def_thread_local( Struct*,                       current_struct,                 TRUE,   NULL);            // current struct (for struct member thunk evals)
    def_thread_local( Value*,                        current_container,              TRUE,   NULL);            // current container for nested property access
    def_thread_local( int,                           container_index,                TRUE,   0);               // current container index (if any)
    def_thread_local( Value*,                        container_prop,                 TRUE,   NULL);            // current container prop (if any)
    def_thread_local( Value*,                        current_prop,                   TRUE,   NULL);            // most recent prop access (if any)

    def_thread_local( Value*,                        source_file,                    TRUE,   NULL);            // current source file
    def_thread_local( UINT_PTR,                      source_pos,                     TRUE,   0);               // current pos in source file
    def_thread_local( UINT_PTR,                      source_line,                    TRUE,   0);               // current pos in source file, instead of INT_PTR, will have an int to simplify things

    def_thread_local( int,                           needs_redraw,                   TRUE,   0);               // 0- no redraw needed, 1 - redraw needed, 2 - complete redraw needed
    def_thread_local( BOOL,                          redraw_mode,                    TRUE,   1);               // redraw on
    def_thread_local( BOOL,                          pivot_mode,                     TRUE,   0);               // pivot off
    def_thread_local( BOOL,                          undo_mode,                      TRUE,   1);               // undo on
    def_thread_local( Value*,                        current_level,                  TRUE,   &all_objects);    // $objects
    def_thread_local( BOOL,                          use_time_context,               TRUE,   0);               // use MAX time slider
    def_thread_local( TimeValue,                     current_time,                   TRUE,   0);
    def_thread_local( Value*,                        current_coordsys,               TRUE,   n_default);
    def_thread_local( Value*,                        center_mode,                    TRUE,   n_default);

    def_thread_local( int,                           rand_accum,                     FALSE,  0);               // for our own rand()
    def_thread_local( HANDLE,                        message_event,                  TRUE,   NULL);            // listener_message synch event
    def_thread_local( int,                           stream_rand_accum,              TRUE,   0);               // for stream_rand()

    def_thread_local( MSZipPackage*,                 current_pkg,                    TRUE,   NULL);            // currently open zip package, if any

    def_thread_local( void*,                         alloc_frame,                    FALSE,  NULL);            // top frame of allocator stack
    def_thread_local( void*,                         alloc_tos,                      FALSE,  NULL);            // top of allocator stack
    def_thread_local( void*,                         alloc_stack_lim,                FALSE,  NULL);            // limit of allocator stack

    def_thread_local( Control*,                      current_controller,             TRUE,   NULL);            // currently evaluating scripted controller

    def_thread_local( BOOL,                          try_mode,                       TRUE,   0);               // try(...)
    def_thread_local( MAXScriptException*,           current_exception,              TRUE,   NULL);            // current exception that was thrown, if any. Non-null only in catch expression
    def_thread_local( UnknownSystemException*,       current_UnknownSystemException, FALSE,  NULL);            // current UnknownSystemException that was thrown, if any. 

    def_thread_local( BOOL,                          thread_not_active,              TRUE,   FALSE);           // set to TRUE when thread found not to be active any more

    def_thread_local( BOOL,                          firstChanceHandlingofErrorOccurred,  TRUE,   FALSE);      // set to TRUE when first chance handling of error has occurred
    def_thread_local( BOOL,                          err_source_captured,            TRUE,   FALSE);           // set to TRUE when err_source_* captured
    def_thread_local( Value*,                        err_source_file,                TRUE,   NULL);            // source file where error thrown
    def_thread_local( UINT_PTR,                      err_source_pos,                 TRUE,   0);               // pos in source file where error thrown
    def_thread_local( UINT_PTR,                      err_source_line,                TRUE,   0);               // line in source file where error thrown

    def_thread_local( Value*,                        current_frame_owner,            TRUE,   NULL);            // owner of current stack frame while compiling
    
    def_thread_local( BOOL,                          is_dubugger_thread,             TRUE,   FALSE);           // owner of current stack frame while compiling

    def_thread_local( DWORD,                         source_flags,                   TRUE,   0);               // current source file flags
    def_thread_local( DWORD,                         err_source_flags,               TRUE,   0);               // source file flags for source file where error thrown

    def_thread_local( BOOL,                          disable_trace_back,             TRUE,   FALSE);           // true if no stack traceback is to occur if an error occurs

    def_thread_local( StringStream*,                 resource_value_reader,          TRUE,   NULL);            // Stringstream used to prevalidate resource Values read from resource files

    def_thread_local( StringStream*,                 eval_Stringstream1,             FALSE,  NULL);            // Stringstream 1 used to evaluate scripts
    def_thread_local( StringStream*,                 eval_Stringstream2,             FALSE,  NULL);            // Stringstream 2 used to evaluate scripts
    def_thread_local( BOOL,                          eval_Stringstream1_inuse,       FALSE,  FALSE);           // true if eval_Stringstream1 is in use
    def_thread_local( BOOL,                          eval_Stringstream2_inuse,       FALSE,  FALSE);           // true if eval_Stringstream2 is in use

    def_thread_local( Parser*,                       eval_Parser1,                   FALSE,  NULL);            // Parser 1 used to evaluate scripts
    def_thread_local( Parser*,                       eval_Parser2,                   FALSE,  NULL);            // Parser 2 used to evaluate scripts
    def_thread_local( BOOL,                          eval_Parser1_inuse,             FALSE,  FALSE);           // true if eval_Parser1 is in use
    def_thread_local( BOOL,                          eval_Parser2_inuse,             FALSE,  FALSE);           // true if eval_Parser2 is in use

    def_thread_local( MAXScript::ScriptSource,       scriptSource,                   FALSE,  MAXScript::ScriptSource::NotSpecified);           // true if running CodeTree whose source was compiled with ScriptSource == Embedded

    def_thread_local( void*,                         reserved_ptr1,                  TRUE,   NULL);            // reserved for future internal usage
    def_thread_local( void*,                         reserved_ptr2,                  TRUE,   NULL);            // reserved for future internal usage
    def_thread_local( void*,                         reserved_ptr3,                  TRUE,   NULL);            // reserved for future internal usage
    def_thread_local( void*,                         reserved_ptr4,                  TRUE,   NULL);            // reserved for future internal usage

    def_thread_local( void*,                         reserved_ptr5,                  FALSE,  NULL);            // reserved for future internal usage
    def_thread_local( void*,                         reserved_ptr6,                  FALSE,  NULL);            // reserved for future internal usage
    def_thread_local( void*,                         reserved_ptr7,                  FALSE,  NULL);            // reserved for future internal usage
    def_thread_local( void*,                         reserved_ptr8,                  FALSE,  NULL);            // reserved for future internal usage

    def_thread_local( Value*,                        reserved_value1,                TRUE,   NULL);            // reserved for future internal usage
    def_thread_local( Value*,                        reserved_value2,                TRUE,   NULL);            // reserved for future internal usage
    def_thread_local( Value*,                        reserved_value3,                TRUE,   NULL);            // reserved for future internal usage
    def_thread_local( Value*,                        reserved_value4,                TRUE,   NULL);            // reserved for future internal usage
                                                                             
    def_thread_local( Value*,                        reserved_value5,                FALSE,  NULL);            // reserved for future internal usage
    def_thread_local( Value*,                        reserved_value6,                FALSE,  NULL);            // reserved for future internal usage
    def_thread_local( Value*,                        reserved_value7,                FALSE,  NULL);            // reserved for future internal usage
    def_thread_local( Value*,                        reserved_value8,                FALSE,  NULL);            // reserved for future internal usage

    def_thread_local( DWORD,                         reserved_dword1,                TRUE,   0);               // reserved for future internal usage
    def_thread_local( DWORD,                         reserved_dword2,                TRUE,   0);               // reserved for future internal usage
    def_thread_local( DWORD,                         reserved_dword3,                TRUE,   0);               // reserved for future internal usage
    def_thread_local( DWORD,                         reserved_dword4,                TRUE,   0);               // reserved for future internal usage
                                                                                                       
    def_thread_local( DWORD,                         reserved_dword5,                FALSE,  0);               // reserved for future internal usage
    def_thread_local( DWORD,                         reserved_dword6,                FALSE,  0);               // reserved for future internal usage
    def_thread_local( DWORD,                         reserved_dword7,                FALSE,  0);               // reserved for future internal usage
    def_thread_local( DWORD,                         reserved_dword8,                FALSE,  0);               // reserved for future internal usage
#pragma warning(pop)
