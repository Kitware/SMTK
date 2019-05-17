Note any non-backward compatible changes are in ***bOld italics***.

## Changes to Attribute Resource
### Changes to representing Analyses
Analyses are now represented by their own class instead of a collections of maps and vectors managed directly by the Attribute Resource.  The smtk::attribute::Analyses class manages a collection of smtk::attribute::Analyses::Analysis objects.

It is relatively simple to convert to the new design:

* smtk::attribute::Resource::analyses() will now return a reference to the analyses object in the resource.  All analysis-based operations now go through it
* To get all analyses - note that the return type is different!
	* Old Method
		* resource->analyses()
	* New Method
		* resource->analyses()
* To get the number of analyses
	* Old Method
		* resource->numberOfAnalyses()
	* New Method
		* resource->analyses().size()
* To create a new analysis
	* Old Method
		* resource->defineAnalysis(name, categories)
	* New Method
		* Analyses::Analysis a = resource->analyses().create(name)
		* a->setLocalCategories(categories)
* To set the parent of an analysis
	* Old Method
		* resource->setAnalysisParent(name, parentName)
	* New Method (2 ways)
		* Without getting the analysis object
			* resource->analyses().setAnalysisParent(name, parentName)
		* Through the analysis object
			* Analyses::Analysis a = resource->analyses().find(name)
			* Analyses::Analysis p = resource->analyses().find(parentName)
			* a->setParent(p) <-- if a and p are not nullptr
* To get the parent of an analysis
	* Old Method
		* resource->analysisParent(name)
	* New Method
		* Analyses::Analysis a = resource->analyses().find(name)
		* a->parent() <-- if a is not nullptr
* To get the children of an analysis
	* Old Method
		* resource->analysisChildren(name)
	* New Method
		* Analyses::Analysis a = resource->analyses().find(name)
		* a->children() <-- if a is not nullptr
* To get the local categories associated with an analysis
	* Old Method
		* resource->analysisCategories(name)
	* New Method
		* Analyses::Analysis a = resource->analyses().find(name)
		* a->localCategories() <-- if a is not nullptr
* To get the all the categories associated with an analysis including those inherited from its parent analysis
	* Old Method
		* none
	* New Method
		* Analyses::Analysis a = resource->analyses().find(name)
		* a->categories() <-- if a is not nullptr
* To get the top level analyses (those without parents)
	* Old Method
		* resource->topLevelAnalyses(name)
	* New Method
		* Analyses::Analysis a = resource->analyses().topLevel()
		* a->children() <-- if a is not nullptr
* To build an Analysis Definition representing the analyses
	* Old Method
		* resource->buildAnalysesDefinition(typeName)
	* New Method
		* Analyses::Analysis a = resource->analyses().buildAnalysesDefinition(resource, typeName)

### Exclusive Analyses
With the above changes also comes a way to indicate if an analysis' children represent analyses that can be combined by selecting a subset of them or if exclusively if only should be selected.  You can also indicate if the top level analyses are suppose to be exclusive with respects to each other.

```xml
  <Analyses Exclusive="true">
    <Analysis Type="Heat Transfer" Exclusive="true">
      <Cat>Heat Transfer</Cat>
    </Analysis>
    <Analysis Type="Enclosure Radiation" BaseType="Heat Transfer">
      <Cat>Enclosure Radiation</Cat>
    </Analysis>
    <Analysis Type="Induction Heating" BaseType="Heat Transfer">
      <Cat>Induction Heating</Cat>
    </Analysis>
    <Analysis Type="Fluid Flow">
      <Cat>Fluid Flow</Cat>
    </Analysis>
    <Analysis Type="Solid Mechanics">
      <Cat>Solid Mechanics</Cat>
    </Analysis>
  </Analyses>

```
