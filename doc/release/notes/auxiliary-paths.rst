Auxiliary file discovery
========================

``smtk::common::Paths`` now provides ``auxiliarySearchPaths()`` and
``findAuxiliaryPath(relativePath)`` for locating installed workflows, templates,
scripts, and configuration files. These auxiliary files are distinct from SMTK
Resources managed by ``smtk::resource::Manager``.

The search order is:

1. The running macOS app bundle's ``Contents/Resources/smtk`` directory,
   when a bundle is present.
2. ``../share/smtk`` relative to the directory containing the loaded SMTK library.
3. ``share/smtk`` beneath SMTK's detected top-level directory.

Search directories are normalized and deduplicated while preserving priority;
nonexistent candidates remain in the returned list. Runtime locations support
relocated installations and app bundles without embedding an install prefix.

``findAuxiliaryPath()`` searches for each requested file or directory separately,
allowing a partial app bundle to fall back to the install tree. It returns an
empty path when no match exists or when the request is empty, rooted, or escapes
the auxiliary directory through ``..``. Applications and plugins can use this
API instead of constructing platform-specific installation paths themselves.

Both methods are available in Python. Search directories and successful lookup
results are returned as ``pathlib.Path`` objects; unsuccessful lookups return
``None``.
