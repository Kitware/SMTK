//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_session_discrete_MergeOperation_h
#define __smtk_session_discrete_MergeOperation_h

#include "smtk/bridge/discrete/Operation.h"
#include "smtk/bridge/discrete/Resource.h"
#include "vtkMergeOperation.h"
#include "vtkNew.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

/**\brief Merge adjacent cells into a single cell.
  *
  * The source and target cells are merged, with the result being stored in target.
  * All boundaries of the source are either modified (so that they now refer to
  * the target) or are deleted (because they served as a boundary between the source
  * and target).
  *
  * The source and target must be adjacent and have the same parametric dimension.
  */
class SMTKDISCRETESESSION_EXPORT MergeOperation : public Operation
{
public:
  smtkTypeMacro(MergeOperation);
  smtkCreateMacro(MergeOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(Operation);

  bool ableToOperate() override;

protected:
  MergeOperation();
  Result operateInternal() override;
  const char* xmlDescription() const override;
  int fetchCMBCellId(
    smtk::bridge::discrete::Resource::Ptr& resource, const std::string& parameterName) const;
  int fetchCMBCellId(smtk::bridge::discrete::Resource::Ptr& resource,
    const smtk::attribute::ModelEntityItemPtr& entItem, int idx) const;

  vtkNew<vtkMergeOperation> m_op;
};

} // namespace discrete
} // namespace bridge

} // namespace smtk

#endif // __smtk_session_discrete_MergeOperation_h
