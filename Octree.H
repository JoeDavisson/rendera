/*
Copyright (c) 2014 Joe Davisson.

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

#ifndef OCTREE_H
#define OCTREE_H

#include "Rendera.H"

class Octree
{
public:
  struct node_t
  {
    float value;
    struct node_t *child[8];
  };

  Octree();
  virtual ~Octree();

  void clear(struct node_t *);
  void write(const int &, const int &, const int &, const float &);
  void writePath(const int &, const int &, const int &, const float &);
  float read(const int &, const int &, const int &);

  struct node_t *root;
};

#endif

