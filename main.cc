#include <unigui/theme/surface_style.h>
#include <cstdio>
#include <cassert>
using namespace unigui::theme;
int main(){
  ImVec4 wb(0.20f,0.20f,0.24f,0.5f);
  for(auto st:AllSurfaceStyles()){ assert(BackdropColor(wb,st).w==1.0f); }
  assert(BackdropColor(wb,SurfaceStyle::Glass).x < wb.x);
  assert(BackdropColor(wb,SurfaceStyle::Solid).x == wb.x);
  assert(BackdropColor(wb,SurfaceStyle::Minimal).x == wb.x);
  // frosted darkest among translucent
  assert(BackdropColor(wb,SurfaceStyle::Frosted).x <= BackdropColor(wb,SurfaceStyle::Glass).x);
  printf("BACKDROP OK\n");
  return 0;
}
