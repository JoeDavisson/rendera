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

#include "Octree.H"

void Octree::Octree()
{
  node = new node_t[16777216];

  // create full linear octree
  int i;

  for(i = 0; i < 16777216; i++)
  {
    node[i]->value = -1;
    node[i]->child[0] = 8 * n + 1;
    node[i]->child[1] = 8 * n + 2;
    node[i]->child[2] = 8 * n + 3;
    node[i]->child[3] = 8 * n + 4;
    node[i]->child[4] = 8 * n + 5;
    node[i]->child[5] = 8 * n + 6;
    node[i]->child[6] = 8 * n + 7;
    node[i]->child[7] = 8 * n + 8;
  }
}

void Octree::~Octree()
{
  delete[] node;
}

void Octree::add(int color, float value)
{
  int i;

  for(i = 7; i >= 0; i--)
  {
  }
}

void Octree::remove(int color)
{
}

