/*
    motor2d

    Copyright (C) 2015 Florian Kesseler

    This project is free software; you can redistribute it and/or modify it
    under the terms of the MIT license. See LICENSE.md for details.
*/

#include <stdlib.h>
#include <string.h>
#include "mesh.h"
#include "graphics.h"
#include "../math/minmax.h"

// TODO: What happens when changing the number of vertices after setting a custom vertex map or draw range?
// TODO: Apparently, LOVE does nothing about this situation, so that's what I'm gonna do, to.

void graphics_Mesh_new(graphics_Mesh *mesh, size_t vertexCount, graphics_Vertex const* vertices, graphics_Image const* texture, graphics_MeshDrawMode mode, bool useVertexColor) {
  mesh->texture = texture;
  mesh->drawMode = mode;

  mesh->vertices = 0;
  mesh->indices = 0;
  mesh->indexBufferSize = 0;

  mesh->useVertexColor = useVertexColor;
  mesh->useDrawRange = false;

  mesh->customIndexBuffer = false;

  glGenVertexArrays(1, &mesh->vertexArray);
  glBindVertexArray(mesh->vertexArray);
  glGenBuffers(1, &mesh->indexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
  glGenBuffers(1, &mesh->vertexBuffer);
  graphics_Mesh_setVertices(mesh, vertexCount, vertices);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(graphics_Vertex), 0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(graphics_Vertex), (GLvoid const*)(2*sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(graphics_Vertex), (GLvoid const*)(4*sizeof(float)));
}


void graphics_Mesh_free(graphics_Mesh *mesh) {
  glDeleteBuffers(1,      &mesh->vertexBuffer);
  glDeleteBuffers(1,      &mesh->indexBuffer);
  glDeleteVertexArrays(1, &mesh->vertexArray);
  free(mesh->indices);
  free(mesh->vertices);
}


void graphics_Mesh_setVertices(graphics_Mesh *mesh, size_t vertexCount, graphics_Vertex const* vertices) {
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(graphics_Vertex), vertices, GL_DYNAMIC_DRAW);

  if(mesh->vertexCount != vertexCount) {
    free(mesh->vertices);
    mesh->vertices = malloc(sizeof(graphics_Vertex) * vertexCount);
    mesh->vertexCount = vertexCount;
  }

  memcpy(mesh->vertices, vertices, vertexCount * sizeof(graphics_Vertex));

  if(!mesh->customIndexBuffer) {
    graphics_Mesh_setVertexMap(mesh, 0, 0);
  }
}


graphics_Vertex const* graphics_Mesh_getVertices(graphics_Mesh const* mesh, size_t *count) {
  *count = mesh->vertexCount;
  return mesh->vertices;
}



#define makeFillDefaultIndexBufferFunc(type)                             \
  static void fillDefaultIndexBuffer_ ## type(void *out, size_t count) { \
    type *t = (type*)out;                                                \
    for(size_t i = 0; i < count; ++i) {                                  \
      t[i] = (type)i;                                                    \
    }                                                                    \
  }
makeFillDefaultIndexBufferFunc(uint8_t)
makeFillDefaultIndexBufferFunc(uint16_t)
makeFillDefaultIndexBufferFunc(uint32_t)
#undef makeFillDefaultIndexBufferFunc


static size_t indexSize(graphics_Mesh const* mesh) {
  if(mesh->vertexCount > 0xFFFF) {
    return sizeof(uint32_t);
  } else if(mesh->vertexCount > 0xFF) {
    return sizeof(uint16_t);
  }
  return sizeof(uint8_t);
}


static void createDefaultIndexBuffer(graphics_Mesh *mesh) {
  mesh->customIndexBuffer = false;

  size_t idxSize = indexSize(mesh);
  size_t newSize = idxSize * mesh->vertexCount;
  if(newSize != mesh->indexBufferSize) {
    free(mesh->indices);
    mesh->indices = malloc(newSize);
    mesh->indexBufferSize = newSize;
  }

  switch(idxSize) {
  case 1:
    fillDefaultIndexBuffer_uint8_t(mesh->indices, mesh->vertexCount);
    break;
  case 2:
    fillDefaultIndexBuffer_uint16_t(mesh->indices, mesh->vertexCount);
    break;
  case 4:
    fillDefaultIndexBuffer_uint32_t(mesh->indices, mesh->vertexCount);
    break;
  }

  mesh->customIndexBuffer = false;
}


#define makeFillCustomIndexBufferFunc(type)                                                      \
  static void fillCustomIndexBuffer_ ## type(graphics_Mesh *out, size_t count, uint32_t const* indices) { \
    type *t = (type*)out->indices;                                                                        \
    for(size_t i = 0; i < count; ++i) {                                                          \
      t[i] = (type)indices[i];                                                                          \
    }                                                                                            \
  }
makeFillCustomIndexBufferFunc(uint8_t)
makeFillCustomIndexBufferFunc(uint16_t)
makeFillCustomIndexBufferFunc(uint32_t)
#undef makeFillCustomIndexBufferFunc

static void createCustomIndexBuffer(graphics_Mesh *mesh, size_t count, uint32_t const* indices) {
  mesh->customIndexBuffer = true;

  size_t idxSize = indexSize(mesh);
  size_t newSize = idxSize * count;
  if(newSize != mesh->indexBufferSize) {
    free(mesh->indices);
    mesh->indices = malloc(newSize);
    mesh->indexBufferSize = newSize;
  }

  switch(idxSize) {
  case 1:
    fillCustomIndexBuffer_uint8_t(mesh, count, indices);
    break;
  case 2:
    fillCustomIndexBuffer_uint16_t(mesh, count, indices);
    break;
  case 4:
    fillCustomIndexBuffer_uint32_t(mesh, count, indices);
    break;
  }
}


void graphics_Mesh_setVertexMap(graphics_Mesh *mesh, size_t count, uint32_t const* indices) {
  if(indices == 0) {
    createDefaultIndexBuffer(mesh);
  } else {
    createCustomIndexBuffer(mesh, count, indices);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBufferSize, mesh->indices, GL_DYNAMIC_DRAW);
}


// Somewhat weird interface: return type must be cast to pointer to smallest unsigned
// integer const that can represent the value returned in *count.
// Weird, but efficient!
void const* graphics_Mesh_getVertexMap(graphics_Mesh const* mesh, size_t *count) {
  *count = mesh->indexBufferSize;
  return mesh->indices;
}


static graphics_Quad const fullQuad = {0.0f, 0.0f, 1.0f, 1.0f};
static float const defaultColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
static GLenum const glTypes[] = {GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, 0, GL_UNSIGNED_INT};
void graphics_Mesh_draw(graphics_Mesh const* mesh, float x, float y, float r, float sx, float sy, float ox, float oy, float kx, float ky) {

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mesh->texture->texID);
  mat4x4 tr2d;
  m4x4_newTransform2d(&tr2d, x, y, r, sx, sy, ox, oy, kx, ky);

  int start = clamp(mesh->useDrawRange ? mesh->drawStart : 0, 0, mesh->indexBufferSize);
  int end   = clamp(mesh->useDrawRange ? mesh->drawEnd : mesh->indexBufferSize - 1, start, mesh->indexBufferSize - 1);
  int count = end - start + 1;

  size_t idxSize = indexSize(mesh);
  graphics_drawArray(
    &fullQuad,
    &tr2d,
    mesh->vertexArray,
    mesh->indexBuffer,
    start * idxSize,
    count,
    mesh->drawMode,
    glTypes[idxSize-1],
    mesh->useVertexColor ? defaultColor : graphics_getColor(),
    1.0f,
    1.0f,
    mesh->useVertexColor
  );
}


void graphics_Mesh_setVertexColors(graphics_Mesh *mesh, bool use) {
  mesh->useVertexColor = use;
}


bool graphics_Mesh_getVertexColors(graphics_Mesh const *mesh) {
  return mesh->useVertexColor;
}


void graphics_Mesh_resetDrawRange(graphics_Mesh *mesh) {
  mesh->useDrawRange = false;
}


void graphics_Mesh_setDrawRange(graphics_Mesh *mesh, int min, int max) {
  mesh->useDrawRange = true;
  mesh->drawStart = min;
  mesh->drawEnd = max;
}


bool graphics_Mesh_getDrawRange(graphics_Mesh const* mesh, int *min, int *max) {
  if(mesh->useDrawRange) {
    *min = mesh->drawStart;
    *max = mesh->drawEnd;
    return true;
  }
  return false;
}


graphics_Vertex const* graphics_Mesh_getVertex(graphics_Mesh const *mesh, size_t index) {
  return mesh->vertices + index;
}


void graphics_Mesh_setVertex(graphics_Mesh *mesh, size_t index, graphics_Vertex const *vertex) {
  memcpy(mesh->vertices + index, vertex, sizeof(*vertex));
  glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
  glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(*vertex), sizeof(*vertex), vertex);
}


size_t graphics_Mesh_getVertexCount(graphics_Mesh const *mesh) {
  return mesh->vertexCount;
}


void graphics_Mesh_setDrawMode(graphics_Mesh *mesh, graphics_MeshDrawMode mode) {
  mesh->drawMode = mode;
}


graphics_MeshDrawMode graphics_Mesh_getDrawMode(graphics_Mesh const *mesh) {
  return mesh->drawMode;
}
