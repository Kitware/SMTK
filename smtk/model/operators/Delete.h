//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef smtk_model_Delete_h
#define smtk_model_Delete_h

#include "smtk/operation/XMLOperation.h"

#include "smtk/model/EntityRef.h"

namespace smtk
{
namespace model
{

/**\brief Delete an entity from a model resource
  *
  * This operation is not generally what you should use;
  * it only deletes SMTK entities and does nothing with
  * native modeling kernels. Instead, you should prefer
  * the delete operation provided with the model session
  * you are using.
  */
class SMTKCORE_EXPORT Delete : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::model::Delete);
  smtkCreateMacro(Delete);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;

  template<typename U, typename V, typename W, typename X>
  bool checkAndAddBoundingCells(
    const smtk::model::EntityRef& ent,
    bool deleteBoundingCells,
    U& verts,
    V& edges,
    W& faces,
    X& other);
  template<typename U, typename V, typename W, typename X>
  void addBoundaryCells(const smtk::model::EntityRef& ent, U& verts, V& edges, W& faces, X& other);

  int m_numInUse;
  int m_numWarnings;
  smtk::model::EntityRefs m_notRemoved;
  smtk::model::EntityRefs m_modified;
  smtk::model::EntityArray m_expunged;
};

} //namespace model
} // namespace smtk

#endif // smtk_model_Delete_h
