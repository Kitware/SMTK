# Icons for SMTK's application components

Some of these icons were generated from SVG files.
To regenerate them after editing the SVG file, run inkscape like so:

```sh
  inkscape -w 64 -h 64 --export-png=Instances@2x.png instance_nobg.svg
```

A script that does the above for each of several icons is
named `generate-icons.sh`.

Be sure that the document page size is set properly to be
an exactly square area with enough border padding that
the icons will not be trimmed should Qt embed them in
buttons without rounded corners.
