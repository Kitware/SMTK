#include "moab/Core.hpp"
#include "moab/Range.hpp"
#include "MBTagConventions.hpp"
#include <iostream>
#include <fstream>
#include <limits>
#include <cstdlib>
#include <math.h>

/* Exit values */
#define USAGE_ERROR 1
#define READ_ERROR 2
#define WRITE_ERROR 3
#define SURFACE_NOT_FOUND 4
#define OTHER_ERROR 5

static void usage_error( const char* name )
{
  std::cerr << "Usauge: " << name << 
    " [-g|-p] <Surface_ID> <input_file>" << std::endl
    << "\t-g           -  Write GNU Plot data file (default)." << std::endl
    << "\t-p           -  Write encapsulated postscript file"  << std::endl
    << "\t-s           -  Write an SVG file"                   << std::endl
    << "\t<Surface_ID> -  ID of surface containing mesh to export (0 for entire file)." << std::endl
    << "\t<input_file> -  Mesh file to read." << std::endl
    << std::endl
    << "  This utility plots the mesh of a single geometric surface "
    << "projected to a plane.  The output file is written to stdout."
    << std::endl;
  
  exit(USAGE_ERROR);
}

struct CartVect3D {

    double x, y, z;
    
    CartVect3D() {}
    
    CartVect3D( double x_, double y_, double z_ )
      : x(x_), y(y_), z(z_) {}

    CartVect3D& operator+=( const CartVect3D& o )
      { x += o.x; y += o.y; z += o.z; return *this; }
  
    CartVect3D& operator-=( const CartVect3D& o )
      { x -= o.x; y -= o.y; z -= o.z; return *this; }
      
    CartVect3D& operator*=( const CartVect3D& );
      
    CartVect3D& operator+=( double v )
      { x += v; y += v; z += v; return *this; }
      
    CartVect3D& operator-=( double v )
      { x -= v; y -= v; z -= v; return *this; }
      
    CartVect3D& operator*=( double v )
      { x *= v; y *= v; z *= v; return *this; }
      
    CartVect3D& operator/=( double v )
      { x /= v; y /= v; z /= v; return *this; }
    
    double len() const
      { return sqrt( x*x + y*y + z*z ); }
};

//static CartVect3D operator-( const CartVect3D a )
//  { return CartVect3D(-a.z, -a.y, -a.z);  }

//static CartVect3D operator+( const CartVect3D a, const CartVect3D b )
//  { return CartVect3D(a.x+b.x, a.y+b.y, a.z+b.z); }

static CartVect3D operator-( const CartVect3D a, const CartVect3D b )
  { return CartVect3D(a.x-b.x, a.y-b.y, a.z-b.z); }

static double operator%( const CartVect3D a, const CartVect3D b )
  { return a.x*b.x + a.y*b.y + a.z*b.z; }

static CartVect3D operator*( const CartVect3D a, const CartVect3D b )
{
  CartVect3D result;
  result.x = a.y * b.z - a.z * b.y;
  result.y = a.z * b.x - a.x * b.z;
  result.z = a.x * b.y - a.y * b.x;
  return result;
}

CartVect3D& CartVect3D::operator*=( const CartVect3D& o )
  { *this = *this * o; return *this; }

static void find_rotation( CartVect3D plane_normal,
                           double matrix[3][3] )
{
    // normalize
  plane_normal /= plane_normal.len();
  if (fabs(plane_normal.x) < 0.1)
    plane_normal.x = 0.0;
  if (fabs(plane_normal.y) < 0.1)
    plane_normal.y = 0.0;
  if (fabs(plane_normal.z) < 0.1)
    plane_normal.z = 0.0;
  
    // calculate vector to rotate about
  const CartVect3D Z(0,0,1);
  CartVect3D vector = plane_normal * Z;
  const double len = vector.len();
  
    // If vector is zero, no rotation
  if (len < 1e-2) {
    matrix[0][0] = matrix[1][1] = matrix[2][2] = 1.0;
    matrix[0][1] = matrix[1][0] = 0.0;
    matrix[0][2] = matrix[2][0] = 0.0;
    matrix[1][2] = matrix[2][1] = 0.0;
    return;
  }
  vector /= len;

  const double cosine = plane_normal % Z;
  const double sine = sqrt( 1 - cosine*cosine );
  
  std::cerr << "Rotation: " << acos(cosine) << " [" << vector.x << ' ' << vector.y << ' ' << vector.z << ']' << std::endl;
  
  const double x = vector.x;
  const double y = vector.y;
  const double z = vector.z;
  const double c = cosine;
  const double s = sine;
  const double o = 1.0 - cosine;
  
  matrix[0][0] =    c + x*x*o;
  matrix[0][1] = -z*s + x*y*o;
  matrix[0][2] =  y*s + x*z*o;
  
  matrix[1][0] =  z*s + x*z*o;
  matrix[1][1] =    c + y*y*o;
  matrix[1][2] = -x*s + y*z*o;
  
  matrix[2][0] = -y*s + x*z*o;
  matrix[2][1] =  x*s + y*z*o;
  matrix[2][2] =    c + z*z*o;
}
  
static void transform_point( CartVect3D& point, double matrix[3][3] )
{
  const double x = point.x;
  const double y = point.y;
  const double z = point.z;
  
  point.x = x*matrix[0][0] + y*matrix[0][1] + z*matrix[0][2];
  point.y = x*matrix[1][0] + y*matrix[1][1] + z*matrix[1][2];
  point.z = x*matrix[2][0] + y*matrix[2][1] + z*matrix[2][2];
}
  

static void write_gnuplot( std::ostream& stream, 
                           const std::vector<CartVect3D>& list );

static void write_svg( std::ostream& stream,
                       const std::vector<CartVect3D>& list );

static void write_eps( std::ostream& stream,
                       const std::vector<CartVect3D>& list,
                       int surface_id );

enum FileType {
  POSTSCRIPT,
  GNUPLOT,
  SVG
};

using namespace moab;

int main(int argc, char* argv[])
{
  Interface* moab = new Core();
  ErrorCode result;
  std::vector<CartVect3D>::iterator iter;
  FileType type = GNUPLOT;

  int idx = 1;
  if (argc == 4)
  {
    if (!strcmp(argv[idx],"-p"))
      type = POSTSCRIPT;
    else if (!strcmp(argv[idx],"-g"))
      type = GNUPLOT;
    else if (!strcmp(argv[idx],"-s"))
      type = SVG;
    else
      usage_error(argv[0]);
    ++idx;
  }
  
    

    // scan CL args
  int surface_id;
  if (argc - idx != 2)
    usage_error(argv[0]);
  char* endptr;
  surface_id = strtol( argv[idx], &endptr, 0 );
  if (!endptr || *endptr)
    usage_error(argv[0]);
  ++idx;
  
    // Load mesh
  result = moab->load_mesh( argv[idx] );
  if (MB_SUCCESS != result) {
    if (MB_FILE_DOES_NOT_EXIST == result)
      std::cerr << argv[idx] << " : open failed.\n";
    else
      std::cerr << argv[idx] << " : error reading file.\n";
    return READ_ERROR;
  }
  
    // Get tag handles
  EntityHandle surface;
  const int dimension = 2; // surface
  if (surface_id) {
    Tag tags[2];
    result = moab->tag_get_handle( GEOM_DIMENSION_TAG_NAME, 1, MB_TYPE_INTEGER, tags[0] );
    if (MB_SUCCESS != result) {
      std::cerr << "No geometry tag.\n";
      return OTHER_ERROR;
    }
    result = moab->tag_get_handle( GLOBAL_ID_TAG_NAME, 1, MB_TYPE_INTEGER, tags[1] );
    if (MB_SUCCESS != result) {
      std::cerr << "No ID tag.\n";
      return OTHER_ERROR;
    }

      // Find entityset for surface.
    const void* tag_values[] = { &dimension, &surface_id };
    Range surfaces;
    moab->get_entities_by_type_and_tag( 0, MBENTITYSET,
                                        tags, tag_values,
                                        2, surfaces );
    if (surfaces.size() != 1) {
      std::cerr << "Found " << surfaces.size() 
                << " surfaces with ID " << surface_id
                << std::endl;
      return SURFACE_NOT_FOUND;
    }
    surface = *surfaces.begin();
  }
  else {
    surface = 0;
  }
  
    // Get surface mesh
  Range elements;
  result = moab->get_entities_by_dimension( surface, dimension, elements );
  if (MB_SUCCESS != result) {
    std::cerr << "Internal error\n";
    return OTHER_ERROR;
  }
  
    // Calculate average corner normal in surface mesh
  CartVect3D normal(0,0,0);
  std::vector<EntityHandle> vertices;
  std::vector<CartVect3D> coords;
  for (Range::iterator i = elements.begin(); i != elements.end(); ++i)
  {
    vertices.clear();
    result = moab->get_connectivity( &*i, 1, vertices, true );
    if (MB_SUCCESS != result) {
      std::cerr << "Internal error\n";
      return OTHER_ERROR;
    }
    coords.clear();
    coords.resize( vertices.size() );
    result = moab->get_coords( &vertices[0], vertices.size(),
                               reinterpret_cast<double*>(&coords[0]) );
    if (MB_SUCCESS != result) {
      std::cerr << "Internal error\n";
      return OTHER_ERROR;
    }
    
    for (size_t j = 0; j < coords.size(); ++j) {
      CartVect3D v1 = coords[(j+1)%coords.size()] - coords[j];
      CartVect3D v2 = coords[(j+1)%coords.size()] - coords[(j+2)%coords.size()];
      normal += (v1 * v2);
    }
  }
  normal /= normal.len();

  
    // Get edges from elements
  Range edge_range;
  result = moab->get_adjacencies( elements, 1, true, edge_range, Interface::UNION );
  if (MB_SUCCESS != result) {
    std::cerr << "Internal error\n";
    return OTHER_ERROR;
  }
  
    // Get vertex coordinates for each edge
  std::vector<EntityHandle> edges( edge_range.size() );
  std::copy( edge_range.begin(), edge_range.end(), edges.begin() );
  vertices.clear();
  result = moab->get_connectivity( &edges[0], edges.size(), vertices, true );
  if (MB_SUCCESS != result) {
    std::cerr << "Internal error\n";
    return OTHER_ERROR;
  }
  coords.clear();
  coords.resize( vertices.size() );
  result = moab->get_coords( &vertices[0], vertices.size(), 
                             reinterpret_cast<double*>(&coords[0]) );
  if (MB_SUCCESS != result) {
    std::cerr << "Internal error\n";
    return OTHER_ERROR;
  }
  
    // Rotate points such that the projection into the view plane
    // can be accomplished by disgarding the 'z' coordinate of each
    // point.
  
  std::cerr << "Plane normal: [" << normal.x << ' ' << normal.y << ' ' << normal.z << ']' << std::endl;
  double transform[3][3];
  find_rotation( normal, transform );
  
  for (iter = coords.begin(); iter != coords.end(); ++iter)
    transform_point( *iter, transform );
  
    // Write the file.
  
  switch (type) {
    case POSTSCRIPT:
      write_eps( std::cout, coords, surface_id );
      break;
    case SVG:
      write_svg( std::cout, coords );
      break;
    default:
      write_gnuplot( std::cout, coords );
      break;
  }
  return 0;
}
    
void  write_gnuplot( std::ostream& stream, 
                     const std::vector<CartVect3D>& coords )
{ 
  std::vector<CartVect3D>::const_iterator iter;

  stream << std::endl;
  for (iter = coords.begin(); iter != coords.end(); ++iter) {
    stream << iter->x << ' ' << iter->y << std::endl;
    ++iter;
    stream << iter->x << ' ' << iter->y << std::endl;
    stream << std::endl;
  }
  std::cerr << "Display with gnuplot command \"plot with lines\"\n"; 
}

static void box_max( CartVect3D& cur_max, const CartVect3D& pt )
{
  if (pt.x > cur_max.x)
    cur_max.x = pt.x;
  if (pt.y > cur_max.y)
    cur_max.y = pt.y;
  //if (pt.z > cur_max.z)
  //  cur_max.z = pt.z;
}

static void box_min( CartVect3D& cur_min, const CartVect3D& pt )
{
  if (pt.x < cur_min.x)
    cur_min.x = pt.x;
  if (pt.y < cur_min.y)
    cur_min.y = pt.y;
  //if (pt.z > cur_min.z)
  //  cur_min.z = pt.z;
}


void write_eps( std::ostream& s, const std::vector<CartVect3D>& coords, int id )
{
    // Coordinate range to use within EPS file
  const int X_MAX = 540;  // 540 pts / 72 pts/inch = 7.5 inches
  const int Y_MAX = 720;  // 720 pts / 72 pts/inch = 10 inches 
  
  std::vector<CartVect3D>::const_iterator iter;
 
    // Get bounding box
  const double D_MAX = std::numeric_limits<double>::max();
  CartVect3D min(  D_MAX,  D_MAX, 0 );
  CartVect3D max( -D_MAX, -D_MAX, 0 );
  for (iter = coords.begin(); iter != coords.end(); ++iter)
  {
    box_max( max, *iter );
    box_min( min, *iter );
  }
  
    // Calcuate translation to page coordiantes
  CartVect3D offset = CartVect3D(0,0,0) - min;
  CartVect3D scale  = max - min;
  scale.x = X_MAX / scale.x;
  scale.y = Y_MAX / scale.y;
  if (scale.x > scale.y)  // keep proportions
    scale.x = scale.y;
  else
    scale.y = scale.x;
  
  //std::cerr << "Min: " << min.x << ' ' << min.y << 
  //           "  Max: " << max.x << ' ' << max.y << std::endl
  //          << "Offset: " << offset.x << ' ' << offset.y <<
  //           "  Scale: " << scale.x << ' ' << scale.y << std::endl;
  
    // Write header stuff
  s << "%!PS-Adobe-2.0 EPSF-2.0"                      << std::endl;
  s << "%%Creator: MOAB surfplot"                     << std::endl;
  s << "%%Title: Surface " << id                      << std::endl;
  s << "%%DocumentData: Clean7Bit"                    << std::endl;
  s << "%%Origin: 0 0"                                << std::endl;
  int max_x = (int)((max.x + offset.x) * scale.x);
  int max_y = (int)((max.y + offset.y) * scale.y);
  s << "%%BoundingBox: 0 0 " << max_x << ' ' << max_y << std::endl;
  s << "%%Pages: 1"                                   << std::endl;
  
  s << "%%BeginProlog"                                << std::endl;
  s << "save"                                         << std::endl;
  s << "countdictstack"                               << std::endl;
  s << "mark"                                         << std::endl;
  s << "newpath"                                      << std::endl;
  s << "/showpage {} def"                             << std::endl;
  s << "/setpagedevice {pop} def"                     << std::endl;
  s << "%%EndProlog"                                  << std::endl;
  
  s << "%%Page: 1 1"                                  << std::endl;
  s << "1 setlinewidth"                               << std::endl;
  s << "0.0 setgray"                                  << std::endl;
  
  for (iter = coords.begin(); iter != coords.end(); ++iter)
  {
    double x1 = (iter->x + offset.x) * scale.x;
    double y1 = (iter->y + offset.y) * scale.y;
    if (++iter == coords.end())
      break;
    double x2 = (iter->x + offset.x) * scale.x;
    double y2 = (iter->y + offset.y) * scale.y;
    
    s << "newpath"                                    << std::endl;
    s << x1 << ' ' << y1 << " moveto"                 << std::endl;
    s << x2 << ' ' << y2 << " lineto"                 << std::endl;
    s << "stroke"                                     << std::endl;
  }
  
  s << "%%Trailer"                                    << std::endl;
  s << "cleartomark"                                  << std::endl;
  s << "countdictstack"                               << std::endl;
  s << "exch sub { end } repeat"                      << std::endl;
  s << "restore"                                      << std::endl;  
  s << "%%EOF"                                        << std::endl;

}


void write_svg( std::ostream& file,
                const std::vector<CartVect3D>& coords )
{
  
  std::vector<CartVect3D>::const_iterator iter;
 
    // Get bounding box
  const double D_MAX = std::numeric_limits<double>::max();
  CartVect3D min(  D_MAX,  D_MAX, 0 );
  CartVect3D max( -D_MAX, -D_MAX, 0 );
  for (iter = coords.begin(); iter != coords.end(); ++iter)
  {
    box_max( max, *iter );
    box_min( min, *iter );
  }
  CartVect3D size = max - min;
  
    // scale to 640 pixels on a side
  double scale = 640.0 / (size.x > size.y ? size.x : size.y);
  size *= scale;
  
  

  file << "<?xml version=\"1.0\" standalone=\"no\"?>"                << std::endl;
  file << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" " 
       << "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">"    << std::endl;
  file <<                                                               std::endl;
  file << "<svg width=\"" << (int)size.x
       << "\" height=\"" << (int)size.y
       << "\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">" << std::endl;
  
  int left = (int)(min.x * scale);
  int top = (int)(min.y * scale);
  iter = coords.begin(); 
  while (iter != coords.end()) {
    file << "<line "
         << "x1=\"" << (int)(scale * iter->x) - left << "\" "
         << "y1=\"" << (int)(scale * iter->y) - top << "\" ";
    ++iter;
    file << "x2=\"" << (int)(scale * iter->x) - left << "\" "
         << "y2=\"" << (int)(scale * iter->y) - top << "\" "
         << " style=\"stroke:rgb(99,99,99);stroke-width:2\""
         << "/>" << std::endl;
    ++iter;
  }
  
    // Write footer
  file << "</svg>" << std::endl;
}
    

    
  

    
