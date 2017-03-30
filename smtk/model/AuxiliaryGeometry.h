//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_AuxiliaryGeometry_h
#define __smtk_model_AuxiliaryGeometry_h

#include "smtk/model/EntityRef.h"

namespace smtk
{
namespace model
{

/**\brief An EntityRef subclass for representing unmodeled geometry to be included in a scene.
  *
  * Auxiliary geometry entities
  * + are scene, preview, or construction geometry that is not a CAD (B-Rep) model;
  * + may hold **either** a tessellation or a reference to external geometry
  *   (by defining a "url" string property);
  * + may be used directly in a model **or** be managed external to a model (by an
  *   application using SMTK) and included in the model only via Instance entities.
  *
  * \warning There is nothing in SMTK that forces an AuxiliaryGeometry entity
  *          to have a "url" or a tessellation but not both.
  *          If both exist, SMTK's behavior is not well-defined.
  *
  * Note that because auxiliary geometry may not have a tessellation (in the case
  * where it has a "url" string property), the geometry will not be available
  * in SMTK; only when using ParaView extensions will it be loaded.
  * When SMTK is build with ParaView extensions enabled, then any "url"-based
  * auxiliary geometry items will be loaded and added to the vtkMultiBlockDataSet
  * for the model. Thus, it will be available on the ParaView server side, but not
  * in the ParaView client.
  */
class SMTKCORE_EXPORT AuxiliaryGeometry : public EntityRef
{
public:
  SMTK_ENTITYREF_CLASS(AuxiliaryGeometry, EntityRef, isAuxiliaryGeometry);

  bool hasUrl() const;
  std::string url() const;
  void setUrl(const std::string& url);
};

typedef std::vector<AuxiliaryGeometry> AuxiliaryGeometries;

} // namespace model
} // namespace smtk

#endif // __smtk_model_AuxiliaryGeometry_h
