Task System
-----------


Changes to task::Adaptor API
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The signature for ``task::Adaptor::reconfigureTask()`` has changed.
Adaptors must now provide a method that takes two arguments:
the state of the upstream task before and after the current event.
This information is now passed so that adaptors can fire on any
transition between task states, not just from non-completable to completable.

If you have written a custom adaptor, you will need to change its
signature. An easy way to maintain your adaptor's current behavior
with the new API is to change your class (Foo) from something like this:

.. code:: cpp

   class Foo : public smtk::task::Adaptor
   {
   public:
     // …
     bool reconfigureTask() override
     {
       bool didChange = false;
       // Update this->to(),setting didChange on modification.
       return didChange;
     }
   };

to the following (by adding arguments and a small early-exit conditional
to your existing implementation):

.. code:: cpp

   class Foo : public smtk::task::Adaptor
   {
   public:
     // …
     bool reconfigureTask(smtk::task::State prev, smtk::task::State next) override
     {
       if (
         next < smtk::task::State::Completed ||
         prev > smtk::task::State::Completable)
       {
         return false;
       }
       // Update this->to().
       return didChange;
     }
   };
