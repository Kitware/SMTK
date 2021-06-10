Changes to smtk::project::ResourceContainer API
============================

Changes to the ``smtk::project::ResourceContainer`` API to  allow for non-unique roles
to be assigned to Resources in a project.

Deprecated version >= 21.6
``smtk::project::ResourceContainer::getByRole -> smtk::resource::ResourcePtr``

New API
``smtk::project::ResourceContainer::findByRole -> std::set<smtk::resource::ResourcePtr>``
