/* Copyright 2007-2009 Brad King, Chuck Stewart
   Distributed under the Boost Software License, Version 1.0.
   (See accompanying file rtvl_license_1_0.txt or copy at
   http://www.boost.org/LICENSE_1_0.txt) */
#ifndef rtvl_level_refine_hxx
#define rtvl_level_refine_hxx

#include "rtvl_level_refine.h"

#include "rtvl/rtvl_tensor.hxx"
#include "rtvl/rtvl_tokens.h"
#include "rtvl/rtvl_vote.hxx"
#include "rtvl/rtvl_votee.hxx"
#include "rtvl/rtvl_voter.hxx"
#include "rtvl/rtvl_weight_smooth.hxx"

#include <rgtl/rgtl_object_array_points.hxx>
#include <rgtl/rgtl_octree_cell_bounds.hxx>
#include <rgtl/rgtl_octree_objects.hxx>
#include <rgtl/rgtl_serialize_istream.h>
#include <rgtl/rgtl_serialize_ostream.h>

#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_vector_fixed.h>

#include <vcl_algorithm.h>
#include <vcl_compiler.h>
#include <vcl_cmath.h>
#include <vcl_fstream.h>
#include <vcl_list.h>
#include <vcl_map.h>
#include <vcl_memory.h>
#include <vcl_string.h>
#include <vcl_vector.h>

template <typename T>
using vcl_auto_ptr = std::auto_ptr<T>;

template <unsigned int N>
class rtvl_level_refine_quadtree;

template <unsigned int N>
class rtvl_level_refine_level
{
public:
  rtvl_level_refine_level()
  {
    this->token_calculated = 0;
    this->calculate_tokens = 0;
    this->tokens = new rtvl_tokens<N>;
  };
  ~rtvl_level_refine_level()
  {
    if (this->token_calculated)
    {
      delete[] this->token_calculated;
    }
    if (this->calculate_tokens)
    {
      delete this->calculate_tokens;
    }
    if (this->tokens)
    {
      delete this->tokens;
    }
  };

  rtvl_tokens<N>* tokens;
  vcl_vector<rtvl_tensor<N> >* calculate_tokens;
  rtvl_level_refine_quadtree<N>* quad;      // quad that contains this refine_level
  vcl_vector<unsigned int> selection; // samples selected to move to the next level;
  bool* token_calculated; // instead can we just test if saliency is 0????  don't think so
  double scale;
  int index;
  // even if data not loaded we can know how many points, and thus esitimate memory usage
  unsigned int number_of_points;
  vcl_string filename;

  // a "temporary" variable for finding index of point within level based on an index
  // into points from many levels
  unsigned int offset;

  // not sure we need this (might just compare length of tensor, initially 0, to
  // length of "points"
  bool tensor_initialized;
};

template <unsigned int N>
class rtvl_level_refine_quadtree
{
public:
  rtvl_level_refine_quadtree()
  {
    this->child = 0;
    this->quadUp = 0;
    this->quadRight = 0;
    this->tree_depth = 0;
    this->parent = 0;
    this->position = 0;
  };
  ~rtvl_level_refine_quadtree()
  {
    typename vcl_vector<rtvl_level_refine_level<N>*>::const_iterator levels_iter = this->levels.begin();
    for (; levels_iter != this->levels.end(); levels_iter++)
    {
      delete *(levels_iter);
    }
    if (this->tree_depth != 0)
    {
      delete[] this->child;
    }
  };
  void init(int depth, double* bounds);

  void initialize_points(int depth, unsigned int i, unsigned int j, double scale,
    vcl_vector<unsigned int> pointIndices, rgtl_object_array_points<N>& points);
  void add_selections_for_next_level(double scale, double scale_multiplier, int processLevel,
    vcl_vector<rtvl_level_refine_level<N>*>& levels_subset);

  rtvl_level_refine_quadtree<N>* find_quad_with_level(int level);
  void get_lengths(double* lengths)
  {
    lengths[0] = this->bounds[1] - this->bounds[0];
    lengths[1] = this->bounds[3] - this->bounds[2];
  }
  rtvl_level_refine_quadtree<N>* get_quad(unsigned int depth, unsigned int i, unsigned int j);
  unsigned int get_number_of_points(int level);
  unsigned int gather_points(int level, double padLength,
    vcl_vector<rtvl_level_refine_level<N>*>& levels_vector, rgtl_object_array_points<N>& points,
    rtvl_level_refine_level<N>**(&pointsLevels), bool countOnly);
  void gather_levels(int level, vcl_vector<rtvl_level_refine_level<N>*>& levels_vector);
  void gather_levels_within_bounds(
    int level, vcl_vector<rtvl_level_refine_level<N>*>& levels_vector, double required_bounds[4]);
  bool within_bounds(double required_bounds[4]);
  void build_quad(vcl_list<rtvl_level_refine_quadtree<N>*>& quad_stack);

  rtvl_level_refine_quadtree<N>* parent;
  // 2 3
  // 0 1
  char position;
  rtvl_level_refine_quadtree<N>* child;
  double bounds[4]; // of this quad
  int tree_depth;
  rtvl_level_refine_quadtree<N>* quadRight;
  rtvl_level_refine_quadtree<N>* quadUp;

  vcl_vector<rtvl_level_refine_level<N>*> levels;
};

template <unsigned int N>
void rtvl_level_refine_quadtree<N>::init(int depth, double* quad_bounds)
{
  vcl_list<rtvl_level_refine_quadtree<N>*> quad_stack;

  vcl_memcpy(this->bounds, quad_bounds, sizeof(double) * 4);
  this->tree_depth = depth;

  quad_stack.push_back(this);

  while (quad_stack.size() > 0)
  {
    quad_stack.front()->build_quad(quad_stack);
    quad_stack.pop_front();
  }
}

template <unsigned int N>
void rtvl_level_refine_quadtree<N>::build_quad(vcl_list<rtvl_level_refine_quadtree<N>*>& quad_stack)
{
  if (this->tree_depth > 0)
  {
    this->child = new rtvl_level_refine_quadtree<N>[4];
    double child_bounds[4];
    for (int i = 0; i < 2; i++)
    {
      child_bounds[i] = this->bounds[i];
      // could do !i, but I think this is a little more clear
      child_bounds[i ? 0 : 1] = (this->bounds[1] + this->bounds[0]) / 2.0;
      for (int j = 0; j < 4; j += 2)
      {
        child_bounds[j ? 3 : 2] = this->bounds[j ? 3 : 2];
        child_bounds[j ? 2 : 3] = (this->bounds[2] + this->bounds[3]) / 2.0;
        vcl_memcpy(this->child[i + j].bounds, child_bounds, sizeof(double) * 4);
        this->child[i + j].tree_depth = this->tree_depth - 1;
        this->child[i + j].parent = this;
        this->child[i + j].position = i + j;
        quad_stack.push_back(this->child + i + j);
      }

      // setup "up" and "right" within this quad
      this->child[0].quadRight = this->child + 1;
      this->child[0].quadUp = this->child + 2;
      this->child[2].quadRight = this->child + 3;
      this->child[1].quadUp = this->child + 3;
    }
  }

  // do we need to connect up to quad to our right
  if (!this->quadRight && this->parent && this->parent->quadRight)
  {
    this->quadRight = this->parent->quadRight->child + this->position - 1;
  }
  // do we need to connect up to quad to our right
  if (!this->quadUp && this->parent && this->parent->quadUp)
  {
    this->quadUp = this->parent->quadUp->child + this->position - 2;
  }
}

template <unsigned int N>
void rtvl_level_refine_quadtree<N>::initialize_points(int depth, unsigned int i, unsigned int j,
  double scale, vcl_vector<unsigned int> indices, rgtl_object_array_points<N>& points)
{
  if (depth != 0)
  {
    unsigned int mask = 0x01 << --depth;
    this->child[((i & mask) ? 1 : 0) + ((j & mask) ? 2 : 0)].initialize_points(
      depth, i, j, scale, indices, points);
    return;
  }

  rtvl_level_refine_level<N>* level = new rtvl_level_refine_level<N>;
  int number_of_points = static_cast<int>(indices.size());
  level->tokens->points.set_number_of_points(number_of_points);
  level->number_of_points = number_of_points;
  level->quad = this;
  level->scale = scale;
  double p[N];
  for (int n = 0; n < number_of_points; n++)
  {
    points.get_point(indices[n], p);
    level->tokens->points.set_point(n, p);
  }
  level->tokens->scale = scale;
  level->index = 0;

  this->levels.push_back(level);
}

template <unsigned int N>
void rtvl_level_refine_quadtree<N>::add_selections_for_next_level(double scale, double scale_multiplier,
  int processLevel, vcl_vector<rtvl_level_refine_level<N>*>& levels_subset)
{
  typename vcl_vector<rtvl_level_refine_level<N>*>::const_iterator levels_subset_iter =
    levels_subset.begin();
  unsigned int total_points = 0;
  for (; levels_subset_iter != levels_subset.end(); levels_subset_iter++)
  {
    total_points += static_cast<unsigned int>((*levels_subset_iter)->selection.size());
  }

  rtvl_level_refine_level<N>* next_level = new rtvl_level_refine_level<N>;
  next_level->scale = scale;
  next_level->tokens->scale = scale;
  next_level->index = processLevel;
  next_level->number_of_points = total_points;
  next_level->quad = this;
  next_level->tokens->points.set_number_of_points(total_points);

  // Initialize tokens in the next level with ball tensors, and (for purpose of next level)
  // not yet calculated (even though we may be moving some information from this level.
  next_level->calculate_tokens = new vcl_vector<rtvl_tensor<N> >;
  next_level->calculate_tokens->resize(total_points);
  next_level->token_calculated = new bool[total_points];
  memset(next_level->token_calculated, false, total_points);

  unsigned int new_points_index = 0;
  for (levels_subset_iter = levels_subset.begin(); levels_subset_iter != levels_subset.end();
       levels_subset_iter++)
  {
    rtvl_level_refine_level<N>* this_level = (*levels_subset_iter);
    unsigned int n = static_cast<unsigned int>(this_level->selection.size());
    //    unsigned int n = static_cast<unsigned int>((*levels_subset_iter)->selection.size());

    for (unsigned int i = 0; i < n; i++)
    {
      unsigned int id = this_level->selection[i];
      double p[N];
      this_level->tokens->points.get_point(id, p);
      next_level->tokens->points.set_point(new_points_index, p);

      // Communicate the tensor to the next level.
      double const max_saliency = 10;
      next_level->calculate_tokens->at(new_points_index) = this_level->tokens->tokens[id];
      next_level->calculate_tokens->at(new_points_index++)
        .next_scale(scale_multiplier, max_saliency);
    }
    this_level->selection.clear();
  }

  this->levels.push_back(next_level);
}

template <unsigned int N>
rtvl_level_refine_quadtree<N>* rtvl_level_refine_quadtree<N>::find_quad_with_level(int level)
{
  typename vcl_vector<rtvl_level_refine_level<N>*>::iterator level_iter = this->levels.begin();
  for (; level_iter != this->levels.end(); level_iter++)
  {
    if ((*level_iter)->index == level)
    {
      return this;
    }
  }

  if (!this->tree_depth)
  {
    return 0;
  }
  return this->child[0].find_quad_with_level(level);
}

template <unsigned int N>
rtvl_level_refine_quadtree<N>* rtvl_level_refine_quadtree<N>::get_quad(
  unsigned int depth, unsigned int i, unsigned int j)
{
  if (depth == 0)
  {
    return this->child[((i & 0x01) ? 1 : 0) + ((j & 0x01) ? 2 : 0)];
  }

  unsigned int mask = 0x01 << --depth;
  return this->child[((i & mask) ? 1 : 0) + ((j & mask) ? 2 : 0)].get_quad(depth, i, j);
}

template <unsigned int N>
unsigned int rtvl_level_refine_quadtree<N>::get_number_of_points(int level)
{
  typename vcl_vector<rtvl_level_refine_level<N>*>::iterator level_iter = this->levels.begin();
  for (; level_iter != this->levels.end(); level_iter++)
  {
    if ((*level_iter)->index == level)
    {
      return (*level_iter)->number_of_points;
    }
  }

  // do we have children?
  if (!this->child)
  {
    vcl_cerr << "did get_number_of_points but level not found in quad\n";
    return 0;
  }

  unsigned int sum = 0;
  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 4; j += 2)
    {
      sum += this->child[i + j].get_number_of_points(level);
    }
  }

  return sum;
}

template <unsigned int N>
void rtvl_level_refine_quadtree<N>::gather_levels(
  int level, vcl_vector<rtvl_level_refine_level<N>*>& levels_vector)
{
  // does this quad (itself, not child) contain the requested level?
  typename vcl_vector<rtvl_level_refine_level<N>*>::iterator level_iter = this->levels.begin();
  for (; level_iter != this->levels.end(); level_iter++)
  {
    if ((*level_iter)->index == level)
    {
      levels_vector.push_back(*level_iter);
      return;
    }
  }

  // do we have children?
  if (!this->child)
  {
    vcl_cerr << "did gather_levels but level not found in quad\n";
    return;
  }

  // ok, so the quad doesn't contain the indicating refine level... check it's children
  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 4; j += 2)
    {
      this->child[i + j].gather_levels(level, levels_vector);
    }
  }
}

template <unsigned int N>
void rtvl_level_refine_quadtree<N>::gather_levels_within_bounds(
  int level, vcl_vector<rtvl_level_refine_level<N>*>& levels_vector, double required_bounds[4])
{
  // check the bounds
  if (!this->within_bounds(required_bounds))
  {
    return;
  }

  // does this quad (itself, not child) contain the requested level?
  typename vcl_vector<rtvl_level_refine_level<N>*>::iterator level_iter = this->levels.begin();
  for (; level_iter != this->levels.end(); level_iter++)
  {
    if ((*level_iter)->index == level)
    {
      levels_vector.push_back(*level_iter);
      return;
    }
  }

  // do we have children?
  if (!this->child)
  {
    vcl_cerr << "did gather_levels_within_bounds but level not found in quad\n";
    return;
  }

  // ok, so the quad doesn't contain the indicating refine level... check it's children
  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 4; j += 2)
    {
      this->child[i + j].gather_levels_within_bounds(level, levels_vector, required_bounds);
    }
  }
}

template <unsigned int N>
unsigned int rtvl_level_refine_quadtree<N>::gather_points(int level, double padLength,
  vcl_vector<rtvl_level_refine_level<N>*>& levels_vector, rgtl_object_array_points<N>& points,
  rtvl_level_refine_level<N>**(&pointsLevels), bool countOnly)
{
  levels_vector.clear(); // just in case reusing

  // given how we process, don't need quads below (lesser y) or left (lesser x)
  // the bounds for this request are bounds of this quad and then padded
  double required_bounds[4];
  required_bounds[0] = bounds[0];
  required_bounds[1] = bounds[1] + padLength;
  required_bounds[2] = bounds[2];
  required_bounds[3] = bounds[3] + padLength;

  // we're gathering points to process this quad, so add levels for this quad
  this->gather_levels(level, levels_vector);

  // now, levels to support padding on the right
  rtvl_level_refine_quadtree* tempQuadRight = this->quadRight;
  while (tempQuadRight && tempQuadRight->bounds[0] < required_bounds[1])
  {
    tempQuadRight->gather_levels_within_bounds(level, levels_vector, required_bounds);
    tempQuadRight = tempQuadRight->quadRight;
  }

  // and above ("entire above", which includes above and to the right)
  rtvl_level_refine_quadtree* tempQuadUp = this->quadUp;
  while (tempQuadUp && tempQuadUp->bounds[2] < required_bounds[3])
  {
    tempQuadUp->gather_levels_within_bounds(level, levels_vector, required_bounds);
    tempQuadRight = tempQuadUp->quadRight;
    while (tempQuadRight && tempQuadRight->bounds[0] < required_bounds[1])
    {
      tempQuadRight->gather_levels_within_bounds(level, levels_vector, required_bounds);
      tempQuadRight = tempQuadRight->quadRight;
    }
    tempQuadUp = tempQuadUp->quadUp;
  }

  // so now should have all levels which contain the requested points... how many in all?
  typename vcl_vector<rtvl_level_refine_level<N>*>::iterator level_iter = levels_vector.begin();
  unsigned int total_points = 0;
  for (; level_iter != levels_vector.end(); level_iter++)
  {
    total_points += (*level_iter)->number_of_points;
  }

  if (total_points == 0)
  {
    //RSB!!! can this happen and be correct result?
    vcl_cerr << "did gather_points but got no points!\n";
    return 0;
  }

  if (countOnly)
  {
    return total_points;
  }

  points.set_number_of_points(total_points);

  // now add the points... but from index of point need to be able to get to rtvl_level_refine_level
  // and the index of the point in the level (ca nbe done more efficiently in terms of memory, but
  // for now (approximately) optimize for speed... not sure if we might have a lot of levels
  // in the levels_vector
  pointsLevels = new rtvl_level_refine_level<N>*[total_points];

  unsigned int point_index = 0;
  for (level_iter = levels_vector.begin(); level_iter != levels_vector.end(); level_iter++)
  {
    // if haven't allocated tokens, do so now
    if ((*level_iter)->tokens->tokens.size() == 0) // or != (*level_iter)->number_of_points
    {
      (*level_iter)->tokens->tokens.resize((*level_iter)->number_of_points);
      (*level_iter)->token_calculated = new bool[(*level_iter)->number_of_points];
      memset((*level_iter)->token_calculated, false, (*level_iter)->number_of_points);
      if ((*level_iter)->calculate_tokens == 0)
      {
        (*level_iter)->calculate_tokens = new vcl_vector<rtvl_tensor<N> >;
        (*level_iter)->calculate_tokens->resize((*level_iter)->number_of_points);
      }
    }
    (*level_iter)->offset = point_index;
    for (unsigned int i = 0; i < (*level_iter)->number_of_points; i++)
    {
      double p[N];
      (*level_iter)->tokens->points.get_point(i, p);
      points.set_point(point_index, p);

      pointsLevels[point_index++] = *level_iter;
    }
  }
  return total_points;
}

template <unsigned int N>
bool rtvl_level_refine_quadtree<N>::within_bounds(double required_bounds[4])
{
  int i;
  for (i = 0; i < 4; i += 2)
  {
    if ((required_bounds[i] >= this->bounds[i]) && (required_bounds[i] <= this->bounds[i + 1]))
    {
      continue;
    }
    if ((this->bounds[i] >= required_bounds[i]) && (this->bounds[i] <= required_bounds[i + 1]))
    {
      continue;
    }
    if ((required_bounds[i + 1] >= this->bounds[i]) &&
      (required_bounds[i + 1] <= this->bounds[i + 1]))
    {
      continue;
    }
    if ((this->bounds[i + 1] >= required_bounds[i]) &&
      (this->bounds[i + 1] <= required_bounds[i + 1]))
    {
      continue;
    }
    return false;
  }
  return true;
}

template <unsigned int N>
class rtvl_level_refine_internal
{
public:
  rtvl_level_refine_internal(rtvl_level_refine<N>* e);
  void init(unsigned int num_points, double* points, double* bounds_in);
  void set_mask_size(double f);
  void extract_tokens(int level, double* bounds, rtvl_tokens<N>& out) const;
  double get_level_scale(int level);
  unsigned int get_vote_count() const { return vote_count; }
  void select_scale();
  double get_current_scale() const;
  void set_scale(double scale);

  int initialize_refine_level(unsigned int memory_limit, int requested_min_depth);
  int refine_next_block(const char* base_filename);

  void build_tree();

  void next_scale();

  //  unsigned int count_tokens();

  unsigned int estimate_refine_levels();
  bool last_level();

private:
  void setup_level();
  void refine_block(rgtl_object_array_points<N>& blockPoints, rtvl_level_refine_level<N>**(&pointsLevels),
    double refine_bounds[4]);
  void select_block_samples(vcl_vector<rtvl_level_refine_level<N>*>& levels_vector,
    rgtl_object_array_points<N>& blockPoints, rtvl_level_refine_level<N>**(&pointsLevels),
    double selection_bounds[4]);

  rtvl_level_refine<N>* external;
  double scale_multiplier;
  double mask_size;
  vcl_auto_ptr<rtvl_level_refine_quadtree<N> > tree;
  //vcl_auto_ptr< rtvl_tokens<N> > level;
  vcl_auto_ptr<rgtl_octree_objects<N> > objects;
  rgtl_octree_cell_bounds<N> bounds;
  unsigned int vote_count;
  double data_bounds[N * 2];
  int current_level;
  double current_scale;
  vcl_vector<double> level_scale;
  rtvl_level_refine_quadtree<N>* refine_quad_base;
  rtvl_level_refine_quadtree<N>* refine_quad_next;
  rgtl_object_array_points<N> points;
  bool save_next_level_samples_in_parent;
  bool is_last_level;
};

template <unsigned int N>
rtvl_level_refine_internal<N>::rtvl_level_refine_internal(rtvl_level_refine<N>* e)
  : external(e)
{
  scale_multiplier = vcl_sqrt(2.0);
  mask_size = 1;
  vote_count = 0;
  current_level = 0;
  save_next_level_samples_in_parent = false;
  current_scale = -1;
}

template <unsigned int N>
void rtvl_level_refine_internal<N>::init(unsigned int num_points, double* points_in, double* bounds_in)
{
  vcl_memcpy(data_bounds, bounds_in, sizeof(double) * N * 2);

  // Store points... released after building the tree
  this->points.set_number_of_points(num_points);
  for (unsigned int i = 0; i < num_points; i++)
  {
    this->points.set_point(i, points_in);
    points_in += N;
  }
}

template <unsigned int N>
void rtvl_level_refine_internal<N>::build_tree()
{
  this->level_scale.clear();
  //RSB!!!
  // get point from outside world, make them our own... enough to select_scale...
  // once we have a scale, then decide on level for quad-tree
  // load the points into memory, computing what level quad is necessary as we go...
  // enough to select_scale only????

  // initial implementation assumes we can load all points in memory!  But let's
  // make only one pass over the points to determine their location
  tree.reset(new rtvl_level_refine_quadtree<N>);
  int tree_depth = 3; // to do is figuring out required depth... but not yet
  tree->init(tree_depth, this->data_bounds);
  int numberOfDivisions = 0x01 << tree_depth;
  // ~10% more than if average
  int numberPerLeaf =
    (int)(1.1 * this->points.get_number_of_points() / (numberOfDivisions * numberOfDivisions));
  vcl_vector<unsigned int>** treePoints;
  treePoints = new vcl_vector<unsigned int>*[numberOfDivisions];
  for (int i = 0; i < numberOfDivisions; i++)
  {
    treePoints[i] = new vcl_vector<unsigned int>[numberOfDivisions];
    for (int j = 0; j < numberOfDivisions; j++)
    {
      treePoints[i][j].reserve(numberPerLeaf);
    }
  }
  double xFactor = (double)numberOfDivisions / (this->data_bounds[1] - this->data_bounds[0]);
  double yFactor = (double)numberOfDivisions / (this->data_bounds[3] - this->data_bounds[2]);
  int xIndex, yIndex;
  double p[N];
  for (int i = 0; i < this->points.get_number_of_points(); ++i)
  {
    // determine the leaf this point goes into
    this->points.get_point(i, p);
    xIndex = int((p[0] - this->data_bounds[0]) * xFactor);
    yIndex = int((p[1] - this->data_bounds[2]) * yFactor);
    xIndex = (xIndex == numberOfDivisions) ? xIndex - 1 : xIndex;
    yIndex = (yIndex == numberOfDivisions) ? yIndex - 1 : yIndex;
    treePoints[xIndex][yIndex].push_back(i);
  }

  for (int i = 0; i < numberOfDivisions; i++)
  {
    for (int j = 0; j < numberOfDivisions; j++)
    {
      // setup the appropriate quad in the tree
      tree->initialize_points(
        tree_depth, i, j, this->current_scale, treePoints[i][j], this->points);
    }
    delete[] treePoints[i];
  }
  delete[] treePoints;
}

template <unsigned int N>
unsigned int rtvl_level_refine_internal<N>::estimate_refine_levels()
{
  // find maximum dimension
  double maxLength = 0;
  for (unsigned int i = 0; i < N; i += 2)
  {
    if (this->data_bounds[i + 1] - this->data_bounds[i] > maxLength)
    {
      maxLength = this->data_bounds[i + 1] - this->data_bounds[i];
    }
  }
  maxLength *= 1.01;

  unsigned int number_of_levels = 1;
  double test_scale = this->current_scale;
  while (maxLength > 6 * test_scale)
  {
    number_of_levels++;
    test_scale *= this->scale_multiplier;
  }
  return number_of_levels;
}

template <unsigned int N>
void rtvl_level_refine_internal<N>::set_mask_size(double f)
{
  if (f < 0.00)
  {
    f = 0.00;
  }
  if (f > 1)
  {
    f = 1;
  }
  this->mask_size = f;
}

template <unsigned int N>
void rtvl_level_refine_internal<N>::set_scale(double scale)
{
  this->current_scale = scale;
}

template <unsigned int N>
void rtvl_level_refine_internal<N>::next_scale()
{
  this->level_scale.push_back(current_scale);
  this->current_scale *= this->scale_multiplier;
  this->current_level++;
}

template <unsigned int N>
double rtvl_level_refine_internal<N>::get_current_scale() const
{
  return this->current_scale;
}

template <unsigned int N>
void rtvl_level_refine_internal<N>::setup_level()
{
  // Compute spatial data structure bounds.
  double bds[N][2];
  //level->points.compute_bounds(bds); RSB!!!
  bounds.compute_bounds(bds, 1.01);

  // Create a spatial structure for the current point set.
  //  objects.reset(new rgtl_octree_objects<N>(level->points, bounds, 10));  RSB!!!
}

template <unsigned int N>
void rtvl_level_refine_internal<N>::select_scale()
{
  // setup octree for computing the scale
  double bds[N][2];
  for (unsigned int i = 0; i < N; i++)
  {
    bds[i][0] = this->data_bounds[i * 2];
    bds[i][1] = this->data_bounds[i * 2 + 1];
  }
  bounds.compute_bounds(bds, 1.01);
  objects.reset(new rgtl_octree_objects<N>(points, bounds, 10));

  unsigned int const k = 6;
  // Compute a (k+1)-order distance transform so we can find for each
  // input point a kth-closest neighbor.
  objects->compute_distance_transform(k + 1);

  // Compute the nearest point to every point.
  unsigned int n = this->points.get_number_of_points();
  vcl_vector<double> distances(n, 0.0);

  for (unsigned int id = 0; id < n; ++id)
  {
    double p[N];
    this->points.get_point(id, p);
    double squared_distances[k + 1];
    int nc = objects->query_closest(p, k + 1, 0, squared_distances, 0);
    if (nc == (k + 1))
    {
      distances[id] = vcl_sqrt(squared_distances[k]);
    }
  }

  // Choose a scale based on an order-statistic.
  unsigned int nth = static_cast<unsigned int>(distances.size() / 10);
  vcl_nth_element(distances.begin(), distances.begin() + nth, distances.end());
  this->current_scale = distances[nth];
}

// min_number_of_chunks 1, 4, 16, 64, 128...
template <unsigned int N>
int rtvl_level_refine_internal<N>::initialize_refine_level(
  unsigned int /*memory_limit*/, int requested_min_depth)
{
  this->refine_quad_next = this->refine_quad_base = 0;
  // based on current scale, size of blocks in tree, etc

  // find where data for this scale is located
  rtvl_level_refine_quadtree<N>* quad_with_level_data =
    this->tree->find_quad_with_level(this->current_level);
  if (!quad_with_level_data)
  {
    return 0;
  }

  // determine how we're going to process this level; to do that we need to find
  // what size block we can process given the memory limitation set upon us
  // have call that returns points present which are needed to process a particular block (quad)
  unsigned int max_number_of_points = 100000;

  // start at quad_with_level_data
  rtvl_level_refine_quadtree<N>* levelStartQuad = quad_with_level_data;
  unsigned int max_count = 0, current_count;

  vcl_vector<rtvl_level_refine_level<N>*> levels_vector;
  rgtl_object_array_points<N> blockPoints;
  rtvl_level_refine_level<N>** pointsLevels;
  while (levelStartQuad && (this->tree->tree_depth - levelStartQuad->tree_depth) > 1 &&
    max_count < max_number_of_points)
  {
    this->refine_quad_base = levelStartQuad;
    rtvl_level_refine_quadtree<N>* rowStartQuad = levelStartQuad;
    rtvl_level_refine_quadtree<N>* currentQuad = rowStartQuad;
    while (currentQuad)
    {
      current_count = currentQuad->gather_points(this->current_level, 6 * this->current_scale,
        levels_vector, blockPoints, pointsLevels, true);
      if (current_count > max_count)
      {
        max_count = current_count;
      }
      currentQuad = currentQuad->quadRight;
      if (!currentQuad)
      {
        currentQuad = rowStartQuad->quadUp;
        rowStartQuad = currentQuad;
      }
    }
    if (this->tree->tree_depth - levelStartQuad->tree_depth <= requested_min_depth)
    {
      // arbitrary min so that we don't over divide... perhaps should be fraction of
      // max_number_of_points??
      unsigned int depth = this->tree->tree_depth - levelStartQuad->tree_depth;
      while (max_count < 2500 && depth-- > 1)
      {
        this->refine_quad_base = this->refine_quad_base->parent;
        max_count *= 4;
      }
      if (this->refine_quad_base->parent == this->tree.get())
      {
        this->refine_quad_base = this->refine_quad_base->parent;
      }
      break;
    }
    levelStartQuad = levelStartQuad->parent;
  }

  // if not set, then must have data at or next to tree level, so process as 1 block
  if (!this->refine_quad_base)
  {
    this->refine_quad_base = this->tree.get();
  }
  this->refine_quad_next = this->refine_quad_base;

  unsigned int count = 0x01;
  count <<= (this->tree->tree_depth - this->refine_quad_base->tree_depth) * 2;

  // for now, push up a level, every other time
  if (this->current_level == 0)
  {
    this->save_next_level_samples_in_parent = false;
  }
  else
  {
    this->save_next_level_samples_in_parent = !this->save_next_level_samples_in_parent;
  }
  if (this->save_next_level_samples_in_parent && quad_with_level_data == this->refine_quad_base)
  {
    this->save_next_level_samples_in_parent = false;
  }

  this->is_last_level = this->last_level();

  return count;

  // to do the refine, we need 3*scale... while we're "at it", also do the select samples
  // when doing samples, need to mark points masked by selected points that are outside
  // block we're computing... need to have [(3 + 2 * mask_size) * scale] padding, so that we
  // only mask within the block IFF there are points near block boundary that are higher salicency
  // than other points in the adjacent block;
  // since [(3 + 2 * mask_size) * scale] is ALMOST 6 * scale for default mask_size of 1, use
  // 6 * scale padding, and can save refine result; however, also need to have appropriate mask
  // results for select_samples of the block boundary
}

template <unsigned int N>
int rtvl_level_refine_internal<N>::refine_next_block(const char* base_filename)
{
  if (!this->refine_quad_next)
  {
    return 0;
  }

  vcl_vector<rtvl_level_refine_level<N>*> levels_vector;
  rgtl_object_array_points<N> blockPoints;
  rtvl_level_refine_level<N>** pointsLevels = 0;
  this->refine_quad_next->gather_points(this->current_level, 6.0 * this->current_scale,
    levels_vector, blockPoints, pointsLevels, false);

  // shouldn't have to recompute the bounds, for now, do so
  double bds[N][2];
  blockPoints.compute_bounds(bds);
  this->bounds.compute_bounds(bds, 1.01);

  // Create a spatial structure for the current point set.
  this->objects.reset(new rgtl_octree_objects<N>(blockPoints, bounds, 10));

  double refine_bounds[4];
  refine_bounds[0] = this->refine_quad_next->bounds[0];
  refine_bounds[1] = this->refine_quad_next->bounds[1] + 3.0 * this->current_scale;
  refine_bounds[2] = this->refine_quad_next->bounds[2];
  refine_bounds[3] = this->refine_quad_next->bounds[3] + 3.0 * this->current_scale;

  this->refine_block(blockPoints, pointsLevels, refine_bounds);

  // free the tokens we used to calculate this levels tokens...
  vcl_vector<rtvl_level_refine_level<N>*> refine_quad_levels_vector;
  this->refine_quad_next->gather_levels(this->current_level, refine_quad_levels_vector);
  typename vcl_vector<rtvl_level_refine_level<N>*>::const_iterator levels_iter =
    refine_quad_levels_vector.begin();
  for (; levels_iter != refine_quad_levels_vector.end(); levels_iter++)
  {
    delete (*levels_iter)->calculate_tokens;
    (*levels_iter)->calculate_tokens = 0;
  }

  // IF there is another level, select samples now
  if (!this->is_last_level)
  {
    double selection_bounds[4];
    if (this->refine_quad_next->bounds[0] == this->data_bounds[0])
    {
      selection_bounds[0] = this->refine_quad_next->bounds[0];
    }
    else
    {
      selection_bounds[0] =
        this->refine_quad_next->bounds[0] + this->current_scale * this->mask_size;
    }
    selection_bounds[1] = this->refine_quad_next->bounds[1] + this->current_scale * this->mask_size;
    if (this->refine_quad_next->bounds[2] == this->data_bounds[2])
    {
      selection_bounds[2] = this->refine_quad_next->bounds[2];
    }
    else
    {
      selection_bounds[2] =
        this->refine_quad_next->bounds[2] + this->current_scale * this->mask_size;
    }
    selection_bounds[3] = this->refine_quad_next->bounds[3] + this->current_scale * this->mask_size;
    this->select_block_samples(levels_vector, blockPoints, pointsLevels, selection_bounds);
  }

  // write the tokens to disk, and then clear the memory
  levels_iter = refine_quad_levels_vector.begin();
  int output_counter = 0;
  for (; levels_iter != refine_quad_levels_vector.end(); levels_iter++)
  {
    (*levels_iter)->filename = base_filename;
    char buf[20];
    sprintf(buf, "_%d.tvl", output_counter++);
    (*levels_iter)->filename += buf;
    vcl_ofstream fout((*levels_iter)->filename.c_str(), vcl_ios::out | vcl_ios::binary);
    rgtl_serialize_ostream saver(fout);
    saver << *(*levels_iter)->tokens;
    delete (*levels_iter)->tokens;
    (*levels_iter)->tokens = 0;
  }

  // put the samples (of refine_quad_next) in the appropriate quad...
  delete[] pointsLevels;
  // move to next block
  this->refine_quad_next = this->refine_quad_next->quadRight;
  if (!this->refine_quad_next)
  {
    this->refine_quad_base = this->refine_quad_base->quadUp;
    this->refine_quad_next = this->refine_quad_base;
  }

  return output_counter;
}

template <unsigned int N>
void rtvl_level_refine_internal<N>::refine_block(rgtl_object_array_points<N>& blockPoints,
  rtvl_level_refine_level<N>**(&pointsLevels), double refine_bounds[4])
{
  unsigned int n = blockPoints.get_number_of_points();
  vnl_matrix_fixed<double, N, N> zero(0.0);
  vcl_vector<vnl_matrix_fixed<double, N, N> > tensors;
  tensors.resize(n, zero);

  rtvl_weight_smooth<N> tvw(this->current_scale);

  vote_count = 0;
  for (unsigned int i = 0; i < n; ++i)
  {
    if (pointsLevels[i]->token_calculated[i - pointsLevels[i]->offset])
    {
      continue;
    }

    vnl_vector_fixed<double, N> votee_location;
    blockPoints.get_point(i, votee_location.data_block());

    // if point not within refine_bounds, skip
    if (votee_location[0] < refine_bounds[0] || votee_location[0] > refine_bounds[1] ||
      votee_location[1] < refine_bounds[2] || votee_location[1] > refine_bounds[3])
    {
      continue;
    }

    // setup the votee and mark as calculated (so we won't repeat)
    rtvl_votee<N> votee(votee_location, tensors[i]);
    pointsLevels[i]->token_calculated[i - pointsLevels[i]->offset] = true;

    // Lookup points within reach of the votee.
    vcl_vector<int> voter_ids;
    int num_voters =
      objects->query_sphere(votee_location.data_block(), 3 * this->current_scale, voter_ids);

    // Cast a vote at every votee.
    vnl_vector_fixed<double, N> voter_location;
    for (int vi = 0; vi < num_voters; vi++)
    {
      unsigned int j = voter_ids[vi];
      if (j == i)
      {
        continue;
      }
      blockPoints.get_point(j, voter_location.data_block());
      rtvl_voter<N> voter(
        voter_location, pointsLevels[j]->calculate_tokens->at(j - pointsLevels[j]->offset));
      rtvl_vote(voter, votee, tvw, true);
    }
    vote_count += num_voters;
    // Set refined token
    pointsLevels[i]->tokens->tokens[i - pointsLevels[i]->offset].set_tensor(tensors[i]);
  }
}

template <unsigned int N>
void rtvl_level_refine_internal<N>::select_block_samples(vcl_vector<rtvl_level_refine_level<N>*>& levels_vector,
  rgtl_object_array_points<N>& blockPoints, rtvl_level_refine_level<N>**(&pointsLevels),
  double selection_bounds[4])
{

  unsigned int n = blockPoints.get_number_of_points();

  // actually compute the number of tokens we have tokens for

  // Queue the tokens ordered from most to least salient.
  typedef vcl_multimap<double, unsigned int, vcl_greater<double> > saliency_map_type;
  saliency_map_type saliency_map;
  vcl_vector<saliency_map_type::iterator> saliency_map_index;
  saliency_map_index.resize(n, saliency_map.end());
  for (unsigned int i = 0; i < n; i++)
  {
    rtvl_tensor<N> const& tensor = pointsLevels[i]->tokens->tokens[i - pointsLevels[i]->offset];
    double s = tensor.lambda(0);
    if (!pointsLevels[i]->token_calculated[i - pointsLevels[i]->offset])
    {
      s = -1; // be clear which sample we will not select
    }

    saliency_map_type::value_type entry(s, i);
    saliency_map_index[i] = saliency_map.insert(entry);
  }

  // go through current list of quad "levels" and mask samples already selected
  typename vcl_vector<rtvl_level_refine_level<N>*>::iterator level_iter = levels_vector.begin();
  for (; level_iter != levels_vector.end(); level_iter++)
  {
    vcl_vector<unsigned int>::iterator selection_iter = (*level_iter)->selection.begin();
    for (; selection_iter != (*level_iter)->selection.end(); selection_iter++)
    {
      double p[N];
      (*level_iter)->tokens->points.get_point(*selection_iter, p);
      vcl_vector<int> masked_ids;
      int num_masked = objects->query_sphere(p, this->current_scale * this->mask_size, masked_ids);
      // Remove the masked points.
      for (int i = 0; i < num_masked; i++)
      {
        saliency_map_type::iterator& mi = saliency_map_index[masked_ids[i]];
        if (mi != saliency_map.end())
        {
          saliency_map.erase(mi);
          mi = saliency_map.end();
        }
      }
    }
  }

  while (!saliency_map.empty() && saliency_map.begin()->first >= 0)
  {
    // Select the next-most-salient sample.
    unsigned int id = saliency_map.begin()->second;

    double p[N];
    blockPoints.get_point(id, p);

    // only save as selection if within input bounds
    if (p[0] >= selection_bounds[0] && p[0] <= selection_bounds[1] && p[1] >= selection_bounds[2] &&
      p[1] <= selection_bounds[3])
    {
      pointsLevels[id]->selection.push_back(id - pointsLevels[id]->offset);
    }
    else if (p[0] < selection_bounds[0] || p[1] < selection_bounds[2])
    {
      saliency_map_type::iterator& mi = saliency_map_index[saliency_map.begin()->second];
      if (mi != saliency_map.end())
      {
        saliency_map.erase(mi);
        mi = saliency_map.end();
        continue;
      }
    }

    // Lookup points covered by the masking sphere.
    vcl_vector<int> masked_ids;
    int num_masked = objects->query_sphere(p, this->current_scale * this->mask_size, masked_ids);

    // Remove the masked points.
    for (int i = 0; i < num_masked; i++)
    {
      saliency_map_type::iterator& mi = saliency_map_index[masked_ids[i]];
      if (mi != saliency_map.end())
      {
        saliency_map.erase(mi);
        mi = saliency_map.end();
      }
    }
  }

  // DO NOT save samples at quad depth above where we are processing!!!!

  // will now have all samples selected for this->refine_quad_next; now actually
  // collect the results, and place in the next level
  vcl_vector<rtvl_level_refine_level<N>*> refine_quad_levels_vector;
  this->refine_quad_next->gather_levels(this->current_level, refine_quad_levels_vector);

  typename vcl_vector<rtvl_level_refine_level<N>*>::const_iterator levels_iter =
    refine_quad_levels_vector.begin();
  for (; levels_iter != refine_quad_levels_vector.end(); levels_iter++)
  {
    vcl_vector<rtvl_level_refine_level<N>*> levels_subset;
    levels_subset.push_back(*levels_iter);
    rtvl_level_refine_quadtree<N>* next_level_quad = (*levels_iter)->quad;
    if (this->save_next_level_samples_in_parent)
    {
      // following IF is ALWAYS true, since we do NOT save samples above where we are processing
      //if (this->refine_quad_next->tree_depth > (*levels_iter)->quad->tree_depth)
      // then next 3 "levels" should be combined with this level
      levels_subset.push_back(*(++levels_iter));
      levels_subset.push_back(*(++levels_iter));
      levels_subset.push_back(*(++levels_iter));
      // temporary check
      //rtvl_level_refine_quadtree<N> *parent = next_level_quad->parent;
      //vcl_vector< rtvl_level_refine_level<N>* >::const_iterator levels_subset_iter = levels_subset.begin();
      //for (; levels_subset_iter != levels_subset.end(); levels_subset_iter++)
      //  {
      //  if ((*levels_subset_iter)->quad->parent != parent)
      //    {
      //    vcl_cerr << "Parents don't match!!!!!!!\n";
      //    return;
      //    }
      //  }
      next_level_quad = next_level_quad->parent;
    }

    next_level_quad->add_selections_for_next_level(this->current_scale * this->scale_multiplier,
      this->scale_multiplier, this->current_level + 1, levels_subset);
  }
}

template <unsigned int N>
bool rtvl_level_refine_internal<N>::last_level()
{
  // find maximum dimension
  double maxLength = 0;
  for (unsigned int i = 0; i < N; i += 2)
  {
    if (this->data_bounds[i + 1] - this->data_bounds[i] > maxLength)
    {
      maxLength = this->data_bounds[i + 1] - this->data_bounds[i];
    }
  }
  maxLength *= 1.01;

  // This is the last level if the scale covers the entire bounds.
  return maxLength <= (6.0 * this->current_scale);
}

template <unsigned int N>
void rtvl_level_refine_internal<N>::extract_tokens(
  int level, double* bounds_in, rtvl_tokens<N>& out) const
{
  vcl_vector<rtvl_level_refine_level<N>*> levels_vector;
  if (!bounds_in)
  {
    // get entire level
    this->tree->gather_levels(level, levels_vector);
  }
  else
  {
    this->tree->gather_levels_within_bounds(level, levels_vector, bounds_in);
  }

  // Count the salient tokens.
  unsigned int number_of_salient_points = 0;
  typename vcl_vector<rtvl_level_refine_level<N>*>::iterator level_iter = levels_vector.begin();
  for (; level_iter != levels_vector.end(); level_iter++)
  {
    // if not loaded, load...
    if ((*level_iter)->number_of_points > 0)
    {
      if ((*level_iter)->tokens == 0)
      {
        (*level_iter)->tokens = new rtvl_tokens<N>;
        vcl_ifstream fin((*level_iter)->filename.c_str(), vcl_ios::in | vcl_ios::binary);
        rgtl_serialize_istream loader(fin);
        loader >> *(*level_iter)->tokens;
      }
      vcl_vector<rtvl_tensor<N> >& tokens = (*level_iter)->tokens->tokens;
      if (!bounds_in || ((*level_iter)->quad->bounds[0] >= bounds_in[0] &&
                          (*level_iter)->quad->bounds[1] <= bounds_in[1] &&
                          (*level_iter)->quad->bounds[2] >= bounds_in[2] &&
                          (*level_iter)->quad->bounds[3] <= bounds_in[3]))
      {
        for (size_t i = 0; i < tokens.size(); i++)
        {
          if (tokens[i].lambda(0) > 1.0)
          {
            number_of_salient_points++;
          }
        }
      }
      else // only partially within bounds, so only count tokens within bounds
      {
        rgtl_object_array_points<N>& token_points = (*level_iter)->tokens->points;
        for (int i = 0; i < static_cast<int>(tokens.size()); i++)
        {
          if (tokens[i].lambda(0) > 1.0)
          {
            double p[N];
            token_points.get_point(i, p);
            if (p[0] >= bounds_in[0] && p[0] <= bounds_in[1] && p[1] >= bounds_in[2] &&
              p[1] <= bounds_in[3])
            {
              number_of_salient_points++;
            }
          }
        }
      }
    }
  }

  // Copy the salient tokens into the output.
  int count_index = 0;
  out.points.set_number_of_points(number_of_salient_points);
  out.tokens.resize(number_of_salient_points);
  out.scale = -1;
  if (number_of_salient_points)
  {
    for (level_iter = levels_vector.begin(); level_iter != levels_vector.end(); level_iter++)
    {
      if ((*level_iter)->number_of_points > 0)
      {
        if (out.scale == -1)
        {
          out.scale = (*level_iter)->tokens->scale;
        }
        vcl_vector<rtvl_tensor<N> >& tokens = (*level_iter)->tokens->tokens;
        rgtl_object_array_points<N>& token_points = (*level_iter)->tokens->points;
        for (int i = 0; i < static_cast<int>(tokens.size()); i++)
        {
          if (tokens[i].lambda(0) > 1.0)
          {
            double p[N];
            token_points.get_point(i, p);
            if (!bounds_in || (p[0] >= bounds_in[0] && p[0] <= bounds_in[1] &&
                                p[1] >= bounds_in[2] && p[1] <= bounds_in[3]))
            {
              out.points.set_point(count_index, p);
              out.tokens[count_index++] = tokens[i];
            }
          }
        }
        // excessively deleting (and thus reloading)... but will have to do until
        // more sophisticated method is in place
        delete (*level_iter)->tokens;
        (*level_iter)->tokens = 0;
      }
    }
  }
}

template <unsigned int N>
double rtvl_level_refine_internal<N>::get_level_scale(int level)
{
  if (level < static_cast<int>(this->level_scale.size()))
  {
    return this->level_scale[level];
  }
  return 0;
}

template <unsigned int N>
rtvl_level_refine<N>::rtvl_level_refine(unsigned int num_points, double* points, double* bounds)
  : internal_(0)
{
  vcl_auto_ptr<internal_type> internal(new internal_type(this));
  internal->init(num_points, points, bounds);
  this->internal_ = internal.release();
}

template <unsigned int N>
rtvl_level_refine<N>::~rtvl_level_refine()
{
  delete this->internal_;
}

template <unsigned int N>
void rtvl_level_refine<N>::set_mask_size(double f)
{
  this->internal_->set_mask_size(f);
}

template <unsigned int N>
double rtvl_level_refine<N>::compute_scale() const
{
  // only allow computing of scale if < 0, guaranteeing it is done on the
  // initial level
  if (this->internal_->get_current_scale() >= 0)
  {
    vcl_cerr << "Scale can only be computed \"immediately\" after init\n";
    return -1;
  }

  this->internal_->select_scale();
  return this->internal_->get_current_scale();
}

template <unsigned int N>
void rtvl_level_refine<N>::set_scale(double scale)
{
  if (scale <= 0)
  {
    vcl_cerr << "scale must be > 0!\n";
    return;
  }
  this->internal_->set_scale(scale);
}

template <unsigned int N>
void rtvl_level_refine<N>::build_tree()
{
  this->internal_->build_tree();
}

template <unsigned int N>
int rtvl_level_refine<N>::initialize_refine_level(
  unsigned int memory_limit, int requested_min_depth) const
{
  return this->internal_->initialize_refine_level(memory_limit, requested_min_depth);
}

template <unsigned int N>
int rtvl_level_refine<N>::refine_next_block(const char* base_filename)
{
  return this->internal_->refine_next_block(base_filename);
}

template <unsigned int N>
void rtvl_level_refine<N>::get_tokens(int level, double* bounds, rtvl_tokens<N>& tokens) const
{
  this->internal_->extract_tokens(level, bounds, tokens);
}

template <unsigned int N>
double rtvl_level_refine<N>::get_level_scale(int level)
{
  return this->internal_->get_level_scale(level);
}

template <unsigned int N>
unsigned int rtvl_level_refine<N>::get_vote_count() const
{
  return this->internal_->get_vote_count();
}

template <unsigned int N>
unsigned int rtvl_level_refine<N>::estimate_refine_levels()
{
  return this->internal_->estimate_refine_levels();
}

template <unsigned int N>
bool rtvl_level_refine<N>::next_scale()
{
  bool last_level = this->internal_->last_level();
  this->internal_->next_scale();
  return !last_level; // return  wether NOT last scale
}

#define RTVL_LEVEL_REFINE_INSTANTIATE(N)                                                                 \
  template class rtvl_level_refine_internal<N>;                                                          \
  template class rtvl_level_refine<N>

#endif
