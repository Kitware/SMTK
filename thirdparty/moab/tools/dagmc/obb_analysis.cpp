#include <iostream>
#include <fstream>
#include <sstream>

#include "dagmc_preproc.hpp"
#include "DagMC.hpp"
#include "moab/Core.hpp"
#include "moab/Interface.hpp"
#include "moab/OrientedBoxTreeTool.hpp"
#include "moab/CartVect.hpp"

#include "OrientedBox.hpp"

using namespace moab;

class TriCounter : public OrientedBoxTreeTool::Op { 


public:   
  int count;
  Interface* mbi;
  OrientedBoxTreeTool* tool;
  const CartVect& pt;

  TriCounter( Interface* mbi_p, OrientedBoxTreeTool *tool_p, const CartVect& p ):
    OrientedBoxTreeTool::Op(), count(0), mbi(mbi_p), tool(tool_p), pt(p)
  {}

  virtual ErrorCode visit( EntityHandle node,int , bool& descend ){
    OrientedBox box;
    ErrorCode rval = tool->box( node, box );
    
    descend = box.contained( pt, 1e-6 );

    return rval;
  }
  
  virtual ErrorCode leaf( EntityHandle node ){

    int numtris;
    ErrorCode rval = tool->get_moab_instance()->get_number_entities_by_type( node, MBTRI, numtris );
    count += numtris;
    return rval;
  }

};

ErrorCode obbvis_create( DagMC& dag, std::vector<int> &volumes, int grid, std::string& filename ){

  ErrorCode rval = MB_SUCCESS;
  OrientedBoxTreeTool& obbtool = *dag.obb_tree();
  
  CartVect min, max;
  EntityHandle vol = dag.entity_by_id( 3, volumes.front() );
  rval = dag.getobb( vol, min.array(), max.array() );
  CHECKERR(dag, rval);

  /* Compute an axis-aligned bounding box of all the requested volumes */
  for( std::vector<int>::iterator i = volumes.begin()+1; i!=volumes.end(); ++i ){
    CartVect i_min, i_max; 
    vol = dag.entity_by_id( 3, *i );
    rval = dag.getobb( vol, i_min.array(), i_max.array() );
    for( int j = 0; j < 3; ++j ){ 
      min[j] = std::min( min[j], i_min[j] );
      max[j] = std::max( max[j], i_max[j] );
    }
  }

  // These vectors could be repurposed to describe an OBB without changing the loops below
  CartVect center = (min+max) / 2.0;
  CartVect v1(max[0]-min[0]/2,0,0);
  CartVect v2(0,max[1]-min[1]/2,0);
  CartVect v3(0,0,max[2]-min[2]/2);
  
  /* Compute the vertices of the visualization grid.  Calculation points are at the center 
     of each cell in this grid, so make grid+1 vertices in each direction. */
  int numpoints = pow((double)(grid+1),3);
  double* pgrid = new double[ numpoints * 3 ];
  int idx = 0;


  for( int i = 0; i <= grid; ++i ){
    CartVect x = -v1 + ((v1*2.0) * (i/(double)grid));

    for( int j = 0; j <= grid; ++j ){
      CartVect y = -v2 + ((v2*2.0) * (j/(double)grid));

      for( int k = 0; k <= grid; ++k ){        
        CartVect z = -v3 + ((v3*2.0) * (k/(double)grid));
        
        CartVect p = center + x + y + z;
        for( int d = 0; d<3; ++d ){ pgrid[idx++] = p[d]; }
      }
    }
  }

  /* Create a new MOAB to use for output, and build the vertex grid */
  Core mb2;
  Range r;
  rval = mb2.create_vertices( pgrid, numpoints, r);
  CHECKERR( mb2, rval );
  
  Tag lttag;
  rval = mb2.tag_get_handle( "LEAFTRIS", sizeof(int), MB_TYPE_INTEGER, lttag,
                             MB_TAG_EXCL|MB_TAG_CREAT|MB_TAG_BYTES|MB_TAG_DENSE, 0);
  CHECKERR( mb2, rval );

  int row = grid + 1;
  int side = row * row;
  EntityHandle connect[8];
  EntityHandle hex;

  // offset from grid corner to grid center 
  CartVect grid_hex_center_offset = (v1+v2+v3) * 2 * (1.0 / grid);


  // collect all the surfaces from the requested volumes to iterate over --
  // this prevents checking a shared surface more than once.
  Range surfs;
  for(  std::vector<int>::iterator it = volumes.begin(); it!=volumes.end(); ++it ){
    
    vol = dag.entity_by_id(3,*it);
    Range it_surfs;
    rval = dag.moab_instance()->get_child_meshsets( vol, it_surfs );
    CHECKERR(dag,rval);
    surfs.merge(it_surfs);

  }
  std::cout << "visualizing " << surfs.size() << " surfaces." << std::endl;

  /* Build hexes for any point with 1 or more leaftris */
  for( int i = 0; i < grid; ++i ){
    for( int j = 0; j < grid; ++j ){
      for( int k = 0; k < grid; ++k ){

        idx = (side * k) + (row * j) + i;
	assert( idx + 1 + row + side > numpoints - 1 );

        CartVect loc = CartVect((pgrid+(idx*3)) ) + grid_hex_center_offset ;
        TriCounter tc(dag.moab_instance(), &obbtool, loc );

	for( Range::iterator it = surfs.begin(); it!=surfs.end(); ++it ){
	  
	  EntityHandle surf_tree;
	  rval = dag.get_root( *it, surf_tree );
	  CHECKERR(dag,rval);

	  rval = obbtool.preorder_traverse( surf_tree, tc );
	  CHECKERR(dag,rval);

	}

	if( tc.count == 0 ) continue; 

        connect[0] = r[ idx ];
        connect[1] = r[ idx + 1       ];
        connect[2] = r[ idx + 1 + row ];
        connect[3] = r[ idx +     row ];
        connect[4] = r[ idx +           side ];
        connect[5] = r[ idx + 1 +       side ];
        connect[6] = r[ idx + 1 + row + side ];
        connect[7] = r[ idx +     row + side ];

        rval = mb2.create_element( MBHEX, connect, 8, hex );
        CHECKERR(mb2,rval);

        rval = mb2.tag_set_data( lttag, &hex, 1, &(tc.count) );
        CHECKERR(mb2,rval);

      }
    }
  }

  if( verbose ){ std::cout << "Writing " << filename << std::endl; }
  rval = mb2.write_file( filename.c_str() );
  CHECKERR(mb2,rval);

  return rval;
}

// stats code borrowed from OrientedBoxTreeTool.cpp
static inline double std_dev( double sqr, double sum, double count )
{
  sum /= count;
  sqr /= count;
  return sqrt( sqr - sum*sum );
}

class TriStats : public OrientedBoxTreeTool::Op { 


public:   
  unsigned min, max, sum, leaves;
  double sqr;

  const static unsigned ten_buckets_max = 5;
  unsigned ten_buckets[ ten_buckets_max ];
  double ten_buckets_vol[ ten_buckets_max ];

  Interface* mbi;
  OrientedBoxTreeTool* tool;

  double tot_vol; 
  
  TriStats( Interface* mbi_p, OrientedBoxTreeTool *tool_p, EntityHandle root):
    OrientedBoxTreeTool::Op(), sum(0), leaves(0), sqr(0), mbi(mbi_p), tool(tool_p)
  {
    min = std::numeric_limits<unsigned>::max();
    max = std::numeric_limits<unsigned>::min();
    
    for( unsigned i = 0; i < ten_buckets_max; ++i ){ 
      ten_buckets[i] = 0; 
      ten_buckets_vol[i] = 0.;
    }

    OrientedBox box;
    ErrorCode rval = tool->box( root, box );
    CHECKERR( mbi, rval );
    tot_vol = box.volume();

  }

  virtual ErrorCode visit( EntityHandle ,int , bool& descend ){
    
    descend = true;
    return MB_SUCCESS;

  }
  
  virtual ErrorCode leaf( EntityHandle node ){

     Range tris;
     ErrorCode rval = tool->get_moab_instance()->get_entities_by_type( node, MBTRI, tris );
     unsigned count = tris.size();

     sum += count;
     sqr += (count * count);
     if( min > count ) min = count;
     if( max < count ) max = count;
     
     for( unsigned i = 0; i < ten_buckets_max; ++i ){
       if( count > std::pow((double)10,(int)(i+1)) ){ 
	 ten_buckets[i] += 1; 
	 OrientedBox box;
	 rval = tool->box( node, box );
	 CHECKERR( mbi, rval );
	 ten_buckets_vol[i] += box.volume();
       } 
     }

     leaves++;

     return rval;
  }

  std::string commafy( int num ){
    std::stringstream str;
    str << num;
    std::string s = str.str();
    
    int n = s.size();
    for( int i = n-3; i >= 1; i -= 3 ){
      s.insert(i, 1, ',');
      n++;
    }

    return s;

  }
  
  void write_results( std::ostream& out ){

    out << commafy(sum) << " triangles in " << commafy(leaves) << " leaves." << std::endl;

    double avg = sum / (double)leaves; 
    double stddev = std_dev( sqr, sum, leaves );

    out << "Tris per leaf: Min " << min << ", Max " << max << ", avg " << avg << ", stddev " << stddev << std::endl;

    for (unsigned i = 0; i < ten_buckets_max; ++i ){
      if( ten_buckets[i] ){
        out << "Leaves exceeding " << std::pow((double)10,(int)(i+1)) << " triangles: " << ten_buckets[i];
	
	double frac_total_vol =  ten_buckets_vol[i]/tot_vol;
	double avg_ftv = frac_total_vol / ten_buckets[i];

	out << " (avg " << avg_ftv * 100.0 << "% of OBB volume)" << std::endl;
      }
    }
  }

};

static std::string make_property_string( DagMC& dag, EntityHandle eh, std::vector<std::string> &properties )
{
  ErrorCode ret;
  std::string propstring;
  for( std::vector<std::string>::iterator p = properties.begin();
    p != properties.end(); ++p )
  {
    if( dag.has_prop( eh, *p ) ){
      std::vector< std::string> vals;
      ret = dag.prop_values( eh, *p, vals );
      CHECKERR(dag,ret);
      propstring += *p;
      if( vals.size() == 1 ){
        propstring += "=";
        propstring += vals[0];
      }
      else if( vals.size() > 1 ){
        // this property has multiple values, list within brackets
        propstring += "=[";
        for( std::vector<std::string>::iterator i = vals.begin();
             i != vals.end(); ++i )
        {
            propstring += *i;
            propstring += ",";
        }
        // replace the last trailing comma with a close braket
        propstring[ propstring.length()-1 ] = ']';
      }
      propstring += ", ";
    }
  }
  if( propstring.length() ){
    propstring.resize( propstring.length() - 2 ); // drop trailing comma
  }
  return propstring;
}

ErrorCode obbstat_write( DagMC& dag, std::vector<int> &volumes, 
                         std::vector<std::string> &properties, std::ostream& out ){

  ErrorCode ret = MB_SUCCESS;
  OrientedBoxTreeTool& obbtool = *dag.obb_tree();

  // can assume that volume numbers are valid.
  for( std::vector<int>::iterator i = volumes.begin(); i!=volumes.end(); ++i){
    EntityHandle vol_root;
    EntityHandle vol = dag.entity_by_id(3,*i);
    CHECKERR(dag,ret);

    if( vol == 0 ){
      std::cerr << "ERROR: volume " << *i << " has no entity." << std::endl;
      continue;
    }

    ret = dag.get_root( vol, vol_root );
    CHECKERR(dag,ret);

    out << "\nVolume " << *i << " " << std::flush;

    if( dag.is_implicit_complement(vol) ) out << "(implicit complement) ";
    out << std::endl;

    std::string propstring = make_property_string( dag, vol, properties );
    if( propstring.length() ) out << "Properties: " << propstring << std::endl;

    // get all surfaces in volume
    Range surfs;
    ret = dag.moab_instance()->get_child_meshsets( vol, surfs );
    CHECKERR(dag,ret);

    out << "   with " << surfs.size() << " surfaces" << std::endl;
    
    TriStats ts( dag.moab_instance(), &obbtool, vol_root );
    ret = obbtool.preorder_traverse( vol_root, ts );
    CHECKERR(dag,ret);
    ts.write_results( out );

    if( verbose ){
      out << "Surface list: " << std::flush;
      for( Range::iterator j = surfs.begin(); j!=surfs.end(); ++j){
        out << dag.get_entity_id(*j);
        std::string props = make_property_string( dag, *j, properties );
        if( props.length() ) out << "(" << props << ")";
        if( j+1 != surfs.end() ) out << ",";
      }
      out << std::endl;
      ret = obbtool.stats( vol_root, out ); 
      CHECKERR(dag,ret);
    }

    out << "\n    ------------ " << std::endl;

  }

  return ret;
}
