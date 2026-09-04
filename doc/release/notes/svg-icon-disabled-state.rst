Qt System
=========

Disabled SVG icons
------------------

Icons rendered by ``smtk::extension::SVGIconEngine`` now appear faded when
their Qt icon mode is ``QIcon::Disabled``. Previously, the custom icon engine
rendered disabled icons identically to enabled icons because it did not apply
Qt's synthesized disabled appearance. This makes disabled actions visually
distinct on both light and dark application palettes.
