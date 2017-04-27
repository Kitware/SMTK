//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extension_QEntityItemDelegate_h
#define __smtk_extension_QEntityItemDelegate_h

#include "smtk/extension/qt/Exports.h"

#include <QStyledItemDelegate>

namespace smtk
{
namespace extension
{

class QEntityItemModel;
/**\brief Present the contents of an smtk::model::Manager instance via QEntityItemModel.
  *
  */
class SMTKQTEXT_EXPORT QEntityItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  Q_PROPERTY(int swatchSize READ swatchSize WRITE setSwatchSize)
  Q_PROPERTY(int titleFontSize READ titleFontSize WRITE setTitleFontSize)
  Q_PROPERTY(int subtitleFontSize READ subtitleFontSize WRITE setSubtitleFontSize)
  Q_PROPERTY(int titleFontWeight READ titleFontWeight WRITE setTitleFontWeight)
  Q_PROPERTY(int subtitleFontWeight READ subtitleFontWeight WRITE setSubtitleFontWeight)
  Q_PROPERTY(int textVerticalPad READ textVerticalPad WRITE setTextVerticalPad)
  Q_PROPERTY(bool drawSubtitle READ drawSubtitle WRITE setDrawSubtitle)
public:
  QEntityItemDelegate(QWidget* parent = 0);

  int titleFontSize() const;
  int subtitleFontSize() const;
  int titleFontWeight() const;
  int subtitleFontWeight() const;
  int swatchSize() const;
  int textVerticalPad() const;
  bool drawSubtitle() const;

public slots:
  void setTitleFontSize(int tfs);
  void setSubtitleFontSize(int sfs);
  void setTitleFontWeight(int tfs);
  void setSubtitleFontWeight(int sfs);
  void setSwatchSize(int sws);
  void setTextVerticalPad(int tvp);
  void setDrawSubtitle(bool includeSubtitle);

signals:
  void requestVisibilityChange(const QModelIndex&);
  void requestColorChange(const QModelIndex&);

public:
  virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

  virtual void paint(
    QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  virtual QWidget* createEditor(
    QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
  virtual void setModelData(
    QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

  // return which icon the Point position is on
  // 'visible', 'color', or empty string;
  std::string determineAction(const QPoint& pPos, const QModelIndex& idx,
    const QStyleOptionViewItem& option,
    const smtk::extension::QEntityItemModel* entityMod = nullptr) const;

protected slots:
  virtual void commitAndCloseEditor();

protected:
  virtual bool eventFilter(QObject* editor, QEvent* event);
  virtual bool editorEvent(QEvent* event, QAbstractItemModel* model,
    const QStyleOptionViewItem& option, const QModelIndex& index);

  int m_swatchSize;
  int m_titleFontSize;
  int m_subtitleFontSize;
  int m_titleFontWeight;
  int m_subtitleFontWeight;
  int m_textVerticalPad;
  bool m_drawSubtitle;
};

} // namespace extension
} // namespace smtk

#endif // __smtk_extension_QEntityItemDelegate_h
