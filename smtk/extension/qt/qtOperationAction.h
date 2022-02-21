//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtOperationAction_h
#define smtk_extension_qt_qtOperationAction_h

#include "smtk/extension/qt/Exports.h"

#include "smtk/attribute/utility/Queries.h"
#include "smtk/operation/Operation.h"

#include <QPointer>
#include <QTimer>
#include <QWidgetAction>

#include <string>

// VTK's wrapper parser does not properly handle Qt macros on macos.
#if defined(__VTK_WRAP__) && !defined(Q_SLOTS)
#define Q_DISABLE_COPY(x)
#define Q_SLOTS
#define Q_SIGNALS public
#define Q_OBJECT
#endif

class qtOperationTypeModel;

/**\brief A QAction for users to request an operation.
  *
  * This class adapts an operation manager and operation type-index
  * into a QWidgetAction for menus, toolbars, and tool-buttons.
  * Depending on the operation's metadata, users may run the operation
  * with its default parameters immediately and/or edit parameters
  * before running the operation.
  */
class SMTKQTEXT_EXPORT qtOperationAction : public QWidgetAction
{
  Q_OBJECT
public:
  qtOperationAction(
    const std::shared_ptr<smtk::operation::Manager>& operationManager,
    const std::shared_ptr<smtk::view::Manager>& viewManager,
    smtk::operation::Operation::Index typeIndex,
    qtOperationTypeModel* parent = nullptr);
  ~qtOperationAction() override;

  /// The label of the operation (or the operation's type-name if no label is provided).
  std::string name() const { return m_operationName; }
  /// The type-index of the operation.
  smtk::operation::Operation::Index operationIndex() const { return m_typeIndex; }

  static constexpr int longPress = 200; // milliseconds

protected Q_SLOTS:
  void parameterTimerElapsed();

Q_SIGNALS:
  void acceptDefaults();
  void editParameters();

protected:
  QWidget* createWidget(QWidget* parent) override;
  void deleteWidget(QWidget* widget) override;

  bool eventFilter(QObject* watched, QEvent* event) override;

  /**\brief Take an operation label and turn it into something
    *  something appropriate for a button.
    *
    * This method removes anything to the left of double-colons ("::")
    * or space-separated dashes (" - "), then finds the space
    * character closest to the center of the string and turns it into
    * a line break.
    */
  void setOperationName(const std::string& label);

  /// The operation-type model for which this action was created.
  QPointer<qtOperationTypeModel> m_model;
  /// The operation type-index (essentially the row in m_model).
  smtk::operation::Operation::Index m_typeIndex;
  /// A label for the operation.
  std::string m_operationName;
  /// Whether the operation has editable parameters.
  smtk::attribute::utility::EditableParameters m_editableParameters;
  /// A timer used to detect long-presses/double-clicks of buttons.
  QTimer m_timer;
};

#endif // smtk_extension_qt_qtOperationAction_h
