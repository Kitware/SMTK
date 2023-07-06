Third-party dependencies
------------------------

PEGTL
~~~~~

SMTK will now build with either PEGTL v2.7.1 or v2.8.3.
Our continuous integration machines have had their superbuild
upgraded to v2.8.3 to fix an issue with parse-trees that would
lead to duplicate terminal nodes. Using an version of PEGTL
older than v2.8.3 could lead to incorrect units.
