Job Subsystem
=============

ShellQueue interpreter and Windows support
------------------------------------------

``smtk::qt::job::ShellQueue`` can now run job scripts using an optional
interpreter. Applications may configure the interpreter executable, arguments,
and process environment with ``setInterpreter()``,
``setInterpreterArguments()``, and ``setProcessEnvironment()``, respectively.
The interpreter and its arguments are also exposed through Python bindings.
When no interpreter is configured, scripts continue to be executed directly.

This enables native Windows applications to run Bash job scripts using an
application-provided Bash installation, such as the MSYS2 Bash distributed
with OpenFOAM. On Windows, the default shell queue resolves ``bash.exe`` from
``PATH`` and invokes scripts with ``--noprofile --norc``; applications may
replace this with an absolute interpreter path.

ShellQueue now retains each job's ``QProcess`` instead of launching a detached
process. Startup failures and process exit status are reflected in job state,
and job launches requested by worker operations are transferred safely to the
queue's Qt thread. On Windows, cancellation uses a Windows Job Object to
terminate the interpreter and its descendant process tree. If Job Object
assignment is unavailable, ``taskkill /T /F`` is used as a fallback. This is
important for shell scripts that launch solver or MPI child processes.

Running shell jobs are no longer terminated when an SMTK application exits.
When the application restarts, ``ShellQueue`` reloads persisted jobs, restores
their project/task origin links as resources become available, and resumes
monitoring each active case's ``logs/progress`` file. Explicit cancellation is
still supported after a restart using the persisted process identifier; on
Windows, this uses ``taskkill /T /F`` to terminate the recovered process tree.
