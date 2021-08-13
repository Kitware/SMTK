.. _smtk-adaptor-classes:

Guide to adaptor classes
========================

The following sections describe the different adaptor subclasses that
exist for use in your workflows, the use cases that motivate them,
and how to configure them.

Adaptor
-------

The :smtk:`base Adaptor class<smtk::task::Adaptor>` is abstract and
cannot be instantiated on its own but does have configuration parameters
that are common to all adaptors and must be provided:

* ``id``: this is an integer identifying the adaptor.
  Although currently unused, all adaptor JSON should provide a unique,
  non-zero value as it may be used in the future to reference adaptors
  during configuration.
* ``type``: this is the fully-qualified class name of the adaptor
  class that should be created.
* ``from``: an integer used to identify the task which should be
  observed and used to configure another.
* ``to``: an integer used to identify the task which should be
  configured when the "from" task has changed into a completable
  or completed state.

Example
"""""""

.. code:: json

   {
     "id": 1,
     "type": "smtk::task::Adaptor",
     "from": 1,
     "to": 2
   }

ResourceAndRole
---------------

The :smtk:`ResourceAndRole <smtk::task::adaptor::ResourceAndRole>` adaptor
exists to pass information from GatherResources to FillOutAttributes.
When configured, only attribute resources chosen by the user will be examined
for validity.

The ResourceAndRole adaptor accepts no additional JSON configuration beyond the base
Adaptor class.

Example
"""""""

.. code:: json

   {
     "id": 1,
     "type": "smtk::task::adaptor::ResourceAndRole",
     "from": 1, /* must be a GatherResources task */
     "to": 2    /* must be a FillOutAttributes task */
   }
