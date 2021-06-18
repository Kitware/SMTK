
SMTK 21.05 Release Notes
=========

See also `SMTK 21.04 Release Notes`_ for previous changes.

.. _`SMTK 21.04 Release Notes`: smtk-21.04.md


Changes to Project Subsystem
------

A major revision was undertaken to the ``smtk::project`` namespace that is not
backward compatible. The previous Project class was hard-coded to a use single
attribute resource and one or two model resources. The new Project class is an
SMTK resource that can support any number of resources including custom ones,
with assigned roles. The new Project class can optionally be scoped to a list
of available operations and resource types. New Project instances store and
retrieve their contents as atomic operations with the file system.
Projects can now be described entirely using Python, making it
easier to implement custom project types for different solvers.

More information is available in the SMTK user guide and there is also a new
`create_a_project tutorial <https://smtk.readthedocs.io/en/latest/tutorials/create_a_project/index.html>`_.

ParaView-extension pipeline-creation changes
-----

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
~~~~

If you have code external to SMTK that invokes
:cxx:`pqSMTKRenderResourceBehavior::instance()->createPipelineSource()`,
you should eliminate it, but be aware that you cannot rely on the pipeline
source being present or initialized at the time many observers are invoked.

Graph-resource ArcMap
------

The graph resource now provides a custom subclass of TypeMap named smtk::graph::ArcMap.
This subclass deletes the copy and assignment constructors to prevent modifications
to accidental copies of arcs rather than to the resource's arc container.

External changes
~~~~~~~~~

smtk::graph::ResourceBase::ArcSet type-alias was removed and
the ArcMap class can now be used directly.

If you maintain external code that depends on smtk::graph::Resource, you will
need to replace the ResourceBase::ArcSet type-alias with ArcMap (which lives
in smtk::graph not smtk::graph::ResourceBase).

CMake package directory
-----

The CMake package directory for SMTK is now in a location that CMake searches
by default. This removes the need to do ``-Dsmtk_DIR`` and instead the install
prefix can be given in the ``CMAKE_PREFIX_PATH`` variable.
