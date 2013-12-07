#include "smtk/Qt/qtEntityItemDelegate.h"

#include "smtk/Qt/qtEntityItemModel.h"
#include "smtk/Qt/qtEntityItemEditor.h"

#include <QApplication>
#include <QPainter>

namespace smtk {
  namespace model {

QEntityItemDelegate::QEntityItemDelegate(QWidget* parent)
  : QStyledItemDelegate(parent)
{
}

QSize QEntityItemDelegate::sizeHint(
  const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  QIcon icon = qvariant_cast<QIcon>(index.data(QEntityItemModel::EntityIconRole));
  QSize iconsize = icon.actualSize(option.decorationSize);
  QFont titleFont = QApplication::font();
  titleFont.setPixelSize(24);
  titleFont.setBold(true);
  QFontMetrics titleFM(titleFont);
  QFont subtitleFont = titleFont;
  //subtitleFont.setPixelSize(subtitleFont.pixelSize() - 2);
  subtitleFont.setWeight(subtitleFont.weight() - 2);
  QFontMetrics subtitleFM(subtitleFont);
  int minHeight = titleFM.height() + 2 /*inter-line spacing*/ + subtitleFM.height();
  if (minHeight < iconsize.height())
    {
    minHeight = iconsize.height();
    }
  minHeight += 4; // 2-pixel border at top and bottom

  return(QSize(iconsize.width(), minHeight));
}

void QEntityItemDelegate::paint(
  QPainter* painter,
  const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  QStyledItemDelegate::paint(painter,option,index);

  painter->save();

  QIcon icon = qvariant_cast<QIcon>(index.data(QEntityItemModel::EntityIconRole));
  QSize iconsize = icon.actualSize(option.decorationSize);
  QFont titleFont = QApplication::font();
  QFont subtitleFont = titleFont;
  titleFont.setPixelSize(24);
  titleFont.setBold(true);
  //subtitleFont.setPixelSize(subtitleFont.pixelSize() - 2);
  subtitleFont.setWeight(subtitleFont.weight() - 2);
  QFontMetrics titleFM(titleFont);
  QFontMetrics subtitleFM(subtitleFont);

  QString titleText = qvariant_cast<QString>(index.data(QEntityItemModel::TitleTextRole));
  QString subtitleText = qvariant_cast<QString>(index.data(QEntityItemModel::SubtitleTextRole));

  QRect titleRect = option.rect;
  QRect subtitleRect = option.rect;
  QRect iconRect = option.rect;

  iconRect.setRight(iconsize.width() + 30);
  iconRect.setTop(iconRect.top() + 1);
  titleRect.setLeft(iconRect.right());
  subtitleRect.setLeft(iconRect.right());
  titleRect.setTop(titleRect.top() + 1);
  titleRect.setBottom(titleRect.top() + titleFM.height());
  subtitleRect.setTop(titleRect.bottom() + 2);

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
  QWidget* parent,
  const QStyleOptionViewItem& option,
  const QModelIndex &index) const
{
  (void)option;
  (void)index;
  smtk::model::QEntityItemEditor* editor = new QEntityItemEditor(parent);
  QObject::connect(
    editor, SIGNAL(editingFinished()),
    this, SLOT(commitAndCloseEditor()));
  return editor;
}

void QEntityItemDelegate::setEditorData(
  QWidget* editor,
  const QModelIndex& index) const
{
  smtk::model::QEntityItemEditor* entityEditor =
    qobject_cast<smtk::model::QEntityItemEditor*>(editor);
  if (entityEditor)
    {
    entityEditor->setTitle(index.data(QEntityItemModel::TitleTextRole).toString());
    // TODO: editor should also allow adjusting entity type?
    }
}

void QEntityItemDelegate::setModelData(
  QWidget* editor,
  QAbstractItemModel* model,
  const QModelIndex &index) const
{
  smtk::model::QEntityItemEditor* entityEditor =
    qobject_cast<smtk::model::QEntityItemEditor*>(editor);
  if (entityEditor)
    {
    // TODO: editor should also allow adjusting entity type?
    model->setData(
      index, entityEditor->title(), QEntityItemModel::TitleTextRole);
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
