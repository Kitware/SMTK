//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_ResourcePhrase_h
#define smtk_view_ResourcePhrase_h

#include "smtk/view/DescriptivePhrase.h"

#include "smtk/resource/PropertyType.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace view
{

/**\brief Describe a resource component for user presentation.
  *
  */
class SMTKCORE_EXPORT ResourcePhrase : public DescriptivePhrase
{
public:
  smtkTypeMacro(ResourcePhrase);
  smtkSharedPtrCreateMacro(DescriptivePhrase);
  Ptr setup(const smtk::resource::ResourcePtr& rsrc, int mutability = 0,
    DescriptivePhrase::Ptr parent = DescriptivePhrasePtr());
  virtual ~ResourcePhrase();

  std::string title() override;
  bool isTitleMutable() const override;
  bool setTitle(const std::string& newTitle) override;
  std::string subtitle() override;

  smtk::resource::ResourcePtr relatedResource() const override;
  smtk::resource::ComponentPtr relatedComponent() const override { return nullptr; }
  smtk::resource::FloatList relatedColor() const override;
  bool isRelatedColorMutable() const override;
  bool setRelatedColor(const smtk::resource::FloatList& rgba) override;

  void setMutability(int whatsMutable);

protected:
  ResourcePhrase();

  smtk::resource::ResourcePtr m_resource;
  int m_mutability;
};

typedef std::vector<std::shared_ptr<ResourcePhrase> > ResourcePhraseArray;

} // view namespace
} // smtk namespace

#endif
