import lldb


def useCountWatch(debugger, command, result, internal_dict):
    """
    Add a hardware watch point for a shared object's use count.
    Call this function like so:

       useCountWatch className classAddress

    where

       className      is the name of a class that inherits enable_shared_from_this<T>
       classAddress   is the address of an instance of that class (i.e., "this")

    Note that useCountWatch must be called **outside** the constructor of the object
    since the shared-pointer control object is not initialized or readable inside
    the constructor.

    Example:

        b smtk::resource::Resource::Resource
        c

        # Debugger stops in constructor for resource.

        p this

        # Save the address printed out, then step until
        # construction is complete.

        # Now add a watch point for the resource's use-count:
        useCountWatch smtk::resource::Resource 0x142f240f0

        # This should print a message containing the
        # watchpoint's ID as well as the actual address of
        # the shared-pointer use-count.  Save it and then
        # add commands to run at the watch point if desired.

        # The following will append a stack trace each time
        # the counter changes to the given file, assuming
        # the watchpoint ID is 1 and the use-count address
        # is 0xdeadbeef:
        wa co add 1
        > useCountTrace 0xdeadbeef /tmp/trace.txt
        > c
        > DONE

        # Now continue execution:
        c

    The example above will store a sequence of backtraces
    in /tmp/trace.txt: one for every change in use-count
    of the smtk::resource::Resource at 0x142f240f0 .
    You can then use the analyzer in stack_analysis.py
    to parse the stack traces and debug your issue.

    See the help string in stack_analysis.py for more
    information on processing the log file (/tmp/trace.txt).
    """
    args = command.split(' ')
    # Try to look up use count address (assuming libc++)
    expr = '&((%s *) %s)->__weak_this_.__cntrl_->__shared_owners_' % (
        args[0], args[1])
    ucs = 8  # use-count size (in bytes)
    # Get the address of the shared_ptr use count:
    ex = debugger.GetSelectedTarget().GetProcess().GetSelectedThread().GetFrameAtIndex(
        0).EvaluateExpression(expr)
    err = lldb.SBError()
    addr = ex.GetData().GetAddress(err, 0)
    if not err.Success() or addr == 0:
        # Try to look up use-count address (assuming libstdc++)
        expr = ' &((%s *) %s)->_M_weak_this._M_refcount._M_pi->_M_use_count' % (
            args[0], args[1])
        ucs = 4  # use-count size (in bytes)
        ex = debugger.GetSelectedTarget().GetProcess().GetSelectedThread().GetFrameAtIndex(
            0).EvaluateExpression(expr)
        err.Clear()
        addr = ex.GetData().GetAddress(err, 0)
    # Add a hardware watchpoint
    wp = debugger.GetSelectedTarget().WatchAddress(addr, ucs, False, True, err)
    if not err.Success():
        result.AppendMessage('Could not add watchpoint for %s' % str(addr))
        return
    result.AppendMessage('Watchpoint %d added for address 0x%x size %d.' %
                         (wp.GetID(), wp.GetWatchAddress(), ucs))


def useCountTrace(debugger, command, result, internal_dict):
    """
    Use this command inside a watchpoint to append
    (1) an 8-byte integer value stored at the given address and
    (2) a stack trace
    to the filename provided.
    This is useful for determining where loose or cyclic shared_ptr<>
    references are being created.

    Syntax:
        useCountTrace address filename [size]
    where
        address    is the address of the shared-pointer use count.
                   This is returned by useCountWatch.
        filename   is the name of a text file to which the information
                   listed above should be appended.
        size       is the number of bytes in the integer use-count.
                   The value you should use is reported by useCountWatch.

    See the help for useCountWatch for an example.
    """
    args = command.split(' ')
    stream = lldb.SBStream()
    err = lldb.SBError()
    proc = debugger.GetSelectedTarget().process
    stream.RedirectToFile(args[1] if len(args) > 1 else '/tmp/trace.txt', True)
    addr = int(args[0], 0)
    ucs = int(args[2]) if len(args) > 2 else 8
    ctr = proc.ReadUnsignedFromMemory(addr, ucs, err)
    if err.Success():
        stream.write('---- %8d\n' % ctr)
    else:
        stream.write('---- ## %s\n' % str(err))
    # print(dir(lldb.process.threads))
    thr = proc.GetSelectedThread()
    thr.GetDescription(stream)
    for x in range(thr.GetNumFrames()):
        thr.GetFrameAtIndex(x).GetDescription(stream)
    # We would like to continue here, but calling proc.Continue()
    # does not continue execution on exit but rather continues
    # within this block until the next breakpoint/watch (i.e.,
    # calls to continue are nested recursively until the stack
    # is exhausted).


def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(
        'command script add -f shared_ptr.useCountTrace useCountTrace')
    debugger.HandleCommand(
        'command script add -f shared_ptr.useCountWatch useCountWatch')
    # print('The "useCountTrace" and "useCountWatch" python commands have been installed.')
