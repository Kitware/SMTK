Resource System
---------------

Resource Manager No Longer Adds Created Resources To Itself
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

While some signatures of ``smtk::resource::Manager::create()``
no longer added their returned resources to the resource
manager, not all of them did.
This inconsistency has been corrected such that no call to
create a resource adds it to the manager; this required
changes to several tests.
If your project relies on resources being automatically
added to the resource manager as it creates them, you will need
to change your code manually add them with ``smtk::resource::Manager::add()``
after your call to ``create()``.

Note that in proper applications (as opposed to tests or batch
scripts), you should always create resources inside an
:smtk:`Operation <smtk::operation::Operation>` and append it
to a :smtk:`smtk::attribute::ResourceItem` named "resource"
in your operation's result. This will result in the resource
being added to the manager at the completion of the operation
rather than immediately (during the operation).
The immediate addition of newly-created (and thus empty) resources
was problematic when the resource was further modified by the
operation since the the order of observations in Qt-based applications
cause the application to ignore newly-created components in the
new resource.
