Attribute writer now has exclusions
-----------------------------------

The :smtk:`smtk::io::AttributeWriter` class now accepts definitions to
include (pre-existing) and exclude (new functionality).
Exclusions are processed after inclusions, so it is possible to include
a base Definition and exclude some subset of its children Definitions
for more exact pruning of which definitions and instances should be
output by the writer.
