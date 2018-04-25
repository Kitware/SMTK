//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtColorButton - A button used to select colors.
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_bridge_rgg_qt_qtColorButton_h
#define __smtk_bridge_rgg_qt_qtColorButton_h

#include "smtk/bridge/rgg/qt/Exports.h"

#include <QPushButton>

class QColor;
class QPushButton;

class SMTKQTRGGSESSION_EXPORT qtColorButton : public QPushButton
{
  Q_OBJECT
  typedef QWidget Superclass;

public:
  qtColorButton(QWidget* parent, QColor = Qt::white);
  ~qtColorButton();
  static QColor generateColor();

signals:
  void colorModified();

public slots:
  QColor getColor();
  void getColor(std::vector<double>& color);

  void chooseColor();
  void setColor(QColor color);

protected:
  /// renders an icon for the color.
  QIcon renderColorSwatch(const QColor&);
  QColor m_color;
  static int s_colorIndex; // Used to generate a random color
};

#endif
