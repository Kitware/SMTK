//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtBaseObjectNode.h"

#include "smtk/resource/PersistentObject.h"

namespace smtk
{
namespace extension
{

qtBaseObjectNode::qtBaseObjectNode(
  qtDiagramGenerator* generator,
  smtk::resource::PersistentObject* obj,
  QGraphicsItem* parent)
  : Superclass(generator, parent)
{
  this->setObjectName(QString("node ") + QString::fromStdString(obj->name()));
}

qtBaseObjectNode::~qtBaseObjectNode() = default;

std::string qtBaseObjectNode::name() const
{
  return this->object()->name();
}

} // namespace extension
} // namespace smtk
