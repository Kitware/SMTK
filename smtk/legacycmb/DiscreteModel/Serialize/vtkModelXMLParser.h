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
// .NAME vtkModelXMLParser parses XML files.
// .SECTION Description
// This is a subclass of vtkXMLParser that constructs a DOM representation
// of parsed XML using vtkXMLElement.
#ifndef __vtkModelXMLParser_h
#define __vtkModelXMLParser_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkXMLParser.h"
#include "cmbSystemConfig.h"

class vtkXMLElement;

class VTKDISCRETEMODEL_EXPORT vtkModelXMLParser : public vtkXMLParser
{
public:
  vtkTypeMacro(vtkModelXMLParser,vtkXMLParser);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkModelXMLParser* New();

  // Description:
  // Write the parsed XML into the output stream.
  void PrintXML(ostream& os);

  // Description:
  // Get the root element from the XML document.
  vtkXMLElement* GetRootElement();

  // Description:
  // Get/Set the file from which to read the configuration.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkModelXMLParser();
  ~vtkModelXMLParser();

  void StartElement(const char* name, const char** atts);
  void EndElement(const char* name);
  void CharacterDataHandler(const char* data, int length);

  void AddElement(vtkXMLElement* element);
  void PushOpenElement(vtkXMLElement* element);
  vtkXMLElement* PopOpenElement();

  // The root XML element.
  vtkXMLElement* RootElement;

  // The stack of elements currently being parsed.
  vtkXMLElement** OpenElements;
  unsigned int NumberOfOpenElements;
  unsigned int OpenElementsSize;

  // Counter to assign unique element ids to those that don't have any.
  unsigned int ElementIdIndex;

  // Called by Parse() to read the stream and call ParseBuffer.  Can
  // be replaced by subclasses to change how input is read.
  virtual int ParseXML();

private:
  vtkModelXMLParser(const vtkModelXMLParser&);  // Not implemented.
  void operator=(const vtkModelXMLParser&);  // Not implemented.
};

#endif
