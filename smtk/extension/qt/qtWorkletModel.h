//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtWorkletModel_h
#define smtk_extension_qt_qtWorkletModel_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtOperationLauncher.h"

#include <QAbstractListModel>
#include <QPointer>

#include "smtk/common/Managers.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/task/Worklet.h"
#include "smtk/view/Information.h"
#include "smtk/view/SelectionObserver.h"

/**\brief A QAbstractListModel populated with SMTK worklets.
  *
  * The columns of the model are fixed by the Column enumeration.
  * Worklets are ordered by their name.
  * By using a QSortFilterProxyModel, you can subset and sort worklets
  * using a variety of techniques.
  *
  * The class constructor accepts configuration via an
  * smtk::view::Information instance; it expects to find a
  * configuration component named "Model" in `info.configuration()->details()`.
  */
class SMTKQTEXT_EXPORT qtWorkletModel : public QAbstractListModel
{
  Q_OBJECT
public:
  qtWorkletModel(const smtk::view::Information& info, QObject* parent = nullptr);
  ~qtWorkletModel() override = default;

  /// The columns of the model.
  enum class Column
  {
    Label,       //!< The operation's label (either from its XML or an override).
    WorkletOp,   //!< The type index that uniquely identifies the operation.
    Description, //!< The column to display a worklet's description.
                 // ----
    Count        //!< Not a valid value; when cast to an integer it is the number of columns.
  };

  /// Convert to/from a Column enumerant.
  static Column columnEnumFromName(const std::string& colName);
  static std::string columnNameFromEnum(Column enumerant);

  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QStringList mimeTypes() const override;
  QMimeData* mimeData(const QModelIndexList& indexes) const override;

  /// Return the index of the row with the given operation type-index.
  QModelIndex findByName(smtk::string::Token workletName) const;

  /// Set the category expression to be used to determine worklets that can be placed
  /// at the top-level of a workflow
  void setToplevelCatagoryExpression(const smtk::common::Categories::Expression& exp);

  /// Set the task to be the potential parent of em-placed worklets.
  ///
  /// Note that \a task can be null indicating that the worklet would be em-placed at
  /// the workflow's top-level
  void setParentTask(const smtk::task::TaskPtr& task);

protected:
  /// Invoked when a worklet is added or removed.
  void workletUpdate(
    const smtk::operation::Operation& operation,
    smtk::operation::Operation::Result result);

  // Invoked when either the top-level expression or the parent task
  // is changed.
  void rebulidModel();

  // List of available worklets
  std::vector<std::shared_ptr<smtk::task::Worklet>> m_worklets;
  // List of worklets that can be em-placed based on the
  // current constraints imposed either by the top-level expression
  // or by the current parent task
  std::vector<std::shared_ptr<smtk::task::Worklet>> m_appropriateWorklets;
  std::shared_ptr<smtk::operation::Manager> m_operationManager;
  std::shared_ptr<smtk::view::Manager> m_viewManager;

  smtk::operation::Observers::Key m_operationObserverKey;
  std::string m_secondaryColor{ "#ffffff" };

  smtk::extension::qtOperationLauncher* m_launcher{ nullptr };

  smtk::task::TaskPtr m_parentTask;
  smtk::common::Categories::Expression m_topLevelExpression;

private:
  Q_DISABLE_COPY(qtWorkletModel);
};

#endif // smtk_extension_qt_qtWorkletModel_h
