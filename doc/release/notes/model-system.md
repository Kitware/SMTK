## Model system changes

### Resource and Entity API

+ The model resource's `queryOperation()` method is now implemented in
  Entity.cxx and includes the ability to filter entities by their type
  bits (previously available) and their property values (new functionality).

### Operations

+ The "assign colors" operation now provides a way to set opacity independently
  of or in tandem with colors. The user interface takes advantage of this to
  provide an opacity slider. Since all model-entity colors have been stored as
  a 4-component RGBA vector, the model representation now properly sets block
  opacities.
