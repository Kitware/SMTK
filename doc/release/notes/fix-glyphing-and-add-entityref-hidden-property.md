# Fix the glyphing visibility && selection and add hidden notion to EntityRef

* The visibility control of glyphs has been fixed.
* The rendering of selected/hovered glyphs has been fixed.
* A new entry as hidden property is added to EntityRef. Developers can choose if the vtk representation for this EntityRef should be hidden from geometry generation stage or not(It can still be used for glyphing. See latter explanations). A use case is that an entity should be just used for glyphing only as its own vtk geometry representation makes no sense standalone. So it will not be added to the model multiblock source but its representation can be generated and added to the prototype multiblock source when used as a glyphing prototype. At the same time, developers can choose if the visibility control for this EntityRef should be hidden from the view control in downstream applications(Ex. CMB's model panel).
The APIs which allow developers to hide the entity from tessellation generation /andd view presentation are as below:

// Assume we have a valid EntityRef as ent
EntityRef ent = GetFromSomePlace();

// Hide ent from tessellation generation
ent.setHiddenOptionsStatus(true, HiddenOptions::HideTessellationGeneration);
// Hide ent from view presentation
ent.setHiddenOptionsStatus(true, HiddenOptions::HideViewRepresentation);

// Hide ent from all stages(For now it includes tessellation generation and view presentation)
ent.setHiddenOptionsStatus(true);

// Query all its hidden options status
int hiddenOptions = ent.hiddenOptionsStatus();

// Query a specific hidden option status. 1 means isSet and 0 means notSet.
int hideFromTessGenStatus = ent.hiddenOptionsStatus(HiddenOptions::HideTessellationGeneration);
