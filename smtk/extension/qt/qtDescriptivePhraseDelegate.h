//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_qtDescriptivePhraseDelegate_h
#define smtk_extension_qtDescriptivePhraseDelegate_h

#include "smtk/extension/qt/Exports.h"

#include <QStyledItemDelegate>

namespace smtk
{
namespace extension
{

class qtDescriptivePhraseModel;
/**\brief Present the contents of an smtk::model::Resource instance via qtDescriptivePhraseModel.
  *
  */
class SMTKQTEXT_EXPORT qtDescriptivePhraseDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  Q_PROPERTY(int titleFontSize READ titleFontSize WRITE setTitleFontSize)
  Q_PROPERTY(int subtitleFontSize READ subtitleFontSize WRITE setSubtitleFontSize)
  Q_PROPERTY(int titleFontWeight READ titleFontWeight WRITE setTitleFontWeight)
  Q_PROPERTY(int subtitleFontWeight READ subtitleFontWeight WRITE setSubtitleFontWeight)
  Q_PROPERTY(int textVerticalPad READ textVerticalPad WRITE setTextVerticalPad)
  Q_PROPERTY(bool drawSubtitle READ drawSubtitle WRITE setDrawSubtitle)
  Q_PROPERTY(bool visibilityMode READ visibilityMode WRITE setVisibilityMode)
public:
  qtDescriptivePhraseDelegate(QWidget* parent = nullptr);

  int titleFontSize() const;
  int subtitleFontSize() const;
  int titleFontWeight() const;
  int subtitleFontWeight() const;
  int textVerticalPad() const;
  bool drawSubtitle() const;
  bool highlightOnHover() const;
  bool visibilityMode() const;

public Q_SLOTS:
  void setTitleFontSize(int tfs);
  void setSubtitleFontSize(int sfs);
  void setTitleFontWeight(int tfs);
  void setSubtitleFontWeight(int sfs);
  void setTextVerticalPad(int tvp);
  void setDrawSubtitle(bool includeSubtitle);
  void setVisibilityMode(bool allEditsChangeVisibility);
  void setHighlightOnHover(bool highlightOnMouseover);

public:
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index)
    const override;

  QWidget* createEditor(
    QWidget* parent,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;
  void updateEditorGeometry(
    QWidget* editor,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index)
    const override;

  // return which icon the Point position is on
  // 'visible', 'color', or empty string;
  // std::string determineAction(
  //   const QPoint& pPos, const QModelIndex& idx, const QStyleOptionViewItem& option) const;

protected:
  bool eventFilter(QObject* editor, QEvent* event) override;
  bool editorEvent(
    QEvent* event,
    QAbstractItemModel* model,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) override;

  int m_titleFontSize{ 14 };
  int m_subtitleFontSize{ 10 };
  int m_titleFontWeight{ 2 };
  int m_subtitleFontWeight{ 1 };
  int m_textVerticalPad{ 2 };
  bool m_drawSubtitle{ true };
  bool m_visibilityMode{ false };
  bool m_highlightOnHover{ false };
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtDescriptivePhraseDelegate_h
