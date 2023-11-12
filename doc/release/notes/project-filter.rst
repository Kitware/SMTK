Project system
--------------

Now :smtk:`smtk::project::Project`'s ``queryOperation()`` method
supports component type-name and property queries.
This can be used to fetch tasks and worklets by type.
Note that the inheritance hierarchy of components is not available
to the query system at this point, so you *cannot*, for example,
expect a query on ``smtk::task::Task`` to return objects of
type ``smtk::task::FillOutAttributes`` even though they inherit
that type.
