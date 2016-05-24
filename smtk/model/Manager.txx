//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_Manager_txx
#define __smtk_model_Manager_txx

namespace smtk {
  namespace model {

/**\brief Create face, face-use, loop, and potentially edge-use records given edges.
  *
  * Only one face use (the positive use) will be populated for the face.
  * The edges must be ordered from head to tail in a counter-clockwise loop (relative to
  * the orientation of the face use)  and the \a orderedEdgesWithOrientation
  * object passed to this method must be an STL-like container of
  * pair<smtk::model::Edge, bool> entries.
  * The boolean value stored with each edge indicates the orientation of
  * each model edge relative to the loop.
  */
template<typename T>
bool Manager::insertModelFaceWithOrientedOuterLoop(
  const smtk::common::UUID& faceId, // to be created
  const smtk::common::UUID& faceUseId, // to be created
  const smtk::common::UUID& loopId, // to be created
  const T& orderedEdgesWithOrientation)
{
  // First, create the ID'd top-level entity records:
  Face f = this->insertFace(faceId);
  FaceUse fu = this->insertFaceUse(faceUseId);
  Loop l = this->setLoop(loopId, fu);

  // Now loop over the container's entries (which must behave like std::pair<smtk::model::Edge, bool>)
  // adding edge uses for each edge to the loop.
  for (typename T::const_iterator oeit = orderedEdgesWithOrientation.begin(); oeit != orderedEdgesWithOrientation.end(); ++oeit)
    {
    smtk::model::Edge mutableEdge(oeit->first);
    EdgeUse eu = mutableEdge.findOrAddEdgeUse(oeit->second ? POSITIVE : NEGATIVE, 0);
    l.addUse(eu);
    }
  f.setFaceUse(POSITIVE, fu);
  return true;
}

/**\brief Create an inner loop (hole) in a face-use and add ordered edge uses to it.
 *
  * The edges must be ordered from head to tail in a counter-clockwise loop (relative to
  * the orientation of the face use)  and the \a orderedEdgesWithOrientation
  * object passed to this method must be an STL-like container of
  * pair<smtk::model::Edge, bool> entries.
  * The boolean value stored with each edge indicates the orientation of
  * each model edge relative to the loop.
  */
template<typename T>
bool Manager::insertModelFaceOrientedInnerLoop(
  const smtk::common::UUID& loopId,
  const smtk::common::UUID& preExistingLoopId,
  const T& orderedEdgesWithOrientation)
{
  Loop outer(shared_from_this(),preExistingLoopId);
  if (!outer.isValid())
    {
    smtkErrorMacro(this->log(),
      "Asked to add an inner loop to invalid outer loop " << outer.name());
    return false;
    }
  Loop inner = this->setLoop(loopId, outer);

  // Now loop over the container's entries (which must behave like std::pair<smtk::model::Edge, bool>)
  // adding edge uses for each edge to the loop.
  for (typename T::const_iterator oeit = orderedEdgesWithOrientation.begin(); oeit != orderedEdgesWithOrientation.end(); ++oeit)
    {
    smtk::model::Edge mutableEdge(oeit->first);
    EdgeUse eu = mutableEdge.findOrAddEdgeUse(oeit->second ? POSITIVE : NEGATIVE, 0);
    inner.addUse(eu);
    }
  return true;
}

  } // model namespace
} // smtk namespace

#endif // __smtk_model_Manager_txx
