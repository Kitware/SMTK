//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smoab_model_FaceSets_h
#define __smoab_model_FaceSets_h

#include "CellSets.h"
#include <map>
#include <set>

namespace smoab {
namespace model {

const smoab::EntityHandle VOID_REGION=0;

class FaceCellSet : public CellSet
{
public:
  FaceCellSet(int id, smoab::EntityHandle p,const smoab::Range& c):
    CellSet(p,c),
    ID(id)
    {}

  int faceId() const { return ID; }
  void overrideFaceId(int i) { ID = i; } //USE AT YOUR OWN RISK

private:
  int ID;
};

typedef std::vector<FaceCellSet> FaceCellSets;

//class that store the regions that a faces is adjacent too
struct FacesAdjRegions
{
  FacesAdjRegions(int f, smoab::EntityHandle r0, smoab::EntityHandle r1):
    FaceId(f),
    Region0(r0),
    Region1(r1)
    {
    if (r0 > r1)
      {
      std::swap(this->Region0,this->Region1);
      }
    }

  bool operator<(const FacesAdjRegions& other) const
    {
    return (this->FaceId < other.FaceId);
    }

  smoab::EntityHandle otherId(smoab::EntityHandle other) const
    {
    if(other == Region0)
      {
      return Region1;
      }
    return Region0;
    }

  int FaceId;
  smoab::EntityHandle Region0;
  smoab::EntityHandle Region1;
};

smoab::model::FaceCellSets findFaceSets(smoab::CellSets shells,
                                 smoab::CellSets boundaries,
                                 std::set<smoab::model::FacesAdjRegions>& faceMaps)
{
  typedef smoab::CellSets::iterator iterator;
  typedef smoab::model::FaceCellSets::iterator faceIterator;
  typedef std::set<smoab::model::FacesAdjRegions>::const_iterator FaceAdjIterator;

  //we need to properly label each unique face in shells
  //we do this by intersecting each shell with each other shell
  //to find shell on shell contact, and than we intersect each
  //resulting shell with the boundary conditions
  //the end result of these intersections will be the new modelfaces
  int faceId = 1;
  smoab::model::FaceCellSets shellFaces;

  //first intersect each shell with each other shell
  std::set<smoab::model::FacesAdjRegions> shellFaceContacts;
  for(iterator i=shells.begin();i!= shells.end();++i)
    {
    //copy the cells so we can add a face that represents
    //all the cells of the region that aren't shared with another region
    int numCells = i->cells().size(); //size() on range is slow, so cache it
    for(iterator j = i+1;
        j != shells.end() && numCells > 0;
        ++j)
      {
      //intersect i and j to make a new face
      smoab::Range intersection = smoab::intersect(i->cells(),j->cells());
      if(!intersection.empty())
        {
        //don't want to increment faceId when the intersection is empty
        smoab::model::FaceCellSet face(faceId,i->entity(),intersection);
        shellFaces.push_back(face);
        i->erase(intersection);
        j->erase(intersection);
        numCells -= intersection.size();

        //add this to the face map
        smoab::model::FacesAdjRegions faceInfo(faceId,i->entity(),j->entity());
        shellFaceContacts.insert(faceInfo);

        ++faceId;
        }
      }
    //if all the cells for shell i are used, don't add a new
    //empty face
    if(numCells > 0)
      {
      smoab::model::FaceCellSet face(faceId,i->entity(),i->cells());
      shellFaces.push_back(face);

      //add this to the face map
      smoab::model::FacesAdjRegions faceInfo(faceId,VOID_REGION,i->entity());
      shellFaceContacts.insert(faceInfo);

      ++faceId;
      }
    }

  //now we have all the faces that match shell on shell contact
  //we know process all the new faces to see if they intersect
  //with any boundary sets. A boundary set can span multiple
  //shells so we want to process it as a second loop
  if(boundaries.size()==0)
    {
    //no boundaries we are done.
    faceMaps = shellFaceContacts;
    return shellFaces;
    }

  //store the end before we start adding boundary faces, which
  //we don't need to check agianst other boundaries
  faceId = 0; //reset the faced id

  //store in a new face set, expanding the current one causes incorrect results
  smoab::model::FaceCellSets faces;
  for(faceIterator i=shellFaces.begin();i != shellFaces.end();++i)
    {
    //determine from the shell faces if the new face we are creating
    //is bounded by two regions or just one
    smoab::model::FacesAdjRegions idToSearchFor(i->faceId(),-3,-2);
    FaceAdjIterator adjRegions = shellFaceContacts.find(idToSearchFor);
    smoab::EntityHandle otherRegionId = adjRegions->otherId(i->entity());

    int numCells = i->cells().size(); //size() on range is slow, so cache it
    for(iterator j=boundaries.begin();j != boundaries.end(); ++j)
      {
      smoab::Range intersect = smoab::intersect(i->cells(),j->cells());
      if(!intersect.empty())
          {
          //don't want to increment faceId when the intersection is empty
          smoab::model::FaceCellSet face(faceId,j->entity(),intersect);
          faces.push_back(face);
          i->erase(intersect);
          numCells -= intersect.size();
          smoab::model::FacesAdjRegions faceInfo(faceId,i->entity(),otherRegionId);
          faceMaps.insert(faceInfo);

          ++faceId;
          }
      }
    if(numCells > 0)
      {
      smoab::model::FaceCellSet face(faceId,i->entity(),i->cells());
      faces.push_back(face);
      smoab::model::FacesAdjRegions faceInfo(faceId,i->entity(),otherRegionId);
      faceMaps.insert(faceInfo);

      ++faceId;
      }
    }
  return faces;
}

template<typename T>
std::vector<T> faceIdsPerCell(const smoab::model::FaceCellSets& faces)
{
  typedef smoab::model::FaceCellSets::const_iterator iterator;
  typedef std::vector<smoab::EntityHandle>::const_iterator EntityHandleIterator;
  typedef smoab::Range::const_iterator RangeIterator;

  //find all the cells that are in the faceCellSet, and than map
  //the proper face id to that relative position, here comes lower_bounds!
  std::vector<smoab::EntityHandle> searchableCells;
  smoab::Range faceRange = smoab::getAllCells(faces);
  smoab::RangeToVector(faceRange,searchableCells);

  //faceIds will be the resulting array
  std::vector<T> faceIds(searchableCells.size());

  //construct the start and end iterators for the lower bounds call

  EntityHandleIterator s_begin = searchableCells.begin();
  EntityHandleIterator s_end = searchableCells.end();

  //search the face cell sets
  for(iterator i=faces.begin(); i!=faces.end(); ++i)
    {
    T value = static_cast<T>(i->faceId());
    const smoab::Range& entitiesCells = i->cells();
    for(RangeIterator j=entitiesCells.begin(); j != entitiesCells.end();++j)
      {
      EntityHandleIterator result = std::lower_bound(s_begin,
                                                     s_end,
                                                     *j);
      std::size_t newId = std::distance(s_begin,result);
      faceIds[newId] = value;
      }
    }
  return faceIds;
}

template<typename T>
std::map<smoab::EntityHandle,T> findRegions(
    const std::set<smoab::model::FacesAdjRegions>& regionFaceAdj,
    const smoab::Interface& interface)
  {
  //lets talk about what this is doing.
  //When we are looking to determine regions we need to walk the geometric
  //parent child relationship. We are basically trying to find
  //for each region the 3d geometric parent. that will become the region id
  //for that cell

  smoab::GeomTag threeDTag(3);
  smoab::EntityHandle root = interface.getRoot();
  smoab::Range realParents = interface.findEntitiesWithTag(threeDTag,root);

  std::set<smoab::EntityHandle> childIds;
  typedef typename std::set<smoab::model::FacesAdjRegions>::const_iterator iterator;
  for(iterator i=regionFaceAdj.begin(); i!=regionFaceAdj.end();++i)
    {
    childIds.insert(i->Region0);
    childIds.insert(i->Region1);
    }

  T newParentId = T();

  typedef typename std::map<smoab::EntityHandle,T>::iterator parentIterator;
  std::map<smoab::EntityHandle,T> foundParents;
  std::map<smoab::EntityHandle,T> returnValues;
  typedef typename std::set<smoab::EntityHandle>::const_iterator set_iterator;
  for(set_iterator i=childIds.begin(); i != childIds.end(); ++i)
    {
    typedef typename smoab::Range::const_iterator range_iterator;

    /*
     *  IF A REGION IS SET TO VOID_REGION WE NEED TO PUSH THAT VALUE DOWN
     *  AS THE MATERIAL, SINCE THE MOAB DEFAULT TAG VALUE WILL
     *  BE CONSIDIERED A REGION, AND WE WANT TO SAY IT BOUNDS THE
     *  VOID REGION
     *
     */
    if(*i == VOID_REGION)
      {
      //we represents void with -1, while moab represents it with zero
      returnValues[VOID_REGION]= -1;
      continue;
      }

    smoab::Range parents = interface.findParentEntities(*i);
    T currentParentId = newParentId;
    if(!parents.empty())
      {
      //we presume that the intersection will only return a single value
      smoab::Range s = smoab::intersect(parents,realParents);
      const smoab::EntityHandle& p = s.front();

      //check if we have this parent in the map already
      parentIterator fp = foundParents.find(p);
      if(fp == foundParents.end())
        {
        //this parent id has never been seen before. Add it to the found parent
        //map incase another child is owned by it.
        std::pair<smoab::EntityHandle,T> parentMatch(p,currentParentId);
        foundParents.insert(parentMatch);
        ++newParentId;
        }
      else
        {
        currentParentId = fp->second;
        }
      }
    else
      {
      //no match so we increment the parent id
      ++newParentId;
      }
    std::pair<smoab::EntityHandle,T> match(*i,currentParentId);
    returnValues.insert(match);
    }

  return returnValues;
  }

template<typename T>
std::map<smoab::EntityHandle,T> findDomains(const smoab::CellSets& parentSets,
    const smoab::Tag *domainTag,
    const smoab::Interface& interface)
  {

  //the basic model of attack is as follow.
  //for each entity in the parent sets, find the tag value and convert it to
  //type T

  std::map<smoab::EntityHandle,T> returnValues;

  typedef smoab::CellSets::const_iterator iterator;
  for(iterator i=parentSets.begin(); i != parentSets.end(); ++i)
    {
    std::pair<smoab::EntityHandle,T> key_value;
    key_value.first = i->entity();
    key_value.second = interface.getTagData<T>(*domainTag,i->entity());
    returnValues.insert(key_value);
    }
  return returnValues;
  }

 } } //smoab::model

#endif
