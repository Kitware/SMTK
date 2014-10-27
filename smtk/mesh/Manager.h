//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_Manager_h
#define __smtk_mesh_Manager_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/model/EntityRef.h"

#include "smtk/common/UUID.h"
#include "smtk/common/UUIDGenerator.h"

namespace smtk {
namespace mesh {

class SMTKCORE_EXPORT Manager : public smtk::enable_shared_from_this<Manager>
{
  typedef std::map< smtk::common::UUID, smtk::mesh::CollectionPtr > ContainerType;

public:
  smtkTypeMacro(Manager);
  smtkCreateMacro(Manager);
  virtual ~Manager();

  smtk::mesh::CollectionPtr makeCollection();
  smtk::mesh::CollectionPtr makeCollection(smtk::mesh::moab::InterfacePtr interface);

  std::size_t numberOfCollections() const;
  bool hasCollection( const smtk::mesh::CollectionPtr& collection ) const;

  typedef ContainerType::const_iterator const_iterator;
  typedef ContainerType::iterator iterator;
  const_iterator collectionBegin() const;
  const_iterator collectionEnd() const;
  const_iterator findCollection( const smtk::common::UUID& collectionID ) const;

  //returns the mesh collection that relates to the given uuid.
  //If no association exists this will return an invalid pointer
  smtk::mesh::CollectionPtr collection( const smtk::common::UUID& collectionID ) const;

  //Removes a collection from a given Manager. This doesn't explicitly release
  //the memory of the collection, it only stops the tracking of the collection
  //by the manager.
  bool removeCollection( const smtk::mesh::CollectionPtr& collection );

  //----------------------------------------------------------------------------
  //Model Association commands
  std::size_t numberOfAssociations() const;
  bool isAssociatedCollection( const smtk::mesh::CollectionPtr& collection );
  bool isAssociatedToACollection( const smtk::model::EntityRef& eref ) const;

  const_iterator associatedCollectionBegin() const;
  const_iterator associatedCollectionEnd() const;
  const_iterator findAssociatedCollection( const smtk::model::EntityRef& eref ) const;

  //For a given model cursor return the associated Mesh Collection
  //from that mesh collection you will be able to query what sunset of
  //the collection is associated with the model cursor. If no association exists
  //this will return an invalid Collection that is not associated with any
  //Manager.
  smtk::mesh::CollectionPtr associatedCollection( const smtk::model::EntityRef& c ) const;

  //Note a model cursor can only be related to a single meshCollection, and
  //a collection can only be associated with a single manager. So this will
  //reparent the
  bool addAssociation( const smtk::model::EntityRef& eref,
                       const smtk::mesh::CollectionPtr& collection);

  //remove the association of a cursor.
  //Returns False if the cursor had no association to begin with.
  bool removeAssociation( const smtk::model::EntityRef& eref );

private:
  //needs to be created using shared_ptr
  Manager();

  //used to get the next entity id
  friend class smtk::mesh::Collection;

  smtk::common::UUID nextEntityId();

  //returns true if the collection was added or already is part of this manager
  //if the collection is currently part of a different manager we will reparent
  //it to this collection
  bool addCollection(const smtk::mesh::CollectionPtr& collection);

  class InternalStorageImpl;
  smtk::shared_ptr< InternalStorageImpl > m_collector;
  smtk::shared_ptr< InternalStorageImpl > m_associator;

  smtk::common::UUIDGenerator m_uuidGenerator;
};


}
}

#endif //__smtk_mesh_Manager_h