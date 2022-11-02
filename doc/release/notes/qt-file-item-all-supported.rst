Qt File Item Changes
--------------------

The QFileDialog created by qtFileItem now includes an "All supported types" entry
as the first set of file extensions when a :smtk:`smtk::attribute::FileItemDefinition`
is marked to accept existing files. On many platforms, this simplifies browsing since
users no longer have to select a specific file-type of interest before they are shown
all acceptable files.

This is achieved using a new ``FileItemDefinition::getSummarizedFileFilters()``
method that is available for you to use in your custom applications as well.
