//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtResourceDiagramSummary.h"

#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"
#include "smtk/extension/qt/diagram/qtResourceDiagram.h"
#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/extension/qt/diagram/qtComponentNode.h"
#include "smtk/extension/qt/diagram/qtGroupingNode.h"
#include "smtk/extension/qt/diagram/qtResourceNode.h"

#include "smtk/task/Active.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Task.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsTextItem>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QPainter>
#include <QStringBuilder>
#include <QTimer>

class QAbstractItemModel;
class QItemSelection;
class QTreeView;

namespace smtk
{
namespace extension
{

qtResourceDiagramSummary::qtResourceDiagramSummary(qtResourceDiagram* generator, QWidget* parent)
  : Superclass(parent)
  , m_generator(generator)
{
}

qtResourceDiagramSummary::~qtResourceDiagramSummary() = default;

void qtResourceDiagramSummary::dataUpdated()
{
  QString html;
  if (m_subject)
  {
    html = this->describe(m_subject.data());
    if (html != this->text())
    {
      this->setText(html);
    }
  }
  else
  {
    // TODO: If no subject, describe selection.
    if (!this->text().isEmpty())
    {
      this->setText(QString());
    }
  }
}

bool qtResourceDiagramSummary::setSubject(qtBaseNode* subject)
{
  if (m_subject == subject)
  {
    return false;
  }
  m_subject = subject;
  this->dataUpdated();
  return true;
}

qtDiagram* qtResourceDiagramSummary::diagram() const
{
  return m_generator->diagram();
}

QString qtResourceDiagramSummary::describe(qtBaseNode* node)
{
  QString html;
  if (auto* groupNode = dynamic_cast<qtGroupingNode*>(node))
  {
    html = "<b>" % QString::fromStdString(groupNode->name()) % "</b>";
  }
  else if (auto* objectNode = dynamic_cast<qtBaseObjectNode*>(node))
  {
    if (auto* parent = dynamic_cast<qtBaseNode*>(objectNode->parentItem()))
    {
      html = "<b>" % QString::fromStdString(objectNode->object()->name()) % "</b><br><span>" %
        QString::fromStdString(parent->name()) % "</span>";
    }
    else
    {
      // TODO: Look up object()->typeName() in resource::Manager::objectTypeNames()
      //       in order to decorate type.
      html = "<b>" % QString::fromStdString(objectNode->object()->name()) % "</b><br><span>" %
        QString::fromStdString(objectNode->object()->typeName()) % "</span>";
    }
  }
  else
  {
    html = "—";
  }
  return html;
}

} // namespace extension
} // namespace smtk
