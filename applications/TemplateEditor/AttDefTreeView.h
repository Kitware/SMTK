//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __AttDefTreeView_h
#define __AttDefTreeView_h
#include <QKeyEvent>
#include <QTreeView>

/**
 * \brief Regular QTreeView with additional keyboard shortcuts.
 */
class AttDefTreeView : public QTreeView
{
  Q_OBJECT

public:
  AttDefTreeView(QWidget* parent = nullptr)
    : QTreeView(parent){};

  ~AttDefTreeView() = default;

signals:
  void showDialog(const QModelIndex&);

protected:
  void keyPressEvent(QKeyEvent* event) override
  {
    switch (event->key())
    {
      case Qt::Key_Enter:
      case Qt::Key_Return:
      {
        auto sm = QTreeView::selectionModel();
        emit showDialog(sm->currentIndex());
      }
      break;
    }

    QTreeView::keyPressEvent(event);
  };

private:
  AttDefTreeView(const AttDefTreeView&) = delete;
  void operator=(const AttDefTreeView&) = delete;
};
#endif //__AttDefTreeView_h
