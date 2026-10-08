Explicit job-result directories
===============================

Task styles using ``job-results`` must now supply a nonempty ``directory``
string relative to the job's case directory. Use ``"."`` for the case directory
itself. Missing, empty, or non-string values are rejected with a diagnostic.
Previously SMTK assumed a solver-specific results subdirectory when omitted;
existing styles relying on that default must now name their results directory.

Solver-specific reader integration coverage and historical workflow examples
now live in their downstream repositories. Generic task layouts, result routing,
artifact inspection, and job monitoring remain in SMTK.
