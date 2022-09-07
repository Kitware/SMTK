Resource manager search
-----------------------

Now you can use :smtk:`smtk::resource::Manager::search` to find any
persistent object (whether it is a resource or component) given its
UUID. Note that this is a potentially expensive operation since all
resources held must be asked whether they own an object with the
given ID. Also, this method returns the first object found with the
given ID; if you have created a copy of a file without altering UUIDs
there may be several components with the same identifier – only one
will be returned.
