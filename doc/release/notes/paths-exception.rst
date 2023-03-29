Common directory utility
------------------------

Previously, the :smtk:`smtk::common::Paths::directory` method would
throw a boost filesystem exception if a user called it without
permission to the path passed to it. This has been fixed by capturing
the exception internally.
