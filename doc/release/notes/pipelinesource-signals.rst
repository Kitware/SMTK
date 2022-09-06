Pipeline Source Signals
------------------------

The `pqSMTKBehavior` class now emits a Qt signal each time it creates
or deletes a `pqPipelineSource` instance corresponding to an SMTK
resource.


```
void pipelineSourceCreated(
  smtk::resource::Resource::Ptr smtkResource, pqSMTKResource* pipelineSource);

void aboutToDestroyPipelineSource(
    smtk::resource::Resource::Ptr smtkResource, pqSMTKResource* pipelineSource);
```
