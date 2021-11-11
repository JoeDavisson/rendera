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

#include "Quadtree.H"

Quadtree::Quadtree()
{
  root = new node_type;
  root->rx = 0;
  root->ry = 0;
  root->value = 0;

  for(int i = 0; i < 4; i++)
    root->child[i] = 0;
}

Quadtree::~Quadtree()
{
  clear(root);
}

void Quadtree::clear(node_type *node)
{
  for(int i = 0; i < 4; i++)
    if(node->child[i])
      clear(node->child[i]);

  delete node;
}

void Quadtree::write(const int x, const int y, const float value)
{
  node_type *node = root;

  for(int i = 15; i >= 0; i--)
  {
    const int index = ((x >> i) & 1) << 0 |
                      ((y >> i) & 1) << 1;

    if(node->child[index])
    {
      node = node->child[index];
      continue;
    }

    node->child[index] = new node_type;
    node = node->child[index];
    node->rx = 0;
    node->ry = 0;
    node->value = 0;

    for(int j = 0; j < 4; j++)
      node->child[j] = 0;
  }

  node->rx = x;
  node->ry = y;
  node->value = value;
}

void Quadtree::writePath(const int x, const int y, const float value)
{
  node_type *node = root;

  for(int i = 15; i >= 0; i--)
  {
    const int index = ((x >> i) & 1) << 0 |
                      ((y >> i) & 1) << 1;

    if(node->child[index])
    {
      node = node->child[index];
      continue;
    }

    node->child[index] = new node_type;
    node = node->child[index];

    node->rx = x;
    node->ry = y;
    node->value = value;

    for(int j = 0; j < 4; j++)
      node->child[j] = 0;
  }
}

float Quadtree::read(const int x, const int y, int *rx, int *ry)
{
  node_type *node = root;

  for(int i = 15; i >= 0; i--)
  {
    const int index = ((x >> i) & 1) << 0 |
                      ((y >> i) & 1) << 1;

    if(node->child[index])
      node = node->child[index];
    else
      break;
  }

  *rx = node->rx;
  *ry = node->ry;

  return node->value;
}

