//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/qtSMTKUtilities.h"
#include "smtk/common/UUID.h"
#include "smtk/extension/qt/qtResourceBrowser.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/model/EntityRef.h"

using qtItemConstructor = smtk::extension::qtItemConstructor;
using qtModelViewConstructor = smtk::extension::qtModelViewConstructor;
using qtResourceBrowser = smtk::extension::qtResourceBrowser;

SMTKViewConstructorMap qtSMTKUtilities::s_viewConstructors;
SMTKItemConstructorMap qtSMTKUtilities::s_itemConstructors;
SMTKModelViewConstructorMap qtSMTKUtilities::s_modelViewConstructors;

const SMTKViewConstructorMap& qtSMTKUtilities::viewConstructors()
{
  return qtSMTKUtilities::s_viewConstructors;
}

const SMTKItemConstructorMap& qtSMTKUtilities::itemConstructors()
{
  return qtSMTKUtilities::s_itemConstructors;
}

const SMTKModelViewConstructorMap& qtSMTKUtilities::modelViewConstructors()
{
  auto& ctors = qtSMTKUtilities::s_modelViewConstructors;
  auto defEntry = ctors.find("");
  if (defEntry == ctors.end())
  {
    ctors[""] = qtResourceBrowser::createDefaultView;
  }
  return ctors;
}

void qtSMTKUtilities::registerViewConstructor(
  const std::string& viewName, qtSMTKViewConstructor viewc)
{
  // this will overwrite the existing constructor if the viewname exists in the map
  qtSMTKUtilities::s_viewConstructors[viewName] = viewc;
}

void qtSMTKUtilities::registerItemConstructor(const std::string& itemName, qtItemConstructor itemc)
{
  // this will overwrite the existing constructor if the itemName exists in the map
  qtSMTKUtilities::s_itemConstructors[itemName] = itemc;
}

void qtSMTKUtilities::registerModelViewConstructor(
  const std::string& viewName, smtk::extension::qtModelViewConstructor viewc)
{
  qtSMTKUtilities::s_modelViewConstructors[viewName] = viewc;
}

void qtSMTKUtilities::updateViewConstructors(smtk::extension::qtUIManager* uiMan)
{
  if (!uiMan || qtSMTKUtilities::viewConstructors().empty())
    return;

  SMTKViewConstructorMap::const_iterator it;
  for (it = qtSMTKUtilities::viewConstructors().begin();
       it != qtSMTKUtilities::viewConstructors().end(); ++it)
  {
    uiMan->registerViewConstructor(it->first, it->second);
  }
}

void qtSMTKUtilities::updateItemConstructors(smtk::extension::qtUIManager* uiMan)
{
  if (!uiMan || qtSMTKUtilities::itemConstructors().empty())
    return;

  SMTKItemConstructorMap::const_iterator it;
  for (it = qtSMTKUtilities::itemConstructors().begin();
       it != qtSMTKUtilities::itemConstructors().end(); ++it)
  {
    uiMan->registerItemConstructor(it->first, it->second);
  }
}

QVariant qtSMTKUtilities::UUIDToQVariant(const smtk::common::UUID& uuid)
{
  QVariant vdata(QByteArray(
    reinterpret_cast<const char*>(uuid.begin()), static_cast<int>(smtk::common::UUID::size())));
  return vdata;
}

QVariant qtSMTKUtilities::entityRefToQVariant(const smtk::model::EntityRef& ent)
{
  return qtSMTKUtilities::UUIDToQVariant(ent.entity());
}
smtk::common::UUID qtSMTKUtilities::QVariantToUUID(QVariant variant)
{
  QByteArray uuidData = variant.toByteArray();
  if (uuidData.size() != static_cast<int>(smtk::common::UUID::size()))
  {
    return smtk::common::UUID();
  }

  smtk::common::UUID uuid(
    reinterpret_cast<smtk::common::UUID::const_iterator>(uuidData.constData()),
    reinterpret_cast<smtk::common::UUID::const_iterator>(uuidData.constData() + uuidData.size()));
  return uuid;
}

smtk::model::EntityRef qtSMTKUtilities::QVariantToEntityRef(
  QVariant variant, smtk::model::ResourcePtr mResource)
{
  smtk::common::UUID uuid = qtSMTKUtilities::QVariantToUUID(variant);
  return smtk::model::EntityRef(mResource, uuid);
}
