Code style
==========

* No tabs or trailing whitespace are allowed.
* Indent blocks by 2 spaces.
* Class names should be camel case, starting with an uppercase.
* Class member variables should start with :cxx:`m_` or :cxx:`s_` for per-instance or class-static variables, respectively.
* Class methods should be camel case starting with a lowercase character (except acronyms which should be all-uppercase).
* Use shared pointers and a static :cxx:`create()` method for classes that own significant storage or must be passed by
  reference to their superclass.
