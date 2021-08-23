Replacing qtBaseView::getObject()
---------------------------------

This method returns the instance's view::Configuration but the name didn't reflect that.  Also the getObject method returned a smtk::view::ConfigurationPtr which means that a new shared pointer was always being created and as a result, incrementing its reference count.  The new configuration() method returns a const smtk::view::ConfigurationPtr& instead which does not effect the shared point reference count.

Developer changes
~~~~~~~~~~~~~~~~~~
* getObject method is now deprecated.
