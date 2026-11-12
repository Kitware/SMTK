ParaView Extensions
===================

Qt 5 and Qt 6 support
---------------------

SMTK's ParaView extensions now support ParaView 6 built with either Qt 5
or Qt 6. Qt 5.15 remains supported.

Developer changes
~~~~~~~~~~~~~~~~~

The Qt major version used by SMTK is controlled by ``SMTK_QT_VERSION``,
which accepts ``Auto``, ``5``, or ``6`` and defaults to ``Auto``. When
ParaView support is enabled, SMTK automatically uses the same Qt major
version as ParaView and reports a configuration error if a different version
is explicitly requested. When building with VTK but without ParaView,
automatic selection follows VTK's Qt major version. Otherwise, it selects
Qt 6 when available and falls back to Qt 5. Set ``SMTK_QT_VERSION`` to ``5``
to require Qt 5 when both versions are installed.

SMTK's installed CMake package records the selected Qt major version and
loads the corresponding Qt package for downstream projects. Projects linking
directly to Qt should use targets based on ``SMTK_QT_VERSION`` rather than
hard-coding ``Qt5`` targets.

Deprecated APIs removed by Qt 6 have been replaced by APIs that remain
compatible with Qt 5.15. Notable changes include:

* ``QRegExp`` and ``QRegExpValidator`` were replaced by
  ``QRegularExpression`` and ``QRegularExpressionValidator``.
* ``QLayout::setMargin()`` was replaced by ``setContentsMargins()``.
* Screen geometry uses ``QScreen`` where available while retaining the
  ``QDesktopWidget`` implementation required by older Qt 5 versions.
* String-based signal/slot connections were replaced by typed connections.
* Deprecated ``QVariant`` construction and keyboard-modifier arithmetic were
  replaced by their cross-version equivalents.
* Manual MOC includes are retained only for Qt 5 where they are required.

These changes are not intended to alter SMTK algorithms or application
behavior.
