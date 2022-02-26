ParaView Extensions
-------------------

The ParaView representation now renders components with coordinate-frame
transforms applied.
This is implemented using a new :smtk:`vtkApplyTransforms` filter;
for resources without any transforms, this simplifies to an inexpensive
copy of the input blocks.
When transforms exists, VTK's ``vtkTransformFilter`` is used which can
be expensive for large geometry.
In the future, this cost may be avoided using a custom mapper.
