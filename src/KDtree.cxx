/*
Copyright (c) 2024 Joe Davisson.

This file is part of Rendera.

Rendera is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Rendera is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rendera; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "KDtree.H"

// based on example at https://rosettacode.org/wiki/K-d_tree

int KDtree::distance(const node_type *a, const node_type *b)
{
  int t;
  int d = 0;
  int dim = 3;

  while (dim > 0)
  {
    dim--;
    t = a->x[dim] - b->x[dim];
    d += t * t;
  }

  return d;
}

void KDtree::swapNodes(node_type *a, node_type *b)
{
  int temp[3];

  std::swap(a->index, b->index);
  std::copy(a->x, a->x + 3, temp);
  std::copy(b->x, b->x + 3, a->x);
  std::copy(temp, temp + 3, b->x);
}

KDtree::node_type *KDtree::median(node_type *first, node_type *last,
                                  const int index)
{
  if (last <= first)
    return 0;

  node_type *p; 
  node_type *temp; 
  node_type *mid = first + (last - first) / 2; 

  while (true)
  {
    const int pivot = mid->x[index];

    swapNodes(mid, last - 1);
    temp = first;

    for (p = first; p < last; p++)
    {
      if (last == first + 1)
        return first;

      if (p->x[index] < pivot)
      {
        if (p != temp)
          swapNodes(p, temp);

        temp++;
      }
    }

    swapNodes(temp, last - 1);

    if (temp == mid)
      return mid;
    else if (temp->x[index] > mid->x[index])
      last = temp;
    else
      first = temp + 1;
  }

  return 0;
}

KDtree::node_type *KDtree::build(node_type *tree, const int length, int i)
{
  node_type *node;

  if (length == 0)
    return 0;

  if ((node = median(tree, tree + length, i)))
  {
    i = (i + 1) % 3;
    node->left = build(tree, node - tree, i);
    node->right = build(node + 1, tree + length - (node + 1), i);
  }

  return node;
}

void KDtree::nearest(node_type *r, node_type *node,
                     int i, node_type **best, int *best_dist)
{
  if (r == 0)
    return;

  const int d = distance(r, node);
  const int dx = r->x[i] - node->x[i];

  if ((*best == 0) || d < *best_dist)
  {
    *best_dist = d;
    *best = r;
  }

  if (*best_dist == 0)
    return;

  i++;

  if (i >= 3)
     i = 0;

  if (dx > 0)
    nearest(r->left, node, i, best, best_dist);
  else
    nearest(r->right, node, i, best, best_dist);

  if (*best_dist > dx * dx)
  {
    if(dx > 0)
      nearest(r->right, node, i, best, best_dist);
    else
      nearest(r->left, node, i, best, best_dist);
  }
}

