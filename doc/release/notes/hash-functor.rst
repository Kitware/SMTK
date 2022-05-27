Support for old hash namespace conventions dropped
--------------------------------------------------

SMTK now requires compilers to support specializations
of hash inside the ``std`` namespace.
Previously, SMTK accepted specializations in non-standard
locations such as the global namespace, in the ``std::tr1``
namespace, and others.

Developer notes
~~~~~~~~~~~~~~~

If your repository uses any of the following macros, use the
replacements as described:

+ ``SMTK_HASH_NS`` should be replaced with ``std``
+ ``SMTK_HASH_NS_BEGIN`` should be replaced with ``namespace std {``
+ ``SMTK_HASH_NS_END`` should be replaced with ``}``
+ Any code enabled by ``SMTK_HASH_SPECIALIZATION`` should be unconditionally
  enabled and any code disabled by this macro should be removed.
