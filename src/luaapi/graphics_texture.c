#include "graphics_texture.h"
#include "graphics_canvas.h"

graphics_Image* l_graphics_toTextureOrError(lua_State* state, int index) {
  if(l_graphics_isImage(state, index)) {
    return &l_graphics_toImage(state, index)->image;
  } else if(l_graphics_isCanvas(state, index)) {
    return &l_graphics_toCanvas(state, index)->image;
  }

  lua_pushstring(state, "Texture needed");
  lua_error(state); // does not return
  return 0;         // satisfy compiler
}
