+ Added a convenience method to `geometry::Resource` to
  fetch the first available `geometry::Geometry` object
  for use in querying geometric bounds.
+ Fixed a situation where adding a backend to `geometry::Manager`
  after resources have been added to the linked `resource::Manager`
  would not construct `geometry::Geometry` for the new backend.
