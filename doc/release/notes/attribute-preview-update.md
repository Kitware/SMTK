# Update to qtAttributePreview example

The qtAttributePreview example application was updated to support
loading model files if built with the VTK session. The command line
arguments were also updated for convenience. The new usage is:

```
Usage: ./bin/qtAttributePreview [options] attribute_filename
Load attribute template and display editor panel

Options:
  -h, --help                Displays this help.
  -o, --output-file <main>  Output attribute file (.sbi)
  -m, --model-file <main>   Model file (using vtk session)
  -v, --view-name <main>    Specific View to display

Arguments:
  attribute_filename        Attribute file (.sbt)
```
