ParaView Extensions
===================

Honor the configured container engine path
-----------------------------------------

Container job queues now honor the **Container Engine Path** setting after
saved settings load and when users apply changes to the setting. Previously,
the configured path could appear in Preferences while queues continued to use
the default ``podman`` command.

This fixes container job startup in macOS applications launched from Finder
when Podman is configured with an absolute path but is absent from the
application's ``PATH``. Clearing the setting restores the default ``podman``
command. Pending edits do not affect queues until applied.
