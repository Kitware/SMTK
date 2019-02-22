//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_workflow_OperationFilterSort_h
#define smtk_workflow_OperationFilterSort_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/operation/Metadata.h"

namespace smtk
{
namespace workflow
{

/**\brief Choose how to present operations to users.
  *
  * In a simulation processing workflow, it is often desirable
  * to focus users on a particular task by reordering or eliminating
  * operations that are not useful from the user interface.
  * It is also important to adapt operation terminology to
  * the domain expert rather than train the domain expert.
  *
  * This class allows changes to how and when operations are
  * made visible to users in an application.
  * It is used by the smtk::view::AvailableOperators class
  * (1) to choose which of the applicable operations are
  * presented to a user, and (2) to add presentation information
  * such as icons and labels to these operations.
  *
  * This class simply holds a filter-list of operation indices
  * it wishes to allow, but the API it presents allows other
  * subclasses to tailor operations to a multistage workflow
  * where operations appear/disappear as other conditions are met.
  * Specifically, if an external condition is met, this class
  * may signal a reconfiguration event to its observers (of
  * which the AvailableOperations class is one).
  */
class SMTKCORE_EXPORT OperationFilterSort : smtkEnableSharedPtr(OperationFilterSort)
{
public:
  using Index = smtk::operation::Operation::Index;
  using Data = struct
  {
    std::string name;
    std::string description;
    std::string iconName;
    int precedence;
  };
  using WorkingSet = std::set<Index>;
  using Output = std::vector<Index>;
  using FilterList = std::map<Index, Data>;
  using Observer = std::function<void()>;
  using Observers = smtk::common::Observers<Observer>;
  smtkTypeMacroBase(smtk::workflow::OperationFilterSort);
  smtkCreateMacro(OperationFilterSort);
  virtual ~OperationFilterSort();

  FilterList& filterList() { return m_filterList; }
  const FilterList& filterList() const { return m_filterList; }

  /// Choose a subset of operations to present to a used from \a workingSet.
  void apply(const WorkingSet& workingSet, Output& operationsToDisplay);

  /// Return the observers associated filter sort.
  Observers& observers() { return m_observers; }
  const Observers& observers() const { return m_observers; }

  /// Call all observers to indicate the filter list has possibly changed.
  void triggerObservers() const;

protected:
  OperationFilterSort();

  FilterList m_filterList;
  Observers m_observers;
};
}
}

#endif
