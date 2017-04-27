//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smoab_model_BoundaryConditions_h
#define __smoab_model_BoundaryConditions_h

#include "FaceSets.h"

namespace smoab
{
namespace model
{

template <typename T>
class BoundaryConditions
{
public:
  void add(T value, int faceId)
  {
    if (faceId >= static_cast<int>(this->Conditions.size()))
    {
      this->Conditions.resize(faceId + 1, -1);
    }
    this->Conditions[faceId] = value;
  }

  template <typename U>
  void fill(U* bcArray)
  {
    bcArray->SetNumberOfValues(this->Conditions.size());
    T* raw = static_cast<T*>(bcArray->GetVoidPointer(0));
    std::copy(this->Conditions.begin(), this->Conditions.end(), raw);
  }

private:
  std::vector<T> Conditions;
};

template <typename T>
smoab::model::BoundaryConditions<T> extractBCS(const smoab::model::FaceCellSets& faces,
  const smoab::CellSets& boundaries, const std::vector<T>& boundaryValues)
{
  typedef typename smoab::CellSets::const_iterator cs_iterator;
  typedef typename smoab::model::FaceCellSets::const_iterator fc_iterator;
  typedef typename std::vector<T>::const_iterator bv_iterator;
  smoab::model::BoundaryConditions<T> bcs;

  bv_iterator values = boundaryValues.begin();
  for (cs_iterator i = boundaries.begin(); i != boundaries.end(); ++i, ++values)
  {
    //the face cell entity id should match a boundary id
    for (fc_iterator j = faces.begin(); j != faces.end(); ++j)
    {
      if (i->entity() == j->entity())
      {
        bcs.add(*values, j->faceId());
      }
    }
  }
  return bcs;
}
}
} //namespace smoab::model

#endif // __smoab_model_BoundaryConditions_h
