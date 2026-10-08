SVG icons for task diagrams and job-runner views
-----------------------------------------------

Task diagrams and job-runner views now use embedded SVG icons instead of
Font Awesome fonts. This avoids missing icons and font-loading warnings on
systems that restrict application fonts, including Windows installations
with Untrusted Font Blocking enabled. The icons are rendered directly with
Qt SVG and do not require an SVG image-format or icon-engine plugin.

Task folder indicators retain their solid and outlined appearances, follow
the diagram text color, and scale with the view. They are drawn separately
from the editable task name, so renaming a task does not include an icon
character.

Job-runner artifact settings, visibility controls, log buttons, and status
indicators also use SVGs. The control icons follow their displaying widget's
text palette as the appearance changes, including light and dark mode on
macOS. Disabled icons preserve the palette's text opacity. Status indicators
retain their status-specific colors.

The bundled Font Awesome font files and their startup registration have been
removed from SMTK's Qt extension. Applications that relied on SMTK to register
these fonts must provide their own fonts or migrate their remaining glyphs
to icons.
