Markup resource ontology support
--------------------------------

A new :smtk:`smtk::markup::ontology::Source` class supports registration
of ontologies; plugins can invoke its static ``registerOntology()``
method and pass in a simple object that provides ontology metadata.
This is done rather than providing operations which create nodes for
all the identifiers in the ontology since some ontologies can be
quite large (Uberon is a 90+ MiB OWL file as of 2022-12-01) and
frequently only a few identifiers from the ontology will be used
by any particular resource.

The :smtk:`smtk::markup::TagIndividual` operation has significant
new functionality for editing tags, including querying any registered
ontology object for identifiers by name and creating instances of
them. The operation can now both add and remove ontology identifiers.

Finally, there is now a Qt item-widget – :smtk:`smtk::extension::qtOntologyItem`
used by the the updated TagIndividual operation to provide identifier
completion and description information. If your application has a
single ontology registered, the TagIndividual operation will automatically
autocomplete with identifiers from this ontology. Otherwise you will need
to subclass the operation to specify the name of the ontology to use
for autocompletion.
