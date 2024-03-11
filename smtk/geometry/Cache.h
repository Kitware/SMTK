//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_geometry_Cache_h
#define smtk_geometry_Cache_h

#include "smtk/geometry/GeometryForBackend.h"
#include "smtk/geometry/Resource.h"

#include "smtk/resource/CopyOptions.h"

#include <map>

namespace smtk
{
namespace geometry
{

/**\brief A geometry provider that uses a cache to answer queries.
  *
  * This takes two template parameters:
  * + BaseClass, which specifies a movable reference to an object's geometry
  * + ThisClass, which is the subclass of this class that provides
  *   methods for obtaining bounds from geometry and computing geometry on
  *   a cache miss. Specifically, ThisClass must have the following methods:
  *     + update() — ensure every renderable object has an up-to-date cache entry.
  *     + queryGeometry(const PersistentObject::Ptr&) — return a CacheEntry for
  *       the input object (which may be invalid)
  *     + geometricBounds(const DataType&, BoundingBox&) — obtain bounds
  *       from cached geometry
  *
  */
template<typename BaseClass>
class Cache : public BaseClass
{
public:
  smtkTypeMacro(Cache);
  smtkSharedFromThisMacro(Geometry);
  smtkSuperclassMacro(BaseClass);

  using UniquePtr = Geometry::UniquePtr;
  using GenerationNumber = Geometry::GenerationNumber;
  using DataType = typename BaseClass::DataType;
  using BoundingBox = Geometry::BoundingBox;
  using Visitor = Geometry::Visitor;
  static constexpr GenerationNumber Invalid = Geometry::Invalid;
  static constexpr GenerationNumber Initial = Geometry::Initial;

  /// The values held by the geometry cache.
  struct CacheEntry
  {
    GenerationNumber m_generation; //!< A generation number or Invalid.
    DataType m_geometry;           //!< Geometry held by the cache.

    CacheEntry()
      : m_generation(Invalid)
    {
    }

    CacheEntry(CacheEntry&&) noexcept = default;
    CacheEntry(const CacheEntry&) = default;
    CacheEntry(GenerationNumber gen, const DataType& data)
      : m_generation(gen)
      , m_geometry(data)
    {
    }
    CacheEntry& operator=(const CacheEntry&) = default;

    bool isValid() const { return m_generation != Invalid; }
  };

  /**\brief Create a geometric representation of an object on demand.
    *
    * Subclasses should override this unless the resource keeps the
    * cache up to date via some other method not provided by this class.
    * In that case, the provided implementation should never be invoked;
    * if it is, it will result in cache entries being deleted.
    *
    * This method should *always* edit the cache entry,
    * either by marking it invalid (assigning Invalid to m_generation)
    * or updating the geometry in m_geometry.
    * This method should never modify the cache itself.
    *
    * Anything that calls this method is responsible for checking
    * whether existing cache entries are up to date before calling
    * this method. Only call this when the cache *must* be updated.
    *
    * If the cache entry is valid upon entry, this method should
    * either update the geometry and increment the generation number
    * or mark the cache entry as invalid.
    *
    * Returning the the entry in an invalid state indicates that the
    * object has no geometric representation and any existing cache
    * entry will be removed.
    */
  virtual void queryGeometry(const smtk::resource::PersistentObject::Ptr&, CacheEntry& cache) const
  {
    cache.m_generation = Invalid;
  }

  /// Return the generation number of geometry held by the cache for the given object.
  ///
  /// If no cache entry existed previously, this will construct one.
  GenerationNumber generationNumber(const smtk::resource::PersistentObject::Ptr& obj) const override
  {
    if (obj)
    {
      auto it = m_cache.find(obj->id());
      bool found = it != m_cache.end();
      if (found && it->second.m_geometry)
      { // Cache is clean.
        return it->second.m_generation;
      }
      else if (found)
      { // Cache was marked dirty.
        this->queryGeometry(obj, it->second);
        if (!it->second.isValid())
        {
          m_cache.erase(it);
        }
        else
        {
          return it->second.m_generation;
        }
      }
      else
      { // No cache entry yet; try to add one.
        CacheEntry entry;
        this->queryGeometry(obj, entry);
        if (entry.isValid())
        {
          m_cache[obj->id()] = entry;
          return entry.m_generation;
        }
      }
    }
    return Invalid;
  }

  /// Return the geometric bounds
  void bounds(const smtk::resource::PersistentObject::Ptr& obj, BoundingBox& bds) const override
  {
    if (obj)
    {
      auto it = m_cache.find(obj->id());
      bool found = it != m_cache.end();
      if (found && it->second.m_geometry)
      { // Cache is clean:
        this->geometricBounds(it->second.m_geometry, bds);
        return;
      }
      else if (found)
      { // Cache was marked dirty. Update it:
        this->queryGeometry(obj, it->second);
        if (it->second.isValid())
        {
          this->geometricBounds(it->second.m_geometry, bds);
          return;
        }
        else
        {
          m_cache.erase(it);
        }
      }
      else
      {
        CacheEntry entry;
        this->queryGeometry(obj, entry);
        if (entry.isValid())
        {
          m_cache[obj->id()] = entry;
          this->geometricBounds(entry.m_geometry, bds);
          return;
        }
      }
    }
    // Object is invalid or has no geometry; return invalid bounds.
    bds[0] = bds[2] = bds[4] = 0.0;
    bds[1] = bds[3] = bds[5] = -1.0;
  }

  /// Provide access to the actual cached geometry reference.
  DataType& data(const smtk::resource::PersistentObject::Ptr& obj) const override
  {
    if (obj)
    {
      auto it = m_cache.find(obj->id());
      bool found = it != m_cache.end();
      if (found && it->second.m_geometry)
      { // Cache is clean.
        return it->second.m_geometry;
      }
      else if (found)
      { // Cache was marked dirty. Update it:
        this->queryGeometry(obj, it->second);
        if (it->second.isValid())
        {
          return it->second.m_geometry;
        }
        else
        {
          m_cache.erase(it);
        }
      }
      else
      { // No cache existed. See if we can create an entry:
        CacheEntry entry;
        this->queryGeometry(obj, entry);
        if (entry.isValid())
        {
          m_cache[obj->id()] = entry;
          return m_cache[obj->id()].m_geometry;
        }
      }
    }
    static DataType invalid;
    return invalid;
  }

  /// Visit each persistent object that has renderable geometry.
  ///
  /// This implementation calls ThisClass::update() to
  /// ensure the cache contains up-to-date geometry for every
  /// renderable object. If the cache is always up-to-date,
  /// the ThisClass::update() is a no-op. Otherwise,
  /// it is the subclass's duty to visit the resource's persistent
  /// objects and query each for an updated cache entry as needed.
  ///
  /// Visitors may erase the cache entry for the object they are
  /// passed, but may not erase others as that may invalidate
  /// iteration.
  void visit(Visitor visitor) const override
  {
    this->update();
    auto rsrc = this->resource();
    if (rsrc)
    {
      auto it = m_cache.begin();
      for (auto entry = it; entry != m_cache.end(); entry = it)
      {
        ++it;
        auto comp = rsrc->find(entry->first);
        if (comp)
        {
          visitor(comp, entry->second.m_generation);
        }
        else if (entry->first == rsrc->id())
        {
          visitor(rsrc, entry->second.m_generation);
        }
        // else remove cache entry?
      }
    }
  }

  /// Copy data from another backend as directed by \a options.
  ///
  /// This implementation will only work if the \a source geometry
  /// is the same type as this object.
  bool copyGeometry(const UniquePtr& source, smtk::resource::CopyOptions& options) override
  {
    // Exit without error if we should not copy renderable geometry.
    if (!options.copyGeometry())
    {
      return true;
    }

    // Error out if given data of a different type.
    const auto* sourceCache = dynamic_cast<Cache*>(source.get());
    if (!sourceCache)
    {
      return false;
    }

    for (const auto& entry : sourceCache->m_cache)
    {
      if (options.shouldOmitId(entry.first))
      {
        continue;
      }
      if (
        auto* targetObj =
          options.targetObjectFromSourceId<smtk::resource::PersistentObject>(entry.first))
      {
        m_cache[targetObj->id()] = entry.second;
      }
      else
      {
        m_cache[entry.first] = entry.second;
      }
    }
    return true;
  }

  /// Remove a cache entry's geometry (but keep its generation number intact).
  ///
  /// This method should be called by resources when an object has had
  /// its geometry modified; it results in the cached geometry being freed
  /// so that the next request for bounds or geometry will require recomputation.
  void markModified(const smtk::resource::PersistentObject::Ptr& obj) override
  {
    if (obj)
    {
      ++this->BaseClass::m_lastModified;
      auto it = m_cache.find(obj->id());
      if (it != m_cache.end())
      {
        DataType blank; // Assume default constructor creates "null" data.
        it->second.m_geometry = blank;
      }
      else
      {
        CacheEntry invalid;
        m_cache[obj->id()] = invalid;
      }
    }
  }

  /// Remove a cache entry completely.
  ///
  /// This method should be called by resources/operations when an object
  /// is about to be or has been deleted.
  /// In this case, not only is the geometry freed, but the cache entry
  /// is also removed so that visitation will no longer query the resource
  /// for geometry with the given UUID.
  bool erase(const smtk::common::UUID& uid) override { return m_cache.erase(uid) > 0; }

protected:
  mutable std::map<smtk::common::UUID, CacheEntry> m_cache;
};

} // namespace geometry
} // namespace smtk

#endif // smtk_geometry_Cache_h
