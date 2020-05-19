

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/MembershipBadge.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/io/Logger.h"
#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/FloatData.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/operators/SetProperty.h"
#include "smtk/view/Badge.h"
#include "smtk/view/BadgeSet.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/IconFactory.h"
#include "smtk/view/Manager.h"

namespace smtk
{
namespace extension
{
namespace qt
{

MembershipBadge::MembershipBadge()
  : m_parent(nullptr)
{
}

MembershipBadge::MembershipBadge(
  smtk::view::BadgeSet& parent, const smtk::view::Configuration::Component&)
  : m_iconOn("<?xml version=\"1.0\" encoding=\"UTF-8\"?><svg width=\"64\" height=\"64\" "
             "version=\"1.1\" viewBox=\"0 0 64 64\" xmlns=\"http://www.w3.org/2000/svg\">  <g "
             "transform=\"translate(0 -988.36)\">    <rect x=\"10.64\" y=\"1009.1\" "
             "width=\"42.72\" height=\"22.555\" ry=\"7.9581\" stroke=\"#000\" "
             "stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\"/>  </g></svg>")
  , m_iconOff("<?xml version=\"1.0\" encoding=\"UTF-8\"?><svg width=\"64\" height=\"64\" "
              "version=\"1.1\" viewBox=\"0 0 64 64\" xmlns=\"http://www.w3.org/2000/svg\">  <g "
              "transform=\"translate(0 -988.36)\">    <rect x=\"7.6513\" y=\"1006.1\" "
              "width=\"48.697\" height=\"28.533\" ry=\"7.9581\" fill=\"none\" stroke=\"#000\" "
              "stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\"/>  </g></svg>")
  , m_parent(&parent)
{
}

MembershipBadge::~MembershipBadge() = default;

std::string MembershipBadge::icon(
  const DescriptivePhrase* phrase, const std::array<float, 4>& /*background*/) const
{
  auto persistentObj = phrase->relatedObject();
  auto valIt = m_members.find(persistentObj);
  int member = 0;
  if (valIt != m_members.end())
  {
    member = valIt->second;
  }
  if (member)
    return m_iconOn;
  return m_iconOff;
}

void MembershipBadge::action(const smtk::view::DescriptivePhrase* phrase)
{
  auto persistentObj = phrase->relatedObject();
  if (phrase->content() == nullptr || !persistentObj)
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(), "Can not access content or object for membership!");
    return;
  }
  auto valIt = m_members.find(persistentObj);
  if (valIt != m_members.end())
  {
    this->m_members[valIt->first] = !valIt->second;
  }
  else
  {
    this->m_members[persistentObj] = 1;
  }
  emit membershipChange(this->m_members[persistentObj]);
}
}
}
}
