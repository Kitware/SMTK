Enable QT_NO_KEYWORDS builds
----------------------------

SMTK compile definitions now include QT_NO_KEYWORDS, which means all uses of
`emit`, `foreach`, `signals`, and `slots` has been replaced with
`Q_EMIT`, `Q_FOREACH`, `Q_SIGNALS`, and `Q_SLOTS`, and must be updated for
any SMTK plugins that use the `smtk_add_plugin` cmake macro. This is to avoid
conflicts with 3rd-party libraries, like TBB, which use the old keywords.
