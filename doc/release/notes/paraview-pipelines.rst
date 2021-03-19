ParaView-extension pipeline-creation changes
============================================

We have moved responsibility for creating pqSMTKResource proxies
(which are ParaView pqPipelineSource subclasses) from consumers
like the VisibilityBadge and operation panel to pqSMTKRenderResourceBehavior.
This changes some initialization since proxies will not have an SMTK
resource assigned to them at the time many signals are emitted.
However, it simplifies things for consumers and fixes an issue
where duplicate resources were being created.

This adds storage to pqSMTKBehavior to avoid :math:`O(n^2)` computational
complexity when many resource-proxies are added to a paraview/modelbuilder client.

Note that we use :cxx:`QTimer::singleShot()` to delay creation
of the pipeline until outside of the resource-manager observer
(to avoid deadlocks but also to avoid duplicate pipelines created
by other SMTK-PV behaviors that make empty pipelines (with no SMTK
resource until after the resource has been added to the manager).
This happens, for example, with ParaView's File→Open.

Finally, because resource proxies may be re-assigned a (different)
SMTK resource, we monitor each proxy and update the lookup map as needed.

External-facing changes
-----------------------

If you have code external to SMTK that invokes
:cxx:`pqSMTKRenderResourceBehavior::instance()->createPipelineSource()`,
you should eliminate it, but be aware that you cannot rely on the pipeline
source being present or initialized at the time many observers are invoked.
