## Documentation option changed

Now instead of `SMTK_ENABLE_DOCUMENTATION`, SMTK provides `SMTK_BUILD_DOCUMENTATION`,
which is an enumerated string option that can be

+ `never` — No documentation is built, and no documentation tools are required.
+ `manual` — Only build when requested; documentation tools (doxygen and
  sphinx) must be located during configuration.
+ `always` — Build documentation as part of the default target; documentation
  tools are required. This is useful for automated builds that
  need "make; make install" to work, since installation will fail
  if no documentation is built.

Our build infrastructure uses `always` but most developers will prefer `manual`.
