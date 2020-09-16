Generator
=========

SMTK's generator pattern (:smtk:`Generator <smtk::common::Generator>`)
is similar to a factory, but with several key differences:

1.  The generated instances are registered to the static list of
generator types at compile time.

2. the API does not expose a means of selecting exactly which
registered generator instance is used to satisfy a call. Instead, each
generator instance attempts to consume the input in sequence.

An instance of the generator can reject an input at either the
`valid()` check or during the call operator's execution (where it can
throw an exception to trigger the next generator to attempt to consume
the input).

An example that demonstrates the prinicples and API of this pattern
can be found at `smtk/mesh/interpolation/PointCloudGenerator.h`.
