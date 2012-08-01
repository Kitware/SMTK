//=============================================================================
//   This file is part of VTKEdge. See vtkedge.org for more information.
//
//   Copyright (c) 2010 Kitware, Inc.
//
//   VTKEdge may be used under the terms of the BSD License
//   Please see the file Copyright.txt in the root directory of
//   VTKEdge for further information.
//
//   Alternatively, you may see: 
//
//   http://www.vtkedge.org/vtkedge/project/license.html
//
//
//   For custom extensions, consulting services, or training for
//   this or any other Kitware supported open source project, please
//   contact Kitware at sales@kitware.com.
//
//
//=============================================================================
//
//  Copyright (c) CREATIS (Centre de Recherche et d'Applications en Traitement de
//  l'Image). All rights reserved. See Doc/License.txt or
//  http://www.creatis.insa-lyon.fr/Public/Gdcm/License.html for details.
//
//     This software is distributed WITHOUT ANY WARRANTY; without even
//     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//     PURPOSE.  See the above copyright notices for more information.
//
//=============================================================================

// .NAME vtkUUID - (Static) Class for generating/constructing UUIDs
// .SECTION Description
// This class provides ability to "generate" (using system/platform calls) a
// UUID, as well as to "construct" a UUID from MAC address, hostname, and
// random number generation.  The main Generate/Construct methods returns the
// uuid as binary, which can be converted to the 26 character string by calling
// ConvertBinaryUUIDToString.
// Note: The MAC address code comes from gdcm (see the copyright above).

#ifndef __vtkUUID_h
#define __vtkUUID_h

#include "vtkObject.h"

class VTK_EXPORT vtkUUID : public vtkObject
{
public:
  static vtkUUID* New();
  vtkTypeMacro(vtkUUID, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Generate a (binary) UUID using system/platform method call.  Note, it may
  // fail (return value -1), in which case you can call ConstructUUID.
  static int GenerateUUID(unsigned char uuid[16]);

  // Description:
  // Construct a (binary) UUID from MAC address (if can successfully acquire),
  // hostname, and random # generation.  This fn is guaranteed to create an
  // "uuid" (a semi-unique number) based on random # generation, regardless of
  // whether the MAC address and/or hostname is obtained.
  static void ConstructUUID(unsigned char uuid[16]);

  // Description:
  // Convert a (16-byte) binary UUID to its string form (in hexadecimal):
  // XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
  static void ConvertBinaryUUIDToString(unsigned char uuid[16],
    std::string &uuidString);

  // Description:
  // Get the 6-byte binary MAC address.  Returns -1 on failure.
  static int GetMACAddress(unsigned char addr[6]);

protected:
  vtkUUID() {};
  ~vtkUUID() {};

private:
  vtkUUID(const vtkUUID&); // Not implemented.
  void operator=(const vtkUUID&); // Not implemented.

  // the main (system specific) code for determining the MAC address
  static int GetMacAddrSys(unsigned char *addr);
};

#endif
