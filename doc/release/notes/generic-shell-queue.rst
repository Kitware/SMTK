Generic local shell queue
=========================

The default ``shell_queue`` no longer discovers or initializes a solver-specific
installation on Windows. It resolves ``bash.exe`` using ``PATH`` and passes
``--noprofile --norc``. Applications can supply an interpreter, arguments, and
process environment through the existing ``ShellQueue`` API or a derived queue.
Process execution, monitoring, persistence, and cancellation remain in SMTK.
