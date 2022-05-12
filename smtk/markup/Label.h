//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Label_h
#define smtk_markup_Label_h

#include "smtk/markup/Component.h"

namespace smtk
{
namespace markup
{

class SMTKMARKUP_EXPORT Label : public smtk::markup::Component
{
public:
  smtkTypeMacro(smtk::markup::Label);
  smtkSuperclassMacro(smtk::markup::Component);

  template<typename... Args>
  Label(Args&&... args)
    : smtk::markup::Component(std::forward<Args>(args)...)
  {
  }

  ~Label() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  /**\brief Return the container of components that this label references.
    */
  //@{
  ArcEndpointInterface<arcs::LabelsToSubjects, ConstArc, OutgoingArc> subjects() const;
  ArcEndpointInterface<arcs::LabelsToSubjects, NonConstArc, OutgoingArc> subjects();
  //@}

protected:
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Label_h
