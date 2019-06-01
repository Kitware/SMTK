## Improvements to thread safety for operations

qtOperationLauncher has been updated to fix a bug where operation
results were not correctly propagated to the right Qt
signals.

Operations now lock their specifications whenever they interact with
them. We have also added a thread-safe means of generating unique
names for parameters and results.
