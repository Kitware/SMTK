#include "smtk/Qt/qtEntityItemDelegate.h"

#include "smtk/Qt/qtEntityItemModel.h"
#include "smtk/Qt/qtEntityItemEditor.h"

#include <QApplication>
#include <QPainter>

namespace smtk {
  namespace model {

QEntityItemDelegate::QEntityItemDelegate(QWidget* owner)
  : QStyledItemDelegate(owner), m_swatchSize(16), m_titleFontSize(16), m_subtitleFontSize(12)
{
}

int QEntityItemDelegate::swatchSize() const
{
  return this->m_swatchSize;
}

void QEntityItemDelegate::setSwatchSize(int sfs)
{
  this->m_swatchSize = sfs;
}

int QEntityItemDelegate::titleFontSize() const
{
  return this->m_titleFontSize;
}

void QEntityItemDelegate::setTitleFontSize(int tfs)
{
  this->m_titleFontSize = tfs;
}

int QEntityItemDelegate::subtitleFontSize() const
{
  return this->m_subtitleFontSize;
}

void QEntityItemDelegate::setSubtitleFontSize(int sfs)
{
  this->m_subtitleFontSize = sfs;
}

QSize QEntityItemDelegate::sizeHint(
  const QStyleOptionViewItem& option,
  const QModelIndex& idx) const
{
  QIcon icon = qvariant_cast<QIcon>(idx.data(QEntityItemModel::EntityIconRole));
  QSize iconsize = icon.actualSize(option.decorationSize);
  QFont titleFont = QApplication::font();
  titleFont.setPixelSize(this->m_titleFontSize);
  titleFont.setBold(true);
  QFontMetrics titleFM(titleFont);
  QFont subtitleFont = titleFont;
  //std::cout << "Subtitle is " << subtitleFont.pixelSize() << "\n";
  subtitleFont.setPixelSize(this->m_subtitleFontSize);
  //subtitleFont.setPixelSize(subtitleFont.pixelSize() - 2);
  subtitleFont.setWeight(subtitleFont.weight() - 2);
  QFontMetrics subtitleFM(subtitleFont);
  int minHeight = titleFM.height() + 2 /*inter-line spacing*/ + subtitleFM.height();
  if (minHeight < iconsize.height())
    {
    minHeight = iconsize.height();
    }
  minHeight += 4; // 2-pixel border at top and bottom

  return(QSize(iconsize.width() + this->m_swatchSize, minHeight));
}

void QEntityItemDelegate::paint(
  QPainter* painter,
  const QStyleOptionViewItem& option,
  const QModelIndex& idx) const
{
  QStyledItemDelegate::paint(painter,option,idx);

  painter->save();

  QIcon icon = qvariant_cast<QIcon>(idx.data(QEntityItemModel::EntityIconRole));
  QSize iconsize = icon.actualSize(option.decorationSize);
  QColor swatchColor = qvariant_cast<QColor>(idx.data(QEntityItemModel::EntityColorRole));
  QFont titleFont = QApplication::font();
  QFont subtitleFont = titleFont;
  titleFont.setPixelSize(this->m_titleFontSize);
  titleFont.setBold(true);
  subtitleFont.setPixelSize(this->m_subtitleFontSize);
  //subtitleFont.setPixelSize(subtitleFont.pixelSize() - 2);
  subtitleFont.setWeight(subtitleFont.weight() - 2);
  QFontMetrics titleFM(titleFont);
  QFontMetrics subtitleFM(subtitleFont);

  QString titleText = qvariant_cast<QString>(idx.data(QEntityItemModel::TitleTextRole));
  QString subtitleText = qvariant_cast<QString>(idx.data(QEntityItemModel::SubtitleTextRole));
  //std::cout << "Paint " << idx.internalPointer() << " " << idx.row() << " " << titleText.toStdString().c_str() << "\n";

  QRect titleRect = option.rect;
  QRect subtitleRect = option.rect;
  QRect iconRect = option.rect;
  QRect colorRect = option.rect;

  colorRect.setRight(colorRect.left() + this->m_swatchSize);
  int swdelta = (colorRect.height() - this->m_swatchSize) / 2;
  swdelta = (swdelta < 0 ? 0 : swdelta);
  colorRect.adjust(0, swdelta, 0, -swdelta);
  iconRect.setLeft(colorRect.right());
  iconRect.setRight(iconRect.left() + iconsize.width() + 30);
  iconRect.setTop(iconRect.top() + 1);
  titleRect.setLeft(iconRect.right());
  subtitleRect.setLeft(iconRect.right());
  titleRect.setTop(titleRect.top() + 1);
  titleRect.setBottom(titleRect.top() + titleFM.height());
  subtitleRect.setTop(titleRect.bottom() + 2);

  painter->save();
  painter->setBrush(swatchColor);
  painter->setPen(Qt::NoPen);
  painter->drawRect(colorRect);
  painter->restore();
  //painter->drawPixmap(QPoint(iconRect.right()/2,iconRect.top()/2),icon.pixmap(iconsize.width(),iconsize.height()));
  painter->drawPixmap(
    QPoint(
      iconRect.left() + iconsize.width() / 2 + 2,
      iconRect.top() + iconsize.height() / 2 + 3),
    icon.pixmap(iconsize.width(), iconsize.height()));

  painter->setFont(titleFont);
  painter->drawText(titleRect, titleText);


  painter->setFont(subtitleFont);
  painter->drawText(subtitleRect, subtitleText);

  painter->restore();
}

QWidget* QEntityItemDelegate::createEditor(
  QWidget* owner,
  const QStyleOptionViewItem& option,
  const QModelIndex &idx) const
{
  (void)option;
  (void)idx;
  smtk::model::QEntityItemEditor* editor = new QEntityItemEditor(owner);
  QObject::connect(
    editor, SIGNAL(editingFinished()),
    this, SLOT(commitAndCloseEditor()));
  return editor;
}

void QEntityItemDelegate::setEditorData(
  QWidget* editor,
  const QModelIndex& idx) const
{
  smtk::model::QEntityItemEditor* entityEditor =
    qobject_cast<smtk::model::QEntityItemEditor*>(editor);
  if (entityEditor)
    {
    entityEditor->setTitle(idx.data(QEntityItemModel::TitleTextRole).toString());
    // TODO: editor should also allow adjusting entity type?
    }
}

void QEntityItemDelegate::setModelData(
  QWidget* editor,
  QAbstractItemModel* model,
  const QModelIndex &idx) const
{
  smtk::model::QEntityItemEditor* entityEditor =
    qobject_cast<smtk::model::QEntityItemEditor*>(editor);
  if (entityEditor)
    {
    // TODO: editor should also allow adjusting entity type?
    model->setData(
      idx, entityEditor->title(), QEntityItemModel::TitleTextRole);
    }
}

void QEntityItemDelegate::commitAndCloseEditor()
{
  smtk::model::QEntityItemEditor* entityEditor =
    qobject_cast<smtk::model::QEntityItemEditor*>(sender());
  emit commitData(entityEditor);
  emit closeEditor(entityEditor);
}

  } // namespace model
} // namespace smtk
