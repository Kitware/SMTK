#include "smtk/model/Face.h"

#include "smtk/model/Edge.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Volume.h"

namespace smtk {
  namespace model {

smtk::model::Edges Face::edges() const
{
  Edges result;
  Cursors all = this->boundaryEntities(/*dim = */ 1);
  for (Cursors::iterator it = all.begin(); it != all.end(); ++it)
    {
    if (it->isEdge())
      result.push_back(*it);
    }
  return result;
}

smtk::model::Volumes Face::volumes() const
{
  Volumes result;
  Cursors all = this->bordantEntities(/*dim = */ 3);
  for (Cursors::iterator it = all.begin(); it != all.end(); ++it)
    {
    if (it->isVolume())
      result.push_back(*it);
    }
  return result;
}

/**\brief Return the face-use with its sense opposite the natural normal.
  *
  * This may return an invalid entry if no such use exists.
  */
FaceUse Face::negativeUse() const
{
  std::set<int> arr = this->m_manager->findCellHasUsesWithOrientation(
    this->m_entity, NEGATIVE);
  return arr.empty() ?
    FaceUse() :
    relationFromArrangement(HAS_USE, *arr.begin(), 0).as<FaceUse>();
}

/**\brief Return the face-use with its sense codirectional with the natural normal.
  *
  * This may return an invalid entry if no such use exists.
  */
FaceUse Face::positiveUse() const
{
  std::set<int> arr = this->m_manager->findCellHasUsesWithOrientation(
    this->m_entity, POSITIVE);
  return arr.empty() ?
    FaceUse() :
    relationFromArrangement(HAS_USE, *arr.begin(), 0).as<FaceUse>();
}

  } // namespace model
} // namespace smtk
