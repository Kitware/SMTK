.. _release-notes-23.01:

=========================
SMTK 23.01 Release Notes
=========================

See also :ref:`release-notes-22.11` for previous changes.


SMTK Platform and Software Process Changes
==========================================

CMake string encoding
---------------------

The way SMTK encodes the contents of files into its libraries has changed
in several regards.

Newly deprecated functions
~~~~~~~~~~~~~~~~~~~~~~~~~~

The CMake functions in ``CMake/EncodeStringFunctions.cmake`` (used by ``smtk_encode_file()``)
have been replaced by a C++ executable ``utilities/encode/smtk_encode_file.cxx`` to allow
processing of large files efficiently. These functions are now deprecated.

If you use the functions in ``CMake/EncodeStringFunctions.cmake``, be aware that these
are deprecated and will be removed in a future version. If you have small files and
wish to use C++11 literals, be aware that you can no longer use the generated
headers with a C or C++ compiler unless it supports C++11 raw string literals.

Refactored function
~~~~~~~~~~~~~~~~~~~

The ``smtk_encode_file()`` CMake macro now calls a C++ binary of the same name (whose source
is located in ``utilities/encode/``). The new utility can generate files containing

+ a python block quote (as before);
+ a C++11 raw string literal (rather than the previous, escaped C98 string constant); or
+ a C++11 function that returns a ``std::string`` (all new).

The latter change (C++11 function output) was made to accommodate encoding files
larger than 64kiB on windows, which prohibits arbitrary-length literals. The CMake
implementation of string splitting was rejected for performance reasons (CMake would
deplete its heap space trying to process files more than a few megabytes).

The new macro and executable also introduce a change in behavior;
previously, if you invoked ``smtk_encode_file("foo/bar.xml" …)`` inside the
subdirectory ``baz/xyzzy`` of your project's source, the generated file would
be at ``baz/xyzzy/bar_xml.h``. Now the path to the generated file will be
``baz/xyzzy/foo/bar_xml.h``.
This pattern is used by SMTK for operations and icons; if your external
library provides these, you will need to adjust your include directives.

Removal of previously deprecated functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Finally, the deprecated CMake functions ``smtk_operation_xml`` and ``smtk_pyoperation_xml``
from ``CMake/SMTKOperationXML.cmake`` have been removed.


SMTK Common Changes
===================

More path helper functions
--------------------------

The :smtk:`smtk::common::Paths` class now provides two additional
helper functions:

+ ``areEquivalent()`` to test whether two paths resolve to the
  same location on the filesystem; and
+ ``canonical()`` to return the absolute, canonical (free of symbolic
  links, ``.``, and ``..``) version of a path on the filesystem.


SMTK General Resource Related Changes
=====================================


SMTK Attribute Resource Changes
===============================

SMTK Graph Resource Changes
===========================

SMTK I/O Changes
================

Added the ability to "Export" ItemBlocks in Attribute Template Files
--------------------------------------------------------------------

Previously ItemBlocks were file-scope only.  This change extends ItemBlocks so that an ItemBlock defined in one
file can be consumed by another file that includes it. To maintain backward compatibility, an ItemBlock that is to
be exported must include the **Export** XML attribute and be set to *true*.  In order to better organize ItemBlocks,
SMTK now supports the concept of Namespaces.

**Note** Namespaces are only used w/r ItemBlocks and can not be nested.  To indicate an ItemBlock is defined with an specific Namespace NS,
you will need to use the **Namespace** XML attribute.  As a shortcut, if all of the ItemBlocks in a file are to belong to the same Namespace,
you can specify the **Namespace** XML attribute at the **ItemBlocks** level as well.  If no Namespace is specified, SMTK assumes the ItemBlock
belongs to the global Namespace represented by "".



SMTK Operation Changes
======================

SMTK Markup Resource Changes
============================

Markup resource ontology support
--------------------------------

A new :smtk:`smtk::markup::ontology::Source` class supports registration
of ontologies; plugins can invoke its static ``registerOntology()``
method and pass in a simple object that provides ontology metadata.
This is done rather than providing operations which create nodes for
all the identifiers in the ontology since some ontologies can be
quite large (Uberon is a 90+ MiB OWL file as of 2022-12-01) and
frequently only a few identifiers from the ontology will be used
by any particular resource.

The :smtk:`smtk::markup::TagIndividual` operation has significant
new functionality for editing tags, including querying any registered
ontology object for identifiers by name and creating instances of
them. The operation can now both add and remove ontology identifiers.

Finally, there is now a Qt item-widget – :smtk:`smtk::extension::qtOntologyItem`
used by the the updated TagIndividual operation to provide identifier
completion and description information. If your application has a
single ontology registered, the TagIndividual operation will automatically
autocomplete with identifiers from this ontology. Otherwise you will need
to subclass the operation to specify the name of the ontology to use
for auto-completion.

Resource read bugs
------------------

The :smtk:`smtk::resource::json::jsonResource` method had a bug
where a resource's location would be assigned its *previous*
location on the filesystem (i.e., where it was written to rather
than where it was read from). This behavior has been changed so
that the location is only assigned when the resource's
pre-existing location is an empty string.

Read operations are expected to set the resource's location to
match the filename provided to them.
The :smtk:`smtk::markup::Read` operation in particular has been
fixed to do this so that relative paths to geometric data are
now resolved properly.

Finding markup components by name
---------------------------------

Finding components in a markup resource by name is now supported.

This also implicitly supports searches by both type
and name since the output container type is templated
and objects will be cast to the container's value-type
before insertion.

Currently, output containers must hold raw pointers to nodes
rather than weak or shared pointers to nodes. This may
change in future versions.

SMTK ParaView Related Changes
=============================

ParaView Pipeline Sources
-------------------------

Previously, a ParaView pipeline source would be created any time
a resource was added to the resource manager owned by SMTK's
ParaView integration. This has been changed so ParaView pipelines
are only created when the resource provides a VTK renderable
geometry cache (i.e., when
``resource->geometry(smtk::extension::vtk::geometry::Backend())``
returns non-null).

This change was made to support workflows where many non-geometric
resources are loaded and the overhead of having ParaView attempt
to render them becomes significant.

ParaView plugins reorganized
----------------------------

SMTK's ParaView plugins have been reorganized into
+ a subset (``core``) which, while they may register new UI elements,
  do not introduce persistent user interface elements (panels, menus,
  toolbars) to ParaView's default interface.
+ a subset (``gui``) which do register new user interface elements
  that appear in ParaView-based applications by default.

This also involves splitting the auto-start class into two classes,
one for each set of plugins.

Going forward, if you add a plugin (or new functionality to an existing
plugin) please ensure you choose the correct target.


SMTK Python Related Changes
===========================


SMTK 3D Widget Changes
======================



SMTK UI Related Changes
=======================

Tab order in extensible group items
------------------------------------

Extensible group items are displayed in a Qt table widget instance with
one row for each subgroup. Previously, the table widget would not
respond to tab key input (typically used for navigating between cells).
This has been corrected for most mainstream cases: double items,
integer items, string items, and reference items (dropdown box format).
Tab-ordering also works with the discrete and expression versions of
value items.

There are some cases that have not yet been implemented. The overall
tab order might not be consistently preserved between the table widget
and other items before and after it in the attribute editor. For optional
items, the checkbox is not reliably included in the tab order. The tab
behavior is also not defined for discrete items that include conditional
children.

Improvements for operation views
--------------------------------

Sometimes custom items need access to the operation view which created
them. This was not possible with :smtk:`smtk::extension::qtOperationView`
because internally it created a :smtk:`smtk::extension::qtInstancedView`
which owned the item. So, we add the operation view to the instanced-view's
configuration-information type-container and add a method so items can
fetch the configuration-information type-container from their parent view.

The new :smtk:`smtk::extension::qtOntologyItem` is an example of a
qtItem intended specifically for operation views that needs to reset
its state when its operation completes (and it needs to own the lock on
the operation parameters in order to do this so it does not change the
operation's configuration while the operation running).

The operation view now provides a ``disableApplyAfterRun`` property which
custom item views may set to indicate that re-running an operation
with identical parameters is valid (or, in the case of qtOntologyItem,
indicates the item cannot know when users have altered parameters due to
deficiencies in Qt).

Removal of Deprecated Code
==========================

All code that was deprecated before prior to SMTK 22.11 has been removed.

Important Bug Fixes
===================

Fixing qtGroupItem::updateItemData Method
-----------------------------------------

Method was not properly setting the Contents Frame visibility
properly.  This has now been corrected.

Fixes for ReferenceItem widgets
-------------------------------

Neither :smtk:`smtk::extension::qtReferenceItem`
nor :smtk:`smtk::extension::qtReferenceTree` properly called
their owning-view's ``valueChanged()`` method when modified.
Now they do. **If you had to work around this issue before, be
aware that you may have to undo those workarounds to avoid
double-signaling**.

Sort newly-created phrases
--------------------------

The default :smtk:`PhraseModel <smtk::view::PhraseModel>` implementation
of ``handleCreated()`` did not sort phrases to be inserted. This has been
corrected, but **if your subphrase generator does not expect phrases to be
sorted by object-type and then title, this change will introduce improper
behavior and you will need to subclass this method to respond appropriately**.
