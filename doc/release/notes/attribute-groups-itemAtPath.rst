Attribute `itemAtPath()` extended for groups
--------------------------------------------

The attribute resource's method `itemAtPath()` has been extended to
recognize `#N` as a path component representing sub-group number, where
`N` is the index of the sub-group. Python operation tracing will
generate paths using this notation.
