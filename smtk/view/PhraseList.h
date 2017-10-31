//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_PhraseList_h
#define smtk_view_PhraseList_h

#include "smtk/view/DescriptivePhrase.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityTypeBits.h"

namespace smtk
{
namespace view
{

/**\brief Describe a list of child phrases to the user.
  *
  * Subphrase generators create instances of this class when a parent phrase
  * has more than one child, or when a list of phrases should be grouped
  * together because they have some commonality.
  *
  * Instances of this class generate their own titles by default
  * based on the number and type of their children.
  * For example, if a PhraseList has 3 children and they are all
  * SMTK model faces, its default title will be "3 faces".
  */
class SMTKCORE_EXPORT PhraseList : public DescriptivePhrase
{
public:
  smtkTypeMacro(PhraseList);
  smtkSharedPtrCreateMacro(DescriptivePhrase);

  /**\brief Initialize this list with an iterable container of subphrases.
    *
    * This templated method is provided so that arrays of shared-pointers
    * to **subclasses** of DescriptivePhrase are also accepted.
    * It is mostly intended for use by SubphraseGenerator and its subclasses.
    */
  template <typename T>
  Ptr setup(const T& entities, DescriptivePhrase::Ptr parent = DescriptivePhrasePtr(),
    DescriptivePhraseType listType = DescriptivePhraseType::LIST);

  virtual ~PhraseList() {}

  std::string title() override;
  std::string subtitle() override;

  /// A convenience method for obtaining related components for each of the phrases in the list.
  smtk::resource::ComponentArray relatedComponents() const;
  /** \brief A convenience method for obtaining related components in a user-specified container.
    *
    * This method dynamically casts each phrase's relatedComponent() to the given container's
    * type and back-inserts it into a new container of the templated type.
    * Use this method when you know the components of interest in the array are a given
    * subclass of smtk::resource::Component (e.g., smtk::model::Entity) by passing in
    * a container of that type (e.g., relatedComponentsAs<smtk::model::EntityArray>()).
    */
  template <typename T>
  T relatedComponentsAs() const
  {
    T result;
    auto comps = this->relatedComponents();
    for (auto comp : comps)
    {
      result.insert(result.end(), dynamic_pointer_cast<typename T::value_type::element_type>(comp));
    }
    return result;
  }

  /**\brief Allow subphrase generators to set summary information for model components in the list.
    *
    * If an instance of PhraseList contains model entities, the SubphraseGenerator
    * subclass (or other code) which populates the list of components should call this
    * method.
    *
    * The flags are used to generate the title summary in situations
    * where setCustomTitle() is not used.
    */
  virtual void setModelFlags(smtk::model::BitFlags commonFlags, smtk::model::BitFlags unionFlags);

  bool isRelatedColorMutable() const override;
  smtk::resource::FloatList relatedColor() const override;

  /**\brief This method is for subphrase generators that wish to customize the list title.
    *
    * This method is only intended for setup and ignores the mutability of the title,
    * unlike setTitle(). Generally, this method should only be called when the title
    * is **not** mutable.
    */
  void setCustomTitle(const std::string& title) { m_title = title; }

protected:
  PhraseList();

  void generateTitle();

  smtk::model::BitFlags m_commonFlags;
  smtk::model::BitFlags m_unionFlags;
  std::string m_title;
};

} // view namespace
} // smtk namespace

#endif
