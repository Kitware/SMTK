Attribute Builder (Python, experimental)
========================================

An extension to the Python API is available for editing attribute components
from a dictionary-style specificiation. The API is implemented as a single
Python class :py:class:`smtk.attribute_builder.AttributeBuilder`. The initial
use for this feature is for replaying Python operation traces. To support this,
the following method is provided:

.. method:: smtk.attribute_builder.AttributeBuilder.build_attribute(self, att: smtk.attribute.Attribute, spec: dict:[str, list], resource_dict:[str, smtk.resource.Resource]=None) -> None

This method modifies the contents of the input attribute based on the input
specification. In addition to the attribute and specification arguments, the
:func:`builder_attribute()` method includes an optional :py:attr:`resource_dict`
argument for specifying SMTK resource instances that can be referenced in the
specification dictionary.


Specification
-------------

The input specification is a Python dictionary object with two fields for
specifying a list of :py:attr:`associations` and a list of :py:attr:`items`.
Here is an example of the specification format:

.. code-block:: python

    {
      'associations': [{'resource': 'model'}],
      'items': [
          {'path': 'string-item', 'value': 'xyzzy'},
          {'path': '/double-item', 'value': [3.14159, None, 2.71828]},
          {'path': 'int-item', 'enabled': True, 'value': 2},
          {'path': 'int-item/conditional-double', 'value': 42.42},
          {'path': 'comp-item', 'value': [
              {'resource': 'model', 'component': 'casting'},
              {'resource': 'model', 'component': 'symmetry'},
          ]},
          {'path': '/group-item/subgroup-double', 'value': 73.73},
      ]
  }

.. note:: The current implementation will raises a Python exception if it encounters a syntax error in the specification or if some editing step fails.

Associations List
^^^^^^^^^^^^^^^^^

Attribute associations are specified by a list of dictionary items. Each item
includes a :py:attr:`resource` key with its value set to one of the keys in the
resource dictionary (passed in to the :py:meth:`build_attribute` method).
This is sufficient to specify a resource association.
To specify a component belonging to a resource, a :py:attr:`component` field
is added to specify the name of the component.


Items List
^^^^^^^^^^

Item modifications are specified by a list of dictionary items. The fields for
each dictionary item can include :py:attr:`path` (required),
:py:attr:`enabled`, and :py:attr:`value`.

* The :py:attr:`path` field specifies the path from the attribute to the item being specified. It uses the same syntax as the :py:meth:`smtk.attribute.Attribute.itemAt()` method, with forward slash as the separator and the :py:attr:activeOnly argument set to :py:const:false. (Starting the path with a forward slash is optional.)

* The :py:attr:`value` field is for setting the item value. This can be a single value or a list of values. The type (string, int, float) and number of values must be consistent with the item and its corresponding item-definition, of course. For :py:class:`ReferenceItem`, :py:class:`ComponentItem`, and :py:class:`ResourceItem`, the value is specified using the same syntax used in the :py:attr:`associations` list. If the value, or any of the values in a list, are :py:const:`None` the corresponding item value will be :py:attr:`unset`.

* The :py:attr:`enabled` field is for setting the item's enabled state.
