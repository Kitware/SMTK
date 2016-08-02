//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "ImportImage.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Model.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"
#include "smtk/model/SessionRef.h"

#include "smtk/bridge/polygon/ImportImage_xml.h"

#include "smtk/io/ExportJSON.h"
#include "cJSON.h"
#include <vtksys/SystemTools.hxx>

using namespace smtk::model;

namespace smtk {
  namespace bridge {

  namespace polygon {

ImportImage::ImportImage()
{
}

bool ImportImage::ableToOperate()
{
  if(!this->specification()->isValid())
    return false;
  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->associations();
  smtk::model::Model model(modelItem->value(0));
  if (!model.isValid())
    {
    return false;
    }

  std::string filename = this->specification()->findFile("filename")->value();
  if (filename.empty())
    return false;
  // support 2d models by vtkCMBGeometryReader
  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(filename);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  if (ext == ".vti" ||
      ext == ".dem" ||
      ext == ".tif" ||
      ext == ".tiff" )
    {
    return true;
    }

  return false;
}

OperatorResult ImportImage::operateInternal()
{
  smtk::attribute::ModelEntityItem::Ptr modelItem = this->specification()->associations();
  smtk::model::Model model(modelItem->value(0));
  std::string filename = this->specification()->findFile("filename")->value();
  if (!model.isValid() || filename.empty())
    {
    std::cerr << "A model is not set, or the file name is empty!\n";
    return this->createResult(OPERATION_FAILED);
    }

  // TODO:
  // base on what type of file is selected, we use different readers to read the file,
  // then save the data into smtk (as auxilary geometry type object ?)
  // For now, we just set the filename to a "image_url" property on the assoicated model.
  model.setStringProperty("image_url", filename);
  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);
  this->addEntityToResult(result, model, MODIFIED);
  return result;
}

    } // namespace polygon
  } // namespace bridge

} // namespace smtk

smtkImplementsModelOperator(
  SMTKPOLYGONSESSION_EXPORT,
  smtk::bridge::polygon::ImportImage,
  polygon_import_image,
  "import image",
  ImportImage_xml,
  smtk::bridge::polygon::Session);
