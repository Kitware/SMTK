//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/io/vtkMedHelper.h"
#include "vtkObjectFactory.h"

HdfNode* HdfNode::findChild(const std::string& name)
{
  for (auto& i : children)
  {
    if (i->name == name)
    {
      return i;
    }
  }
  return nullptr;
}

static herr_t buildTree(hid_t locId, const char* name, const H5L_info_t* /*info*/, void* opData)
{
  const std::string strName = name;
  H5O_info_t infoBuffer;
  H5Oget_info_by_name(locId, name, &infoBuffer, H5P_DEFAULT);

  // Create a node in the tree for this, doubly link it
  auto* parentNode = static_cast<HdfNode*>(opData);
  auto* currNode = new HdfNode{ locId, name, parentNode->path + name + '/' };
  parentNode->children.push_back(currNode);
  currNode->parent = parentNode;

  // Test what the link is pointing too
  if (infoBuffer.type == H5O_TYPE_GROUP)
  {
    H5Literate_by_name(
      locId, name, H5_INDEX_NAME, H5_ITER_NATIVE, nullptr, buildTree, currNode, H5P_DEFAULT);
  }

  return 0;
}

extern HdfNode* rootBuildTree(hid_t rootId)
{
  auto* rootNode = new HdfNode{ rootId, ".", "./" };
  H5Literate(rootId, H5_INDEX_NAME, H5_ITER_NATIVE, nullptr, buildTree, rootNode);
  return rootNode;
}

void deleteTree(HdfNode* node)
{
  for (auto& i : node->children)
  {
    deleteTree(i);
  }
  delete node;
}
