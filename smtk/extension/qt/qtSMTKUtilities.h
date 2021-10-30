//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_qtSMTKUtilities_h
#define smtk_attribute_qtSMTKUtilities_h

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtUIManager.h" // for qtItemConstructor definition

#include <QVariant>

#include <functional>
#include <map>

class QTreeView;

namespace smtk
{
namespace extension
{
using qtModelViewConstructor = std::function<QTreeView*(QWidget*)>;
}
} // namespace smtk

typedef std::map<std::string, smtk::extension::qtItemConstructor> SMTKItemConstructorMap;
typedef std::map<std::string, smtk::extension::qtModelViewConstructor> SMTKModelViewConstructorMap;

/// qtSMTKUtilities is a collection of arbitrary utility functions that can be
/// used by an smtk application.
class SMTKQTEXT_EXPORT qtSMTKUtilities
{

public:
  static const SMTKItemConstructorMap& itemConstructors();
  static const SMTKModelViewConstructorMap& modelViewConstructors();

  // this will overwrite the existing constructor if viewName exists in the map
  static void registerItemConstructor(
    const std::string& itemName,
    smtk::extension::qtItemConstructor itemc);

  // this will overwrite the existing constructor if viewName exists in the map
  static void registerModelViewConstructor(
    const std::string& viewName,
    smtk::extension::qtModelViewConstructor viewc);

  static void updateItemConstructors(smtk::extension::qtUIManager* uiMan);

  // convenient method for qvariant - UUID/EntityRef conversion
  static QVariant UUIDToQVariant(const smtk::common::UUID& uuid);
  static QVariant entityRefToQVariant(const smtk::model::EntityRef& ent);
  static smtk::common::UUID QVariantToUUID(QVariant variant);
  static smtk::model::EntityRef QVariantToEntityRef(
    QVariant variant,
    smtk::model::ResourcePtr mresource);

private:
  static SMTKItemConstructorMap s_itemConstructors;
  static SMTKModelViewConstructorMap s_modelViewConstructors;
};

#endif
