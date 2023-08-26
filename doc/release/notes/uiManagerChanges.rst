Added ReadOnly Mode to qtUIManager
----------------------------------

Added API to set the qtUIManager to be read-only.  Setting this to be true will
tell all the Views being displayed by the manager that no modifications should be
allowed.

This mode has been added to support the task system, where modifications need to be disallowed when a task is not active or marked completed.

Developer changes
~~~~~~~~~~~~~~~~~~

Added the following methods:

qtUIManager::setReadOnly(bool val)
bool qtUIManager::isReadOnly() const
