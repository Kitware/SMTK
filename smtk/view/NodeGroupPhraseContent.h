//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_view_NodeGroupPhraseContent_h
#define smtk_view_NodeGroupPhraseContent_h

#include "smtk/view/PhraseContent.h"

#include "smtk/model/Entity.h"

namespace smtk
{
namespace view
{

/**\brief A phrase whose children are all components of a given type-name.
  *
  * Subphrase generators create instances of this class when they wish to
  * aggregate nodes of a given type.
  *
  * Instances of this class generate their titles based on the number of
  * child items they have and the singular/plural form of the type name
  * you provide.
  * If no singular/plural type-names are provided, the childType() will be
  * truncated to just text beyond the last double-colon and downcased.
  * For example, if a NodeGroupPhraseContent has a childType() of
  * "smtk::foo::Bar" and 3 children, its title will be "3 bars". If a single
  * child exists, the title will be "1 bar."
  */
class SMTKCORE_EXPORT NodeGroupPhraseContent : public smtk::view::PhraseContent
{
public:
  smtkTypeMacro(smtk::view::NodeGroupPhraseContent);
  smtkSharedPtrCreateMacro(PhraseContent);

  /// Return the class type-name of the type of component that should be listed in our subphrases.
  std::string childType() const { return m_childType; }

  /**\brief Initialize this list with an iterable container of subphrases.
    *
    * This templated method is provided so that arrays of shared-pointers
    * to **subclasses** of DescriptivePhrase are also accepted.
    * It is mostly intended for use by SubphraseGenerator and its subclasses.
    */
  Ptr setup(
    smtk::view::DescriptivePhrasePtr parent,
    const std::string& childType,
    const std::string& singular = std::string(),
    const std::string& plural = std::string(),
    int mutability = 0);

  /**\brief Create a descriptive phrase whose content is a NodeGroupPhraseContent instance.
    *
    * This creates a phrase that a subphrase generator can expand on
    * by calling `parent->parent()->relatedResource()->filter(childType)`.
    */
  static smtk::view::DescriptivePhrasePtr createPhrase(
    smtk::view::DescriptivePhrasePtr parent,
    const std::string& childType,
    const std::string& singular = std::string(),
    const std::string& plural = std::string(),
    int mutability = 0,
    const smtk::view::DescriptivePhrases& children = smtk::view::DescriptivePhrases());

  ~NodeGroupPhraseContent() override = default;

  bool displayable(ContentType attr) const override;
  bool editable(ContentType attr) const override
  {
    return (m_mutability & static_cast<int>(attr)) != 0;
  }

  std::string stringValue(ContentType attr) const override;
  int flagValue(ContentType attr) const override;

  bool editStringValue(ContentType attr, const std::string& val) override;
  bool editFlagValue(ContentType attr, int val) override;

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
  template<typename T>
  T relatedComponentsAs() const
  {
    T result;
    auto comps = this->relatedComponents();
    for (auto comp : comps)
    {
      result.insert(
        result.end(), std::dynamic_pointer_cast<typename T::value_type::element_type>(comp));
    }
    return result;
  }

  void setMutability(int whatsMutable);

  /**\brief This method is for subphrase generators that wish to customize the list title.
    *
    * This method is only intended for setup and ignores the mutability of the title,
    * unlike setTitle(). Generally, this method should only be called when the title
    * is **not** mutable.
    */
  void setCustomTitle(const std::string& title) { m_title = title; }

  bool operator==(const PhraseContent& other) const override;

protected:
  NodeGroupPhraseContent();

  void defaultSingularPlural();
  std::string generateTitle() const;

  int m_mutability{ 0 };
  mutable std::string m_childType;
  mutable std::string m_singular;
  mutable std::string m_plural;
  mutable std::string m_title;
};

} // namespace view
} // namespace smtk

#endif
