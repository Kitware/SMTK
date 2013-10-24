namespace smtk {
  namespace model {

typedef std::map<UUID,Cell> UUIDsToCells;
typedef UUIDsToCells::iterator UUIDWithCell;

/// A specialized implementation of BRepModel for UUIDs and the Cell class.
template<>
class SMTKCORE_EXPORT BRepModel<UUID,UUIDs,Cell>
{
public:
  typedef BRepModel<UUID,UUIDs,Cell> self_type;
  typedef UUIDsToCells storage_type;
  typedef typename storage_type::iterator iter_type;

  /**\brief Construction requires a container for storage.
    *
    * Storage is kept separate so that it can easily be serialized and deserialized.
    */
  BRepModel()
    : Topology(new UUIDsToCells), DeleteStorage(true)
    { } // TODO: throw() when topology == NULL?

  /**\brief Construction requires a container for storage.
    *
    * Storage is kept separate so that it can easily be serialized and deserialized.
    */
  BRepModel(UUIDsToCells* topology, bool shouldDelete)
    : Topology(topology), DeleteStorage(shouldDelete)
    { } // TODO: throw() when topology == NULL?

  ~BRepModel()
    {
    if (this->DeleteStorage)
      {
      delete this->Topology;
      this->Topology = NULL;
      }
    }

  /// Change whether or not we should delete storage upon our own destruction.
  void SetDeleteStorage(bool d)
    {
    this->DeleteStorage = d;
    }

  UUIDsToCells& topology()
    {
    return *this->Topology;
    }

  const UUIDsToCells& topology() const
    {
    return *this->Topology;
    }

  /// Entity construction
  //@{
  /// Insert a new cell of the specified \a dimension, returning an iterator with a new, unique UUID.
  BRepModel::iter_type InsertCellOfDimension(int dimension)
    {
    UUID actual;
    do
      {
      actual = UUID::Random();
      }
    while (this->Topology->find(actual) != this->Topology->end());
    return this->SetCellOfDimension(actual, dimension);
    }

  /// Insert the specified cell, returning an iterator with a new, unique UUID.
  BRepModel::iter_type InsertCell(Cell& c)
    {
    UUID actual;
    do
      {
      actual = UUID::Random();
      }
    while (this->Topology->find(actual) != this->Topology->end());
    return this->SetCell(actual, c);
    }

  /**\brief Map a new cell of the given \a dimension to the \a uid.
    *
    * Passing a non-unique \a uid is an error here and will throw an exception.
    */
  BRepModel::iter_type SetCellOfDimension(const UUID& uid, int dimension)
    {
    UUIDsToCells::iterator it;
    if (uid.IsNull())
      {
      std::ostringstream msg;
      msg << "Nil UUID";
      throw msg.str();
      }
    if ((it = this->Topology->find(uid)) != this->Topology->end() && it->second.dimension() != dimension)
      {
      std::ostringstream msg;
      msg << "Duplicate UUID '" << uid << "' of different dimension " << it->second.dimension() << " != " << dimension;
      throw msg.str();
      }
    std::pair<UUID,Cell> entry(uid,Cell(dimension));
    return this->Topology->insert(entry).first;
    }

  /**\brief Map the specified cell \a c to the given \a uid.
    *
    * Passing a nil or non-unique \a uid is an error here and will throw an exception.
    */
  BRepModel::iter_type SetCell(const UUID& uid, Cell& c)
    {
    UUIDsToCells::iterator it;
    if (uid.IsNull())
      {
      std::ostringstream msg;
      msg << "Nil UUID";
      throw msg.str();
      }
    if ((it = this->Topology->find(uid)) != this->Topology->end())
      {
      if (it->second.dimension() != c.dimension())
        {
        std::ostringstream msg;
        msg << "Duplicate UUID '" << uid << "' of different dimension " << it->second.dimension() << " != " << c.dimension();
        throw msg.str();
        }
      this->RemoveCellReferences(it);
      it->second = c;
      this->InsertCellReferences(it);
      return it;
      }
    std::pair<UUID,Cell> entry(uid,c);
    it = this->Topology->insert(entry).first;
    this->InsertCellReferences(it);
    return it;
    }
  //@}

  /// Queries on entities belonging to the solid.
  //@{
  /// Return the dimension of the manifold that the passed entity represents.
  int Dimension(const UUID& ofEntity)
    {
    UUIDsToCells::iterator it = this->Topology->find(ofEntity);
    return (it == this->Topology->end() ? -1 : it->second.dimension());
    }

  /**\brief Return the (Dimension+1 or higher)-entities that are the immediate bordants of the passed entity.
    *
    * \sa HigherDimensionalBoundaries
    */
  UUIDs BordantEntities(const UUID& ofEntity, int ofDimension)
    {
    UUIDs result;
    UUIDsToCells::iterator it = this->Topology->find(ofEntity);
    if (it == this->Topology->end())
      {
      return result;
      }
    if (ofDimension >= 0 && it->second.dimension() >= ofDimension)
      {
      // can't ask for "higher" dimensional boundaries that are lower than the dimension of this cell.
      return result;
      }
    UUIDsToCells::iterator other;
    for (UUIDArray::iterator ai = it->second.relations().begin(); ai != it->second.relations().end(); ++ai)
      {
      other = this->Topology->find(*ai);
      if (other == this->Topology->end())
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
  UUIDs BordantEntities(const UUIDs& ofEntities, int ofDimension)
    {
    UUIDs result;
    std::insert_iterator<UUIDs> inserter(result, result.begin());
    for (UUIDs::iterator it = ofEntities.begin(); it != ofEntities.end(); ++it)
      {
      UUIDs bdy = this->BordantEntities(*it, ofDimension);
      std::copy(bdy.begin(), bdy.end(), inserter);
      }
    return result;
    }

  /**\brief Return the (Dimension-1 or lower)-entities that are the immediate boundary of the passed entity.
    *
    * \sa LowerDimensionalBoundaries
    */
  UUIDs BoundaryEntities(const UUID& ofEntity, int ofDimension = -2)
    {
    UUIDs result;
    UUIDsToCells::iterator it = this->Topology->find(ofEntity);
    if (it == this->Topology->end())
      {
      return result;
      }
    if (ofDimension >= 0 && it->second.dimension() <= ofDimension)
      {
      // can't ask for "lower" dimensional boundaries that are higher than the dimension of this cell.
      return result;
      }
    UUIDsToCells::iterator other;
    for (UUIDArray::iterator ai = it->second.relations().begin(); ai != it->second.relations().end(); ++ai)
      {
      other = this->Topology->find(*ai);
      if (other == this->Topology->end())
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
  UUIDs BoundaryEntities(const UUIDs& ofEntities, int ofDimension)
    {
    UUIDs result;
    std::insert_iterator<UUIDs> inserter(result, result.begin());
    for (UUIDs::iterator it = ofEntities.begin(); it != ofEntities.end(); ++it)
      {
      UUIDs bdy = this->BoundaryEntities(*it, ofDimension);
      std::copy(bdy.begin(), bdy.end(), inserter);
      }
    return result;
    }

  /**\brief Return lower-dimensional boundaries of the passed d-dimensional entity.
    *
    * \a lowerDimension may be any dimension < d.
    * Unlike BRepModel::BoundaryEntities(), this method will search the boundaries
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
  UUIDs LowerDimensionalBoundaries(const UUID& ofEntity, int lowerDimension)
    {
    UUIDs result;
    UUIDsToCells::iterator it = this->Topology->find(ofEntity);
    if (it == this->Topology->end())
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
      result = this->BoundaryEntities(ofEntity, currentDim--);
      for (int i = delta; i > 0; --i, --currentDim)
        {
        result = this->BoundaryEntities(result, currentDim);
        }
      }
    return result;
    }

  /**\brief Return higher-dimensional bordants of the passed d-dimensional entity.
    *
    * \a higherDimension may be any dimension > d.
    * Unlike BRepModel::BordantEntities(), this method will search the bordants
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
  UUIDs HigherDimensionalBordants(const UUID& ofEntity, int higherDimension)
    {
    UUIDs result;
    UUIDsToCells::iterator it = this->Topology->find(ofEntity);
    if (it == this->Topology->end())
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
      result = this->BordantEntities(ofEntity, currentDim++);
      for (int i = delta; i > 0; --i, ++currentDim)
        {
        result = this->BordantEntities(result, currentDim);
        }
      }
    return result;
    }

  /// Return entities of the requested dimension that share a boundary relationship with the passed entity.
  UUIDs AdjacentEntities(const UUID& ofEntity, int ofDimension)
    {
    // FIXME: Implement adjacency
    (void)ofEntity;
    (void)ofDimension;
    UUIDs result;
    return result;
    }

  /// Return all entities of the requested dimension that are present in the solid.
  UUIDs Entities(int ofDimension)
    {
    UUIDs result;
    for (UUIDWithCell it = this->Topology->begin(); it != this->Topology->end(); ++it)
      {
      if (it->second.dimension() == ofDimension)
        {
        result.insert(it->first);
        }
      }
    return result;
    }
  //@}

  const Cell* FindCell(const UUID& uid) const
    {
    UUIDWithCell it = this->Topology->find(uid);
    if (it == this->Topology->end())
      {
      return NULL;
      }
    return &it->second;
    }

  Cell* FindCell(const UUID& uid)
    {
    UUIDWithCell it = this->Topology->find(uid);
    if (it == this->Topology->end())
      {
      return NULL;
      }
    return &it->second;
    }

protected:
  UUIDsToCells* Topology;
  bool DeleteStorage;

  void RemoveCellReferences(const UUIDWithCell& c)
    {
    UUIDArray::const_iterator bit;
    Cell* ref;
    for (bit = c->second.relations().begin(); bit != c->second.relations().end(); ++bit)
      {
      ref = this->FindCell(*bit);
      if (ref)
        {
        ref->removeRelation(c->first);
        }
      }
    }
  void InsertCellReferences(const UUIDWithCell& c)
    {
    UUIDArray::const_iterator bit;
    Cell* ref;
    for (bit = c->second.relations().begin(); bit != c->second.relations().end(); ++bit)
      {
      ref = this->FindCell(*bit);
      if (ref)
        {
        ref->appendRelation(c->first);
        }
      }
    }
};

  } // model namespace
} // smtk namespace
