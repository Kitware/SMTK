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
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/common/UUID.h"
#include "smtk/model/EntityRef.h"

SMTKViewConstructorMap qtSMTKUtilities::m_viewConstructors;

const SMTKViewConstructorMap& qtSMTKUtilities::viewConstructors()
{
  return qtSMTKUtilities::m_viewConstructors;
}

void qtSMTKUtilities::registerViewConstructor(const std::string& viewname, qtSMTKViewConstructor viewc)
{
  // this will overwrite the existing constructor if the viewname exists in the map
  qtSMTKUtilities::m_viewConstructors[viewname] = viewc;
}

void qtSMTKUtilities::updateViewConstructors(smtk::extension::qtUIManager* uiMan)
{
  if(!uiMan || qtSMTKUtilities::viewConstructors().size() == 0)
    return;

  SMTKViewConstructorMap::const_iterator it;
  for(it = qtSMTKUtilities::viewConstructors().begin();
      it != qtSMTKUtilities::viewConstructors().end(); ++it)
    {
    uiMan->registerViewConstructor(it->first, it->second);
    }
}

QVariant qtSMTKUtilities::UUIDToQVariant(const smtk::common::UUID &uuid)
{
  QVariant vdata(QByteArray(reinterpret_cast<const char*>(
                            uuid.begin()), uuid.size()));
  return vdata;
}

QVariant qtSMTKUtilities::entityRefToQVariant(const smtk::model::EntityRef &ent)
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
     reinterpret_cast<smtk::common::UUID::const_iterator>(uuidData.constData()
                                               + uuidData.size()));
   return uuid;
}

smtk::model::EntityRef qtSMTKUtilities::QVariantToEntityRef(QVariant variant,
                                                  smtk::model::ManagerPtr mgr)
{
  smtk::common::UUID uuid = qtSMTKUtilities::QVariantToUUID(variant);
   return smtk::model::EntityRef(mgr, uuid);
}
