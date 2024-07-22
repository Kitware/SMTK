//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtUnitsLineEdit_h
#define smtk_extension_qt_qtUnitsLineEdit_h

#include <QLineEdit>

#include "smtk/extension/qt/Exports.h"

#include "smtk/PublicPointerDefs.h"

#include <QPointer>
#include <QString>
#include <QStringList>
#include <QTextStream> // for formatDouble

#include "units/System.h"
#include "units/Unit.h"

#include <vector>

class QCompleter;
class QKeyEvent;

namespace smtk
{
namespace extension
{

class qtUIManager;

/**\brief qtUnitsLineEdit provides units-aware line edit */
class SMTKQTEXT_EXPORT qtUnitsLineEdit : public QLineEdit
{
  Q_OBJECT

public:
  using Superclass = QLineEdit;

  qtUnitsLineEdit(
    const QString& baseUnit,
    const std::shared_ptr<units::System>& unitSys,
    qtUIManager* uiManager,
    QWidget* parent = nullptr);

  bool isCurrentTextValid() const;
  void setAndClassifyText(const QString&);
Q_SIGNALS:
  /**
   * Indicates that the user has finished editing and the appropriate units have been added to
   * the value
   */
  void editingCompleted(QString);

public Q_SLOTS:
  void onEditFinished();

protected Q_SLOTS:
  void onTextEdited();

protected:
  void keyPressEvent(QKeyEvent* event) override;

  int m_lastKey = -1;
  QString m_baseUnit;
  const std::shared_ptr<units::System>& m_unitSys;
  qtUIManager* m_uiManager;

  QCompleter* m_completer = nullptr;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qt_qtUnitsLineEdit_h
