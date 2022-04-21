//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtAvailableOperations_h
#define smtk_extension_qtAvailableOperations_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/operation/Operation.h"

#include "smtk/view/AvailableOperations.h"

#include <QListWidget>
#include <QVBoxLayout>

namespace smtk
{
namespace extension
{

/**\brief Present a list of available operations to users.
  *
  * Depending on whether the operation's defaults allow it,
  * an operation may be run immediately (with default parameters)
  * or sent to an editor (where the user may change parameters
  * and run or cancel editing).
  */
class SMTKQTEXT_EXPORT qtAvailableOperations : public QWidget
{
  Q_OBJECT
public:
  qtAvailableOperations(QWidget* parent = nullptr);
  ~qtAvailableOperations() override;

  smtk::view::AvailableOperationsPtr operationSource() const { return m_operationSource; }
  void setOperationSource(smtk::view::AvailableOperationsPtr avail);
  void setUseLabels(bool val) { m_useLabels = val; }
  bool useLabels() const { return m_useLabels; }
  QListWidget* listWidget() const { return m_operationList; }

Q_SIGNALS:
  void tryOperation(const smtk::operation::Operation::Index& op);
  void editOperation(const smtk::operation::Operation::Index& op);
  void hoverOperation(const smtk::operation::Operation::Index& op);

protected:
  QListWidget* m_operationList{ nullptr };
  QVBoxLayout* m_layout{ nullptr };
  smtk::view::AvailableOperationsPtr m_operationSource;
  smtk::view::AvailableOperations::Observers::Key m_operationSourceObserverId;
  bool m_useLabels{ false };

  void updateList();
};
} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtAvailableOperations_h
