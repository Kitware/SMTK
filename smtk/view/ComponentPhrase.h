//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_ComponentPhrase_h
#define smtk_view_ComponentPhrase_h

#include "smtk/view/DescriptivePhrase.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/PropertyType.h"

namespace smtk
{
namespace view
{

/**\brief Describe a resource component for user presentation.
  *
  */
class SMTKCORE_EXPORT ComponentPhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(ComponentPhrase);
  smtkSharedPtrCreateMacro(DescriptivePhrase);
  Ptr setup(const smtk::resource::ComponentPtr& component, int mutability = 0,
    DescriptivePhrase::Ptr parent = DescriptivePhrasePtr());
  virtual ~ComponentPhrase();

  std::string title() override;
  bool isTitleMutable() const override;
  bool setTitle(const std::string& newTitle) override;
  std::string subtitle() override;

  smtk::resource::ResourcePtr relatedResource() const override;
  smtk::resource::ComponentPtr relatedComponent() const override;
  smtk::resource::FloatList relatedColor() const override;
  bool isRelatedColorMutable() const override;
  bool setRelatedColor(const smtk::resource::FloatList& rgba) override;

  void setMutability(int whatsMutable);

protected:
  ComponentPhrase();

  smtk::resource::ComponentPtr m_component;
  int m_mutability;
};

} // view namespace
} // smtk namespace

#endif
