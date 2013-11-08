#include "smtk/model/BRepModel.h"

namespace smtk {
  namespace model {

using smtk::util::UUID;

/**\brief Construction requires a container for storage.
  *
  * Storage is kept separate so that it can easily be serialized and deserialized.
  */
BRepModel::BRepModel()
  : m_topology(new UUIDsToLinks), m_deleteStorage(true)
{
  // TODO: throw() when topology == NULL?
}

/**\brief Construction requires a container for storage.
  *
  * Storage is kept separate so that it can easily be serialized and deserialized.
  */
BRepModel::BRepModel(UUIDsToLinks* topology, bool shouldDelete)
    : m_topology(topology), m_deleteStorage(shouldDelete)
    { } // TODO: throw() when topology == NULL?

BRepModel::~BRepModel()
{
  if (this->m_deleteStorage)
    {
    delete this->m_topology;
    this->m_topology = NULL;
    }
}

/// Change whether or not we should delete storage upon our own destruction.
void BRepModel::setDeleteStorage(bool d)
{
  this->m_deleteStorage = d;
}

UUIDsToLinks& BRepModel::topology()
{
  return *this->m_topology;
}

const UUIDsToLinks& BRepModel::topology() const
{
  return *this->m_topology;
}

/// Entity construction
//@{
/// Insert a new cell of the specified \a dimension, returning an iterator with a new, unique UUID.
BRepModel::iter_type BRepModel::insertLinkOfTypeAndDimension(int entityFlags, int dimension)
{
  UUID actual;
  do
    {
    actual = UUID::random();
    }
  while (this->m_topology->find(actual) != this->m_topology->end());
  return this->setLinkOfTypeAndDimension(actual, entityFlags, dimension);
}

/// Insert the specified cell, returning an iterator with a new, unique UUID.
BRepModel::iter_type BRepModel::insertLink(Link& c)
{
  UUID actual;
  do
    {
    actual = UUID::random();
    }
  while (this->m_topology->find(actual) != this->m_topology->end());
  return this->setLink(actual, c);
}

/**\brief Map a new cell of the given \a dimension to the \a uid.
  *
  * Passing a non-unique \a uid is an error here and will throw an exception.
  */
BRepModel::iter_type BRepModel::setLinkOfTypeAndDimension(const UUID& uid, int entityFlags, int dimension)
{
  UUIDsToLinks::iterator it;
  if (uid.isNull())
    {
    std::ostringstream msg;
    msg << "Nil UUID";
    throw msg.str();
    }
  if ((it = this->m_topology->find(uid)) != this->m_topology->end() && it->second.dimension() != dimension)
    {
    std::ostringstream msg;
    msg << "Duplicate UUID '" << uid << "' of different dimension " << it->second.dimension() << " != " << dimension;
    throw msg.str();
    }
  std::pair<UUID,Link> entry(uid,Link(entityFlags, dimension));
  return this->m_topology->insert(entry).first;
}

/**\brief Map the specified cell \a c to the given \a uid.
  *
  * Passing a nil or non-unique \a uid is an error here and will throw an exception.
  */
BRepModel::iter_type BRepModel::setLink(const UUID& uid, Link& c)
{
  UUIDsToLinks::iterator it;
  if (uid.isNull())
    {
    std::ostringstream msg;
    msg << "Nil UUID";
    throw msg.str();
    }
  if ((it = this->m_topology->find(uid)) != this->m_topology->end())
    {
    if (it->second.dimension() != c.dimension())
      {
      std::ostringstream msg;
      msg << "Duplicate UUID '" << uid << "' of different dimension " << it->second.dimension() << " != " << c.dimension();
      throw msg.str();
      }
    this->removeLinkReferences(it);
    it->second = c;
    this->insertLinkReferences(it);
    return it;
    }
  std::pair<UUID,Link> entry(uid,c);
  it = this->m_topology->insert(entry).first;
  this->insertLinkReferences(it);
  return it;
}

/// A wrappable version of InsertLinkOfTypeAndDimension
UUID BRepModel::addLinkOfTypeAndDimension(int entityFlags, int dim)
{
  return this->insertLinkOfTypeAndDimension(entityFlags, dim)->first;
}

/// A wrappable version of InsertLink
UUID BRepModel::addLink(Link& cell)
{
  return this->insertLink(cell)->first;
}

/// A wrappable version of SetLinkOfTypeAndDimension
UUID BRepModel::addLinkOfTypeAndDimensionWithUUID(const UUID& uid, int entityFlags, int dim)
{
  return this->setLinkOfTypeAndDimension(uid, entityFlags, dim)->first;
}

/// A wrappable version of SetLink
UUID BRepModel::addLinkWithUUID(const UUID& uid, Link& cell)
{
  return this->setLink(uid, cell)->first;
}
//@}

/// Shortcuts for inserting cells with default entity flags.
//@{
BRepModel::iter_type BRepModel::insertCellOfDimension(int dim)
{
  return this->insertLinkOfTypeAndDimension(CELL_ENTITY, dim);
}

BRepModel::iter_type BRepModel::setCellOfDimension(const UUID& uid, int dim)
{
  return this->setLinkOfTypeAndDimension(uid, CELL_ENTITY, dim);
}

UUID BRepModel::addCellOfDimension(int dim)
{
  return this->addLinkOfTypeAndDimension(CELL_ENTITY, dim);
}

UUID BRepModel::addCellOfDimensionWithUUID(const UUID& uid, int dim)
{
  return this->addLinkOfTypeAndDimensionWithUUID(uid, CELL_ENTITY, dim);
}
//@}

/// Queries on entities belonging to the solid.
//@{
/// Return the type of entity that the link represents.
int BRepModel::type(const smtk::util::UUID& ofEntity)
{
  UUIDsToLinks::iterator it = this->m_topology->find(ofEntity);
  return (it == this->m_topology->end() ? INVALID : it->second.entityFlags());
}

/// Return the dimension of the manifold that the passed entity represents.
int BRepModel::dimension(const UUID& ofEntity)
{
  UUIDsToLinks::iterator it = this->m_topology->find(ofEntity);
  return (it == this->m_topology->end() ? -1 : it->second.dimension());
}

/**\brief Return the (Dimension+1 or higher)-entities that are the immediate bordants of the passed entity.
  *
  * \sa HigherDimensionalBoundaries
  */
UUIDs BRepModel::bordantEntities(const UUID& ofEntity, int ofDimension)
{
  UUIDs result;
  UUIDsToLinks::iterator it = this->m_topology->find(ofEntity);
  if (it == this->m_topology->end())
    {
    return result;
    }
  if (ofDimension >= 0 && it->second.dimension() >= ofDimension)
    {
    // can't ask for "higher" dimensional boundaries that are lower than the dimension of this cell.
    return result;
    }
  UUIDsToLinks::iterator other;
  for (UUIDArray::iterator ai = it->second.relations().begin(); ai != it->second.relations().end(); ++ai)
    {
    other = this->m_topology->find(*ai);
    if (other == this->m_topology->end())
      { // TODO: silently skip bad relations or complain?
      continue;
      }
    if (
      (ofDimension >= 0 && other->second.dimension() == ofDimension) ||
      (ofDimension == -2 && other->second.dimension() >= it->second.dimension()))
      {
      result.insert(*ai);
      }
    }
  return result;
}

/**\brief Return the (Dimension+1 or higher)-entities that are the immediate bordants of any of the passed entities.
  *
  * \sa HigherDimensionalBoundaries
  */
UUIDs BRepModel::bordantEntities(const UUIDs& ofEntities, int ofDimension)
{
  UUIDs result;
  std::insert_iterator<UUIDs> inserter(result, result.begin());
  for (UUIDs::const_iterator it = ofEntities.begin(); it != ofEntities.end(); ++it)
    {
    UUIDs bdy = this->bordantEntities(*it, ofDimension);
    std::copy(bdy.begin(), bdy.end(), inserter);
    }
  return result;
}

/**\brief Return the (Dimension-1 or lower)-entities that are the immediate boundary of the passed entity.
  *
  * \sa LowerDimensionalBoundaries
  */
UUIDs BRepModel::boundaryEntities(const UUID& ofEntity, int ofDimension)
{
  UUIDs result;
  UUIDsToLinks::iterator it = this->m_topology->find(ofEntity);
  if (it == this->m_topology->end())
    {
    return result;
    }
  if (ofDimension >= 0 && it->second.dimension() <= ofDimension)
    {
    // can't ask for "lower" dimensional boundaries that are higher than the dimension of this cell.
    return result;
    }
  UUIDsToLinks::iterator other;
  for (UUIDArray::iterator ai = it->second.relations().begin(); ai != it->second.relations().end(); ++ai)
    {
    other = this->m_topology->find(*ai);
    if (other == this->m_topology->end())
      { // TODO: silently skip bad relations or complain?
      continue;
      }
    if (
      (ofDimension >= 0 && other->second.dimension() == ofDimension) ||
      (ofDimension == -2 && other->second.dimension() <= it->second.dimension()))
      {
      result.insert(*ai);
      }
    }
  return result;
}

/**\brief Return the (Dimension-1 or lower)-entities that are the immediate boundary of any of the passed entities.
  *
  * \sa LowerDimensionalBoundaries
  */
UUIDs BRepModel::boundaryEntities(const UUIDs& ofEntities, int ofDimension)
{
  UUIDs result;
  std::insert_iterator<UUIDs> inserter(result, result.begin());
  for (UUIDs::const_iterator it = ofEntities.begin(); it != ofEntities.end(); ++it)
    {
    UUIDs bdy = this->boundaryEntities(*it, ofDimension);
    std::copy(bdy.begin(), bdy.end(), inserter);
    }
  return result;
}

/**\brief Return lower-dimensional boundaries of the passed d-dimensional entity.
  *
  * \a lowerDimension may be any dimension < d.
  * Unlike BRepModel::boundaryEntities(), this method will search the boundaries
  * of the entity's boundaries.
  * For example, a 2-dimensional face normally stores 1-dimensional edges
  * as its immediate boundaries, so if BoundaryEntities() is asked for 0-dimensional
  * entities none will usually be reported (the exception being an isolated vertex
  * lying on the face with no edges attached).
  * But LowerDimensionalBoundaries() will return the corners of the edges when asked
  * for 0-dimensional boundaries.
  *
  * Passing -1 will return all boundary entities of the specified entity,
  * regardless of their dimension.
  */
UUIDs BRepModel::lowerDimensionalBoundaries(const UUID& ofEntity, int lowerDimension)
{
  UUIDs result;
  UUIDsToLinks::iterator it = this->m_topology->find(ofEntity);
  if (it == this->m_topology->end())
    {
    return result;
    }
  if (it->second.dimension() <= lowerDimension)
    {
    // do nothing
    }
  else
    {
    // FIXME: This only works for the "usual" case where
    //        a cell's relations are dimension (d+1) or
    //        (d-1). We should also collect any out-of-place
    //        relations that match lowerDimension as we go.
    int currentDim = it->second.dimension() - 1;
    int delta = currentDim - lowerDimension;
    result = this->boundaryEntities(ofEntity, currentDim--);
    for (int i = delta; i > 0; --i, --currentDim)
      {
      result = this->boundaryEntities(result, currentDim);
      }
    }
  return result;
}

/**\brief Return higher-dimensional bordants of the passed d-dimensional entity.
  *
  * \a higherDimension may be any dimension > d.
  * Unlike BRepModel::bordantEntities(), this method will search the bordants
  * of the entity's immediate bordants.
  * For example, a 1-dimensional edge normally stores 2-dimensional faces
  * as its immediate bordants, so if BoundaryEntities() is asked for 3-dimensional
  * bordants none will usually be reported (the exception being when the edge
  * is contained completely inside the volume and not attached to any boundary).
  * But HigherDimensionalBoundaries() will return all the volumes the edge borders
  * when asked for 3-dimensional boundaries.
  *
  * Passing -1 will return all bordant entities of the specified entity,
  * regardless of their dimension.
  */
UUIDs BRepModel::higherDimensionalBordants(const UUID& ofEntity, int higherDimension)
{
  UUIDs result;
  UUIDsToLinks::iterator it = this->m_topology->find(ofEntity);
  if (it == this->m_topology->end())
    {
    return result;
    }
  if (it->second.dimension() >= higherDimension)
    {
    // do nothing
    }
  else
    {
    int currentDim = it->second.dimension() + 1;
    int delta = higherDimension - currentDim;
    result = this->bordantEntities(ofEntity, currentDim++);
    for (int i = delta; i > 0; --i, ++currentDim)
      {
      result = this->bordantEntities(result, currentDim);
      }
    }
  return result;
}

/// Return entities of the requested dimension that share a boundary relationship with the passed entity.
UUIDs BRepModel::adjacentEntities(const UUID& ofEntity, int ofDimension)
{
  // FIXME: Implement adjacency
  (void)ofEntity;
  (void)ofDimension;
  UUIDs result;
  return result;
}

/// Return all entities of the requested dimension that are present in the solid.
UUIDs BRepModel::entities(int ofDimension)
{
  UUIDs result;
  for (UUIDWithLink it = this->m_topology->begin(); it != this->m_topology->end(); ++it)
    {
    if (it->second.dimension() == ofDimension)
      {
      result.insert(it->first);
      }
    }
  return result;
}
//@}

const Link* BRepModel::findLink(const UUID& uid) const
{
  UUIDWithLink it = this->m_topology->find(uid);
  if (it == this->m_topology->end())
    {
    return NULL;
    }
  return &it->second;
}

Link* BRepModel::findLink(const UUID& uid)
{
  UUIDWithLink it = this->m_topology->find(uid);
  if (it == this->m_topology->end())
    {
    return NULL;
    }
  return &it->second;
}

void BRepModel::removeLinkReferences(const UUIDWithLink& c)
{
  UUIDArray::const_iterator bit;
  Link* ref;
  for (bit = c->second.relations().begin(); bit != c->second.relations().end(); ++bit)
    {
    ref = this->findLink(*bit);
    if (ref)
      {
      ref->removeRelation(c->first);
      }
    }
}

void BRepModel::insertLinkReferences(const UUIDWithLink& c)
{
  UUIDArray::const_iterator bit;
  Link* ref;
  for (bit = c->second.relations().begin(); bit != c->second.relations().end(); ++bit)
    {
    ref = this->findLink(*bit);
    if (ref)
      {
      ref->appendRelation(c->first);
      }
    }
}

  } // model namespace
} // smtk namespace
