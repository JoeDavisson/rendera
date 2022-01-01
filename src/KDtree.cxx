/*
Copyright (c) 2021 Joe Davisson.

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

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "KDtree.H"

inline int KDtree::dist(node_type *a, node_type *b, int dim)
{
  int t;
  int d = 0;

  while(dim--)
  {
    t = a->x[dim] - b->x[dim];
    d += t * t;
  }

  return d;
}

inline void KDtree::swap(node_type *x, node_type *y)
{
  int temp[max_dim];

  std::memcpy(temp, x->x, sizeof(temp));
  std::memcpy(x->x, y->x, sizeof(temp));
  std::memcpy(y->x, temp, sizeof(temp));
}

KDtree::node_type *KDtree::median(node_type *start, node_type *end, int index)
{
  if(end <= start)
    return 0;

  node_type *p; 
  node_type *store; 
  node_type *md = start + (end - start) / 2; 

  while(true)
  {
    const int pivot = md->x[index];

    swap(md, end - 1);

    store = start;

    for(p = start; p < end; p++)
    {
      if(end == start + 1)
        return start;

      if(p->x[index] < pivot)
      {
        if(p != store)
          swap(p, store);

        store++;
      }
    }

    swap(store, end - 1);

    if(store == md)
      return md;
    else if(store->x[index] > md->x[index])
      end = store;
    else
      start = store + 1;
  }
}

KDtree::node_type *KDtree::make_tree(node_type *t, const int len, int i, const int dim)
{
  node_type *n;

  if(!len)
    return 0;

  if((n = median(t, t + len, i)))
  {
    i = (i + 1) % dim;
    n->left = make_tree(t, n - t, i, dim);
    n->right = make_tree(n + 1, t + len - (n + 1), i, dim);
  }

  return n;
}

void KDtree::nearest(node_type *r, node_type *nd, int i, const int dim, node_type **best, int *best_dist)
{
  if(!r)
    return;

  const int d = dist(r, nd, dim);
  const int dx = r->x[i] - nd->x[i];

  if(!*best || d < *best_dist)
  {
    *best_dist = d;
    *best = r;
  }

  if(!*best_dist)
    return;

  if(++i >= dim)
     i = 0;

  nearest(dx > 0 ? r->left : r->right, nd, i, dim, best, best_dist);

  if(dx * dx < *best_dist)
    nearest(dx > 0 ? r->right : r->left, nd, i, dim, best, best_dist);
}

