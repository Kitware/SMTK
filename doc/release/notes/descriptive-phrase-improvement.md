## Add ObjectGroupPhraseContent and QueryFilterSubphraseGenerator

A new PhraseContent subclass is introduced which groups a set of
persistent objects for user presentation. This ObjectGroupPhraseContent
class takes resource and component filter strings plus a title and
populates its children with objects that match the filters.

A new SubPhraseGenerator subclass is introduced which generates
subphrases by querying the filter in ObjectGroupPhraseContent.

Ex. Show a list of components who has a string property as "selectable".

Now PhraseContent and DescriptivePhrase classes learn the ability
to get the undecoratedContent directly via undecoratedContent()
function.

## ComponentItemPhraseModel
Added a phrase model that uses the associatable objects method in the new smtk::attribute::Utilities class.
This method takes uniqueness into consideration.
