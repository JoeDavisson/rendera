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

#ifndef KDTREE_H
#define KDTREE_H

class KDtree
{
public:
  struct node_type
  {
    int x[3] { 0, 0, 0 };
    int index { 0 };
    node_type *left { 0 };
    node_type *right { 0 };
  };

  static int distance(const node_type *, const node_type *);
  static void swapNodes(node_type *, node_type *);
  static node_type *median(node_type *, node_type *, const int);
  static node_type *build(node_type *, const int, const int);
  static void nearest(node_type *, node_type *, node_type **, int *, const int);

private:
  KDtree() { }
  ~KDtree() { }
};

#endif

