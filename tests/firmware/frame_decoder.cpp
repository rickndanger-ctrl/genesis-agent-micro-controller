#include "../../firmware/GenesisMicro/FrameDecoder.h"
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
int main() {
  FrameDecoder d;
  std::vector<std::string> seen;
  auto handler=[&](const char *s,size_t n){seen.emplace_back(s,n);};
  auto feed=[&](std::string s,size_t chunk) {
    for(size_t i=0;i<s.size();i+=chunk) {
      uint8_t p[63]={2,0};p[1]=std::min(chunk,s.size()-i);
      memcpy(p+2,s.data()+i,p[1]);assert(d.receive(p,63,handler));
    }
  };
  std::string request=R"({"method":"v.oai.thstatus","params":[{"id":0,"c":3199320,"b":0.5}],"id":998})";
  for(size_t chunk=1;chunk<=61;++chunk) { seen.clear();feed(request,chunk);assert(seen.size()==1 && seen[0]==request); }
  seen.clear();std::string escaped=R"({"method":"test","params":"brace } and \" and slash \\"})";
  feed(escaped+"\r\n"+request+"\n",17);assert(seen.size()==2 && seen[0]==escaped);
  uint8_t bad[63]={2,62};assert(!d.receive(bad,63,handler));assert(d.used==0);
  bad[1]=1;bad[2]='{';assert(!d.receive(bad,62,handler));
  seen.clear();feed(request,61);assert(seen.size()==1);
  d.reset();uint8_t p[63]={2,61};memset(p+2,'x',61);p[2]='{';assert(d.receive(p,63,handler));p[2]='x';
  bool overflow=false;for(int i=0;i<70;++i) if(!d.receive(p,63,handler)) {overflow=true;break;}assert(overflow&&d.used==0);
  seen.clear();feed(request,61);assert(seen.size()==1);
}
