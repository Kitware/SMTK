//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_paraview_appcomponents_VisibilityBadge_h
#define smtk_extension_paraview_appcomponents_VisibilityBadge_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/view/Badge.h"
#include "smtk/view/BadgeSet.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/DescriptivePhrase.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/SharedFromThis.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace appcomponents
{
using DescriptivePhrase = smtk::view::DescriptivePhrase;

class SMTKPQCOMPONENTSEXT_EXPORT VisibilityBadge : public smtk::view::Badge
{
public:
  smtkTypeMacro(VisibilityBadge);
  smtkSuperclassMacro(smtk::view::Badge);
  smtkSharedFromThisMacro(smtk::view::Badge);
  smtkCreateMacro(smtk::view::Badge);

  VisibilityBadge()
    : m_parent(nullptr)
  {
  }
  VisibilityBadge(smtk::view::BadgeSet& parent, const smtk::view::Configuration::Component&)
    : m_icon("<svg id=\"Layer_1\" data-name=\"Layer 1\" xmlns=\"http://www.w3.org/2000/svg\" "
             "viewBox=\"0 0 64 64\"><title>SVG_Artboards</title><path "
             "d=\"M59.94,31.86S47.49,14.37,31.31,14.37c-16.81,0-28.1,17.49-28.1,17.49S14.5,49.35,"
             "31.31,49.35C47.49,49.35,59.94,31.86,59.94,31.86Z\" style=\"fill:#fff\"/><path "
             "d=\"M31.31,50.48c-17.22,0-28.58-17.27-29-18a1.12,1.12,0,0,1,0-1.22c.47-.73,11.83-18,"
             "29.05-18,16.57,0,29,17.23,29.54,18a1.13,1.13,0,0,1,0,1.31C60.33,33.25,47.88,50.48,31."
             "31,50.48ZM4.57,31.86c2.18,3,12.51,16.37,26.74,16.37C45,48.23,56.09,35,58.52,31.86,56."
             "09,28.74,45,15.5,31.31,15.5,17.08,15.5,6.75,28.82,4.57,31.86Z\" "
             "style=\"fill:#787878\"/><circle cx=\"31.57\" cy=\"31.86\" r=\"16.19\" "
             "style=\"fill:#2896d3\"/><circle cx=\"31.57\" cy=\"31.86\" r=\"9.51\" "
             "style=\"fill:#12141c\"/><g style=\"opacity:0.9\"><circle cx=\"24.2\" cy=\"24.17\" "
             "r=\"6.65\" style=\"fill:#fff\"/></g></svg>")
    , m_iconClosed("<svg id=\"Layer_1\" data-name=\"Layer 1\" xmlns=\"http://www.w3.org/2000/svg\" "
                   "viewBox=\"0 0 64 64\"><title>SVG_Artboards</title><path "
                   "d=\"M31.47,45.11c-12.45,0-21.59-9.19-22-9.58A1.13,1.13,0,0,1,11.1,34c.09.09,8."
                   "88,8.91,20.37,8.91s20.28-9.23,20.36-9.33a1.14,1.14,0,0,1,1.6-.05,1.13,1.13,0,0,"
                   "1,0,1.59C53.09,35.48,43.93,45.11,31.47,45.11Z\" style=\"fill:#787878\"/><path "
                   "d=\"M10.3,26.11a1.13,1.13,0,0,1-.77-.3,1.11,1.11,0,0,1,0-1.59c.38-.41,9.55-10,"
                   "22-10s21.59,9.19,22,9.58a1.12,1.12,0,0,1,0,1.59,1.13,1.13,0,0,1-1.6,0c-.09-.09-"
                   "8.88-8.91-20.36-8.91s-20.28,9.23-20.37,9.33A1.14,1.14,0,0,1,10.3,26.11Z\" "
                   "style=\"fill:#787878\"/><path "
                   "d=\"M31.47,52.6a1.13,1.13,0,0,1-1.13-1.13V44a1.13,1.13,0,1,1,2.25,0v7.48A1.13,"
                   "1.13,0,0,1,31.47,52.6Z\" style=\"fill:#787878\"/><path "
                   "d=\"M21.43,51.14a1.12,1.12,0,0,1-1.09-1.44l2.12-7.18a1.13,1.13,0,1,1,2.16.64l-"
                   "2.11,7.18A1.13,1.13,0,0,1,21.43,51.14Z\" style=\"fill:#787878\"/><path "
                   "d=\"M41.5,51.14a1.13,1.13,0,0,1-1.08-.8l-2.11-7.18a1.13,1.13,0,0,1,2.16-.64l2."
                   "12,7.18a1.12,1.12,0,0,1-1.09,1.44Z\" style=\"fill:#787878\"/><path "
                   "d=\"M51.78,45.94a1.13,1.13,0,0,1-.88-.42l-4.69-5.83A1.12,1.12,0,0,1,48,38.28l4."
                   "7,5.83a1.13,1.13,0,0,1-.88,1.83Z\" style=\"fill:#787878\"/><path "
                   "d=\"M11.15,45.94a1.13,1.13,0,0,1-.88-1.83l4.57-5.67a1.13,1.13,0,0,1,1.76,1."
                   "41L12,45.52A1.11,1.11,0,0,1,11.15,45.94Z\" style=\"fill:#787878\"/></svg>")
    , m_parent(&parent)
  {
  }

  ~VisibilityBadge() override {}

  std::string tooltip(const DescriptivePhrase*) const override
  {
    return std::string("Click to toggle visibility");
  }
  std::string icon(const DescriptivePhrase*, const std::array<float, 4>&) const override
  {
    return m_icon;
  }

  /// take an action when the badge is clicked.
  void action(const DescriptivePhrase*) const override {}
private:
  // borrowed from paraview Qt/Components
  std::string m_icon;
  std::string m_iconClosed;
  // Selection state of items shown in m_phraseModel:
  std::map<std::weak_ptr<smtk::resource::PersistentObject>, int,
    std::owner_less<std::weak_ptr<smtk::resource::PersistentObject> > >
    m_selected;
  const smtk::view::BadgeSet* m_parent;
};
}
}
}
}
#endif
