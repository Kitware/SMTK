## Mesh system changes

### Smaller API changes

+ Now that `smtk::resource::Resource` holds a separate name
  (i.e., independent of location), remove the API that mesh
  resources provided to avoid duplication.
