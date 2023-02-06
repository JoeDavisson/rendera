/*
Copyright (c) 2023 Joe Davisson.

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

#ifndef KDTREE_H
#define KDTREE_H

class KDtree
{
public:
  static const int max_dim = 3;

  struct node_type
  {
    int x[max_dim];
    int index;
    node_type *left, *right;
  };

  static int dist(node_type *, node_type *, int);
  static void swap(node_type *, node_type *);
  static node_type *median(node_type *, node_type *, int);
  static node_type *make_tree(node_type *, const int, int, const int);
  static void nearest(node_type *, node_type *, int, const int, node_type **, int *);

private:
  KDtree() { }
  ~KDtree() { }
};

#endif

