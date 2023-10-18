Graph system
------------

Finding corresponding nodes along an arc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The graph resource now includes a function, ``findArcCorrespondences()``,
that takes in an arc type (as a template parameter),
a pair of nodes (say ``n1`` and ``n2``),
and a lambda that can compare nodes via the arc
(one to ``n1`` and the other attached to ``n2``).
The function then returns pairs of nodes attached to ``n1`` and ``n2``,
respectively.
If no correspondence for a node is found, then a null pointer is
stored in one entry of the pair.
