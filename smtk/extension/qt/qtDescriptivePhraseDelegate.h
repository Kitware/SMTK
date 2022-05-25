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
#include "smtk/view/BadgeSet.h"
#include "smtk/view/Selection.h"

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
  Q_PROPERTY(std::shared_ptr<smtk::view::Selection> selection READ selection WRITE setSelection)
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
  std::shared_ptr<smtk::view::Selection> selection() const;

public Q_SLOTS:
  void setTitleFontSize(int tfs);
  void setSubtitleFontSize(int sfs);
  void setTitleFontWeight(int tfs);
  void setSubtitleFontWeight(int sfs);
  void setTextVerticalPad(int tvp);
  void setDrawSubtitle(bool includeSubtitle);
  void setVisibilityMode(bool allEditsChangeVisibility);
  void setHighlightOnHover(bool highlightOnMouseover);
  void setSelection(const std::shared_ptr<smtk::view::Selection>& seln);

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

  /**\brief If the given \a mouseClickEvent occurred over a badge,
    *       invoke its action and return true.
    *
    * The \a mouseClickEvent must be a QEvent::MouseButtonPress sent to the
    * viewport of the \a view (a QAbstractItemView such as a QTreeView).
    * Thus, this method will be called from an event filter installed on
    * \a view->viewport().
    * If this method returns true, your event filter should consume the
    * event by returning true.
    *
    * Internally, this method will call pickBadge().
    */
  static bool processBadgeClick(QMouseEvent* mouseClickEvent, QAbstractItemView* view);

  /// Return the index of the badge underneath \a pPos (or -1 if none).
  static int pickBadge(
    const QPoint& pPos,
    const QStyleOptionViewItem& option,
    const smtk::view::BadgeSet::BadgeList& badges,
    const smtk::view::DescriptivePhrase* phrase);

protected:
  bool eventFilter(QObject* editor, QEvent* event) override;

  int m_titleFontSize{ 14 };
  int m_subtitleFontSize{ 10 };
  int m_titleFontWeight{ 2 };
  int m_subtitleFontWeight{ 1 };
  int m_textVerticalPad{ 2 };
  bool m_drawSubtitle{ true };
  bool m_visibilityMode{ false };
  bool m_highlightOnHover{ false };
  std::weak_ptr<smtk::view::Selection> m_selection;
};

} // namespace extension
} // namespace smtk

#endif // smtk_extension_qtDescriptivePhraseDelegate_h
