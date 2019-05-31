## Reference Items do not hold references by default

Originally, ReferenceItems cached shared pointers to resources and
components. The use of shared pointers resulted in the undesired
behavior of resources outliving their presence in the resource
manager. There is now an option for ReferenceItems (disabled by
default) to use shared pointers to store their references. By default,
ReferenceItems now use weak pointers, allowing the referenced
resource/component to go out of scope.
