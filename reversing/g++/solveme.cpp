#include <iostream>

#define R(a) (m<(a),(QQ),(QQ)>::r)
#define RR(a,b) (n<b,a,b,a>::q)
#define RRR(a,b,c,d) RR(RR(a,c),RR(b,d))
#define RRRR(s,t,u,v,w,x,y,z) R(RRR(Q(s,w),Q(t,x),Q(u,y),Q(v,z)))
#define RRRP(s,t,u,v,w) RRRR(s,t,u,v,R(TT(P,w)),R(TT(P,J(w))), \
  R(TT(P,J(J(w)))),R(TT(P,J(J(J(w))))))
#define Q(a,b) (n<b,a,b,0>::s)
#define S static const int
#define NNN(q) NN(a,R(b),c,R(d),q)
#define NN(a,c,b,d,q) q = n<(a-1),(c-1),(b+1),(((a+b)>>1)+d)>::q ;
#define QQ ((1<<(1<<3))|1)
#define PT(k,a) key<a>::r
#define WW(a) PT(k,a)
#define WWJD(a) WW(a),WW(III(a)),WW(IV(a)),WW(IV(III(a)))
#define WWKD(a) g<a>::r,g<a+1>::r,g<a+(1<<1)>::r,g<a+(1<<1)|1>::r
#define P(a,b) WWKD(((a)<<2)),WWJD(b)
#define P0 1
#define P1 (P0*f)
#define P2 (P1*P1)
#define P3 (P1*P2)
#define P4 (P2*P2)
#define P5 (P1*P4)
#define P6 (P3*P3)
#define P7 (P3*P4)
#define S0(a,b) ((a)?(a):(b))

#define T(a, ...) a ## __VA_ARGS__
#define TT(a, ...) T(a, __VA_ARGS__)
#define J(x) T(I,x)
#define I(x) x+T(I,0)
#define II(x) I(I(x))
#define III(x) II(II(x))
#define IV(x) III(III(x))
#define I0 1
#define I1 2
#define I2 3
#define I3 4
#define I4 5
#define I5 6
#define I6 7
#define I7 8
#define I8 9
#define I9 0

template <int i>
struct key {
  S r = 0;
};

#define K(i,v) \
template <> \
struct key<i> { \
  S r = v; \
};

#include "key.h"

template <int a, int b, int c>
struct m {
  S r = (a-((a/b)*b));
};

template <int a, int b, int c, int d>
struct n {
  S NNN(q)
  S NNN(r)
  S NNN(s);
};

template <int a, int c, int d>
struct n<a,0,c,d> {
  S r = a;
  S q = c;
  S s = d;
};

template <int s, int t, int u, int v, int w, int x, int y, int z>
struct d {
  S r = RRRR(s,t,u,v,w,x,y,z);
};

template <int b>
struct g {
  S r = ((3 + g<b-1>::r*7) ^ (g<b-1>::r*2))&255;
};

template <>
struct g<0> {
  S r = 13;
};

template <int f, int n>
struct u {
  S r = u<S0((f+1)&7,8),((f)>>3)>::r;
};

template <int f>
struct u<f,1> {
  S r = RRRP(26,43,192,42,0)+RRRP(246,8,221,155,4);
};

template <int f>
struct u<f,0> {
  S r = RRRP(132,141,229,162,4)+RRRP(48,222,109,0,0);
};


template <int a, int b>
struct r {
  S rr = d<P(a,b)>::r;
};


template <int n>
struct gg {
  S r = R((u<n,n|2>::r)) - r<(n>>2),((n)&3)>::rr;
};

template <int n>
struct vvv {
  S r = gg<n>::r|gg<n+1>::r|gg<n+2>::r|gg<n+3>::r;
};

template <int n>
struct vv {
  S r = vvv<0>::r|vvv<4>::r|vvv<8>::r|vvv<12>::r;
};

int main() {
  if (!vv<0>::r) {
    int i;
    char skey[] = {key<0>::r, key<1>::r, key<2>::r, key<3>::r, key<4>::r, key<5>::r, key<6>::r, key<7>::r, key<8>::r, key<9>::r, key<10>::r, key<11>::r, key<12>::r, key<13>::r, key<14>::r, key<15>::r};
    for (i = 0; i < 1<<(1<<(1<<1)); i++)
      std::cout << skey[i];
    std::cout << "\n";
  }
  else {
    std::cout << "Wrong\n";
  }
  return 0;
}
