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

struct _octree
{
  int value;
  struct _octree *parent;
  struct _octree *next[8];
};

static void octree_add(struct _octree *octree, int x, int y, int z, int value)
{
}

static void octree_delete(struct _octree *octree, int x, int y, int z)
{
}

static int octree_read(struct _octree *octree, int x, int y, int z)
{
  return 0;
}

#endif
