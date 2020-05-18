//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_view_Manager_h
#define smtk_view_Manager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/common/TypeName.h"

#include "smtk/view/BadgeFactory.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/ObjectIcons.h"
#include "smtk/view/OperationIcons.h"
#include "smtk/view/PhraseModelFactory.h"
#include "smtk/view/SubphraseGeneratorFactory.h"
#include "smtk/view/ViewWidgetFactory.h"

#include <array>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>

namespace smtk
{
namespace view
{
/// A view Manager is responsible for creating new views (eventually) as well as
/// view components such as PhraseModels and SubPhraseGenerators.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  smtkTypeMacroBase(smtk::view::Manager);
  smtkCreateMacro(smtk::view::Manager);

  virtual ~Manager();

public:
  BadgeFactory& badgeFactory() { return m_badgeFactory; }
  const BadgeFactory& badgeFactory() const { return m_badgeFactory; }

  ObjectIcons& objectIcons() { return m_objectIcons; }
  const ObjectIcons& objectIcons() const { return m_objectIcons; }

  OperationIcons& operationIcons() { return m_operationIcons; }
  const OperationIcons& operationIcons() const { return m_operationIcons; }

  PhraseModelFactory& phraseModelFactory() { return m_phraseModelFactory; }
  const PhraseModelFactory& phraseModelFactory() const { return m_phraseModelFactory; }

  SubphraseGeneratorFactory& subphraseGeneratorFactory() { return m_subphraseGeneratorFactory; }
  const SubphraseGeneratorFactory& subphraseGeneratorFactory() const
  {
    return m_subphraseGeneratorFactory;
  }

  ViewWidgetFactory& viewWidgetFactory() { return m_viewWidgetFactory; }
  const ViewWidgetFactory& viewWidgetFactory() const { return m_viewWidgetFactory; }

private:
  BadgeFactory m_badgeFactory;
  ObjectIcons m_objectIcons;
  OperationIcons m_operationIcons;
  PhraseModelFactory m_phraseModelFactory;
  SubphraseGeneratorFactory m_subphraseGeneratorFactory;
  ViewWidgetFactory m_viewWidgetFactory;

protected:
  Manager();
};
} // namespace view
} // namespace smtk

#endif // smtk_view_Manager_h
