## Changes to Attribute Resource
### Supporting Required Analyses
A smtk::attribute::Analyses::Analysis can now be marked as **required**.  A required Analysis indicates that it is not optional and is considered active if its parent analysis is active (or the analysis is at the top level).  The following methods have been added to smtk::attribute::Analyses::Analysis:

* setRequired(bool)
* bool isRequired() const

The following is a example setting an Analysis to be required via a sbt file:

```xml
    <Analysis Type="Required Analysis" Required="true"/>
```
See smtk/data/attribute/attribute_collection/SimpleAnalysisTest.sbt for a complete example.

**Note** that an Analysis's parent has Exclusive Children then the Analysis' required property is ignored.
