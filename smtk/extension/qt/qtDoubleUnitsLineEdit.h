//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtDoubleUnitsLineEdit_h
#define smtk_extension_qt_qtDoubleUnitsLineEdit_h

#include <QLineEdit>

#include "smtk/extension/qt/Exports.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/qtInputsItem.h"

#include <QPointer>
#include <QScopedPointer> // for ivar
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
class qtInputsItem;

/**\brief qtDoubleUnitsLineEdit provides units-aware line edit double values */
class SMTKQTEXT_EXPORT qtDoubleUnitsLineEdit : public QLineEdit
{
  Q_OBJECT
  Q_ENUMS(RealNumberNotation)
  Q_PROPERTY(RealNumberNotation notation READ notation WRITE setNotation)
  Q_PROPERTY(int precision READ precision WRITE setPrecision)

public:
  using Superclass = QLineEdit;

  /** \brief Creates instance if double item has units; Returns editor as QWidget */
  static qtDoubleUnitsLineEdit* checkAndCreate(qtInputsItem* item, const QString& tooltip);

  qtDoubleUnitsLineEdit(qtInputsItem* item, const units::Unit& unit, const QString& tooltip);
  ~qtDoubleUnitsLineEdit() override;

  /**
   * This enum specifies which notations to use for displaying the value.
   */
  enum RealNumberNotation
  {
    MixedNotation = 0,
    ScientificNotation,
    FixedNotation
  };

  /** \brief Return the text that represents the widget's base tool-tip.
   * The text does not contain the item's converted value
   */
  const QString& baseToolTipText() const { return m_baseTooltip; }

  /**
   * Return the notation used to display the number.
   * \sa setNotation()
   */
  RealNumberNotation notation() const;

  /**
   * Return the precision used to display the number.
   * \sa setPrecision()
   */
  int precision() const;

  /**
   * Returns the text being shown when the widget is not active or under mouse
   * pointer. Primarily intended for test or debugging purposes.
   */
  QString simplifiedText() const;

  //@{
  /**
   * Return a double formatted according to a QTextStream::RealNumberNotation and number
   * of digits of precision.
   */
  static QString
  formatDouble(double value, QTextStream::RealNumberNotation notation, int precision);
  static QString
  formatDouble(double value, qtDoubleUnitsLineEdit::RealNumberNotation notation, int precision);
  //@}

Q_SIGNALS:
  /**
   * Indicates that the user has finished editing and the appropriate units have been added to
   * the value
   */
  void editingCompleted(QObject*);

public Q_SLOTS:
  void onEditFinished();

  /**
   * Set the notation used to display the number.
   * \sa notation()
   */
  void setNotation(RealNumberNotation _notation);

  /**
   * Set the precision used to display the number.
   * \sa precision()
   */
  void setPrecision(int precision);

protected Q_SLOTS:
  void onTextEdited();

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void paintEvent(QPaintEvent* evt) override;
  void resizeEvent(QResizeEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;

  QPointer<qtInputsItem> m_inputsItem;
  units::Unit m_unit;
  QStringList m_unitChoices;
  QString m_baseTooltip;
  int m_lastKey = -1;

  QCompleter* m_completer = nullptr;
  class qtInternals;
  QScopedPointer<qtInternals> m_internals;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qt_qtDoubleUnitsLineEdit_h
