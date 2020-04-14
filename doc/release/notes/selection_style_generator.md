## Representation Selection Stule Customization

Add customization to `vtkSMTKResourceRepresentation` to allow plugins to over-ride how the 3D selection is displayed.
Plugins may register a `vtkSMTKRepresentationStyleSupplier` to change the selection geometry.
The RGG session plugin is using this functionality to not show the selection at all in the 3D view.
