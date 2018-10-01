## Refactor links to use link roles more consistently.

As the use cases for resource/component links has been fleshing out, the
design for these links continues to refine to accommodate these cases. This
commit more closely weaves the use of link roles into smtk::resource::Link's
query methods, and it presents a uniform API for resource and component
links. Also, a test has been added to validate/demonstrate resource and
component linking.

### User-facing changes

The API for resource links is now more dependent on link role values,
and has been made uniform between resource and component links.
