Visitors
========

SMTK now provides an enumeration, ``smtk::common::Visit``, that visitor lambdas
may return to indicate whether visitation should continue (``smtk::common::Visit::Continue``)
or stop (``smtk::common::Visit::Halt``).
This enum is much easier to use than simply returning a ``bool`` as developers
frequently have trouble remembering which value (true or false) corresponds to
which iteration behaviour.

This is currently only used by ``smtk::task::Task::visit()``, but will be
used more widely in the future.
