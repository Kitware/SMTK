//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Collection.h"

#include "smtk/common/UUID.h"

#include <boost/lexical_cast.hpp>

#include <map>
#include <set>

namespace smtk {
namespace mesh {

//----------------------------------------------------------------------------
class Manager::InternalStorageImpl
{
public:
  typedef std::map< smtk::common::UUID, smtk::mesh::CollectionPtr > ContainerType;

  typedef ContainerType::value_type value_type;
  typedef ContainerType::mapped_type mapped_type;
  typedef ContainerType::key_type key_type;
  typedef ContainerType::const_iterator const_iterator;
  typedef ContainerType::iterator iterator;

  InternalStorageImpl():
    Collections()
  {
  }

  //----------------------------------------------------------------------------
  //Returns true when adding a new collection or a collection that already
  //exists.
  bool add(const smtk::common::UUID& uid,
           const smtk::mesh::CollectionPtr& collection)
  {
    const bool is_valid_u = !uid.isNull();
    const bool is_valid_c = collection->isValid();
    const bool not_already_added = this->Collections.count( uid ) == 0;
    const bool can_add = is_valid_u && is_valid_c && not_already_added;

    //do we need to worry about re-parenting?
    if(can_add)
      {
      //we just presume it was added, no need to check the result
      this->Collections.insert( value_type(uid,collection) );
      }

    return can_add || !not_already_added;
  }

  //----------------------------------------------------------------------------
  bool remove( const smtk::common::UUID& uid )
  {
    iterator to_remove = this->Collections.find( uid );
    if(to_remove != this->Collections.end())
      {
      to_remove->second->removeManagerConnection();
      this->Collections.erase( to_remove );
      return true;
      }
    return false;
  }

  //----------------------------------------------------------------------------
  const_iterator begin() const
    { return this->Collections.begin(); }
  iterator begin()
    { return this->Collections.begin(); }

  //----------------------------------------------------------------------------
  const_iterator end() const
    { return this->Collections.end(); }
  iterator end()
    { return this->Collections.end(); }

  //----------------------------------------------------------------------------
  const_iterator find( const smtk::common::UUID& uid) const
    {
    return this->Collections.find( uid );
    }

  //----------------------------------------------------------------------------
  bool has( const smtk::common::UUID& uid) const
    { return this->Collections.find( uid ) != this->Collections.end(); }

  //----------------------------------------------------------------------------
  std::size_t size() const
    { return this->Collections.size(); }

private:
  ContainerType Collections;
};

//----------------------------------------------------------------------------
class Manager::InternalNameGeneratorImpl
{
public:
  InternalNameGeneratorImpl():
  m_value(1),
  m_basename("Mesh_")
  {
  }

  std::string next( const std::set< std::string >& usedNames )
  {
    std::string result(this->m_basename);
    result += boost::lexical_cast< std::string >(this->m_value++);
    while( usedNames.find(result) != usedNames.end() )
      {
      result = (std::string(this->m_basename) += boost::lexical_cast< std::string >(this->m_value++));
      }
    return result;
  }


private:
  int m_value;
  std::string m_basename;
};


//----------------------------------------------------------------------------
Manager::Manager():
  m_collector( new InternalStorageImpl() ),
  m_nameGenerator( new InternalNameGeneratorImpl() ),
  m_uuidGenerator()
{

}

//----------------------------------------------------------------------------
Manager::~Manager()
{ //needs to be in impl file
}

//----------------------------------------------------------------------------
smtk::common::UUID Manager::nextEntityId()
{
  //return the next random uuid
  return this->m_uuidGenerator.random();
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr Manager::makeCollection()
{
  smtk::mesh::CollectionPtr collection( new smtk::mesh::Collection(
                                                this->nextEntityId(),
                                                this->shared_from_this() ) );

  this->addCollection( collection );
  return collection;
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr Manager::makeCollection(const smtk::common::UUID& collectionID)
{
  smtk::mesh::CollectionPtr collection( new smtk::mesh::Collection(
                                                collectionID,
                                                this->shared_from_this() ) );

  this->addCollection( collection );
  return collection;
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr Manager::makeCollection(smtk::mesh::InterfacePtr interface)
{
  smtk::mesh::CollectionPtr collection( new smtk::mesh::Collection(
                                                this->nextEntityId(),
                                                interface,
                                                this->shared_from_this() ) );

  this->addCollection( collection );
  return collection;
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr Manager::makeCollection(const smtk::common::UUID& collectionID,
                                                  smtk::mesh::InterfacePtr interface)
{
  smtk::mesh::CollectionPtr collection( new smtk::mesh::Collection(
                                                collectionID,
                                                interface,
                                                this->shared_from_this() ) );

  this->addCollection( collection );
  return collection;
}


//----------------------------------------------------------------------------
bool Manager::addCollection(const smtk::mesh::CollectionPtr& collection)
{
  //do we need to re-parent the collection?
  return this->m_collector->add(collection->entity(), collection);
}

//----------------------------------------------------------------------------
std::size_t Manager::numberOfCollections() const
{
  return this->m_collector->size();
}

//----------------------------------------------------------------------------
bool Manager::hasCollection( const smtk::mesh::CollectionPtr& collection ) const
{
  return this->m_collector->has( collection->entity() );
}

//----------------------------------------------------------------------------
Manager::const_iterator Manager::collectionBegin() const
{
  return this->m_collector->begin();
}

//----------------------------------------------------------------------------
Manager::const_iterator Manager::collectionEnd() const
{
  return this->m_collector->end();
}

//----------------------------------------------------------------------------
Manager::const_iterator Manager::findCollection( const smtk::common::UUID& collectionID ) const
{
  return this->m_collector->find(collectionID);
}

//----------------------------------------------------------------------------
smtk::mesh::CollectionPtr Manager::collection( const smtk::common::UUID& collectionID ) const
{
  const_iterator result = this->m_collector->find(collectionID);
  if(result == this->m_collector->end())
    { //returning end() result causes undefined behavior and will generally
      //cause a segfault when you query the item
    return smtk::mesh::CollectionPtr();
    }
  return result->second;
}

//----------------------------------------------------------------------------
bool Manager::removeCollection( const smtk::mesh::CollectionPtr& collection )
{
  return this->m_collector->remove( collection->entity() );
}

//----------------------------------------------------------------------------
std::vector<smtk::mesh::CollectionPtr>
Manager::collectionsWithAssociations() const
{
  std::vector<smtk::mesh::CollectionPtr> result;
  for(const_iterator i = this->m_collector->begin();
      i != this->m_collector->end();
      ++i)
    {
    if( i->second->hasAssociations() )
      {
      result.insert(result.end(), i->second);
      }
    }
  return result;
}

//----------------------------------------------------------------------------
bool Manager::isAssociatedToACollection( const smtk::model::EntityRef& eref ) const
{
  bool isAssociated = false;
  for(const_iterator i = this->m_collector->begin();
      i != this->m_collector->end() && isAssociated == false;
      ++i)
    {
    isAssociated = eref.isModel() ?
      i->second->associatedModel() == eref.entity() :
      !(i->second->findAssociatedMeshes( eref ).is_empty());
    }
  return isAssociated;
}

//----------------------------------------------------------------------------
std::vector<smtk::mesh::CollectionPtr>
Manager::associatedCollections( const smtk::model::EntityRef& eref) const
{
  std::vector<smtk::mesh::CollectionPtr> result;
  for(const_iterator i = this->m_collector->begin();
      i != this->m_collector->end();
      ++i)
    {
    bool found = eref.isModel() ?
      i->second->associatedModel() == eref.entity() :
      !(i->second->findAssociatedMeshes( eref ).is_empty());
    if(found)
      {
      result.push_back(i->second);
      }
    }
  return result;
}

//----------------------------------------------------------------------------
smtk::common::UUIDs
Manager::associatedCollectionIds( const smtk::model::EntityRef& eref) const
{
  smtk::common::UUIDs result;
  for(const_iterator i = this->m_collector->begin();
      i != this->m_collector->end();
      ++i)
    {
    bool found = eref.isModel() ?
      i->second->associatedModel() == eref.entity() :
      !(i->second->findAssociatedMeshes( eref ).is_empty());
    if(found)
      {
      result.insert(i->second->entity());
      }
    }
  return result;
}

//----------------------------------------------------------------------------
bool
Manager::assignUniqueName( smtk::mesh::CollectionPtr collection )
{
  //if we don't own this collection, we can't generate a unique name for it
  if(!this->hasCollection(collection) )
    {
    return false;
    }


  std::set< std::string > usedNames;
  for(const_iterator i = this->m_collector->begin(); i != this->m_collector->end(); ++i)
    {
    if(i->second != collection)
      {
      usedNames.insert( i->second->name() );
      }
    }

  const bool nameAlreadyUsed = ( usedNames.find(collection->name()) != usedNames.end() );
  const bool nameEmpty = ( collection->name().empty() );

  if(nameAlreadyUsed || nameEmpty )
    {
    //time to generate a new name, we pass the number of existing meshes
    //to
    collection->name( this->m_nameGenerator->next(usedNames) );
    }

  return true;
}


}
}
