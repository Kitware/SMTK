Attribute Import Operation Changes
----------------------------------

Since Import's internal AttributeReader uses a Logger instance to keep track of errors encountered while doing its parsing, using the system Logger can be problematic.  The main issue is that several operations can be running at the same time and if any of them adds Errors to the Logger while Import is running, it can result in the Import operation thinking that it failed (or in the previous errors getting erased since the Attribute Reader resets the logger when it starts).

To fix this, the Import operation now passes in local Logger into the AttributeReader which is then merged with the system Logger after the Import is completed.
