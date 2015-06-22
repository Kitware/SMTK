//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Events_h
#define __smtk_model_Events_h


#include "smtk/PublicPointerDefs.h" // For EntityRef and EntityRefArray
#include <utility> // For std::pair

namespace smtk {
  namespace model {

/**\brief Enumerate the types of changes a model manager undergoes.
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
  * affect a model manager are typically reported with
  * ADD_EVENT notifications generated before addition
  * and DEL_EVENT notifications after removal.
  * The rationale is that (1) observers should have
  * direct access to the most difficult-to-infer
  * configuration while the event type specifies enough
  * information to infer the simpler configuration and
  * (2) observers may wish to perform other operations
  * after removal to maintain the consistency of Manager.
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
  * @sa ManagerEventRelationType
  */
enum ManagerEventChangeType
{
  ADD_EVENT,  //!< The event is a relationship being added.
  MOD_EVENT,  //!< The event is a relationship being modified.
  DEL_EVENT,  //!< The event is a relationship being removed.

  ANY_EVENT   //!< All change types (used when calling Manager::observe).
};

/**\brief Enumerate the types of relationships which may cause model manager events.
  *
  * This enumerates relationships that may be observed changing,
  * along with the ManagerEventChangeType.
  *
  * For events involving multiple entities,
  * the name of the relationship specifies the order in which
  * arguments to the callback appear. For example: MODEL_INCLUDES_FREE_CELL
  * will always invoke callbacks with the model followed by the free cell.
  * Likewise CELL_INCLUDES_CELL will always provide the higher-dimensional
  * parent cell before the lower-dimensional, embedded child cell.
  *
  * @sa ManagerEventChangeType
  */
enum ManagerEventRelationType
{
  // smtk::model::Entity record existential changes (additions, removals, but not modifications):
  ENTITY_ENTRY,                 //!< An entity entry has been added to (ADD_EVENT) or removed from (DEL_EVENT) the manager.

  // smtk::model::Tessellation record changes (added, removed, modified).
  TESSELLATION_ENTRY,           //!< An entity is being provided or stripped of a tessellation in the manager.

  // smtk::model::{Float,String,Integer}Data changes (added, removed, modified)
  ENTITY_HAS_PROPERTY,          //!< The entity has a property entry being added, modified, or removed.

  // smtk::model::AttributeAssociation changes (added, removed, modified)
  ENTITY_HAS_ATTRIBUTE,         //!< The entity is being associated/disassociated to/from an attribute.

  // Events affecting both Arrangement and Entity records (additions, removals, modifications of arrangements)
  SESSION_INCLUDES_MODEL,       //!< The entity is a session with a child model being added or removed.

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
  MODEL_SUPERSET_OF_MODEL,      //!< The entity is a model whose set of child models is being modified.
  MODEL_SUPERSET_OF_GROUP,      //!< The entity is a model whose set of child groups is being modified.
  SESSION_SUPERSET_OF_MODEL,    //!< The entity is a session whose set of child models is being modified.

  INSTANCE_OF_ENTITY,           //!< The entity is an instance being inserted, removed, or modified (retargeting what it instances).

  INVALID_RELATIONSHIP          //!< The event is invalid. Used internally. This must be the last enum.
};

/**\brief A notification of a model manager event.
  *
  * All events have both a change type and a relationship type.
  */
typedef std::pair<ManagerEventChangeType, ManagerEventRelationType> ManagerEventType;

/// Callbacks for changes in the condition of an entity. WARNING: Likely to change in future releases.
typedef int (*ConditionCallback)(
  ManagerEventType, const smtk::model::EntityRef&, void*);
/// An observer of an entity-condition-change (i.e., addition, update or removal) event.
typedef std::pair<ConditionCallback,void*> ConditionObserver;
/// A trigger entry for an event-observer pair.
typedef std::pair<ManagerEventType,ConditionObserver> ConditionTrigger;

/// Callbacks for one-to-one relationships between entities. WARNING: Likely to change in future releases.
typedef int (*OneToOneCallback)(
  ManagerEventType, const smtk::model::EntityRef&, const smtk::model::EntityRef&, void*);
/// An observer of a one-to-one relationship-event.
typedef std::pair<OneToOneCallback,void*> OneToOneObserver;
/// A trigger entry for an event-observer pair.
typedef std::pair<ManagerEventType,OneToOneObserver> OneToOneTrigger;

/// Callbacks for one-to-many relationships between entities. WARNING: Likely to change in future releases.
typedef int (*OneToManyCallback)(
  ManagerEventType, const smtk::model::EntityRef&, const smtk::model::EntityRefArray&, void*);
/// An observer of a one-to-many relationship-event.
typedef std::pair<OneToManyCallback,void*> OneToManyObserver;
/// A trigger entry for an event-observer pair.
typedef std::pair<ManagerEventType,OneToManyObserver> OneToManyTrigger;

/**\brief Enumerate events that an operator may encounter.
  *
  * No event is provided for operator deletion because
  * (1) operator deletion is not managed by the model Manager class and
  * (2) "this" is not complete in class destructors (subclass data is
  * already freed).
  * So, there is no easy way to observe when an operator is about to be
  * deleted but is still valid.
  */
enum OperatorEventType
{
  CREATED_OPERATOR,   //!< An instance of the Operator class has been created by a model Manager.
  WILL_OPERATE,       //!< The operation will commence if no observers cancel it.
  DID_OPERATE         //!< The operation has completed or been canceled.
};

/// Callbacks for CREATED_OPERATOR and WILL_OPERATE events provide access to the operator. Returning non-zero values cancel the operation.
typedef int (*BareOperatorCallback)(
  OperatorEventType event, const Operator& op, void* user);
/// An observer of CREATED_OPERATOR or WILL_OPERATE events binds a callback and opaque, user-provided data.
typedef std::pair<BareOperatorCallback,void*> BareOperatorObserver;
/// A trigger for CREATED_OPERATOR or WILL_OPERATE events holds the event type and its observer.
typedef std::pair<OperatorEventType,BareOperatorObserver> BareOperatorTrigger;

/// Callbacks for DID_OPERATE events provide access to the operator and the results of the operation. Return values are ignored.
typedef int (*OperatorWithResultCallback)(
  OperatorEventType event, const Operator& op, OperatorResult r, void* user);
/// An observer of DID_OPERATE events binds a callback and opaque, user-provided data.
typedef std::pair<OperatorWithResultCallback,void*> OperatorWithResultObserver;

  } // namespace model
} // namespace smtk

#endif // __smtk_model_Events_h
