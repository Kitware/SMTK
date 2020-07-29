## Changes to Resource
### Removed `smtk::resource::Set`
`smtk::resource::Set` has been removed from SMTK. It's original purpose was to
hold a collection of resources for export. This functionality has been
superceded by Project.

### Added `smtk::resource::query::Manager`
`smtk::resource::query::Manager` is an interface for registering Query types.
SMTK Plugins can interact with `smtk::resource::query::Manager` by adding
methods similar to the following to their Registrar:

```
registerTo(const smtk::resource::query::Manager::Ptr& manager)
{
  manager->registerQuery<ApplicableResourceType, MyQuery>();
}

unregisterFrom(const smtk::resource::query::Manager::Ptr& manager)
{
  manager->unregisterQuery<MyQuery>();
}
```

Additionally, the `smtk_add_plugin()` call for the plugin should be extended
to include `smtk::resource::query::Manager` in its list of managers.
Upon registration, all appropriate resources associated with the same
resource manager as the `smtk::resource::query::Manager` will be able to
construct instances of the newly registered Query.
