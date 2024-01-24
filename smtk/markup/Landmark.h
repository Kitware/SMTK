//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Landmark_h
#define smtk_markup_Landmark_h

#include "smtk/markup/SpatialData.h"

namespace smtk
{
namespace markup
{

/**\brief A spatial landmark.
  *
  * A landmark is a reference to a location and/or coordinate frame that
  * has some significance to the data being modeled.
  *
  * A landmark may or may not have a subject; if it does,
  * the landmark's location and/or direction(s) should be
  * treated as relative to the subject.
  * This way, if a transform is applied to its subject, the
  * landmark will also be transformed.
  */
class SMTKMARKUP_EXPORT Landmark : public smtk::markup::SpatialData
{
public:
  smtkTypeMacro(smtk::markup::Landmark);
  smtkSuperclassMacro(smtk::markup::SpatialData);

  template<typename... Args>
  Landmark(Args&&... args)
    : Superclass(std::forward<Args>(args)...)
  {
  }

  ~Landmark() override;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Landmark_h
