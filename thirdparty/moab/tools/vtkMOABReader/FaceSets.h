#ifndef __smoab_FaceSets_h
#define __smoab_FaceSets_h

#include "CellSets.h"
#include <set>

namespace smoab
{
//----------------------------------------------------------------------------
class FaceCellSet : public CellSet
{
public:
  FaceCellSet(int id, smoab::EntityHandle p,const smoab::Range& cells):
    CellSet(p,cells),
    ID(id)
    {}

  int faceId() const { return ID; }
  void overrideFaceId(int i) { ID = i; } //USE AT YOUR OWN RISK

private:
  int ID;
};


//----------------------------------------------------------------------------
typedef std::vector<FaceCellSet> FaceCellSets;


//----------------------------------------------------------------------------
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

  FacesAdjRegions(int f):
    FaceId(f),
    Region0(-3),
    Region1(-2)
    {}

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
//----------------------------------------------------------------------------
smoab::FaceCellSets findFaceSets(smoab::CellSets shells,
                                 smoab::CellSets boundaries,
                                 std::set<smoab::FacesAdjRegions>& faceMaps)
{
  typedef smoab::CellSets::iterator iterator;
  typedef smoab::FaceCellSets::iterator faceIterator;
  typedef std::set<smoab::FacesAdjRegions>::const_iterator FaceAdjIterator;

  //we need to properly label each unique face in shells
  //we do this by intersecting each shell with each other shell
  //to find shell on shell contact, and than we intersect each
  //resulting shell with the boundary conditions
  //the end result of these intersections will be the new modelfaces
  int faceId = 1;
  smoab::FaceCellSets shellFaces;

  //first intersect each shell with each other shell
  std::set<smoab::FacesAdjRegions> shellFaceContacts;
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
        smoab::FaceCellSet face(faceId++,i->entity(),intersection);
        shellFaces.push_back(face);
        i->erase(intersection);
        j->erase(intersection);
        numCells -= intersection.size();

        //add this to the face map
        smoab::FacesAdjRegions faceInfo(faceId-1,i->entity(),j->entity());
        shellFaceContacts.insert(faceInfo);
        }
      }
    //if all the cells for shell i are used, don't add a new
    //empty face
    if(numCells > 0)
      {
      smoab::FaceCellSet face(faceId++,i->entity(),i->cells());
      shellFaces.push_back(face);

      //add this to the face map
      smoab::FacesAdjRegions faceInfo(faceId-1,-1,i->entity());
      shellFaceContacts.insert(faceInfo);
      }
    }

  //now we have all the faces that match shell on shell contact
  //we know process all the new faces to see if they intersect
  //with any boundary sets. A boundary set can span multiple
  //shells so we want to process it as a second loop

  //store the end before we start adding boundary faces, which
  //we don't need to check agianst other boundaries
  faceId = 1; //reset the faced id

  //store in a new face set, expanding the current one causes incorrect results
  smoab::FaceCellSets faces;
  for(faceIterator i=shellFaces.begin();i != shellFaces.end();++i)
    {
    //determine from the shell faces if the new face we are creating
    //is bounded by two regions or just one
    smoab::FacesAdjRegions idToSearchFor(i->faceId());
    FaceAdjIterator adjRegions = shellFaceContacts.find(idToSearchFor);
    smoab::EntityHandle otherRegionId = adjRegions->otherId(i->entity());

    int numCells = i->cells().size(); //size() on range is slow, so cache it
    for(iterator j=boundaries.begin();j != boundaries.end(); ++j)
      {
      smoab::Range intersect = smoab::intersect(i->cells(),j->cells());
      if(!intersect.empty())
          {
          //don't want to increment faceId when the intersection is empty
          smoab::FaceCellSet face(faceId++,j->entity(),intersect);
          faces.push_back(face);
          i->erase(intersect);
          numCells -= intersect.size();
          smoab::FacesAdjRegions faceInfo(faceId-1,i->entity(),otherRegionId);
          faceMaps.insert(faceInfo);
          }
      }
    if(numCells > 0)
      {
      smoab::FaceCellSet face(faceId++,i->entity(),i->cells());
      faces.push_back(face);
      smoab::FacesAdjRegions faceInfo(faceId-1,i->entity(),otherRegionId);
      faceMaps.insert(faceInfo);
      }
    }
  return faces;
}

//----------------------------------------------------------------------------
template<typename T>
std::vector<T> faceIdsPerCell(const smoab::FaceCellSets& faces)
{
  typedef smoab::FaceCellSets::const_iterator iterator;
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

//----------------------------------------------------------------------------
//given a face adjacency, determine the regions spare tag values
template<typename T>
std::pair<T,T> FaceAdjRegionValues(const smoab::FacesAdjRegions& faceAdj,
                                   smoab::Tag* t,
                                   const smoab::Interface& interface)
  {
  std::pair<T,T> returnValue;
  const int defaultValue = interface.getDefaultTagVaue<int>(*t);

  /*
   *  IF A REGION IS SET TO -1 WE NEED TO PUSH THAT VALUE DOWN
   *  AS THE MATERIAL, SINCE THE MOAB DEFAULT TAG VALUE WILL
   *  BE CONSIDIERED A REGION, AND WE WANT TO SAY IT BOUNDS THE
   *  VOID REGION
   */
  int tagValue = defaultValue; //use tagValue to pass in default value
  if(faceAdj.Region0 != -1)
    {
    tagValue = interface.getTagData(*t,faceAdj.Region0,tagValue);
    }
  else
    {
    tagValue = -1;
    }

  //set the first region tag value into the pair we are returing
  returnValue.first = static_cast<T>(tagValue);

  tagValue = defaultValue; //use tagValue to pass in default value
  tagValue = interface.getTagData(*t,faceAdj.Region1,tagValue);
  if(faceAdj.Region1 != -1)
    {
    tagValue = interface.getTagData(*t,faceAdj.Region1,tagValue);
    }
  else
    {
    tagValue = -1;
    }
 returnValue.second = static_cast<T>(tagValue);

  return returnValue;
  }

 } //smoab

#endif
