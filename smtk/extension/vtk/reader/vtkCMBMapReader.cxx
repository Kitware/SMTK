//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBMapReader.h"

#include "smtk/extension/vtk/meshing/vtkCMBPrepareForTriangleMesher.h"
#include "smtk/extension/vtk/reader/vtkCMBReaderHelperFunctions.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkErrorCode.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkLine.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <vtksys/SystemTools.hxx>
#include <set>

//Turns on the old map file cell data
//very inefficient, but easy to debug
#define WRITE_DEBUG_CELLDATA 1

using namespace ReaderHelperFunctions;

vtkStandardNewMacro(vtkCMBMapReader);

//-----------------------------------------------------------------------------
vtkCMBMapReader::vtkCMBMapReader()
{
  this->FileName = NULL;
  this->NumArcs = 0;
  this->SetNumberOfInputPorts(0);
  this->ArcIds = vtkIntArray::New();
}

//-----------------------------------------------------------------------------
vtkCMBMapReader::~vtkCMBMapReader()
{
  this->SetFileName(0);
  if(this->ArcIds)
    {
    this->ArcIds->Delete();
    this->ArcIds = NULL;
    }
}
//-----------------------------------------------------------------------------
//This is a reader for the SMS Map file for help on the format of the file go to
//http://www.ems-i.com/smshelp/SMS-Help.htm#File_Formats/SMS_Project_Files.htm
  int vtkCMBMapReader::RequestData(
      vtkInformation *vtkNotUsed(request),
      vtkInformationVector **vtkNotUsed(inputVector),
      vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));

  std::string fileNameStr =this->FileName;

  struct stat fs;
  if (stat(fileNameStr.c_str(), &fs) != 0)
    {
    vtkErrorMacro("Unable to open file: "<< fileNameStr);
    this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
    return 0;
    }

  //Open File
  ifstream file(fileNameStr.c_str(), ios::in | ios::binary );
  if(!file)
    {
    vtkErrorMacro("Unable to open file: " << fileNameStr);
    return 1;
    }
  //Read File
    {
    this->UpdateProgress(0);
    //Get number of bytes in the file
    file.seekg(0,ios::end);
    double numBytes = file.tellg();
    file.clear();
    file.seekg(0,ios::beg);

    /*Setup variables*/
    std::stringstream line( std::stringstream::in | std::stringstream::out);
    std::string card = "";

    vtkPoints *points = vtkPoints::New();
    vtkCellArray *verts = vtkCellArray::New();
    vtkCellArray *lineSegments = vtkCellArray::New();

#if WRITE_DEBUG_CELLDATA
    vtkIntArray *cellDataScalars = vtkIntArray::New();
    cellDataScalars->SetNumberOfComponents(1);
    cellDataScalars->SetName(vtksys::SystemTools::
      GetFilenameWithoutExtension(this->FileName).c_str());
    vtkFloatArray *pointScalars = vtkFloatArray::New();

    std::map<vtkIdType, std::vector<vtkIdType> > arcId2vtkLineIds; // maps arcid's to
                                           // the VTK_LINES that create that
                                           // arc
#endif

    std::map<vtkIdType, vtkIdType> arcId2ArcIndex; // maps the arcid to a number to
                                              // index into the arc associated
                                              // field data

    std::map<vtkIdType, vtkIdType> nodeId2vtkPointId;// maps the node id to the
                                              // position in the vtkPoint array
    vtkIdType currPointId = 0;
    this->NumArcs = 0;
    this->ArcIds->Reset();
    this->ArcIds->SetNumberOfComponents(1);

#if WRITE_DEBUG_CELLDATA
    vtkIdType numCellsRead = 0;

    vtkIntArray *cellDataArcIds = vtkIntArray::New();
    vtkIntArray *cellDataPoly1Ids = vtkIntArray::New();
    vtkIntArray *cellDataPoly2Ids = vtkIntArray::New();
    vtkIntArray *cellDataHoleIds = vtkIntArray::New();
    vtkIntArray *cellDataNode1Ids = vtkIntArray::New();
    vtkIntArray *cellDataNode2Ids = vtkIntArray::New();
    cellDataArcIds->SetNumberOfComponents(1);
    cellDataArcIds->SetName("ModelEdgeIds");
    cellDataPoly1Ids->SetNumberOfComponents(1);
    cellDataPoly1Ids->SetName("PolyId1");
    cellDataPoly2Ids->SetNumberOfComponents(1);
    cellDataPoly2Ids->SetName("PolyId2");
    cellDataHoleIds->SetNumberOfComponents(1);
    cellDataHoleIds->SetName("HoleId");
    cellDataNode1Ids->SetNumberOfComponents(1);
    cellDataNode1Ids->SetName("Node1");
    cellDataNode2Ids->SetNumberOfComponents(1);
    cellDataNode2Ids->SetName("Node2");
    pointScalars->SetNumberOfComponents(1);
#endif

    //Initialize the map interface to create a new map file
    vtkCMBPrepareForTriangleMesher* mapInterface = vtkCMBPrepareForTriangleMesher::New();
    mapInterface->SetPolyData(output);
    mapInterface->InitializeNewMapInfo();

    /*Read Header
      MAP                    # file type identifier
      BEGCOV                 # beginning of a new coverage
      COVNAME "name"         # coverage name
      COVATTS attstype       # coverage attributes type
      */
    while(card != "NODE" && card != "POINT")
      {
      //just skip over header for now
      readNextLine(file,line,card);
      this->UpdateProgress(static_cast<double>(file.tellg())/numBytes);
      }

    /*Read Nodes
      NODE                   # new node identifier
      XY x y                 # xy coordinates of node
      ID id                  # id of node
      END                    # end of data for node
      */
    //We are treating points like nodes right now
    while(card == "NODE" || card == "POINT")
      {
      float x=0,y=0,z=0;
      vtkIdType id;

      while(true)
        {
        readNextLine(file,line,card);
        if(card == "XY")
          {
          //Scan in coordinates
          line >> x >> y >> z;
          }
        else if(card == "ID")
          {
          line >> id;//Scan in ID's
          }
        else if(card == "END")
          {
          readNextLine(file,line,card);//Read the next line
          break;
          }
        }

      currPointId = points->InsertNextPoint(x, y, z);


      vtkIdType tup[1] = {currPointId};
      verts->InsertNextCell(1,tup);
      mapInterface->AddNode(id);
#if WRITE_DEBUG_CELLDATA
      pointScalars->InsertNextTuple1(1);
      cellDataScalars->InsertNextTuple1(1);
      cellDataArcIds->InsertNextTuple1(-1);
      cellDataPoly1Ids->InsertNextTuple1(-1);
      cellDataPoly2Ids->InsertNextTuple1(-1);
      cellDataHoleIds->InsertNextTuple1(-1);
      cellDataNode1Ids->InsertNextTuple1(id);
      cellDataNode2Ids->InsertNextTuple1(-1);
      numCellsRead++;
#endif
      nodeId2vtkPointId[id] = currPointId;
      this->UpdateProgress(static_cast<double>(file.tellg())/numBytes);
      }

    /*Read Arcs
      ARC                    # new arc identifier
      ID id                  # id of arc
      NODES id1 id2          # ids of beginning and ending nodes for arc
      ARCVERTICES n          # number of vertices between nodes for arc
      x1 y1                  # xy location of vertices
      x2 y2
      xn yn
      ARCBIAS Value        # bias value for meshing
      END                  # end of data for arc
      */

    // **************** Note for Special Case between MAP file and triangle ****
    // Keep a list of arcs that forms a loop by itself. This list will be useful
    // later to check whether an arc contained in a polygon is actually
    // an inner loop, but the MAP file does not label it as a HARCS, which
    // will confuse triangle's association of face mesh to edge mesh.
    // Basically, triangle thinks the type of arcs should be an inner loop for
    // meshing, but MAP file does not define it that way.
    std::set<vtkIdType> loopedArcs;
    while(card == "ARC")
      {
      vtkIdType id, nodeId1, nodeId2;
      float elevation;
      vtkIdType numArcVerts = 0;
      vtkIdType arcVerts[2];
      // The index that starts a list of the VTK_LINES belonging to
      // this arc in the vtkLineData
      vtkIdType cellArrayOffset = lineSegments->GetNumberOfCells()*3;
      vtkIdType cellArraySize = 3; // the minimum size of a cell array
                                   // each arc can have, if there are
                                   // only end nodes and no arcverts
      while(true)
        {
        readNextLine(file,line,card);

        if(card == "ID")
          {
          line >> id; //Scan in ID's
          this->NumArcs++;
          this->ArcIds->InsertNextValue(id);
          }
        else if(card == "ARCELEVATION")
          {
          line >> elevation;
          }
        else if(card == "NODES")
          {
          line >> nodeId1 >> nodeId2;
          arcVerts[0] = nodeId2vtkPointId[nodeId1];//setup for building the line
          if(nodeId1 == nodeId2)
            {
            loopedArcs.insert(id);
            }
          }
        else if(card == "ARCVERTICES")
          {
          line >> numArcVerts;
          // There are 2+numArcVerts nodes because there are two end nodes plus
          // the arc verticies. Therefore there are 1+numArcVerts lines.
          // Because this reader reads in only lines each cell will be represented by
          // 3 id's
          cellArraySize = (1+numArcVerts)*3;

          //build the line
          for(vtkIdType i = 0; i < numArcVerts; i++)
            {
            float x = 0, y = 0, z = 0;

            readNextLine(file,line);
            line >> x >> y >> z;

            currPointId = points->InsertNextPoint(x,y,z);

            arcVerts[1] = currPointId;
            lineSegments->InsertNextCell(2,arcVerts);
#if WRITE_DEBUG_CELLDATA
            pointScalars->InsertNextTuple1(2);
            //For lines the tupples are formated
            //(arc_vert_id, arc_id, polygon1_id, polygon2_id, hole_polygon_id,
            //node1_id,node2_id)
            cellDataScalars->InsertNextTuple1(2);
            cellDataArcIds->InsertNextTuple1(id);
            cellDataPoly1Ids->InsertNextTuple1(-1);
            cellDataPoly2Ids->InsertNextTuple1(-1);
            cellDataHoleIds->InsertNextTuple1(-1);
            cellDataNode1Ids->InsertNextTuple1(nodeId1);
            cellDataNode2Ids->InsertNextTuple1(nodeId2);

            arcId2vtkLineIds[id].push_back(numCellsRead++);
#endif
            //Put the arcVertId as p1 in the line for the next iteration
            arcVerts[0] = currPointId;
            //give the next iteration a new id
            }
          }
        else if(card == "END")
          {
          readNextLine(file,line,card);//Read the next line
          break;
          }
        }
      // Finish the last line
      arcVerts[1] = nodeId2vtkPointId[nodeId2];
      lineSegments->InsertNextCell(2,arcVerts);
#if WRITE_DEBUG_CELLDATA
      cellDataScalars->InsertNextTuple1(2);
      cellDataArcIds->InsertNextTuple1(id);
      cellDataPoly1Ids->InsertNextTuple1(-1);
      cellDataPoly2Ids->InsertNextTuple1(-1);
      cellDataHoleIds->InsertNextTuple1(-1);
      cellDataNode1Ids->InsertNextTuple1(nodeId1);
      cellDataNode2Ids->InsertNextTuple1(nodeId2);

      arcId2vtkLineIds[id].push_back(numCellsRead++);
#endif

      mapInterface->AddArc(cellArrayOffset,cellArraySize,id,-1,-1,nodeId1,nodeId2);

      arcId2ArcIndex[id] = this->NumArcs - 1;//map id to the num arcs read so far
      this->UpdateProgress(static_cast<double>(file.tellg())/numBytes);
      }

    /*Read Polygons
      POLYGON              # new polygon identifier
      ID id                # id of polygon
      ARCS n               # number of boundary arcs for polygon
      id1                  # ids of boundary arcs
      id2
      .
      idn
      HARCS n              # number of hole-arcs for polygon
      id1                  # ids of hole-arcs
      id2
      .
      idn
      END                  # end of data for polygon
      */
    while(card == "POLYGON")
      {
      vtkIdType numArcsInPoly,numHarcs,id;
      std::vector<vtkIdType> outerLoopIndexes; // store arc info for after we know id
      std::vector< std::vector<vtkIdType> > innerLoopsIndexes; // process hole arcs and store for
                                             // after we know the id
#if WRITE_DEBUG_CELLDATA
      //same as previous maps but the old map structure uses
      //loop ids
      std::vector<vtkIdType> outerLoopIds;
      std::vector< std::vector<vtkIdType> > innerLoopsIds;
#endif
      while(true)
        {
        readNextLine(file,line,card);

        if(card == "ARCS")
          {
          line >> numArcsInPoly;
          //There should only be 1 arcs card per polygon
          outerLoopIndexes.reserve(numArcsInPoly);
#if WRITE_DEBUG_CELLDATA
          outerLoopIds.reserve(numArcsInPoly);
#endif
          for(vtkIdType i = 0; i < numArcsInPoly; i++)
            {
            vtkIdType arcId;
            readNextLine(file,line);
            line >> arcId;

// see above "Note for Special Case between MAP file and triangle"
            if(numArcsInPoly > 1 &&
             loopedArcs.count(arcId) != 0)
              {
              innerLoopsIndexes.push_back(std::vector<vtkIdType>());
              innerLoopsIndexes.back().push_back(arcId2ArcIndex[arcId]);
              }
            else
              {
              outerLoopIndexes.push_back(arcId2ArcIndex[arcId]);
#if WRITE_DEBUG_CELLDATA
              outerLoopIds.push_back(arcId);
#endif
              }
            }
          }
        if(card == "HARCS")
          {
          line >> numHarcs;
          //Create an inner loop vector for every
          //HARC card read
          innerLoopsIndexes.push_back(std::vector<vtkIdType>());
          innerLoopsIndexes.back().reserve(numHarcs);
#if WRITE_DEBUG_CELLDATA
          innerLoopsIds.push_back(std::vector<vtkIdType>());
          innerLoopsIds.back().reserve(numHarcs);
#endif
          for(vtkIdType i = 0; i < numHarcs; i++)
            {
            vtkIdType harcId;
            readNextLine(file,line);
            line >> harcId;
            innerLoopsIndexes.back().push_back(arcId2ArcIndex[harcId]);
#if WRITE_DEBUG_CELLDATA
             innerLoopsIds.back().push_back(harcId);
#endif
            }
          }
        if(card == "ID")
          {
          line >> id;
          }
        if(card == "END")
          {
          readNextLine(file,line,card);
          break;
          }
        this->UpdateProgress(static_cast<double>(file.tellg())/numBytes);
        }

      //we know know the polygon id
      //update the loop table with polygon information
      if( outerLoopIndexes.size() > 0 )
        {
        mapInterface->AddLoopWithArcs(id,true,outerLoopIndexes);
        }
      for(unsigned i = 0; i < innerLoopsIndexes.size(); i++)
        {
        mapInterface->AddLoopWithArcs(id,false,innerLoopsIndexes[i]);
        }

#if WRITE_DEBUG_CELLDATA
      //The loops have been calculated update
      //Now change the arc's field data to associate an arc with
      //a loop
      for(unsigned int i = 0; i < outerLoopIds.size(); i++)
        {
        vtkIdType arcId = outerLoopIds[i];
        std::vector<vtkIdType>::iterator iter;
        for(iter = arcId2vtkLineIds[arcId].begin();
            iter != arcId2vtkLineIds[arcId].end();
            iter++)
          {
          double val1 = cellDataPoly1Ids->GetTuple1((*iter));
          double val2 = cellDataPoly2Ids->GetTuple1((*iter));
          //Insert the polygon id into the lines
          if(val1 == -1)
            {
            val1 = id;
            cellDataPoly1Ids->SetTuple1((*iter),val1);
            }
          else if(val2== -1)
            {
            val2 = id;
            cellDataPoly2Ids->SetTuple1((*iter),val2);
            }
          //If there are 3 polygons that claim to be touching the arc something funky
          //is happening
          else
            {
            vtkErrorMacro("While Reading " << fileNameStr << "\n\
                Arc " << arcId << " had more than 2 Polygons attached to it.");
            }
          }
        }

      //we know know the id process harcs
      std::vector< std::vector<vtkIdType> >::iterator innerLoopIter;
      for(innerLoopIter = innerLoopsIds.begin(); innerLoopIter != innerLoopsIds.end(); innerLoopIter++)
        {
        std::vector<vtkIdType> innerLoop = (*innerLoopIter);
        std::vector<vtkIdType>::iterator harcIter;
        for(harcIter = innerLoop.begin(); harcIter != innerLoop.end(); harcIter++)
          {
          vtkIdType harcId = (*harcIter);
          std::vector<vtkIdType>::iterator iter;
          //Insert the hole polygon id into the lines
          for(iter = arcId2vtkLineIds[harcId].begin(); iter != arcId2vtkLineIds[harcId].end(); iter++)
            {
            double hval = cellDataHoleIds->GetTuple1((*iter));
            hval = id;
            cellDataHoleIds->SetTuple1((*iter),hval);
            }
          }
        }
#endif
      this->UpdateProgress(static_cast<double>(file.tellg())/numBytes);
      }

    //Any data after the polygons (such as drawing objects) are not read or used

    //Create the vtkPolyData
    output->SetPoints(points);
    points->FastDelete();
    output->SetVerts(verts);
    verts->FastDelete();
    output->SetLines(lineSegments);
    lineSegments->FastDelete();

#if WRITE_DEBUG_CELLDATA
    pointScalars->SetName("Point Scalar Data");
    output->GetPointData()->SetScalars(pointScalars);
    pointScalars->FastDelete();
    output->GetCellData()->SetScalars(cellDataScalars);
    cellDataScalars->FastDelete();
    output->GetCellData()->AddArray(cellDataArcIds);
    output->GetCellData()->AddArray(cellDataPoly1Ids);
    output->GetCellData()->AddArray(cellDataPoly2Ids);
    output->GetCellData()->AddArray(cellDataHoleIds);
    output->GetCellData()->AddArray(cellDataNode1Ids);
    output->GetCellData()->AddArray(cellDataNode2Ids);

    cellDataArcIds->FastDelete();
    cellDataPoly1Ids->FastDelete();
    cellDataPoly2Ids->FastDelete();
    cellDataHoleIds->FastDelete();
    cellDataNode1Ids->FastDelete();
    cellDataNode2Ids->FastDelete();
#endif

    mapInterface->FinalizeNewMapInfo();

    vtkSmartPointer<vtkStringArray> filenameFD =
      vtkSmartPointer<vtkStringArray>::New();
    filenameFD->SetName("FileName");
    filenameFD->InsertNextValue(this->FileName);
    output->GetFieldData()->AddArray( filenameFD );

    mapInterface->Delete();

    //Clean up
    file.close();
    }

  return 1;
}


//-----------------------------------------------------------------------------
void vtkCMBMapReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
    << (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "NumArcs: " << this->NumArcs << "\n";
  if(this->ArcIds)
    {
    os << indent << "ArcIds: " << this->ArcIds << "\n";
    }
  else
    {
    os << indent << "ArcIds: (NULL)\n";
    }
}


//----------------------------------------------------------------------------
  int vtkCMBMapReader::RequestInformation(
      vtkInformation *vtkNotUsed(request),
      vtkInformationVector **vtkNotUsed(inputVector),
      vtkInformationVector *vtkNotUsed(outputVector))
{
  if (!this->FileName)
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }

  return 1;
}
