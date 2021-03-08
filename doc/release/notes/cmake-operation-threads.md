# Advanced cmake option SMTK_ENABLE_OPERATION_THREADS

We have added an advanced cmake option to enable/disable running SMTK
operations asynchronously from a thread pool. Previously, operations
were *always* run asynchronously when the `smtk::extension::qtViewRegistrar`
feature was initialized. With the new option, applications can explcitly
disable asynchronous operations by setting SMTK_ENABLE_OPERATION_THREADS
to OFF. The option is available when Qt is enabled (SMTK_ENABLE_QT_SUPPORT)
and is ON by default.
