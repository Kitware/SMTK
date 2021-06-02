Model resource transcription
============================

SMTK now provides a way to avoid an O(n^2) performance
issue when embedding many cells into a model;
previously, each insertion would perform a linear search
of pre-existing relationships. However, many operations
(especially those in the importer group) will not attempt
to re-insert existing relationships. The ``Model::addCell()``
and ``EntityRefArrangementOps::addSimpleRelationship()``
methods now accept a boolean indicating whether to bypass
the linear-time check.

The VTK session provides a static method,
``Session::setEnableTranscriptionChecks()``, for operations
to enable/disable this behavior during transcription.
