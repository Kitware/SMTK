Inspecting partial job artifacts
===============================

JobRunner artifact controls now allow inspection of existing artifacts from a
stopped job even when the artifact's stage did not finish. The view watches
artifact directories so newly created reader markers become available without
switching tasks. Restored job views also refresh when job operations report
progress or completion.

Applications can create reader markers declared by a stopped job to expose
partial results without changing its success status.

Initial artifact updates run after the job view finishes constructing its internal
state, so reopening a project with an existing job does not access uninitialized
view state. A GUI regression test covers restored successful, failed, and running
jobs, including availability of partial artifacts after failure.
