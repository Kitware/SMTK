=============================
Implementing an STMK operator
=============================

.. contents::
.. highlight:: c++
.. role:: cxx(code)
   :language: c++

SMTK allows you to write operators that expose those provided
by an underlying modeling kernel as well as operators that
provide new functionality.

This tutorial will cover writing an operator that provides
new functionality: specifically, we will create an operator
that counts the number of top-level cells in a model.
Given a non-default option, it may instead count the number
of top-level groups in a model.

Operators in SMTK consist of 3 components:

- a class that implements the action of the operator, as
  a subclass of :smtk:`smtk::model::Operator`.

- an SMTK attribute definition that describes the parameters
  the operator accepts (both required and optional)

- an SMTK attribute definition that describes information
  returned by the operator.

The sections below detail each of these.

*********************************
Subclassing smtk::model::Operator
*********************************

We will name our example operator the CounterOperator and
place it in the "ex" example namespace.

.. literalinclude:: implement_an_operator.h
   :start-after: // ++ 1 ++
   :end-before: // -- 1 --
   :linenos:

Our operator is a subclass of :smtk:`smtk::model::Operator`, which
is managed using shared pointers (it uses the :smtk:`smtkEnableSharedPtr` macro
to inherit :cpp:class:`enable_shared_from_this`),
so we use the :smtk:`smtkCreateMacro` to provide a static method for
creating a shared pointer to a new instance and
the :smtk:`smtkSharedFromThisMacro` to override the base
class' :c:func:`shared_from_this` method.
Finally, we invoke the :smtk:`smtkDeclareModelOperator`.
This declares methods for accessing the operator's class-name and
human-readable name.

Beyond these basic requirements, an operator should implement
two inherited virtual methods

+ :smtk:`ableToOperate <Operator::ableToOperate>` which is an opportunity for an
  operator to perform checks on the validity of input parameters
  that cannot be easily encoded using the attribute system; and

+ :smtk:`operateInternal <Operator::operateInternal>` which implements the actual
  behavior of the operator.

Because our operator is simple, we implement :smtk:`Operator::ableToOperate`
in the class header file. We only declare :smtk:`Operator::operateInternal`:

.. literalinclude:: implement_an_operator.h
   :start-after: // ++ 2 ++
   :end-before: // -- 2 --
   :linenos:

By calling :smtk:`Operator::ensureSpecification` in :smtk:`Operator::ableToOperate`,
we force the attribute system to build an attribute instance which
holds specifications for each parameter of this instance of the
Operator.
Then in :smtk:`Operator::operateInternal` we can refer to the specification
without having to verify that it is non-null; an operator's
:smtk:`Operator::operateInternal` method will never be called unless
:smtk:`Operator::ableToOperate` returns true.

The attribute constructed by :smtk:`Operator::ensureSpecification` is built
using XML definitions we provide.
We will cover the format of the XML definitions immediately below
and then continue with the implementation of the operation.

**********************************
Defining operator input parameters
**********************************

The XML we used to declare the operator's parameters and
results uses SMTK's attribute definition system.
All operators should be *derived* definitions whose base
definition (BaseType) is "operator":

.. highlight:: xml
.. literalinclude:: implement_an_operator.xml
   :start-after: <!-- +1+ -->
   :end-before: <!-- -1- -->
   :linenos:
.. highlight:: c++

Inheriting the base definition allows us to use to attribute
system to easily enumerate the list of all operators.

Also, you can see in the example that our operator takes a
single integer parameter named "count groups".
Its default value is 0, indicating that top-level cells (not groups)
will be counted by default.
We could also have made "count groups" a :smtk:`VoidItem` and tested
for whether the attribute was enabled instead of testing
its value.

In the future, operators will have their "primary" operand
expressed as an *association*:
model topology to serve as the primary operand will be associated
with an instance of the operator rather than declared as an item
owned by the operator attribute.
This will simplify use cases where an active selection exists
and the user wishes to perform an operation on it;
operators that can be associated with the selection will be
enabled (while others will be disabled).
Any further parameters may be specified after the user initiates
the operation.

***********************************
Defining operator output parameters
***********************************

The XML description of an operator is not complete until both the
input and output parameters have been specified.
The output parameters are expressed as another
:smtk:`smtk::attribute::Attribute` instance, this time defined as
by inheriting the "result" BaseType.
The result base type includes an integer item named "outcome"
use to store one of the values in the :smtk:`OperatorOutcome` enum.
The outcome indicates whether the operation was successful or not.

.. highlight:: xml
.. literalinclude:: implement_an_operator.xml
   :start-after: <!-- +2+ -->
   :end-before: <!-- -2- -->
   :linenos:
.. highlight:: c++

In addition to the outcome, our edge-counting operator also
returns the number of edges it counted.
Often, SMTK operators will return lists of new, modified, or
removed model entities in one or more :smtk:`smtk::attribute::ModelEntityItem`
instances.

Both the input and output XML are typically maintained
together in a single XML file.

*********************************
Implementing the actual operation
*********************************

Now that we have input and output parameters specified,
the implementation of :smtk:`Operator::operateInternal` can simply
fetch items from an instance of an attribute defined by the XML:

.. literalinclude:: implement_an_operator.cxx
   :start-after: // ++ 2 ++
   :end-before: // -- 2 --
   :linenos:

Note that the base class provides a method to create
a result attribute for you with the "outcome" parameter
set to a value you specify.

In addition to implementing the operation, the only other thing
you must do is register the operator with the proper bridge.
This is done using the :smtk:`smtkImplementsModelOperator` macro:

.. literalinclude:: implement_an_operator.cxx
   :start-after: // ++ 3 ++
   :end-before: // -- 3 --
   :linenos:

.. WARNING::
  This macro must always be invoked in the global namespace
  and your operator's class name fully qualified with the
  namespace in which it lives.

This macro is very important because it ties several
names together:

+ the C++ class name (:cpp:class:`ex::CounterOperator`),

+ the names and XML descriptions of the attribute definitions
  for input parameters ("counter") and result parameters
  "result(counter)", and

+ the SMTK component name used by the autoinitialization
  facility to force components (such as this operator) to
  be registered when initializing static variables.
  In this case the component name "ex_counter" can be
  used with the :smtk:`smtkComponentInitMacro` by adding
  this line:

  .. code-block:: c++

     smtkComponentInitMacro(smtk_ex_counter_operator)

  to a compilation unit containing code that will be run
  by your application. Whenever the compilation unit's
  static variables are initialized, the operator will be
  registered with the bridge class and any bridges constructed
  afterwards will provide the operator.

  Note that "smtk\_" and "\_operator" wrap the name you pass
  to the :smtk:`smtkImplementsModelOperator` macro.

To make maintaining the XML simpler, SMTK provides a macro for
encoding an XML file as a variable in a C++ header.
The variable :c:data:`implement_an_operator_xml` is generated in
CMakeLists.txt by

.. highlight:: cmake
.. literalinclude:: CMakeLists.txt
   :start-after: # ++ 1 ++
   :end-before: # -- 1 --
   :linenos:
.. highlight:: c++

and then used by including the resulting file in your
C++ implementation:

.. literalinclude:: implement_an_operator.cxx
   :start-after: // ++ 1 ++
   :end-before: // -- 1 --
   :linenos:
