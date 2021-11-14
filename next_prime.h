#ifndef NEXT_PRIME_H
#define NEXT_PRIME_H

#define l64 long long
#define INT_MAX 2147483647

l64 mpow(l64 a, l64 n, l64 m) {
    l64 res = 1;
    while (n) {
        if (n & 1)
            res = ((res * a) % m + m) % m;
        a = ((a * a) % m + m) % m;
        n >>= 1;
    }
    return res;
}

l64 legendre(l64 a, l64 m) {
    return mpow(a, (m - 1) >> 1, m);
}

bool is_sprp(l64 n, l64 b = 2) {
    l64 d = n - 1, s = 0;
    while (~d&1) {
        ++s;
        d >>= 1;
    }
    l64 x = mpow(b, d, n);
    if (x == 1 || x == n - 1) return true;

    for (int i = 1; i < s; i++) {
        x = (x * x) % n;
        if (x == 1) return false;
        if (x == n - 1) return true;
    }
    return false;
}

bool is_lucas_prp(l64 n, l64 D) {
    l64 P = 1, Q = (1 - D) >> 2, s = n + 1, r = 0;
    while (~s&1) {
        ++r;
        s >>= 1;
    }
    l64 t = 0;
    while (s > 0) {
        if (s&1) ++t, --s;
        else t <<= 1, s >>= 1;
    }

    l64 U = 0, V = 2, q = 1, inv_2 = (n + 1) >> 1;
    while (t > 0) {
        l64 Uo = U, Vo = V;
        if (t&1) {
            V = (((D * Uo + Vo) * inv_2) % n + n) % n;
            U = (((Uo + Vo) * inv_2) % n + n) % n;
            q = ((q * Q) % n + n) % n;
            --t;
        } else {
            V = ((Vo * Vo - 2 * q) % n + n) % n;
            U = ((Uo * Vo) % n + n) % n;
            q = ((q * q) % n + n) % n;
            t >>= 1;
        }
    }

    while (r > 0) {
        l64 Uo = U, Vo = V;
        U = (Uo * Vo) % n;
        V = (Vo * Vo - 2 * q) % n;
        q = (q * q) % n;
        r--;
    }

    return !U;
}

int smal64_primes[47] = {
        2,  3,  5,  7, 11, 13, 17, 19, 23, 29,
        31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
        73, 79, 83, 89, 97,101,103,107,109,113,
        127,131,137,139,149,151,157,163,167,173,
        179,181,191,193,197,199,211};

int indices[48] = {
        1, 11, 13, 17, 19, 23, 29, 31, 37, 41,
        43, 47, 53, 59, 61, 67, 71, 73, 79, 83,
        89, 97,101,103,107,109,113,121,127,131,
        137,139,143,149,151,157,163,167,169,173,
        179,181,187,191,193,197,199,209};

int offsets[48] = {
        10, 2, 4, 2, 4, 6, 2, 6, 4, 2, 4, 6,
        6, 2, 6, 4, 2, 6, 4, 6, 8, 4, 2, 4,
        2, 4, 8, 6, 4, 6, 2, 4, 6, 2, 6, 6,
        4, 2, 4, 6, 2, 6, 4, 2, 4, 2,10, 2};

l64 max_int = 2147483647;

bool is_prime(l64 n) {

    for (int i = 0; i < 47; ++i) {
        if (smal64_primes[i] == n) return true;
        if (n % smal64_primes[i] == 0) return false;
    }

    if (n <= max_int) {
        l64 i = 211;
        while (i * i < n) {
            for (l64 j = 0; i * i < n && j < 48; i += offsets[j++])
                if (n % i == 0)
                    return false;
        }
        return true;
    }

    if (!is_sprp(n)) return false;

    l64 a = 5, s = 2;
    while (legendre(a, n) != n - 1) {
        s = -s;
        a = s - a;
    }
    return is_lucas_prp(n, a);
}

int next_prime(int val) {

    if (val < 0) val ^= (1<<31);

    if (val == INT_MAX) val = 0xa55aa55a;

    l64 n = val;

    if (n < 2) return 2;
    n = (n + 1) | 1;
    if (n < 212) {
    	int l = 0, r = 47;
    	int mid;
    	while (l < r) {
    		mid = (l+r)/2;
    		if (n <= smal64_primes[mid])
    			r = mid;
    		else
    			l = mid + 1;
    	}
    	return smal64_primes[l];
    }

    l64 x = n % 210, s = 0, e = 47, m = 24;

    while (m != e)
        if (indices[m] < x) {
            s = m;
            m = (s + e + 1) >> 1;
        } else {
            e = m;
            m = (s + e) >> 1;
        }
    l64 i = n + indices[m] - x;
    for (l64 j = 0, o = offsets[m]; ; j++, o = offsets[(m + j) % 48]) {
        if (is_prime(i)) return (int)i;
        i += o;
        if (i < 0) return 5;
    }
}

#endif //NEXT_PRIME_H