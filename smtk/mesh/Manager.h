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

#include "smtk/CoreExports.h"
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

  //Construct a collection. Manager will generate the UUID, and choose
  //the Interface the collection should have
  smtk::mesh::CollectionPtr makeCollection();

  //Construct a collection using a provided UUID, and all the Manager to choose
  //the Interface the collection should have
  //Note: Caller must verify the UUID is valid, and not already in use by
  //this manager
  smtk::mesh::CollectionPtr makeCollection(const smtk::common::UUID& collectionID);

  //Construct a collection. Manager will generate the UUID, and use
  //the Interface provided by the caller
  smtk::mesh::CollectionPtr makeCollection(smtk::mesh::InterfacePtr interface);

  //Construct a collection using the users provided UUID and Interface.
  //Note: Caller must verify the UUID is valid, and not already in use by
  //this manager
  smtk::mesh::CollectionPtr makeCollection(const smtk::common::UUID& collectionID,
                                           smtk::mesh::InterfacePtr interface);

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
  std::vector<smtk::mesh::CollectionPtr> collectionsWithAssociations() const;
  bool isAssociatedToACollection( const smtk::model::EntityRef& eref ) const;

  //For a given model cursor return the associated Mesh Collection(s)
  //from that mesh collection you will be able to query what subset of
  //the collection is associated with the model cursor. If no association exists
  //this will return an invalid Collection that is not associated with any
  //Manager.
  std::vector<smtk::mesh::CollectionPtr> associatedCollections(
                                       const smtk::model::EntityRef& c ) const;

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

  smtk::common::UUIDGenerator m_uuidGenerator;
};

}
}

#endif //__smtk_mesh_Manager_h
