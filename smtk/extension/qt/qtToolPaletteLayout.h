//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qt_qtToolPaletteLayout_h
#define smtk_extension_qt_qtToolPaletteLayout_h

// Adapted from FlowLayout described here:
// http://doc.qt.io/qt-5/qtwidgets-layouts-flowlayout-example.html

#include "smtk/extension/qt/Exports.h"

#include <QLayout>
#include <QList>
#include <QRect>
#include <QStyle>

class SMTKQTEXT_EXPORT qtToolPaletteLayout : public QLayout
{
public:
  explicit qtToolPaletteLayout(
    QWidget* parent,
    int margin = -1,
    int hSpacing = -1,
    int vSpacing = -1);
  explicit qtToolPaletteLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
  ~qtToolPaletteLayout() override;

  void addItem(QLayoutItem* item) override;
  void insertItem(int insertAfter, QLayoutItem* item);
  int horizontalSpacing() const;
  int verticalSpacing() const;
  Qt::Orientations expandingDirections() const override;
  bool hasHeightForWidth() const override;
  int heightForWidth(int) const override;
  int count() const override;
  QLayoutItem* itemAt(int index) const override;
  QSize minimumSize() const override;
  void setGeometry(const QRect& rect) override;
  QSize sizeHint() const override;
  QLayoutItem* takeAt(int index) override;

  /// Return the integer index of the item placed at a given screen \a point (or -1).
  int indexAt(const QPoint& point) const;

private:
  int doLayout(const QRect& rect, bool testOnly) const;
  int smartSpacing(QStyle::PixelMetric pm) const;

  QList<QLayoutItem*> m_itemList;
  mutable QSize m_effectiveSpacing;
  mutable int m_effectiveColumns{ -1 };
  int m_hSpace;
  int m_vSpace;
};

#endif // smtk_extension_qt_qtToolPaletteLayout_h
