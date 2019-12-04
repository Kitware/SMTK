## Changes to qtAttributePreview example program

The command line arguments were revised and expanded.
New features include:
* Model files can be loaded in addition to the attribute
  template. Model files are loaded using VTK session.
* The SMTK Operation template can be preloaded, so that
  operation templates can be loaded and displayed.

The new usage output is:

```
Usage: ./bin/qtAttributePreview [options] attribute_filename
Load attribute template and display editor panel

Options:
  -h, --help                Displays this help.
  -m, --model-file <path>   Model file (using vtk session)
  -o, --output-file <path>  Output attribute file (.sbi)
  -p, --preload-operation   Preload smtk operation template
  -v, --view-name <string>  Specific View to display

Arguments:
  attribute_filename        Attribute file (.sbt)
```
