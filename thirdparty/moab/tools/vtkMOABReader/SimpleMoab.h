
#ifndef __smoab_SimpleMoab_h
#define __smoab_SimpleMoab_h

#include "moab/Core.hpp"
#include "moab/Interface.hpp"
#include "moab/Range.hpp"
#include "moab/CN.hpp"

#include "MBTagConventions.hpp"

#include <iostream>

namespace smoab
{
//adjacency intersect / union named enum to match
//the types from moab
enum adjacency_type{INTERSECT=moab::Interface::INTERSECT,
                    UNION=moab::Interface::UNION};

//make our range equal to moabs range
typedef moab::Range Range;
typedef moab::EntityHandle EntityHandle;
typedef moab::EntityType EntityType;

//bring in range functions
using moab::intersect;
using moab::subtract;
using moab::unite;

class Tag
{
  const std::string Name_;
public:
  Tag(std::string const& n):Name_(n)
    {
    }

  virtual ~Tag()
    {
    }

  const char* name() const { return this->Name_.c_str(); }
  moab::DataType virtual dataType() const { return moab::MB_TYPE_INTEGER; }
  virtual bool isComparable() const { return false; }
  virtual int value() const { return int(); }
};

//lightweight structs to wrap set names, so we detected
//incorrect names at compile time. In the future I expect material and
//boundary conditions to be comparable
class MaterialTag : public Tag{ public: MaterialTag():Tag("MATERIAL_SET"){}};
class DirichletTag : public Tag{ public: DirichletTag():Tag("DIRICHLET_SET"){}};
class NeumannTag: public Tag{public:  NeumannTag():Tag("NEUMANN_SET"){}};
class GroupTag: public Tag{ public: GroupTag():Tag("GROUP"){}};

//geom is the only comparable tag, since it can have a dimension.
class GeomTag: public Tag
  {
  int dim;
public:
  GeomTag(int d):Tag("GEOM_DIMENSION"),dim(d){}
  GeomTag():Tag("GEOM_DIMENSION"), dim(0){}

  virtual ~GeomTag(){}

  bool isComparable() const { return dim > 0; }
  int value() const { return dim; }
  };


//forward declare this->Moab for Tag
struct Interface;

//forward declare the DataSetConverter so it can be a friend of Interface
class DataSetConverter;

//forward declare the LoadGeometry so it can be a friend of Interface
namespace detail{ class LoadGeometry; }
namespace detail{ class LoadPoly; }


//light weight wrapper on a moab this->Moab that exposes only the reduced class
//that we need
class Interface
{
public:
  Interface(const std::string &file)
    {
    this->Moab = new moab::Core();
    this->Moab->load_file(file.c_str());
    }

  ~Interface()
    {
    if(this->Moab)
      {
      delete this->Moab;
      this->Moab = NULL;
      }
    }

  //----------------------------------------------------------------------------
  moab::Tag getMoabTag(const smoab::Tag& simpleTag) const
    {
    moab::Tag tag;
    this->Moab->tag_get_handle(simpleTag.name(),
                               1,
                               simpleTag.dataType(),
                               tag);
    return tag;
    }

  //----------------------------------------------------------------------------
  template<typename T>
  T getDefaultTagVaue(moab::Tag tag) const
    {
    T defaultValue;
    this->Moab->tag_get_default_value(tag,&defaultValue);
    return defaultValue;
    }

  //----------------------------------------------------------------------------
  template<typename T>
  T getDefaultTagVaue(smoab::Tag tag) const
    {
    return this->getDefaultTagVaue<T>(getMoabTag(tag));
    }

  //----------------------------------------------------------------------------
  template<typename T>
  T getTagData(moab::Tag tag, const smoab::EntityHandle& entity, T value) const
    {
    this->Moab->tag_get_data(tag,&entity,1,&value);
    return value;
    }

  //----------------------------------------------------------------------------
  template<typename T>
  T getTagData(smoab::Tag tag, const smoab::EntityHandle& entity, T value = T()) const
    {
    return this->getTagData(getMoabTag(tag),entity,value);
    }

  //----------------------------------------------------------------------------
  //returns the moab name for the given entity handle if it has a sparse Name tag
  std::string name(const smoab::EntityHandle& entity) const
    {
    moab::Tag nameTag;
    moab::ErrorCode rval = this->Moab->tag_get_handle(NAME_TAG_NAME,
                                                      NAME_TAG_SIZE,
                                                      moab::MB_TYPE_OPAQUE,
                                                      nameTag);
    if(rval != moab::MB_SUCCESS) { return std::string(); }

    char name[NAME_TAG_SIZE];
    rval = this->Moab->tag_get_data(nameTag,&entity,1,&name);
    if(rval != moab::MB_SUCCESS) { return std::string(); }

    return std::string(name);
    }

  //----------------------------------------------------------------------------
  //returns the geometeric dimension of an entity.
  int dimension(const smoab::EntityHandle& entity) const
    {
    return this->Moab->dimension_from_handle(entity);
    }

  //----------------------------------------------------------------------------
  //returns the geometeric dimension of an entity.
  smoab::EntityType entityType(const smoab::EntityHandle& entity) const
    {
    return this->Moab->type_from_handle(entity);
    }

  //----------------------------------------------------------------------------
  smoab::EntityHandle getRoot() const { return this->Moab->get_root_set(); }

  //----------------------------------------------------------------------------
  smoab::Range findEntities(const smoab::EntityHandle root, moab::EntityType type) const
    {
    smoab::Range result;
    // get all the sets of that type in the mesh
    this->Moab->get_entities_by_type(root, type, result);
    return result;
    }

  //----------------------------------------------------------------------------
  //given a single entity handle find all items in that mesh set that aren't
  //them selves entitysets. If recurse is true we also recurse sub entitysets
  smoab::Range findAllMeshEntities(smoab::EntityHandle const& entity,
                                   bool recurse=false) const
    {
    smoab::Range result;
    this->Moab->get_entities_by_handle(entity,result,recurse);
    return result;
    }

  //----------------------------------------------------------------------------
  //Find all entities with a given tag. We don't use geom as a tag as that
  //isn't a fast operation. Yes finding the intersection of geom entities and
  //a material / boundary tag will be more work, but it is rarely done currently
  //Returns the found group of entities
  smoab::Range findEntitiesWithTag (const smoab::Tag& tag, smoab::EntityHandle root,
                                    moab::EntityType type = moab::MBENTITYSET) const
    {
    smoab::Range result;

    moab::Tag t = this->getMoabTag(tag);

    // get all the entities of that type in the mesh
    this->Moab->get_entities_by_type_and_tag(root, type, &t, NULL, 1,result);


    if(tag.isComparable())
      {
      int value=0;
      //now we have to remove any that doesn't match the tag value
      smoab::Range resultMatchingTag;
      typedef moab::Range::const_iterator iterator;
      for(iterator i=result.begin();
          i != result.end();
          ++i)
        {
        value = 0;
        moab::EntityHandle handle = *i;
        this->Moab->tag_get_data(t, &handle, 1, &value);
        if(value == tag.value())
          {
          resultMatchingTag.insert(*i);
          }
        }

      return resultMatchingTag;
      }
    else
      {
      //we return all the items we found
      return result;
      }
    }

  //----------------------------------------------------------------------------
  //Find all entities from a given root of a given dimensionality
  smoab::Range findEntitiesWithDimension(const smoab::EntityHandle root,
                                         const int dimension,
                                         bool recurse=false) const
    {
    typedef smoab::Range::const_iterator iterator;

    smoab::Range result;
    this->Moab->get_entities_by_dimension(root,dimension,result,recurse);

    if(recurse)
      {
      smoab::Range children;
      this->Moab->get_child_meshsets(root,children,0);
      for(iterator i=children.begin(); i !=children.end();++i)
        {
        this->Moab->get_entities_by_dimension(*i,dimension,result);
        }
      }
    return result;
    }


  //----------------------------------------------------------------------------
  smoab::Range findHighestDimensionEntities(const smoab::EntityHandle& entity,
                                            bool recurse=false) const
    {
    //the goal is to load all entities that are not entity sets of this
    //node, while also subsetting by the highest dimension

    //lets find the entities of only the highest dimension
    int num_ents=0;
    int dim=3;
    while(num_ents<=0&&dim>0)
      {
      this->Moab->get_number_entities_by_dimension(entity,dim,num_ents,recurse);
      --dim;
      }
    ++dim; //reincrement to correct last decrement
    if(num_ents > 0)
      {
      //we have found entities of a given dimension
      return this->findEntitiesWithDimension(entity,dim,recurse);
      }
    return smoab::Range();
    }

  //----------------------------------------------------------------------------
  //Find all elements in the database that have children and zero parents.
  //this doesn't find
  smoab::Range findEntityRootParents(const smoab::EntityHandle& root) const
    {
    smoab::Range parents;

    typedef moab::Range::const_iterator iterator;
    moab::Range sets;

    this->Moab->get_entities_by_type(root, moab::MBENTITYSET, sets);
    for(iterator i=sets.begin(); i!=sets.end();++i)
      {
      int numParents=0,numChildren=0;
      this->Moab->num_parent_meshsets(*i,&numParents);
      if(numParents==0)
        {
        this->Moab->num_child_meshsets(*i,&numChildren);
        if(numChildren>=0)
          {
          parents.insert(*i);
          }
        }
      }
    return parents;
    }

  //----------------------------------------------------------------------------
  //finds entities that have zero children and zero parents
  smoab::Range findDetachedEntities(const moab::EntityHandle& root) const
    {
    smoab::Range detached;

    typedef moab::Range::const_iterator iterator;
    moab::Range sets;

    this->Moab->get_entities_by_type(root, moab::MBENTITYSET, sets);
    for(iterator i=sets.begin(); i!=sets.end();++i)
      {
      int numParents=0,numChildren=0;
      this->Moab->num_parent_meshsets(*i,&numParents);
      if(numParents==0)
        {
        this->Moab->num_child_meshsets(*i,&numChildren);
        if(numChildren==0)
          {
          detached.insert(*i);
          }
        }
      }
    return detached;
    }

  //----------------------------------------------------------------------------
  //find all children of the entity passed in that has multiple parents
  smoab::Range findEntitiesWithMultipleParents(const smoab::EntityHandle& root) const
    {
    smoab::Range multipleParents;
    typedef moab::Range::const_iterator iterator;

    //for all the elements in the range, find all items with multiple parents
    moab::Range children;
    this->Moab->get_child_meshsets(root,children,0);
    for(iterator i=children.begin(); i!=children.end();++i)
      {
      int numParents=0;
      this->Moab->num_parent_meshsets(*i,&numParents);
      if(numParents>1)
        {
        multipleParents.insert(*i);
        }
      }
    return multipleParents;
    }

  //----------------------------------------------------------------------------
  //find all entities that are adjacent to a single entity
  smoab::Range findAdjacencies(const smoab::EntityHandle& entity,
                               int dimension) const
    {
    const int adjType = static_cast<int>(smoab::INTERSECT);
    smoab::Range result;
    const bool create_if_missing = false;
    this->Moab->get_adjacencies(&entity,
                                1,
                                dimension,
                                create_if_missing,
                                result,
                                adjType);
    return result;
    }

  //----------------------------------------------------------------------------
  smoab::Range findAdjacencies(const smoab::Range& range,
                                    int dimension,
                                    const smoab::adjacency_type type = smoab::UNION) const
    {
    //the smoab and moab adjacent intersection enums are in the same order
    const int adjType = static_cast<int>(type);
    smoab::Range result;
    const bool create_if_missing = false;
    this->Moab->get_adjacencies(range,dimension,
                                create_if_missing,
                                result,
                                adjType);

    return result;
    }

  //----------------------------------------------------------------------------
  //create adjacencies, only works when the dimension requested is lower than
  //dimension of the range of entities
  smoab::Range createAdjacencies(const smoab::Range& range,
                                 int dimension,
                                 const smoab::adjacency_type type = smoab::UNION) const
    {
    //the smoab and moab adjacent intersection enums are in the same order
    const int adjType = static_cast<int>(type);
    smoab::Range result;
    const bool create_if_missing = true;
    this->Moab->get_adjacencies(range,dimension,
                                create_if_missing,
                                result,
                                adjType);

    return result;
    }

  //----------------------------------------------------------------------------
  int numChildMeshSets(const smoab::EntityHandle& root) const
    {
    int numChildren;
    this->Moab->num_child_meshsets(root,&numChildren);
    return numChildren;
    }

  //----------------------------------------------------------------------------
  smoab::Range getChildSets(const smoab::EntityHandle& root) const
    {
    smoab::Range children;
    this->Moab->get_child_meshsets(root,children,0);
    return children;
    }

  //----------------------------------------------------------------------------
  //remove a collection of entities from the database
  void remove(smoab::Range const& toDelete) const
    {
    this->Moab->delete_entities(toDelete);
    }

  //----------------------------------------------------------------------------
  //a entityHandle with value zero means no side element was found
  smoab::EntityHandle sideElement(smoab::EntityHandle const& cell,
                                  int dim, int side) const
    {
    smoab::EntityHandle result(0);
    this->Moab->side_element(cell,dim,side,result);
    return result;
    }

  //----------------------------------------------------------------------------
  //returns all the existing side elements of a cell, elements that
  //are zero mean that side element doesn't exist
  std::vector<smoab::EntityHandle> sideElements(
                                    smoab::EntityHandle const& cell,
                                    int dim) const
    {
    const EntityType volumeCellType = this->Moab->type_from_handle(cell);
    const int numSides = static_cast<int>(moab::CN::NumSubEntities(
                                          volumeCellType, dim));

    std::vector<smoab::EntityHandle> result(numSides);
    for (int side = 0; side < numSides; ++side)
      {
      smoab::EntityHandle *sideElem = &result[side]; //get memory of vector
      this->Moab->side_element(cell,dim,side,*sideElem);
      }
    return result;
    }

  //----------------------------------------------------------------------------
  //prints all elements in a range objects
  void printRange(const smoab::Range& range) const
    {
    typedef Range::const_iterator iterator;
    for(iterator i=range.begin(); i!=range.end(); ++i)
      {
      std::cout << "entity id: " << *i << std::endl;
      this->Moab->list_entity(*i);
      }
    }
  friend class smoab::DataSetConverter;
  friend class smoab::detail::LoadGeometry;
  friend class smoab::detail::LoadPoly;
private:
  moab::Interface* Moab;
};

//----------------------------------------------------------------------------
void RangeToVector(const smoab::Range &range,
                   std::vector<smoab::EntityHandle>& vector )
{
  vector.reserve(range.size());
  std::copy(range.begin(),
            range.end(),
            std::back_inserter(vector));
}



}

#endif
