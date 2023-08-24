//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_task_adaptor_ConfigureOperation_h
#define smtk_task_adaptor_ConfigureOperation_h

#include "smtk/task/Adaptor.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/common/UUID.h"
#include "smtk/task/FillOutAttributes.h"
#include "smtk/task/Task.h"

#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace smtk
{
namespace task
{
namespace adaptor
{

/// Configure a task with a resource and role given a dependent producer.
class SMTKCORE_EXPORT ConfigureOperation : public Adaptor
{
public:
  smtkTypeMacro(smtk::task::adaptor::ConfigureOperation);
  smtkSuperclassMacro(smtk::task::Adaptor);
  smtkCreateMacro(smtk::task::Adaptor);

  struct ParameterSet
  {
    /// Role assigned to input attribute resource
    std::string m_fromRole;
    /// Map of <input item path, parameter item path>
    std::map<std::string, std::string> m_pathMap;
  };

  /// Constructors
  ConfigureOperation();
  ConfigureOperation(const Configuration& config);
  ConfigureOperation(const Configuration& config, Task* from, Task* to);

  bool reconfigureTask() override; // required override

protected:
  void configureSelf(const Configuration& config);

  /// Builds m_attributeSet and m_itemTable
  bool buildInternalData();
  bool updateInternalData(const smtk::task::FillOutAttributes::AttributeSet&, const ParameterSet&);

  /// Creates signal observer
  bool setupAttributeObserver();

  /// Copies items from FillOut task to SubmitOp task
  bool updateOperation() const;

  /// Stores configuration data
  std::vector<ParameterSet> m_parameterSets;

  /// Observers for task state changes and attribute changes
  smtk::task::Task::Observers::Key m_taskObserver;
  smtk::operation::Observers::Key m_attributeObserver;

  /// Stores attributes (uuids) that are to be checked for changes
  std::set<smtk::common::UUID> m_attributeSet;

  /// Table listing <source attribute, source item path, operation parameter (item) path>
  std::vector<std::tuple<smtk::attribute::WeakAttributePtr, std::string, std::string>> m_itemTable;
};

} // namespace adaptor
} // namespace task
} // namespace smtk

#endif // smtk_task_adaptor_ConfigureOperation_h
