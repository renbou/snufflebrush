#ifndef SEEDGEN
#define SEEDGEN

#include "next_prime.h"
#define UINT_MAX 4294967295

typedef unsigned int uint;
typedef unsigned long long uint64;

uint getmax(uint &a, uint &b) { return (a > b ? a : b); }

volatile double _sqrt(double a) {
	asm volatile(
	"movq %1, %%xmm0 \n"
	"sqrtsd %%xmm0, %%xmm1 \n"
	"movq %%xmm1, %0 \n"
	: "=r"(a)
	: "g"(a)
	: "xmm0", "xmm1", "memory"
	);
	return a;
}

volatile double _fabs(double input) {
	double output;
	asm volatile (
	"fldl %1\n"
 	"fabs \n"
  	"fstpl %0\n"
  	: "=m" (output)
  	: "m" (input)
	: "memory"
	);
	return output;
}

void _swap(uint &a, uint &b) {
	uint tmp = a;
	a = b;
	b = tmp;
	return;
}

uint revBits(uint n) {
	n = ((n >> 1) & 0x55555555) | ((n << 1) & 0xaaaaaaaa);
	n = ((n >> 2) & 0x33333333) | ((n << 2) & 0xcccccccc);
	n = ((n >> 4) & 0x0f0f0f0f) | ((n << 4) & 0xf0f0f0f0);
	n = ((n >> 8) & 0x00ff00ff) | ((n << 8) & 0xff00ff00);
	n = ((n >> 16) & 0x0000ffff) | ((n << 16) & 0xffff0000);
	return n;
}

uint _ceil(double n) {
	double eps = 1e-9;
	uint trunc = (uint)n;
	if (_fabs(n-(double)trunc) > eps)
		trunc++;
	return trunc;
}

void copy(uint * f_ind1, uint * f_ind2, uint * t_ind1) {
	while (f_ind1 != f_ind2) {
		*t_ind1 = *f_ind1;
		t_ind1++, f_ind1++;
	}
	return;
}

class rnd {
	uint64 x, m = 1<<31, a = 1103515249, c = 12347;
public:
	rnd() : x(0) {}
	rnd(uint64 seed) : x(seed) {}
	uint intRnd() {
		asm volatile (
		"movq %1, %%rax \n"
  		"movq %2, %%rdx \n"
		"mulq %%rdx \n"
  		"addq %3, %%rax \n"
		"movq $0, %%rdx \n"
		"movq %4, %%rcx \n"
  		"divq %%rcx \n"
		"movq %%rdx, %0 \n"
		: "=r"(this->x)
		: "g"(this->x), "g"(this->a), "g"(this->c), "g"(this->m)
		: "rax", "rcx", "rdx"
		);
		return x;
	}
};

struct rectangle {
	uint w, h;
	rectangle(uint _w, uint _h): w(_w), h(_h) {};
	rectangle(): w(0), h(0) {};

	uint area() { return w*h; }
};

struct location {
	uint layer, x, y, colour;
	location(uint _l, uint _x, uint _y, uint _c): layer(_l), x(_x), y(_y), colour(_c) {};
};

class seedgen {
	uint width, height, rw, rh, numl, numc, numRects, numRectsW, numRectsH, passwordSize, dataSize, rectSelInd, spotSelInd, curMaxSpots, curUsedSpots, curRectLayer, curRect, curRectType;
	uint64 seedRects, seedSpots;
	uint *password;
	uint *rectOrder;
	uint **spotOrder;
	uint *spotMaxInd;
	rectangle *rectDims;
	uint **spotAmountForRect;
	rnd rectRnd, spotRnd;
public:
	uint64 getSpotSeed() { return seedSpots; }
	uint64 getRectSeed() { return seedRects; }

	seedgen(uint _w, uint _h, uint _numl, uint _numc, uint _psize, const char *_pass) : width(_w), height(_h), rw(_ceil(_sqrt(_w))), rh(_ceil(_sqrt(_h))), numl(_numl),
																			 numc(_numc){

		if (width*height*numl*numc < dataSize) throw "Not enough space in image for stego.";

		uint _charsize = sizeof(uint) / sizeof(char);
		uint _intbits = sizeof(uint) * 8;

		numRectsW = _ceil((double)width/(double)rw), numRectsH = _ceil((double)height/(double)rh);
		numRects = numRectsW*numRectsH*numl;
		seedRects = 0, seedSpots = 0, rectSelInd = numRects-1, curMaxSpots = -1;

		/* generate rect sizes */

		rectDims = new rectangle [4];
		uint _fullRectsW = width/rw, _fullRectsH = height/rh;
		uint _widthLeftOvers = (width % (_fullRectsW * rw)), _heightLeftOvers = (height % (_fullRectsH* rh));
		for (int i = 0; i < 4; i++) rectDims[i] = {(i&1 ? _widthLeftOvers : rw), (i>1 ? _heightLeftOvers : rh)};

		/* ///////////// */

		/* generate random password for random seed generation */
		
		uint _pswd_length_temp = _psize / _charsize;
		password = new uint [_psize % _charsize == 0 ? passwordSize = _pswd_length_temp : passwordSize = _pswd_length_temp + 1];
		
		for (int i = 0; i < passwordSize; i++) {
			password[i] = 0;
			for (int k = 0; k < _charsize && i * _charsize + k < _psize; k++)
				password[i] ^= (_pass[i * _charsize + k] << (uint)(sizeof(char) * 8 * k));
		}
		
		uint * _password = new uint [passwordSize];
		copy(password, password + passwordSize, _password);
		
		uint _masks[sizeof(uint)] = {UINT_MAX, 0};
		
		for (int i = 1; i < sizeof(uint); i++)
			_masks[i] = _masks[i - 1] >> (_intbits >> i);

		uint _prevC = 1, _modulo = UINT_MAX-4;

		for (int i = 0; i < passwordSize; i++) {
			for (int k = 0; _masks[k] >= 0xff; k++) {
				uint _lim = sizeof(uint) / (sizeof(uint) >> k);
				uint _curbits = _intbits / _lim;
				for (uint x = 0; x < _lim; x++) {
					uint _to_xor = 	revBits(
						            next_prime(
									((((uint64)_password[i] * (uint64)_prevC) % _modulo) & (_masks[k] << (_curbits * x))) >>
																				   (_curbits * x)));
					_to_xor >>= (_curbits * (_lim - 1 - x));
					_prevC = _to_xor;
					_password[i] ^= _to_xor;
				}
			}
		}
		/* ///////////// */

		/* generate random order for rectangle selection */

		rectOrder = new uint[numRects];
		for (uint i = 0; i < numRects; i++) rectOrder[i] = i;

		uint *_first_primes = new uint[_intbits*2];
		uint _fp_ind = 0;
		uint _temp_x = (_password[0] & (~(1<<31)));

		for (int i = 0; i < _intbits * 2; i++) {
			_temp_x = next_prime(_temp_x);
			_first_primes[i] = _temp_x;
		}

		for (int i = 0; i < _intbits; i++) {
			uint64 tmp = (_password[_first_primes[_fp_ind] % passwordSize]);
			tmp <<= (_first_primes[_fp_ind++] % (_intbits * 2));
			seedRects ^= tmp;
		}

		rectRnd = rnd(seedRects);

		delete [] _first_primes;
		/* ///////////// */

		spotOrder = new uint * [4];
		spotMaxInd = new uint [4];
		for (int i = 0; i < 4; i++) spotOrder[i] = new uint [(spotMaxInd[i] = rectDims[i].area() * numc)];
		for (int i = 0; i < 4; i++)
			for (uint j = 0; j < spotMaxInd[i]; j++)
				spotOrder[i][j] = j;

		_first_primes = new uint[_intbits*2];
		_fp_ind = 0;
		_temp_x = (revBits(_password[0]) & (~(1<<31)));

		for (int i = 0; i < _intbits * 2; i++) {
			_temp_x = next_prime(_temp_x);
			_first_primes[i] = _temp_x;
		}

		for (int i = 0; i < _intbits; i++) {
			uint64 tmp = (_password[_first_primes[_fp_ind++] % passwordSize]);
			tmp <<= (_first_primes[_fp_ind++] % (_intbits * 2));
			seedSpots ^= tmp;
		}

		spotRnd = rnd(seedSpots);

		delete [] _first_primes;
		/* ///////////// */

		delete [] _password;
	}

	void initSize(uint _dataSize, bool equalSeparation) {
		dataSize = _dataSize*8;

		uint * _dataAmountForLayer = new uint [numl];
		double _leftover = 1;

		if (equalSeparation == 1) {
			_leftover /= (double)numl;
			for (int i = 0; i < numl; i++)
				_dataAmountForLayer[i] = _ceil(_leftover*(double)dataSize);
		} else {
			for (int i = 0; i < numl; i++)
				_dataAmountForLayer[i] = _ceil((double) dataSize * (i == numl - 1 ? _leftover : (_leftover /= 2)));
		}

		double * _rectRatios = new double [4];
		uint _fullRectsW = width/rw, _fullRectsH = height/rh;
		uint _widthLeftOvers = (width % (_fullRectsW * rw)), _heightLeftOvers = (height % (_fullRectsH* rh));
		double _totalRectSpotSum = 	width * height;

		for (int i = 0; i < 4; i++) _rectRatios[i] = ((i&1 ? _widthLeftOvers : (_fullRectsW * rw)) *
													  (i>1 ? _heightLeftOvers : (_fullRectsH * rh))) /
													 _totalRectSpotSum;

		double *_rectSpread = new double [4];
		_rectSpread[0] = _fullRectsW*_fullRectsH;
		_rectSpread[1] = _fullRectsH;
		_rectSpread[2] = _fullRectsW;
		_rectSpread[3] = 1.0;

		spotAmountForRect = new uint * [numl];
		for (int i = 0; i < numl; i++) {
			spotAmountForRect[i] = new uint [4];
			for (int j = 0; j < 4; j++)
				spotAmountForRect[i][j] = _ceil((_dataAmountForLayer[i] * _rectRatios[j]) / _rectSpread[j]);
		}

		curMaxSpots = spotAmountForRect[curRectLayer][curRectType];
		return;
	}

	uint nextSpot() {
		_swap(spotOrder[curRectType][spotSelInd], spotOrder[curRectType][spotRnd.intRnd()%spotSelInd]);
		curUsedSpots++;
		return spotOrder[curRectType][spotSelInd--];
	}

	uint nextRect(bool indep = 0) {
		_swap(rectOrder[rectSelInd], rectOrder[rectRnd.intRnd()%rectSelInd]);
		curRectLayer = rectOrder[rectSelInd] % numl;
		curRect = rectOrder[rectSelInd] / numl;
		curRectType = (((curRect % numRectsW + 1) * rw) > width) + (((curRect / numRectsW + 1) * rh) > height ? 2 : 0);
		spotSelInd = spotMaxInd[curRectType]-1, curUsedSpots = 0;
		if (!indep)
			curMaxSpots = spotAmountForRect[curRectLayer][curRectType];
		else
			curMaxSpots = 0;
		return rectOrder[rectSelInd--];
	}

	location nextLoc() {
		if (spotSelInd == 0 || curUsedSpots >= curMaxSpots || curMaxSpots == -1) nextRect();
		uint _spotInRect = nextSpot();
		uint _layer = curRectLayer;
		uint _colour = _spotInRect % numc;
		uint _x = ((curRect % numRectsW) * rw) + ((_spotInRect / numc) % rectDims[curRectType].w), _y = ((curRect / numRectsW) * rh) + ((_spotInRect / numc) / rectDims[curRectType].w);
		return location(_layer, _x, _y, _colour);
	}

	location nextLocDataIndep() {
		if (spotSelInd == 0 || curMaxSpots == -1) nextRect(1);
		uint _spotInRect = nextSpot();
		uint _layer = curRectLayer;
		uint _colour = _spotInRect % numc;
		uint _x = ((curRect % numRectsW) * rw) + ((_spotInRect / numc) % rectDims[curRectType].w), _y = ((curRect / numRectsW) * rh) + ((_spotInRect / numc) / rectDims[curRectType].w);
		return location(_layer, _x, _y, _colour);
	}
};

#endif