## Remove old JSON classes

Classes `smtk::io::LoadJSON` and `smtk::io::SaveJSON` have been
removed. Their functionality is replaced by nlohmann_json bindings,
which are distributed throughout the codebase to reflect their role as
binding code.

### User-facing changes

The old json model file format has been replaced with a format more in
keeping with the internal structure of `smtk::model`.
