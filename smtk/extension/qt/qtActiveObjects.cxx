//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <smtk/extension/qt/qtActiveObjects.h>

qtActiveObjects::qtActiveObjects()
{
  this->m_activeModel = smtk::model::Model();
}

qtActiveObjects::~qtActiveObjects()
{
}

qtActiveObjects& qtActiveObjects::instance()
{
  static qtActiveObjects theInstance;
  return theInstance;
}

smtk::extension::qtSelectionManagerPtr qtActiveObjects::smtkSelectionManager()
{

  return this->m_selectionMgr.lock();
}

void qtActiveObjects::setActiveModel(const smtk::model::Model& inputModel)
{
  this->m_activeModel = inputModel;
  emit this->activeModelChanged();
}
