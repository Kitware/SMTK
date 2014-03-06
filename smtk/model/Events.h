#ifndef __smtk_model_Events_h
#define __smtk_model_Events_h


#include "smtk/PublicPointerDefs.h" // For Cursor and CursorArray
#include <utility> // For std::pair

namespace smtk {
  namespace model {

/**\brief Enumerate the types of changes storage has.
  *
  * The change type serves as a modifier to the relationship.
  * Not all relationships support all of the modifiers.
  * Specifically, ENTITY_ENTRY is never paired with
  * MODIFIED_EVENT because there are many other signals
  * that deal with entity modifications in a more fine-grained
  * sense (e.g., GROUP_SUPERSET_OF_ENTITY is one way in which
  * an smtk::model::Entity record can be modified).
  *
  * Events that affect arrangements but do not otherwise
  * affect storage are typically reported with
  * ADD_EVENT notifications generated before addition
  * and DEL_EVENT notifications after removal.
  * The rationale is that (1) observers should have
  * direct access to the most difficult-to-infer
  * configuration while the event type specifies enough
  * information to infer the simpler configuration and
  * (2) observers may wish to perform other operations
  * after removal to maintain the consistency of Storage.
  *
  * Events that add new smtk::model::Entity, smtk::model::Tessellation,
  * smtk::model::FloatData, smtk::model::StringData, or
  * smtk::model::IntegerData are typically reported
  * with ADD_EVENT notifications generated after addition
  * and DEL_EVENT notifications before removal.
  * The rationale is that observers should have a chance to
  * access the objects being affected (after they are added
  * or before they are removed).
  *
  * @sa StorageEventRelationType
  */
enum StorageEventChangeType
{
  ADD_EVENT,  //!< The event is a relationship being added.
  MOD_EVENT,  //!< The event is a relationship being modified.
  DEL_EVENT,  //!< The event is a relationship being removed.

  ANY_EVENT   //!< All change types (used when calling Storage::observe).
};

/**\brief Enumerate the types of relationships which may cause storage events.
  *
  * This enumerates relationships that may be observed changing,
  * along with the StorageEventChangeType.
  *
  * For events involving multiple entities,
  * the name of the relationship specifies the order in which
  * arguments to the callback appear. For example: MODEL_INCLUDES_FREE_CELL
  * will always invoke callbacks with the model followed by the free cell.
  * Likewise CELL_INCLUDES_CELL will always provide the higher-dimensional
  * parent cell before the lower-dimensional, embedded child cell.
  *
  * @sa StorageEventChangeType
  */
enum StorageEventRelationType
{
  // smtk::model::Entity record existential changes (additions, removals, but not modifications):
  ENTITY_ENTRY,                 //!< An entity entry has been added to (ADD_EVENT) or removed from (DEL_EVENT) storage.

  // smtk::model::Tessellation record changes (added, removed, modified).
  TESSELLATION_ENTRY,           //!< An entity is being provided or stripped of a tessellation in storage.

  // smtk::model::{Float,String,Integer}Data changes (added, removed, modified)
  ENTITY_HAS_PROPERTY,          //!< The entity has a property entry being added, modified, or removed.

  // smtk::model::AttributeAssociation changes (added, removed, modified)
  ENTITY_HAS_ATTRIBUTE,         //!< The entity is being associated/disassociated to/from an attribute.

  // Events affecting both Arrangement and Entity records (additions, removals, modifications of arrangements)
  MODEL_INCLUDES_FREE_CELL,     //!< The entity is a model with a free cell.
  MODEL_INCLUDES_FREE_USE,      //!< The entity is a model with a free use (not really sensical?).
  MODEL_INCLUDES_FREE_SHELL,    //!< The entity is a model with a free shell (not really sensical?).
  MODEL_INCLUDES_MODEL,         //!< The entity is a model that has child model(s), i.e., an assembly.
  MODEL_INCLUDES_GROUP,         //!< The entity is a model that has child group(s).

  CELL_INCLUDES_CELL,           //!< The entity is a cell that includes a lower-dimensional cell.
  CELL_HAS_USE,                 //!< The entity is a cell with a sense that is in use as a boundary.

  SHELL_HAS_USE,                //!< The entity is a shell composed of multiple lower-dimensional uses.
  SHELL_INCLUDES_SHELL,         //!< The entity is a shell that includes another shell (e.g., a void)

  GROUP_SUPERSET_OF_ENTITY,     //!< The entity is a group whose membership is being modified.

  INSTANCE_OF_ENTITY,           //!< The entity is an instance being inserted, removed, or modified (retargeting what it instances).

  INVALID_RELATIONSHIP          //!< The event is invalid. Used internally. This must be the last enum.
};

/**\brief A notification of an event.
  *
  * All events have both a change type and a relationship type.
  */
typedef std::pair<StorageEventChangeType, StorageEventRelationType> StorageEventType;

/// Callbacks for changes in the condition of an entity. WARNING: Likely to change in future releases.
typedef int (*ConditionCallback)(
  StorageEventType, const smtk::model::Cursor&, void*);
/// An observer of an entity-condition-change (i.e., addition, update or removal) event.
typedef std::pair<ConditionCallback,void*> ConditionObserver;
/// A trigger entry for an event-observer pair.
typedef std::pair<StorageEventType,ConditionObserver> ConditionTrigger;

/// Callbacks for one-to-one relationships between entities. WARNING: Likely to change in future releases.
typedef int (*OneToOneCallback)(
  StorageEventType, const smtk::model::Cursor&, const smtk::model::Cursor&, void*);
/// An observer of a one-to-one relationship-event.
typedef std::pair<OneToOneCallback,void*> OneToOneObserver;
/// A trigger entry for an event-observer pair.
typedef std::pair<StorageEventType,OneToOneObserver> OneToOneTrigger;

/// Callbacks for one-to-many relationships between entities. WARNING: Likely to change in future releases.
typedef int (*OneToManyCallback)(
  StorageEventType, const smtk::model::Cursor&, const smtk::model::CursorArray&, void*);
/// An observer of a one-to-many relationship-event.
typedef std::pair<OneToManyCallback,void*> OneToManyObserver;
/// A trigger entry for an event-observer pair.
typedef std::pair<StorageEventType,OneToManyObserver> OneToManyTrigger;


  } // namespace model
} // namespace smtk

#endif // __smtk_model_Events_h
