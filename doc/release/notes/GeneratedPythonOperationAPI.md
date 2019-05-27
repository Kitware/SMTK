## Updates to operation's Python API

The input parameters for SMTK's operations are described using an
instance of smtk::attribute::Attribute. To improve the API for
manipulating operation parameters in Python, we now generate
additional methods for setting & accessing operation parameters using
more intuitive method names. For example, If there is a
double-valued input to an operation with the name "foo", the Python
API for accessing this field has changed from

```
  operation.parameters().findDouble('foo').setValue(3.14159)
  foo = operation.parameters().findDouble('foo').value()
```
to

```
  operation.parameters().setFoo(3.14159)
  foo = operation.parameters().foo()
```

The original Python API for manipulating operation parameters is still
valid; this change simply introduces additional API to the same data.
