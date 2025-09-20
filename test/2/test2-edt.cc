// edt_bench.cpp
// Compile: g++ -O3 -march=native -std=c++17 edt_bench.cpp -o edt_bench
// Run example: ./edt_bench 1024 1024 10 5   (width height runs seed)
//
// Implements:
//  - felzenszwalb EDT (exact squared distance)
//  - chamfer-style 8-neighbor two-pass EDT (approx, additions only)
// Measures time and prints ms per run.

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <algorithm>
#include <functional>
#include <random>
#include <iomanip>

using namespace std;
using steady_clock = std::chrono::steady_clock;
using ms = std::chrono::duration<double, std::milli>;
static const int INF = 0x3f3f3f3f;

static inline void fill_random_binary(vector<uint8_t>& img, int w, int h, double prob, std::mt19937 &rng) {
	std::bernoulli_distribution d(prob);
	for (int i = 0; i < w*h; ++i) img[i] = d(rng) ? 1 : 0;
}

// ---------------- Felzenszwalb & Huttenlocher (exact squared EDT) ----------------
// 1D EDT (from paper) - works on arrays of ints where f[i] is 0 for feature or INF otherwise.
// Produces squared distances.
static void edt_1d(const vector<int>& f, vector<int>& d, int n) {
	vector<int> v(n);
	vector<int> z(n+1);
	int k = 0;
	v[0] = 0;
	z[0] = -INF;
	z[1] = +INF;
	auto square = [](int x){ return x*x; };
	for (int q = 1; q < n; ++q) {
		int s = 0;
		while (true) {
			int p = v[k];
			// intersection:
			// ((f[q] + q*q) - (f[p] + p*p)) / (2*(q-p))
			// To avoid floating rounding here, we compute as integer floor of intersection
			long long num = (long long)(f[q] + q*q) - (long long)(f[p] + p*p);
			long long den = 2LL * (q - p);
			// flooring division
			s = (int)floor((double)num / (double)den);
			if (s <= z[k]) {
				--k;
				if (k < 0) { k = 0; v[0] = q; break; }
			} else {
				break;
			}
		}
		++k;
		v[k] = q;
		z[k] = s;
		z[k+1] = +INF;
	}
	int idx = 0;
	for (int q = 0; q < n; ++q) {
		while (z[idx+1] < q) ++idx;
		int p = v[idx];
		d[q] = (q - p)*(q - p) + f[p];
	}
}

static void felzenszwalb_edt(const vector<uint8_t>& bin, int w, int h, vector<int>& out_sqdist) {
	// out_sqdist size w*h, squared distances
	// Step 1: for each pixel, set f = 0 if feature (foreground pixel==1), else INF
	vector<int> f(w*h);
	for (int i = 0; i < w*h; ++i) f[i] = bin[i] ? 0 : INF;

	// Temporary buffer
	vector<int> d(w*h);

	// Vertical pass: apply 1D EDT on columns
	vector<int> column_f(max(w,h)), column_d(max(w,h));
	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < h; ++y) column_f[y] = f[y*w + x];
		edt_1d(column_f, column_d, h);
		for (int y = 0; y < h; ++y) d[y*w + x] = column_d[y];
	}

	// Horizontal pass: apply 1D EDT on rows (d as input)
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) column_f[x] = d[y*w + x];
		edt_1d(column_f, column_d, w);
		for (int x = 0; x < w; ++x) out_sqdist[y*w + x] = column_d[x];
	}
}

// ---------------- 8-neighborhood two-pass chamfer (approx) ----------------
// We use integer weights: orthogonal = 3, diagonal = 4 (common simple chamfer)
// Two passes: forward (top-left -> bottom-right), backward (bottom-right -> top-left)
// This implementation uses only add/min, no multiply/divide.
static void chamfer_8ssetd(const vector<uint8_t>& bin, int w, int h, vector<int>& out_dist) {
	// binary: 1 = feature; we want distance to nearest feature
	// We'll compute integer distance in chamfer units.
	const int ORTH = 128; // 2^n
	const int DIAG = 181; // ≈ ORTH * √2
	// 1.4142135623730951 = sqrt(2)
	// 1.4140625 = 181/128 ≈ sqrt(2)

	// initialize
	for (int i = 0; i < w*h; ++i)
		out_dist[i] = bin[i] ? 0 : INF;

	// forward pass
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			int idx = y*w + x;
			int v = out_dist[idx];
			if (x > 0) v = min(v, out_dist[idx-1] + ORTH);              // left
			if (y > 0) v = min(v, out_dist[idx-w] + ORTH);              // up
			if (x > 0 && y > 0) v = min(v, out_dist[idx-w-1] + DIAG);   // up-left
			if (x+1 < w && y > 0) v = min(v, out_dist[idx-w+1] + DIAG); // up-right (from previous row)
			out_dist[idx] = v;
		}
	}
	// backward pass
	for (int y = h-1; y >= 0; --y) {
		for (int x = w-1; x >= 0; --x) {
			int idx = y*w + x;
			int v = out_dist[idx];
			if (x+1 < w) v = min(v, out_dist[idx+1] + ORTH);              // right
			if (y+1 < h) v = min(v, out_dist[idx+w] + ORTH);              // down
			if (x+1 < w && y+1 < h) v = min(v, out_dist[idx+w+1] + DIAG); // down-right
			if (x > 0 && y+1 < h) v = min(v, out_dist[idx+w-1] + DIAG);   // down-left
			out_dist[idx] = v;
		}
	}
}

void chamfer_8ssetd_print() {
	vector<uint8_t> bin = {
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,1,1,0,1,0,0,0,
		0,0,0,1,1,1,1,0,0,0,
		0,0,0,1,1,1,1,0,0,0,
		0,0,0,1,1,1,1,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
	};
	vector<int> d_fore(10*10), d_back(10*10);
	chamfer_8ssetd(bin, 10, 10, d_fore);
	vector<uint8_t> bin_inv(10*10);
	for (int i = 0; i < 10*10; ++i) bin_inv[i] = bin[i] ? 0 : 1;
	chamfer_8ssetd(bin_inv, 10, 10, d_back);

	for (int i = 0; i < 10*10; ++i) {
		if (bin[i]) d_fore[i] = -d_back[i] + 128;
	}

	for (int y = 0; y < 10; ++y) {
		for (int x = 0; x < 10; ++x)
			printf("%f ", d_fore[y*10 + x] / 128.0f);
		printf("\n");
	}
	printf("\n");
	for (int y = 0; y < 10; ++y) {
		for (int x = 0; x < 10; ++x)
			printf("%d ", d_fore[y*10 + x] >> 7);
		printf("\n");
	}
}

// To obtain an approximate Euclidean distance in pixels from chamfer units, you can divide by ORTH (or use calibrated factor).
// But for benchmarking we compare runtime only (not numeric equality).

// ---------------- Utility: signed distance (optional) ----------------
// Compute signed distance by running transform on foreground and background.
// For Felzenszwalb (exact) we run it twice (f and 1-f) and subtract sqrt if needed.
// For chamfer we run chamfer on foreground and background and subtract.

static void compute_signed_felzenszwalb(const vector<uint8_t>& bin, int w, int h, vector<int>& out_signed_sq) {
	vector<int> d_fore(w*h), d_back(w*h);
	felzenszwalb_edt(bin, w, h, d_fore);
	// background: feature where bin==0
	vector<uint8_t> bin_inv(w*h);
	for (int i = 0; i < w*h; ++i) bin_inv[i] = bin[i] ? 0 : 1;
	felzenszwalb_edt(bin_inv, w, h, d_back);
	// signed squared distance: foreground dist - background dist (we'll store sign by subtracting)
	for (int i = 0; i < w*h; ++i) {
		// if pixel is foreground (1) distance should be negative inside object?
		if (bin[i]) out_signed_sq[i] = -d_back[i]; // inside: negative distance to background
		else out_signed_sq[i] = d_fore[i];        // outside: positive distance to foreground
	}
}

static void compute_signed_chamfer(const vector<uint8_t>& bin, int w, int h, vector<int>& out_signed_cham) {
	vector<int> d_fore(w*h), d_back(w*h);
	chamfer_8ssetd(bin, w, h, d_fore);
	vector<uint8_t> bin_inv(w*h);

	for (int i = 0; i < w*h; ++i)
		bin_inv[i] = bin[i] ? 0 : 1;
	chamfer_8ssetd(bin_inv, w, h, d_back);

	for (int i = 0; i < w*h; ++i) {
		if (bin[i])
			out_signed_cham[i] = -d_back[i];
		else
			out_signed_cham[i] = d_fore[i];
	}
}

static void compute_signed_chamfer_f32(const vector<uint8_t>& bin, int w, int h, vector<float>& out_signed_cham) {
	vector<int> d_fore(w*h), d_back(w*h);
	chamfer_8ssetd(bin, w, h, d_fore);
	vector<uint8_t> bin_inv(w*h);

	for (int i = 0; i < w*h; ++i)
		bin_inv[i] = bin[i] ? 0 : 1;
	chamfer_8ssetd(bin_inv, w, h, d_back);

	for (int i = 0; i < w*h; ++i) {
		if (bin[i])
			out_signed_cham[i] = (128.0f - d_back[i]) * (1.0f / 128.0f);
		else
			out_signed_cham[i] = d_fore[i] * (1.0f / 128.0f);
	}
}

// ---------------- Benchmark harness ----------------
struct Stats {
	double mean_ms;
	double std_ms;
};

static Stats bench_fn(function<void()> fn, int runs, int warmup = 2) {
	// warmup
	for (int i = 0; i < warmup; ++i) fn();
	vector<double> times;
	times.reserve(runs);
	for (int i = 0; i < runs; ++i) {
		auto t0 = steady_clock::now();
		fn();
		auto t1 = steady_clock::now();
		double elapsed = ms(t1 - t0).count();
		times.push_back(elapsed);
	}
	double sum = 0;
	for (double v : times) sum += v;
	double mean = sum / times.size();
	double var = 0;
	for (double v : times) var += (v - mean)*(v - mean);
	var /= times.size();
	return {mean, sqrt(var)};
}

int test2_edt(int argc, char *argv[]) {
	if (argc < 5) {
		cout << "Usage: " << argv[0] << " <width> <height> <runs> <seed> [density]\n";
		cout << "Example: ./edt_bench 1024 1024 10 42 0.05\n";
		return 0;
	}
	int w = atoi(argv[1]);
	int h = atoi(argv[2]);
	int runs = atoi(argv[3]);
	int seed = atoi(argv[4]);
	double density = 0.02;
	if (argc >= 6) density = atof(argv[5]);

	std::mt19937 rng(seed);
	vector<uint8_t> bin(w*h);
	fill_random_binary(bin, w, h, density, rng);

	// buffers
	vector<int> out_fel(w*h), out_cham(w*h);
	vector<int> signed_fel(w*h), signed_cham(w*h);

	// Bench Felzenszwalb (unsigned)
	auto fn_fel = [&]() {
		felzenszwalb_edt(bin, w, h, out_fel);
		// touch output to avoid optimizing away
		volatile int s = out_fel[0];
		(void)s;
	};
	auto st_fel = bench_fn(fn_fel, runs);

	// Bench chamfer 8SSEDT (unsigned)
	auto fn_cham = [&]() {
		chamfer_8ssetd(bin, w, h, out_cham);
		volatile int s = out_cham[0];
		(void)s;
	};
	auto st_cham = bench_fn(fn_cham, runs);

	// Bench signed: felzenszwalb (two runs inside)
	auto fn_fel_signed = [&]() {
		compute_signed_felzenszwalb(bin, w, h, signed_fel);
		volatile int s = signed_fel[0];
		(void)s;
	};
	auto st_fel_signed = bench_fn(fn_fel_signed, runs);

	// Bench signed: chamfer
	auto fn_cham_signed = [&]() {
		compute_signed_chamfer(bin, w, h, signed_cham);
		volatile int s = signed_cham[0];
		(void)s;
	};
	auto st_cham_signed = bench_fn(fn_cham_signed, runs);

	vector<float> signed_cham_f32(w*h);
	// Bench signed: chamfer f32
	auto fn_cham_signed_f32 = [&]() {
		compute_signed_chamfer_f32(bin, w, h, signed_cham_f32);
		volatile float s = signed_cham_f32[0];
		(void)s;
	};
	auto st_cham_signed_f32 = bench_fn(fn_cham_signed_f32, runs);

	cout.setf(std::ios::fixed); cout<<setprecision(3);
	cout << "Image: " << w << "x" << h << ", density=" << density << ", runs=" << runs << "\n";
	cout << "Felzenszwalb (unsigned)  : " << st_fel.mean_ms << " ms (sd " << st_fel.std_ms << ")\n";
	cout << "Chamfer 8SSEDT (unsigned): " << st_cham.mean_ms << " ms (sd " << st_cham.std_ms << ")\n";
	cout << "Felzenszwalb (signed, 2x): " << st_fel_signed.mean_ms << " ms (sd " << st_fel_signed.std_ms << ")\n";
	cout << "Chamfer (signed, 1x)     : " << st_cham_signed.mean_ms << " ms (sd " << st_cham_signed.std_ms << ")\n";
	cout << "Chamfer (signed f32, 1x) : " << st_cham_signed.mean_ms << " ms (sd " << st_cham_signed.std_ms << ")\n";

	cout << "\nNotes:\n";
	cout << " - Felzenszwalb computes exact SQUARED Euclidean distances.\n";
	cout << " - Chamfer 8SSEDT is an addition-only two-pass approximation (orth=3 diag=4).\n";
	cout << " - Signed: Felzenszwalb runs two full EDTs (fg+bg), chamfer likewise.\n";
	cout << " - Compile with -O3 -march=native for best results.\n\n";

	// chamfer_8ssetd_print();

	return 0;
}