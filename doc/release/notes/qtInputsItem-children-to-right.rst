qtInputsItem children displayed on right
----------------------------------------

The children of a discrete qtInputsItem widget are now displayed to the right
of the combobox, rather than underneath the combobox. This change was made to
decrease wasted space in the Attribute Panel, but also to remove the primary
example case of a Qt-related bug that could squish the child input widgets
when the parent was extensible.

Developer changes
~~~~~~~~~~~~~~~~~~

This makes the Attribute Panel wider than it was before when a qtInputsItem
with children is present (but also shorter). Although this isn't deemed to
be a problem, it's something to be aware of.

User-facing changes
~~~~~~~~~~~~~~~~~~~

When a discrete value combobox had children, they used to display beneath the
combobox, like this:

.. image:: qtInputsItem-before.png

With this change, the children will now display to the right of the combobox,
like this:

.. image:: qtInputsItem-after.png
