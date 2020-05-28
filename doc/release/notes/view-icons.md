+ Renamed `smtk::view::IconFactory` to `smtk::view::ObjectIcons` (since it
  is specifically an icon factory for depicting the types of PersistentObject.
  You will need to replace `#include "smtk/view/IconFactory.h"` with the
  new header and calls to `smtk::view::Manager::iconFactory()` with calls
  to `smtk::view::Manager::objectIcons()`.
+ Added a new `smtk::view::OperationIcons` class (an instance of which is
  held by the `smtk::view::Manager`) that produces icons for operations.
