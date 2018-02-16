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

#include "smtk/extension/qt/qtDescriptivePhraseEditor.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>

namespace smtk
{
namespace extension
{

qtDescriptivePhraseDelegate::qtDescriptivePhraseDelegate(QWidget* owner)
  : QStyledItemDelegate(owner)
  , m_swatchSize(16)
  , m_titleFontSize(14)
  , m_subtitleFontSize(10)
  , m_titleFontWeight(2)
  , m_subtitleFontWeight(1)
  , m_textVerticalPad(2)
  , m_drawSubtitle(true)
{
}

int qtDescriptivePhraseDelegate::swatchSize() const
{
  return this->m_swatchSize;
}

void qtDescriptivePhraseDelegate::setSwatchSize(int sfs)
{
  this->m_swatchSize = sfs;
}

int qtDescriptivePhraseDelegate::titleFontSize() const
{
  return this->m_titleFontSize;
}

void qtDescriptivePhraseDelegate::setTitleFontSize(int tfs)
{
  this->m_titleFontSize = tfs;
}

int qtDescriptivePhraseDelegate::subtitleFontSize() const
{
  return this->m_subtitleFontSize;
}

void qtDescriptivePhraseDelegate::setSubtitleFontSize(int sfs)
{
  this->m_subtitleFontSize = sfs;
}

int qtDescriptivePhraseDelegate::titleFontWeight() const
{
  return this->m_titleFontWeight;
}

void qtDescriptivePhraseDelegate::setTitleFontWeight(int tfw)
{
  this->m_titleFontWeight = tfw;
}

int qtDescriptivePhraseDelegate::subtitleFontWeight() const
{
  return this->m_subtitleFontWeight;
}

void qtDescriptivePhraseDelegate::setSubtitleFontWeight(int sfw)
{
  this->m_subtitleFontWeight = sfw;
}

int qtDescriptivePhraseDelegate::textVerticalPad() const
{
  return this->m_textVerticalPad;
}

void qtDescriptivePhraseDelegate::setTextVerticalPad(int tvp)
{
  this->m_textVerticalPad = tvp;
}

bool qtDescriptivePhraseDelegate::drawSubtitle() const
{
  return this->m_drawSubtitle;
}

void qtDescriptivePhraseDelegate::setDrawSubtitle(bool includeSubtitle)
{
  this->m_drawSubtitle = includeSubtitle;
}

bool qtDescriptivePhraseDelegate::visibilityMode() const
{
  return this->m_visibilityMode;
}

void qtDescriptivePhraseDelegate::setVisibilityMode(bool allEditsChangeVisibility)
{
  this->m_visibilityMode = allEditsChangeVisibility;
}

QSize qtDescriptivePhraseDelegate::sizeHint(
  const QStyleOptionViewItem& option, const QModelIndex& idx) const
{
  QIcon icon = qvariant_cast<QIcon>(idx.data(qtDescriptivePhraseModel::PhraseIconRole));
  QSize iconsize = icon.actualSize(option.decorationSize);
  QFont titleFont = QApplication::font();
  titleFont.setPixelSize(this->m_titleFontSize);
  titleFont.setBold(this->titleFontWeight() > 1 ? true : false);
  QFontMetrics titleFM(titleFont);
  QFont subtitleFont = QApplication::font();
  subtitleFont.setPixelSize(this->m_subtitleFontSize);
  subtitleFont.setBold(this->subtitleFontWeight() > 1 ? true : false);
  QFontMetrics subtitleFM(subtitleFont);
  int minHeight = titleFM.height() + 2 * this->m_textVerticalPad;
  if (this->m_drawSubtitle)
  {
    minHeight += subtitleFM.height();
  }
  if (minHeight < iconsize.height())
  {
    minHeight = iconsize.height();
  }

  return (QSize(iconsize.width() + this->m_swatchSize, minHeight));
}

void qtDescriptivePhraseDelegate::paint(
  QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& idx) const
{
  // If selected, draw the highlight color over entire rectangle
  if (option.state & QStyle::State_Selected)
  {
    painter->save();
    painter->setBrush(QBrush(option.palette.highlight().color()));
    painter->setPen(Qt::NoPen);
    painter->drawRect(option.rect);
    painter->restore();
  }

  painter->save();
  QIcon icon = qvariant_cast<QIcon>(idx.data(qtDescriptivePhraseModel::PhraseIconRole));
  QSize iconsize = icon.actualSize(option.decorationSize);
  QFont titleFont = QApplication::font();
  QFont subtitleFont = QApplication::font();
  titleFont.setPixelSize(this->m_titleFontSize);
  /// add a method to set/get title font
  titleFont.setBold(this->titleFontWeight() > 1 ? true : false);
  // bold the active model title
  if (idx.data(qtDescriptivePhraseModel::ModelActiveRole).toBool())
  {
    titleFont.setBold(true);
  }
  subtitleFont.setPixelSize(this->m_subtitleFontSize);
  subtitleFont.setBold(this->subtitleFontWeight() > 1 ? true : false);

  //subtitleFont.setWeight(subtitleFont.weight() - 2);
  QFontMetrics titleFM(titleFont);

  QString titleText = qvariant_cast<QString>(idx.data(qtDescriptivePhraseModel::TitleTextRole));
  if (idx.data(qtDescriptivePhraseModel::PhraseCleanRole).toInt() == 0)
  {
    titleText = titleText + " *";
  }
  QString subtitleText =
    qvariant_cast<QString>(idx.data(qtDescriptivePhraseModel::SubtitleTextRole));
  //std::cout << "Paint " << idx.internalPointer() << " " << idx.row() << " " << titleText.toStdString().c_str() << "\n";
  QColor swatchColor = qvariant_cast<QColor>(idx.data(qtDescriptivePhraseModel::PhraseColorRole));

  QRect titleRect = option.rect;
  QRect subtitleRect = option.rect;
  QRect iconRect = option.rect;
  QRect colorRect = option.rect;
  // visible icon
  QIcon visicon = qvariant_cast<QIcon>(idx.data(qtDescriptivePhraseModel::PhraseVisibilityRole));
  QSize visiconsize = visicon.actualSize(option.decorationSize);

  colorRect.setLeft(colorRect.left() + visiconsize.width() + 2);
  colorRect.setRight(colorRect.left() + this->m_swatchSize - 1);
  //  colorRect.setTop(colorRect.top() + 1);
  colorRect.setTop(colorRect.top() + 2);
  int swdelta = (colorRect.height() - this->m_swatchSize) / 2;
  swdelta = (swdelta < 0 ? 0 : swdelta);
  colorRect.adjust(0, swdelta, 0, -swdelta);
  iconRect.setLeft(colorRect.left());
  iconRect.setRight(iconRect.left() + iconsize.width() + 15);
  iconRect.setTop(iconRect.top() + 1);
  titleRect.setLeft(iconRect.right());
  subtitleRect.setLeft(iconRect.right());
  titleRect.setTop(titleRect.top() + this->m_textVerticalPad / 2.0);
  titleRect.setBottom(titleRect.top() +
    (this->m_drawSubtitle ? titleFM.height() : option.rect.height() - this->m_textVerticalPad));
  subtitleRect.setTop(titleRect.bottom() + this->m_textVerticalPad);

  if (swatchColor.isValid())
  {
    painter->save();
    painter->setBrush(swatchColor);
    painter->setPen(Qt::NoPen);
    painter->drawRect(colorRect);
    painter->restore();
  }

  //painter->drawPixmap(QPoint(iconRect.right()/2,iconRect.top()/2),icon.pixmap(iconsize.width(),iconsize.height()));
  painter->drawPixmap(
    QPoint(iconRect.left(), iconRect.top() + (option.rect.height() - iconsize.height()) / 2.),
    icon.pixmap(iconsize.width(), iconsize.height()));

  if (!visicon.isNull())
    painter->drawPixmap(QPoint(option.rect.left(), colorRect.top()),
      visicon.pixmap(visiconsize.width(), visiconsize.height()));

  if (option.state.testFlag(QStyle::State_Selected))
    painter->setPen(Qt::white);
  painter->setFont(titleFont);
  painter->drawText(titleRect, Qt::AlignVCenter, titleText);

  if (this->m_drawSubtitle)
  {
    painter->setFont(subtitleFont);
    painter->drawText(subtitleRect, subtitleText);
  }

  painter->restore();
}

QWidget* qtDescriptivePhraseDelegate::createEditor(
  QWidget* owner, const QStyleOptionViewItem& option, const QModelIndex& idx) const
{
  if (m_visibilityMode)
  {
    return nullptr;
  } // Visibility mode does not allow editing the title.
  (void)option;
  (void)idx;
  smtk::extension::qtDescriptivePhraseEditor* editor = new qtDescriptivePhraseEditor(owner);
  return editor;
}

void qtDescriptivePhraseDelegate::updateEditorGeometry(
  QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& idx) const
{
  QIcon icon = qvariant_cast<QIcon>(idx.data(qtDescriptivePhraseModel::PhraseIconRole));
  QSize iconsize = icon.actualSize(option.decorationSize);
  QFont titleFont = QApplication::font();
  QFont subtitleFont = QApplication::font();
  titleFont.setPixelSize(this->m_titleFontSize);
  /// add a method to set/get title font
  titleFont.setBold(this->titleFontWeight() > 1 ? true : false);
  // bold the active model title
  if (idx.data(qtDescriptivePhraseModel::ModelActiveRole).toBool())
  {
    titleFont.setBold(true);
  }
  subtitleFont.setPixelSize(this->m_subtitleFontSize);
  subtitleFont.setBold(this->subtitleFontWeight() > 1 ? true : false);

  //subtitleFont.setWeight(subtitleFont.weight() - 2);
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
  QRect colorRect = option.rect;
  // visible icon
  QIcon visicon = qvariant_cast<QIcon>(idx.data(qtDescriptivePhraseModel::PhraseVisibilityRole));
  QSize visiconsize = visicon.actualSize(option.decorationSize);

  colorRect.setLeft(colorRect.left() + visiconsize.width() + 2);
  colorRect.setRight(colorRect.left() + this->m_swatchSize);
  int swdelta = (colorRect.height() - this->m_swatchSize) / 2;
  swdelta = (swdelta < 0 ? 0 : swdelta);
  colorRect.adjust(0, swdelta, 0, -swdelta);
  iconRect.setLeft(colorRect.left());
  iconRect.setRight(iconRect.left() + iconsize.width() + 15);
  iconRect.setTop(iconRect.top() + 1);
  titleRect.setLeft(iconRect.right());

  editor->setGeometry(titleRect);
}

void qtDescriptivePhraseDelegate::setEditorData(QWidget* editor, const QModelIndex& idx) const
{
  smtk::extension::qtDescriptivePhraseEditor* entityEditor =
    qobject_cast<smtk::extension::qtDescriptivePhraseEditor*>(editor);
  if (entityEditor)
  {
    entityEditor->setTitle(idx.data(qtDescriptivePhraseModel::TitleTextRole).toString());
    // TODO: editor should also allow adjusting entity type?
  }
}

void qtDescriptivePhraseDelegate::setModelData(
  QWidget* editor, QAbstractItemModel* model, const QModelIndex& idx) const
{
  smtk::extension::qtDescriptivePhraseEditor* entityEditor =
    qobject_cast<smtk::extension::qtDescriptivePhraseEditor*>(editor);
  if (entityEditor)
  {
    // TODO: editor should also allow adjusting entity type?
    model->setData(idx, entityEditor->title(), qtDescriptivePhraseModel::TitleTextRole);
  }
}

bool qtDescriptivePhraseDelegate::eventFilter(QObject* editor, QEvent* evt)
{
  if (evt->type() == QEvent::MouseButtonPress)
    return false;
  return QStyledItemDelegate::eventFilter(editor, evt);
}

std::string qtDescriptivePhraseDelegate::determineAction(const QPoint& pPos, const QModelIndex& idx,
  const QStyleOptionViewItem& option,
  const smtk::extension::qtDescriptivePhraseModel* entityMod) const
{
  std::string res;
  if (m_visibilityMode)
  {
    res = "visible";
    return res;
  }
  // with the help of styles, return where the pPos is on:
  // the eye-ball, or the color swatch
  // visible icon
  QIcon visicon = qvariant_cast<QIcon>(idx.data(qtDescriptivePhraseModel::PhraseVisibilityRole));
  QSize visiconsize = visicon.actualSize(option.decorationSize);
  int px = pPos.x();
  int py = pPos.y();
  bool bvis = false, bcolor = false;
  if (!visicon.isNull())
  {
    bvis = px > option.rect.left() && px < (option.rect.left() + visiconsize.width()) &&
      py > option.rect.top() && py < (option.rect.top() + option.rect.height());
  }
  if (!bvis && entityMod && entityMod->getItem(idx)->isRelatedColorMutable())
  {
    bcolor = px > (option.rect.left() + visiconsize.width() + 2) &&
      px < (option.rect.left() + visiconsize.width() + 2 + this->m_swatchSize) &&
      py > option.rect.top() && py < (option.rect.top() + option.rect.height());
  }

  if (bvis)
  {
    res = "visible";
  }
  else if (bcolor)
  {
    res = "color";
  }
  return res;
}

bool qtDescriptivePhraseDelegate::editorEvent(
  QEvent* evt, QAbstractItemModel* mod, const QStyleOptionViewItem& option, const QModelIndex& idx)
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

  // pass in qComponentItemModel for mutability check
  smtk::extension::qtDescriptivePhraseModel* entityMod =
    dynamic_cast<smtk::extension::qtDescriptivePhraseModel*>(mod);
  std::string whichIcon = this->determineAction(e->pos(), idx, option, entityMod);

  if (whichIcon == "visible")
  {
    emit this->requestVisibilityChange(idx);
  }
  else if (whichIcon == "color")
  {
    emit this->requestColorChange(idx);
  }

  return res;
}

} // namespace model
} // namespace smtk
