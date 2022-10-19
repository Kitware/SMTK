.. _smtk-updaters:

Update factory
==============

SMTK resources – particularly the attribute resource – can store
data that presumes or specifies a developer-provided schema in
addition to user-provided content.
As a project matures, developers must make changes to the schema
but wish to provide users with a path to migrate their content
to the new schema rather than abandoning it.

The :smtk:`update::Factory <smtk::common::update::Factory>` provides developers
with a way to register functions that can accept resources in
an old schema and copy it into a resource with a new schema,
adapting the user's content as needed.
