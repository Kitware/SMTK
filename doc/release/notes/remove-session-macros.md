## Remove smtk::model::Session-related macros

Formerly, modeling sessions came with boilerplate code used to
register session information to global static maps. This update
removes the global static session information and the associated code
used to support it.

### User-facing changes

SMTK sessions no longer use session-specific macros for global
registration.
