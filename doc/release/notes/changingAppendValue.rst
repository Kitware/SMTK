Changing the default behavior of smtk::attribute::ReferenceItem::appendValue()
------------------------------------------------------------------------------

The original default behavior was to uniquely append values and to do an "normal"
append when explicitly requested.  Based on how this method is used and the fact that
doing an append unique does incur a performance hit (order N-squared), the new default
behavior does a normal append.  The developer will now need to explicitly indicate that
a unique append is requested.
