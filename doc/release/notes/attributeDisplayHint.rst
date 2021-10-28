Display hint for attribute resources
------------------------------------

The attribute read operation and XML import operations now
mark old resources (version 4 or older) with a property,
``smtk.attribute_panel.display_hint``, upon load.
Newer files (version 5 or newer) must explicitly contain this hint.
The ParaView attribute panel uses (and other UI elements may use)
this property determine whether a loaded attribute should be immediately displayed
in the panel upon being added to a resource manager.

Furthermore, when the attribute panel displays a resource,
it removes the property from any prior resource and adds it
to the new resource to be displayed.
Thus saving a project with several resources will resume upon reload
with the same attribute resource displayed in the editor.

This change to SMTK also removes the default behavior that
displays attribute resources when the ParaView pipeline browser's
selection changes.
This change was made because the pipeline-browser's selection
changes when (a) a user or python script modifies the active
pipeline source or (b) a new pipeline source is created.
The second case was causing many updates to the attribute panel
when projects with many resources are loaded/created.
Now, this behavior is only enabled in "ParaView mode" (i.e., when
post-processing is enabled).
To reduce the impact of this change, we have also added a (default)
option to :smtk:`pqSMTKPipelineSelectionBehavior` that will display
attribute resources as they are selected (specifically: when the SMTK
selection holds a single entry in its primary selection and that
entry is an attribute resource marked as displayable).

Finally, this change corrects the sense of `smtk::attribute::Resource::isPrivate()`,
which was previously reversed.

Developer changes
~~~~~~~~~~~~~~~~~~

File version numbers have increased to prevent breaking changes in behavior.
Old files (XML version 4 or JSON version) will always mark this property to
preserve existing behavior.
New files must enable the hint explicitly.

If you used `smtk::attribute::Resource::isPrivate()` or
`smtk::attribute::Resource::setIsPrivate()` in your code, you should
reverse the sense of values obtained-from/passed-to these methods.
It now returns true when a resource is private (i.e., should not be
shown) and false when a resource is public.

User-facing changes
~~~~~~~~~~~~~~~~~~~

New files – depending on how they were created – may not appear immediately
in the attribute editor.
Also, files saved to the new format will "remember" which one was showing
in the attribute editor.
