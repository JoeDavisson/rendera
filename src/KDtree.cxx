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

int KDtree::dist(node_type *a, node_type *b, int dim)
{
  int t;
  int d = 0;

  while (dim--)
  {
    t = a->x[dim] - b->x[dim];
    d += t * t;
  }

  return d;
}

void KDtree::swap(node_type *x, node_type *y)
{
  int temp[max_dim];
  int temp_index;

  temp_index = x->index;
  x->index = y->index;
  y->index = temp_index;

  std::copy(x->x, x->x + max_dim, temp);
  std::copy(y->x, y->x + max_dim, x->x);
  std::copy(temp, temp + max_dim, y->x);
}

KDtree::node_type *KDtree::median(node_type *begin, node_type *end, int index)
{
  if (end <= begin)
    return 0;

  node_type *p; 
  node_type *temp; 
  node_type *mid = begin + (end - begin) / 2; 

  while (true)
  {
    const int pivot = mid->x[index];

    swap(mid, end - 1);
    temp = begin;

    for (p = begin; p < end; p++)
    {
      if (end == begin + 1)
        return begin;

      if (p->x[index] < pivot)
      {
        if (p != temp)
          swap(p, temp);

        temp++;
      }
    }

    swap(temp, end - 1);

    if (temp == mid)
      return mid;
    else if (temp->x[index] > mid->x[index])
      end = temp;
    else
      begin = temp + 1;
  }
}

KDtree::node_type *KDtree::build(node_type *t,
                                 const int len, int i, const int dim)
{
  node_type *n;

  if (!len)
    return 0;

  if ((n = median(t, t + len, i)))
  {
    i = (i + 1) % dim;
    n->left = build(t, n - t, i, dim);
    n->right = build(n + 1, t + len - (n + 1), i, dim);
  }

  return n;
}

void KDtree::nearest(node_type *r, node_type *nd,
                     int i, const int dim, node_type **best, int *best_dist)
{
  if (r == 0)
    return;

  const int d = dist(r, nd, dim);
  const int dx = r->x[i] - nd->x[i];

  if (!*best || d < *best_dist)
  {
    *best_dist = d;
    *best = r;
  }

  if (!*best_dist)
    return;

  if (++i >= dim)
     i = 0;

  nearest(dx > 0 ? r->left : r->right, nd, i, dim, best, best_dist);

  if (dx * dx < *best_dist)
    nearest(dx > 0 ? r->right : r->left, nd, i, dim, best, best_dist);
}

