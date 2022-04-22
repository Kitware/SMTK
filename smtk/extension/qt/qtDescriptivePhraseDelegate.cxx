//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"

#include "smtk/extension/qt/SVGIconEngine.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"

#include <QAbstractProxyModel>
#include <QApplication>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>

namespace smtk
{
namespace extension
{
const int padding = 7;

qtDescriptivePhraseDelegate::qtDescriptivePhraseDelegate(QWidget* owner)
  : QStyledItemDelegate(owner)
{
}

int qtDescriptivePhraseDelegate::titleFontSize() const
{
  return m_titleFontSize;
}

void qtDescriptivePhraseDelegate::setTitleFontSize(int tfs)
{
  m_titleFontSize = tfs;
}

int qtDescriptivePhraseDelegate::subtitleFontSize() const
{
  return m_subtitleFontSize;
}

void qtDescriptivePhraseDelegate::setSubtitleFontSize(int sfs)
{
  m_subtitleFontSize = sfs;
}

int qtDescriptivePhraseDelegate::titleFontWeight() const
{
  return m_titleFontWeight;
}

void qtDescriptivePhraseDelegate::setTitleFontWeight(int tfw)
{
  m_titleFontWeight = tfw;
}

int qtDescriptivePhraseDelegate::subtitleFontWeight() const
{
  return m_subtitleFontWeight;
}

void qtDescriptivePhraseDelegate::setSubtitleFontWeight(int sfw)
{
  m_subtitleFontWeight = sfw;
}

int qtDescriptivePhraseDelegate::textVerticalPad() const
{
  return m_textVerticalPad;
}

void qtDescriptivePhraseDelegate::setTextVerticalPad(int tvp)
{
  m_textVerticalPad = tvp;
}

bool qtDescriptivePhraseDelegate::drawSubtitle() const
{
  return m_drawSubtitle;
}

void qtDescriptivePhraseDelegate::setDrawSubtitle(bool includeSubtitle)
{
  m_drawSubtitle = includeSubtitle;
}

bool qtDescriptivePhraseDelegate::visibilityMode() const
{
  return m_visibilityMode;
}

void qtDescriptivePhraseDelegate::setVisibilityMode(bool allEditsChangeVisibility)
{
  m_visibilityMode = allEditsChangeVisibility;
}

bool qtDescriptivePhraseDelegate::highlightOnHover() const
{
  return m_highlightOnHover;
}

void qtDescriptivePhraseDelegate::setHighlightOnHover(bool highlightOnHover)
{
  m_highlightOnHover = highlightOnHover;
}

QSize qtDescriptivePhraseDelegate::sizeHint(
  const QStyleOptionViewItem& option,
  const QModelIndex& idx) const
{
  QSize iconsize;
  auto badges =
    qvariant_cast<smtk::view::BadgeSet::BadgeList>(idx.data(qtDescriptivePhraseModel::BadgesRole));
  auto phrase =
    idx.data(qtDescriptivePhraseModel::PhrasePtrRole).value<smtk::view::DescriptivePhrasePtr>();
  std::array<float, 4> backgroundArray = { 1, 1, 1, 1 };
  // find out the size of a badge
  for (auto* badge : badges)
  {
    QIcon badgeIcon(new SVGIconEngine(badge->icon(phrase.get(), backgroundArray)));
    iconsize = badgeIcon.actualSize(option.decorationSize);
    break;
  }
  QFont titleFont = QApplication::font();
  titleFont.setPixelSize(m_titleFontSize);
  titleFont.setBold(this->titleFontWeight() > 1);
  QFontMetrics titleFM(titleFont);
  QFont subtitleFont = QApplication::font();
  subtitleFont.setPixelSize(m_subtitleFontSize);
  subtitleFont.setBold(this->subtitleFontWeight() > 1);
  QFontMetrics subtitleFM(subtitleFont);
  int minHeight = titleFM.height() + 2 * m_textVerticalPad;
  if (m_drawSubtitle)
  {
    minHeight += subtitleFM.height();
  }
  if (minHeight < iconsize.height())
  {
    minHeight = iconsize.height();
  }

  return (QSize(iconsize.width(), minHeight));
}

void qtDescriptivePhraseDelegate::paint(
  QPainter* painter,
  const QStyleOptionViewItem& option,
  const QModelIndex& idx) const
{
  bool setBackground = false;
  QColor background;
  // If selected, draw the highlight color over entire rectangle
  if (option.state.testFlag(QStyle::State_MouseOver) && m_highlightOnHover)
  {
    background = option.palette.highlight().color().lighter(125);
    setBackground = true;
  }
  else if (option.state.testFlag(QStyle::State_Selected))
  {
    background = option.palette.highlight().color();
    setBackground = true;
  }
  else
  {
    background = option.palette.color(QPalette::Background);
  }
  if (setBackground)
  {
    painter->save();
    painter->setBrush(QBrush(background));
    painter->setPen(Qt::NoPen);
    painter->drawRect(option.rect);
    painter->restore();
  }

  painter->save();

  // Fetch the badges for this QModelIndex:
  auto badges =
    qvariant_cast<smtk::view::BadgeSet::BadgeList>(idx.data(qtDescriptivePhraseModel::BadgesRole));
  auto phrase =
    idx.data(qtDescriptivePhraseModel::PhrasePtrRole).value<smtk::view::DescriptivePhrasePtr>();
  std::array<float, 4> backgroundArray = { static_cast<float>(background.redF()),
                                           static_cast<float>(background.greenF()),
                                           static_cast<float>(background.blueF()),
                                           static_cast<float>(background.alphaF()) };

  QIcon icon;
  if (idx.data(qtDescriptivePhraseModel::PhraseLockRole).toInt() == 1)
  {
    icon = QIcon(":/icons/display/locked.png");
  }
  // QSize iconsize = icon.actualSize(option.decorationSize);

  QFont titleFont = QApplication::font();
  QFont subtitleFont = QApplication::font();
  titleFont.setPixelSize(m_titleFontSize);
  /// add a method to set/get title font
  titleFont.setBold(this->titleFontWeight() > 1);
  // bold the active model title
  if (idx.data(qtDescriptivePhraseModel::ModelActiveRole).toBool())
  {
    titleFont.setBold(true);
  }
  subtitleFont.setPixelSize(m_subtitleFontSize);
  subtitleFont.setBold(this->subtitleFontWeight() > 1);

  // subtitleFont.setWeight(subtitleFont.weight() - 2);
  QFontMetrics titleFM(titleFont);

  QString titleText = qvariant_cast<QString>(idx.data(qtDescriptivePhraseModel::TitleTextRole));
  if (idx.data(qtDescriptivePhraseModel::PhraseCleanRole).toInt() == 0)
  {
    titleText = titleText + " *";
  }
  QString subtitleText =
    qvariant_cast<QString>(idx.data(qtDescriptivePhraseModel::SubtitleTextRole));

  QRect titleRect = option.rect;
  QRect subtitleRect = option.rect;
  QRect iconRect = option.rect;
  iconRect.setTop(iconRect.top() + 1);
  // make zero size if there are no badges.
  iconRect.setRight(iconRect.left() + padding);

  for (auto* badge : badges)
  {
    QIcon badgeIcon(new SVGIconEngine(badge->icon(phrase.get(), backgroundArray)));
    QSize iconsize = badgeIcon.actualSize(option.decorationSize);
    // shift each icon to the right, adding some padding.
    iconRect.setRight(iconRect.left() + iconsize.width() + padding);
    // NOTE -  the following QPainter::save() and QPainter::restore() calls are a workaround to fix
    // a crash with Qt - Need to check to see if future versions (> 5.12.6) still needs this.
    // painter->save();
    painter->drawPixmap(
      QPoint(iconRect.left(), iconRect.top() + (option.rect.height() - iconsize.height()) / 2.),
      badgeIcon.pixmap(iconsize.width(), iconsize.height()));
    // painter->restore();
    iconRect.setLeft(iconRect.right());
  }

  titleRect.setLeft(iconRect.right());
  subtitleRect.setLeft(iconRect.right());
  titleRect.setTop(titleRect.top() + m_textVerticalPad / 2.0);
  titleRect.setBottom(
    titleRect.top() +
    (m_drawSubtitle ? titleFM.height() : option.rect.height() - m_textVerticalPad));
  subtitleRect.setTop(titleRect.bottom() + m_textVerticalPad);

  if (setBackground && background.lightness() < 128)
  {
    painter->setPen(Qt::white);
  }
  painter->setFont(titleFont);
  painter->drawText(titleRect, Qt::AlignVCenter, titleText);

  if (m_drawSubtitle)
  {
    painter->setFont(subtitleFont);
    painter->drawText(subtitleRect, subtitleText);
  }

  painter->restore();
}

QWidget* qtDescriptivePhraseDelegate::createEditor(
  QWidget* owner,
  const QStyleOptionViewItem& option,
  const QModelIndex& idx) const
{
  (void)option;

  // Visibility mode does not allow editing the title.
  if (m_visibilityMode)
  {
    return nullptr;
  }

  // Otherwise, edit the title if the item says we can.
  if (idx.data(qtDescriptivePhraseModel::TitleTextMutableRole).toBool())
  {
    QLineEdit* editor = new QLineEdit(owner);
    return editor;
  }
  return nullptr;
}

void qtDescriptivePhraseDelegate::updateEditorGeometry(
  QWidget* editor,
  const QStyleOptionViewItem& option,
  const QModelIndex& idx) const
{
  QFont titleFont = QApplication::font();
  QFont subtitleFont = QApplication::font();
  titleFont.setPixelSize(m_titleFontSize);
  // add a method to set/get title font
  titleFont.setBold(this->titleFontWeight() > 1);
  // bold the active model title
  if (idx.data(qtDescriptivePhraseModel::ModelActiveRole).toBool())
  {
    titleFont.setBold(true);
  }
  subtitleFont.setPixelSize(m_subtitleFontSize);
  subtitleFont.setBold(this->subtitleFontWeight() > 1);

  // subtitleFont.setWeight(subtitleFont.weight() - 2);
  QFontMetrics titleFM(titleFont);

  QString titleText = qvariant_cast<QString>(idx.data(qtDescriptivePhraseModel::TitleTextRole));
  QString subtitleText =
    qvariant_cast<QString>(idx.data(qtDescriptivePhraseModel::SubtitleTextRole));
  if (idx.data(qtDescriptivePhraseModel::PhraseCleanRole).toInt() == 0)
  {
    titleText += " *";
  }

  QRect titleRect = option.rect;
  QRect iconRect = option.rect;
  auto badges =
    qvariant_cast<smtk::view::BadgeSet::BadgeList>(idx.data(qtDescriptivePhraseModel::BadgesRole));
  auto phrase =
    idx.data(qtDescriptivePhraseModel::PhrasePtrRole).value<smtk::view::DescriptivePhrasePtr>();
  std::array<float, 4> backgroundArray = { 1, 1, 1, 1 };
  // find out the size of each badge, so we can shift the editor over correctly.
  for (auto* badge : badges)
  {
    QIcon badgeIcon(new SVGIconEngine(badge->icon(phrase.get(), backgroundArray)));
    QSize iconsize = badgeIcon.actualSize(option.decorationSize);
    // shift each icon to the right, adding some padding.
    iconRect.setRight(iconRect.left() + iconsize.width() + padding);
    // painting would be here...
    iconRect.setLeft(iconRect.right());
  }
  titleRect.setLeft(iconRect.right());

  editor->setGeometry(titleRect);
}

void qtDescriptivePhraseDelegate::setEditorData(QWidget* editor, const QModelIndex& idx) const
{
  QLineEdit* titleEditor = qobject_cast<QLineEdit*>(editor);
  if (titleEditor)
  {
    titleEditor->setText(idx.data(qtDescriptivePhraseModel::EditableTitleTextRole).toString());
  }
}

void qtDescriptivePhraseDelegate::setModelData(
  QWidget* editor,
  QAbstractItemModel* model,
  const QModelIndex& idx) const
{
  QLineEdit* titleEditor = qobject_cast<QLineEdit*>(editor);
  if (titleEditor)
  {
    model->setData(idx, titleEditor->text(), qtDescriptivePhraseModel::TitleTextRole);
  }
}

bool qtDescriptivePhraseDelegate::eventFilter(QObject* editor, QEvent* evt)
{
  if (evt->type() == QEvent::MouseButtonPress)
    return false;
  return QStyledItemDelegate::eventFilter(editor, evt);
}

int determineAction(
  const QPoint& pPos,
  const QStyleOptionViewItem& option,
  const smtk::view::BadgeSet::BadgeList& badges,
  const smtk::view::DescriptivePhrase* phrase)
{

  int badgeIndex = -1;
  // if (m_visibilityMode)
  // {
  //   return -1;
  // }
  // with the help of styles, return which badge the pPos is on.
  // Mirrors paint() code.
  QRect iconRect = option.rect;
  iconRect.setTop(iconRect.top() + 1);
  // make zero size if there are no badges.
  iconRect.setRight(iconRect.left() + padding);
  std::array<float, 4> backgroundArray = { 1, 1, 1, 1 };
  int px = pPos.x();
  int py = pPos.y();

  int i = 0;
  for (auto* badge : badges)
  {
    QIcon badgeIcon(new SVGIconEngine(badge->icon(phrase, backgroundArray)));
    QSize iconsize = badgeIcon.actualSize(option.decorationSize);
    // shift each icon to the right, adding some padding.
    iconRect.setRight(iconRect.left() + iconsize.width() + padding);
    if (iconRect.contains(px, py))
    {
      badgeIndex = i;
      break;
    }
    if (badge->isDefault() && badgeIndex < 0)
    {
      // Don't stop if we find a default badge, but remember if it is the first one.
      badgeIndex = i;
    }
    ++i;
    iconRect.setLeft(iconRect.right());
  }
  return badgeIndex;
}

bool qtDescriptivePhraseDelegate::editorEvent(
  QEvent* evt,
  QAbstractItemModel* mod,
  const QStyleOptionViewItem& option,
  const QModelIndex& idx)
{
  bool res = this->QStyledItemDelegate::editorEvent(evt, mod, option, idx);
  if (evt->type() != QEvent::MouseButtonPress)
  {
    return res;
  }
  QMouseEvent* e = dynamic_cast<QMouseEvent*>(evt);
  if (!e || e->button() != Qt::LeftButton)
  {
    return res;
  }

  auto badges =
    qvariant_cast<smtk::view::BadgeSet::BadgeList>(idx.data(qtDescriptivePhraseModel::BadgesRole));
  auto phrase =
    idx.data(qtDescriptivePhraseModel::PhrasePtrRole).value<smtk::view::DescriptivePhrasePtr>();
  int badgeIndex = determineAction(e->pos(), option, badges, phrase.get());

  if (badgeIndex >= 0)
  {
    badges[badgeIndex]->action(phrase.get(), smtk::view::BadgeActionToggle());
  }

  return res;
}
} // namespace extension
} // namespace smtk
