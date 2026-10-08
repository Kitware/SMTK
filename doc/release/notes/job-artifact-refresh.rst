ParaView Extensions
===================

Refresh displayed artifacts as job stages complete
-------------------------------------------------

User-facing changes
~~~~~~~~~~~~~~~~~~~

Artifacts already loaded in ParaView now refresh automatically when the job
stages that produce them finish successfully, including when a task reruns a
job. Existing visibility, display settings, and downstream filters are preserved.
Artifacts that have not been loaded remain unloaded.

Only artifacts belonging to newly completed stages are refreshed. Repeated
progress updates and final job completion do not reload earlier stages again.
If several stages finish between progress updates, their artifacts are refreshed
together. An artifact shared by multiple stages is refreshed again when a later
stage that declares it finishes.

Developer changes
~~~~~~~~~~~~~~~~~

Job stages must declare every artifact they produce or modify so that the
corresponding cached sources can be refreshed. Stage tracking is independent
of task-view lifetime and resets when a job is rescheduled, including when the
job retains its UUID.

``pqArtifacts::reload(job, firstStage, endStage)`` reloads existing sources for
the zero-based, half-open stage range ``[firstStage, endStage)``. Sources shared
by stages in the range are reloaded once per call, including hidden sources.
The method does not create sources or prompt for file-series expansion.

Regression testing
~~~~~~~~~~~~~~~~~~

The new ``TestArtifactStageTracker`` CTest test verifies the half-open stage
ranges selected for artifact refresh using a synthetic three-stage job. It covers:

* No refresh when a job is scheduled or its first stage starts.
* Refresh of only newly completed stages, including skipped intermediate updates.
* Suppression of duplicate refreshes on repeated progress and success notifications.
* Resetting tracking when the same job UUID is rescheduled.
* Refresh of remaining stages when success arrives without final stage progress.
* Independent tracking of different jobs and removal of a job's stored count.
* Exclusion of failed stages, refresh of stages completed before cancellation,
  and suppression of repeated failure and cancellation notifications.

This unit test checks stage selection only. It does not exercise operation-observer
dispatch, file loading, ParaView reader reloads, representation settings, or rendering.
Run it from the SMTK build directory with
``ctest --output-on-failure -R '^TestArtifactStageTracker$'``.
