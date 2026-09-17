ParaView Extensions
===================

Task layouts and job results
---------------------------

User-facing changes
~~~~~~~~~~~~~~~~~~~

Activating a task can now select or create a named ParaView layout, populate it
with named views, and enable or disable ParaView mode. Workflows can provide
different layouts for setup and results inspection, including arrangements of
multiple chart views. Existing populated layouts retain their views and splits
when revisited.

Tasks can also load results from a successful job and display selected reader
outputs in named views. Task activation continues to update visualization styles
when multiple projects are open. Job agents now preserve their output-port
configuration when saved and reloaded, allowing downstream tasks to access
the job after reopening a project. Restored job agents also reconnect their job
observers and reconcile saved agent state with the current job status. A successful
job makes its task completable even if the saved agent state was incomplete;
the task's saved completion flag is preserved.

Developer changes
~~~~~~~~~~~~~~~~~

Task-manager styles support three activation directives:

* ``paraview-mode`` is a boolean controlling ParaView mode. The older
  ``postprocessing: { "mode": true/false }`` form remains supported;
  ``paraview-mode`` takes precedence when both are present.
* ``layout`` accepts a layout name or a dictionary describing its name and
  views. View leaves specify a ParaView proxy ``type`` and an optional ``name``
  used for both the registered view name and the visible frame title. Pairs of
  child views support ``vertical`` (top/bottom) or ``horizontal`` (left/right)
  splits, an optional ``fraction`` (default 0.5), and nested arrangements.
  View templates initialize new or unsplit, empty layouts; explicit view names
  are reapplied to matching views in existing layouts. Layout selection is
  independent of ParaView mode, and both are applied before ``3d-view`` directives.
* ``job-results`` obtains a completed, successful ``smtk::job::Job`` from a task
  port and opens a results directory relative to ``Job::caseDirectory()``.
  ``port``, ``role``, and ``directory`` default to ``input``, ``job``, and
  ``postProcessing``, respectively. ``reader`` specifies a ParaView source proxy
  with a ``FileName`` property, and ``routes`` maps output-port wildcard patterns
  to view names in the selected layout. Readers created by this directive are
  shared with the job artifact controls through ``pqArtifacts``, avoiding duplicate
  pipeline sources when both interfaces open the same results directory. Previously managed results
  in the selected layout are hidden before applying new routes, including when
  a successful job or results directory is unavailable.

For example, a task style can select a layout with one named chart view:

.. code-block:: json

   {
     "paraview-mode": true,
     "layout": {
       "name": "Results",
       "views": [
         { "type": "XYChartView", "name": "Flow Results" }
       ]
     }
   }
