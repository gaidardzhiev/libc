/*
 * Copyright (C) 2026 Ivan Gaydardzhiev
 * Licensed under the GPL-3.0-only
 */

#include "stdio.h"
#include "malloc.h"
#include "unistd.h"

#define __(a,b) a##b
#define _N 0
#define _1 1
#define _Z(p) ((p)==_N)
#define _W 65536
#define _G 32
#define _R(p,n) realloc((p),(n))
#define _M(n) malloc(n)
#define _F(p) free(p)
#define _RD(b,n) read(0,(b),(n))
#define _WR(b,n) write(1,(b),(n))
#define _WE(b,n) write(2,(b),(n))
#define _MS(a,v,n) memset((a),(v),(n))
#define _X(v) ((unsigned char)(v))
#define _CS 4096
#define _TS 4096

typedef unsigned char _u;
typedef unsigned int _s;
static _u* _tp = (_u*)_N;
static _s _tc = _N;
static _s _tl = _N;
static _s* _bs = (_s*)_N;
static _s _bd = _N;
static _s _bl = _N;
static _u* _pg = (_u*)_N;
static _s _pl = _N;
static void _xe(int _c) {
	_u _b[1];
	_b[_N]=(_u)_c;
	_WE(_b,_1);
}

static void _grow(void) {
	_s _ns = _tl + _G;
	_u* _q = (_u*)_R(_tp, _ns);
	if(_Z(_q)) {
		_xe('M');
		return;
	}
	_MS(_q + _tl, _N, _G);
	_tp = _q;
	_tl = _ns;
}

static void _nxt(void) {
	_tc++;
	if(_tc >= _tl) _grow();
}

static void _prv(void) {
	if(_tc > _N) _tc--;
}

static void _put(void) {
	_WR(_tp + _tc, _1);
}

static void _get(void) {
	_u _b[1];
	if(_RD(_b, _1) == _1) *(_tp + _tc) = _b[_N];
}

static _s _bm(_u*_p, _s _l, _s _i) {
	_s _j = _i + _1, _d = _N;
	while(_j < _l) {
		if(_p[_j] == '[') _d++;
		else if(_p[_j] == ']') {
			if(_Z(_d)) return _j;
			_d--;
		}
		_j++;
	}
	return _i;
}

static _s _fm(_u*_p, _s _i) {
	_s _d = _N;
	while(_i > _N) {
		_i--;
		if(_p[_i] == ']') _d++;
		else if(_p[_i] == '[') {
			if(_Z(_d)) return _i;
			_d--;
		}
	}
	return _N;
}

static void _run(_u*_p, _s _l) {
	_s _i = _N;
	while(_i < _l) {
		_u _o = _p[_i];
		if (_o == '+') (*(_tp+_tc))++;
		else if(_o == '-') (*(_tp+_tc))--;
		else if(_o == '>') _nxt();
		else if(_o == '<') _prv();
		else if(_o == '.') _put();
		else if(_o == ',') _get();
		else if(_o == '[' && _Z(*(_tp+_tc))) _i = _bm(_p,_l,_i);
		else if(_o == ']' && !_Z(*(_tp+_tc))) _i = _fm(_p,_i);
		_i++;
	}
}

int main(void) {
	_s _cap = _TS;
	_tl = _TS;
	_tc = _N;
	_tp = (_u*) _M(_tl);
	if(_Z (_tp)) {
		_xe('M');
		return _1;
	}
	_MS (_tp,_N,_tl) ;
	_bl=_CS;
	_bs=(_s*)_M(_bl*sizeof(_s));
	if(_Z(_bs)) {
		_xe('M');
		_F(_tp);
		return _1;
	}
	_cap=_CS;
	_pg=(_u*)_M(_cap);
	if(_Z(_pg)) {
		_xe('M');
		_F(_tp);
		_F(_bs);
		return _1;
	}
	_pl=_N;
	for(;;) {
		_s _n=_RD(_pg+_pl,_cap-_pl-_1);
		if(_n<=_N) break;
		_pl+=(_s)_n;
		if(_pl+_1>=_cap) {
			_cap*=2;
			_u*_q=(_u*)_R(_pg,_cap);
			if(_Z(_q)) {
				_xe('M');
				break;
			}
			_pg=_q;
		}
	}
	_run(_pg,_pl);
	_F(_tp);
	_F(_bs);
	_F(_pg);
	return _N;
}
