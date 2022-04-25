//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pq_ColorChooserButton_h
#define pq_ColorChooserButton_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtItem.h"

#include <QColor>
#include <QToolButton>

namespace smtk
{
namespace attribute
{
class SMTKQTEXT_EXPORT qtColorButton : public QToolButton
{
  Q_OBJECT
  Q_PROPERTY(QColor chosenColor READ chosenColor WRITE setChosenColor)
public:
  /// constructor requires a QComboBox
  qtColorButton(QWidget* p);
  /// get the color
  QColor chosenColor() const;

Q_SIGNALS:
  /// signal color changed
  void chosenColorChanged(const QColor&);
  /// signal color selected
  void validColorChosen(const QColor&);

public Q_SLOTS:
  /// set the color
  virtual void setChosenColor(const QColor&);

  /// show a dialog to choose the color
  virtual void chooseColor();

protected:
  /// overridden to resize the color icon.
  void resizeEvent(QResizeEvent* rEvent) override;

  /// renders an icon for the color.
  QIcon renderColorSwatch(const QColor&);

  QColor Color;
};
}; // namespace attribute
}; // namespace smtk
#endif
