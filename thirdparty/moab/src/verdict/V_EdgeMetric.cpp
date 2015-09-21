/*=========================================================================

  Module:    $RCSfile: V_EdgeMetric.cpp,v $

  Copyright (c) 2006 Sandia Corporation.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/


/*
 *
 * V_EdgeMetric.cpp contains quality calcultions for edges
 *
 * This file is part of VERDICT
 *
 */

#define VERDICT_EXPORTS

#include "moab/verdict.h"
#include <math.h>

/*! 
  length of and edge
  length is calculated by taking the distance between the end nodes
 */
C_FUNC_DEF double v_edge_length( int /*num_nodes*/, double coordinates[][3] )
{

  double x = coordinates[1][0] - coordinates[0][0];
  double y = coordinates[1][1] - coordinates[0][1];
  double z = coordinates[1][2] - coordinates[0][2];
  return (double)( sqrt (x*x + y*y + z*z) );
}

/*!
  
  higher order function for calculating multiple metrics at once.

  for an edge, there is only one metric, edge length.
*/

C_FUNC_DEF void edge_quality( int num_nodes, double coordinates[][3], 
    unsigned int metrics_request_flag, struct EdgeMetricVals *metric_vals )
{
  if(metrics_request_flag & V_EDGE_LENGTH)
    metric_vals->length = v_edge_length(num_nodes, coordinates);
}


