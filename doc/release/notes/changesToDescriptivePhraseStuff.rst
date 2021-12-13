Changes to Descriptive Phrase Functionality
-------------------------------------------

PhraseModel Changes
~~~~~~~~~~~~~~~~~~~

Deprecated Old Style Visitor Signature
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The deprecated signature returned a boolean.  The support form now returns a smtk::common::Visit Enum.

.. code-block:: c++

  // Deprecated Form
    using SourceVisitor = std::function<bool(
      const smtk::resource::ManagerPtr&,
      const smtk::operation::ManagerPtr&,
      const smtk::view::ManagerPtr&,
      const smtk::view::SelectionPtr&)>;

  // New Form
    using SourceVisitorFunction = std::function<smtk::common::Visit(
      const smtk::resource::ManagerPtr&,
      const smtk::operation::ManagerPtr&,
      const smtk::view::ManagerPtr&,
      const smtk::view::SelectionPtr&)>;

Added Map to find Descriptive Phrase by Persistent Object
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
PhraseModel now maintains a map a Persistent Object's UUIDs to a set of
Descriptive Phrases that contains that Persistent Object.  PhraseModel::trigger method now maintains the map
on Persistent Object insertion or removal.

The method PhraseModel::uuidPhraseMap returns the map.

Also added a protected method PhraseModel::removeFromMap to remove Descriptive Phrases from the map.

TriggerDataChanged Emits 1 Trigger
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Previously, this method would result in a trigger per Phrase in the model.  Now it will only trigger for the root of the model.

DescriptivePhrase Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~
Added a hasChildren method to indicate if the Phrase has / will have sub-phrases.  This is more efficient than requiring the Phrase's sub-phrases to be built in order to determine if the Phrase has children.

ObjectGroupPhraseContent Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~
Added a hasChildren method to indicate if the Phrase Content has / will have children Descriptive Phrases.  This is more efficient than requiring the Phrase's sub-phrases to be built in order to determine if the Phrase has children.

Subphrase Generator Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Added hasChildren method to indicate if the generator would create children sub-phrases for a given Descriptive Phrase.
* Added SubphraseGenerator::parentObjects method to return the Descriptive Phrases that would be parents of new phrases created for a Persistent Object.  **Note** due to performance issues with ModelResource::BordantEntities method, this method has some performance issues.
* Added SubphraseGenerator::createSubPhrase method that uses the above method to directly insert a new Phrase instead of doing a tree traversal.  Performance analysis showed that this approach is slower than the original method due to Model Resource's issues, but the approach seems sound so its been left in so that non-ModelResource Phrase Generators could use it.
* API Change

  * SubphraseGenerator::indexOfObjectInParent now takes in  a const smtk::view::DescriptivePhrasePtr& actualParent instead of a  smtk::view::DescriptivePhrasePtr& actualParent

qrDescriptivePhraseModel Changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The class no longer creates sub-phases when trying to determine if a Qt Item should indicate if it has children.  This speeds things up when dealing with large Phrase Models.

VisibilityBadge Changes
^^^^^^^^^^^^^^^^^^^^^^^
* componentVisibilityChanged no longer triggers the Phase Model.  The original trigger was not needed and caused a performance issue on large Phrase Models.
