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

// based on example at https://rosettacode.org/wiki/K-d_tree#C
// see also https://en.wikipedia.org/wiki/Quickselect
//
// currently used for the "fine" airbrush mode, fill edge feathering, and
// reverse color lookup table

int KDtree::distance(const node_type *a, const node_type *b)
{
  int d = 0;

  for (int dim = 0; dim < 3; dim++)
  {
    const int temp = a->x[dim] - b->x[dim];
    d += temp * temp;
  }

  return d;
}

void KDtree::swapNodes(node_type *a, node_type *b)
{
  node_type temp;

  temp = *a;
  *a = *b;
  *b = temp;
}

KDtree::node_type *KDtree::median(node_type *left, node_type *right,
                                  const int axis)
{
  if (right < left)
    return 0;

  node_type *p; 
  node_type *midpoint = left + (right - left) / 2; 

  while (true)
  {
    const int pivot = midpoint->x[axis];
    node_type *temp = left;

    swapNodes(midpoint, right);

    for (p = left; p <= right; p++)
    {
      if (right == left)
        return left;

      if (p->x[axis] < pivot)
      {
        if (p != temp)
          swapNodes(p, temp);

        temp++;
      }
    }

    swapNodes(temp, right);

    if (midpoint->x[axis] == temp->x[axis])
      return temp;
    else if (midpoint->x[axis] < temp->x[axis])
      right = temp - 1;
    else
      left = temp + 1;
  }
}

KDtree::node_type *KDtree::build(node_type *root,
                           const int length, const int axis)
{
  node_type *node;

  if (length == 0)
    return 0;

  if ((node = median(root, root + length - 1, axis)))
  {
    node->left = build(root, node - root, (axis + 1) % 3);
    node->right = build(node + 1, root + length - (node + 1), (axis + 1) % 3);
  }

  return node;
}

void KDtree::nearest(node_type *root, node_type *test_node,
                     node_type **best_node, int *best_distance, const int axis)
{
  if (root == 0)
    return;

  const int d = distance(root, test_node);
  const int dx = root->x[axis] - test_node->x[axis];

  if ((*best_node == 0) || d < *best_distance)
  {
    *best_distance = d;
    *best_node = root;
  }

  if (*best_distance == 0)
    return;

  nearest(dx > 0 ? root->left : root->right,
          test_node, best_node, best_distance, (axis + 1) % 3);

  if (*best_distance > dx * dx)
  {
    nearest(dx <= 0 ? root->left : root->right,
            test_node, best_node, best_distance, (axis + 1) % 3);
  }
}

