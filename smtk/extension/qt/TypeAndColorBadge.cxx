

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/TypeAndColorBadge.h"
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
#include "smtk/view/BadgeSet.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/IconFactory.h"
#include "smtk/view/Manager.h"
#include "smtk/view/ObjectIconBadge.h"

#include <QColor>
#include <QColorDialog>

namespace smtk
{
namespace extension
{
namespace qt
{

TypeAndColorBadge::TypeAndColorBadge() = default;

TypeAndColorBadge::TypeAndColorBadge(
  smtk::view::BadgeSet& parent, const smtk::view::Configuration::Component& comp)
  : Superclass(parent, comp)
{
}

TypeAndColorBadge::~TypeAndColorBadge() = default;

smtk::resource::FloatList colorValue(smtk::resource::ComponentPtr component)
{
  if (component && component->properties().get<resource::FloatList>().contains("color") &&
    component->properties().get<resource::FloatList>().at("color").size() == 4)
  {
    return component->properties().get<resource::FloatList>().at("color");
  }
  smtk::resource::FloatList rgba({ 0., 0., 0., -1.0 });
  return rgba;
}

QColor getPhraseColor(const smtk::view::DescriptivePhrase* item)
{
  QColor color;
  if (!(item->relatedComponent() || item->phraseType() == smtk::view::DescriptivePhraseType::LIST))
  {
    return color;
  }

  auto component = item->relatedComponent();
  smtk::model::FloatList rgba = colorValue(component);
  bool gotColor = false;
  if (rgba.size() >= 4 && rgba[3] < 0)
  {
    auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(component);
    if (modelComp)
    {
      smtk::model::EntityRef ent(modelComp);
      if (ent.isFace())
      {
        color = qtDescriptivePhraseModel::defaultPhraseColor("Face");
        gotColor = true;
      }
      else if (ent.isEdge())
      {
        color = qtDescriptivePhraseModel::defaultPhraseColor("Edge");
        gotColor = true;
      }
      else if (ent.isVertex())
      {
        color = qtDescriptivePhraseModel::defaultPhraseColor("Vertex");
        gotColor = true;
      }
    }
    if (!gotColor)
    {
      // return an invalid color by default
      color = QColor();
    }
  }
  else
  {
    // Color may be luminance, luminance+alpha, rgb, or rgba:
    switch (rgba.size())
    {
      case 0:
        color = QColor(0, 0, 0, 0);
        break;
      case 1:
        color.setHslF(0., 0., rgba[0], 1.);
        break;
      case 2:
        color.setHslF(0., 0., rgba[0], rgba[1]);
        break;
      case 3:
        color.setRgbF(rgba[0], rgba[1], rgba[2], 1.);
        break;
      case 4:
      default:
        color.setRgbF(rgba[0], rgba[1], rgba[2], rgba[3]);
        break;
    }
  }
  return color;
}

bool editColorValue(smtk::view::PhraseModelPtr model, smtk::resource::ComponentPtr component,
  const resource::FloatList& val)
{
  if (!component)
    return false;
  // Lets try to get the local operation manager
  smtk::operation::ManagerPtr opManager;
  if (model != nullptr)
  {
    opManager = model->operationManager();
  }
  smtk::operation::SetProperty::Ptr op;
  if (opManager)
  {
    op = opManager->create<smtk::operation::SetProperty>();
  }
  else
  {
    op = smtk::operation::SetProperty::create();
  }
  if (op->parameters()->associate(component))
  {
    op->parameters()->findString("name")->setValue("color");
    op->parameters()->findDouble("float value")->setValues(val.begin(), val.end());
    auto res = op->operate();
    if (res->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      return true;
    }
  }
  return false;
}

void TypeAndColorBadge::action(const smtk::view::DescriptivePhrase* phrase) const
{
  if (phrase->phraseModel() == nullptr)
  {
    smtkWarningMacro(smtk::io::Logger::instance(), "Can not access phraseModel for editing color!");
    return;
  }
  std::string dialogInstructions =
    "Choose Color for " + phrase->title() + " (click Cancel to remove color)";
  // If the currentColor is invalid lets set it to opaque white
  QColor currentColor = getPhraseColor(phrase);
  if (!currentColor.isValid())
  {
    currentColor.setRed(255);
    currentColor.setGreen(255);
    currentColor.setBlue(255);
    currentColor.setAlpha(255);
  }

  QColor nextColor = QColorDialog::getColor(currentColor, nullptr, dialogInstructions.c_str(),
    QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);
  bool canceled = !nextColor.isValid();
  if (!canceled)
  {
    smtk::model::FloatList rgba{ nextColor.red() / 255.0, nextColor.green() / 255.0,
      nextColor.blue() / 255.0, nextColor.alpha() / 255.0 };
    editColorValue(phrase->phraseModel(), phrase->relatedComponent(), rgba);
  }
}
}
}
}
