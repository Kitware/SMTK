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

typedef std::vector<AuxiliaryGeometry> AuxiliaryGeometries;

/**\brief An EntityRef subclass for representing unmodeled geometry to be included in a scene.
  *
  * Auxiliary geometry entities
  * + are scene, preview, or construction geometry that is not a CAD (B-Rep) model;
  * + may hold **one of** a tessellation or a reference to external geometry
  *   (by defining a "url" string property) or a set of embedded child auxiliary
  *   geometry entities (thus able to form hierarchies);
  * + may be used directly in a model **or** be managed external to a model (by an
  *   application using SMTK) and included in the model only via Instance entities.
  *
  * \warning There is nothing in SMTK that forces an AuxiliaryGeometry entity
  *          to have a "url" or a tessellation or neither but not both.
  *          If both exist, SMTK's behavior is not well-defined.
  *
  * Note that because auxiliary geometry may not have a tessellation (in the case
  * where it has a "url" string property or children), the geometry will not be available
  * in SMTK; only when using ParaView extensions will it be loaded.
  * When SMTK is built with VTK and ParaView extensions enabled, then any "url"-based
  * auxiliary geometry items will be loaded and added to the vtkMultiBlockDataSet
  * for the model. Thus, it will be available on the ParaView server side, but not
  * in the ParaView client.
  */
class SMTKCORE_EXPORT AuxiliaryGeometry : public EntityRef
{
public:
  SMTK_ENTITYREF_CLASS(AuxiliaryGeometry, EntityRef, isAuxiliaryGeometry);

  /**\brief Change this auxiliary geometry's parent to the given model.
    *
    * The previous parent must have had model \a m as its eventual parent;
    * this method is not intended to allow moving auxiliary geometry
    * from model to model.
    */
  bool reparent(const Model& m);
  /**\brief Change this auxiliary geometry's parent to the given auxiliary geometry \a a.
    *
    * The previous parent and \a a must both be or be owned by the same top-level model.
    */
  bool reparent(const AuxiliaryGeometry& a);

  bool hasURL() const;
  std::string url() const;
  void setURL(const std::string& url);

  bool isModified() const;
  void setIsModified(bool isModified);

  /// Return the children auxiliary geometries of this entity.
  AuxiliaryGeometries auxiliaryGeometries() const;
};

} // namespace model
} // namespace smtk

#endif // __smtk_model_AuxiliaryGeometry_h
