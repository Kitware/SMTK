# SMTK utilities for debuggin with lldb

## Shared-pointer tracking in LLDB

Add this line to your `~/.lldbinit` file:

    command script import /path/to/smtk/utilities/lldb/shared_ptr.py

When you start lldb, it should print a notice
that 2 new commands (useCountWatch and useCountTrace)
have been added. You can use these to print
backtrace information to a log file for later analysis
of where a shared-pointer's use-count has changed (and
of most interest, what still owns a shared pointer that
is keeping some object alive that you want destroyed).

Type `help useCountWatch` or `help useCountTrace` in
lldb for help on how to use the commands.

See `utilities/lldb/stack_analysis.py` for information
on how to use the log file.
