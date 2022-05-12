Graph resource arc-evaluators
-----------------------------

The graph resource's  ``evaluateArcs<>()`` method has
changed the way it invokes evaluators in two ways.

1.  It will always pass a pointer (or const-pointer, depending on
    whether the resource is mutable) as an argument to your evaluator.
    The resource pointer is passed just before any forwarded arguments
    you provide to ``evaluateArcs<>()`` and is included when invoking
    your functor's ``begin()`` and ``end()`` methods.
    Note that the resource pointer will appear **after** the arc
    implementation object passed to your functor's parenthesis operator.

2. The graph resource now accepts functors with no ``begin()`` or ``end()``
   method; you need only provide a functor with a parenthesis operator.

In order to migrate to new versions of SMTK, you must change your
functors to accept this resource pointer. If you were already passing
the resource in, you may wish to remove the redundant argument and
modify places where you invoke ``evaluateArcs<>()`` to avoid passing it.
