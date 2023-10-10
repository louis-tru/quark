
// Ref:
// https://raphlinus.github.io/graphics/2020/04/21/blurred-rounded-rects.html
// https://madebyevan.com/shaders/fast-rounded-rectangle-shadows/

// vec4 erf(vec4 x) {
// 	vec4 s = sign(x), a = abs(x);
// 	x = 1.0 + (0.278393 + (0.230389 + 0.078108 * (a * a)) * a) * a;
// 	return s - s / (x * x * x * x);
// }

pub fn compute_erf7(x: f64) -> f64 {
	// 2/sqrt(π), 1.12837916709551257389615890312154517f64
	let x = x * std::f64::consts::FRAC_2_SQRT_PI; // 1.1283791670955126
	let xx = x * x;
	let x = x + (0.24295 + (0.03395 + 0.0104 * xx) * xx) * (x * xx);
	x / (1.0 + x * x).sqrt()
}

/// Generate a blurred rounded rectangle using distance field approximation.
pub fn gen_distfield(width: usize, height: usize, w: f64, h: f64, r: f64, s: f64) -> Vec<u8> {
		// To avoid divide by 0; potentially should be a bigger number for antialiasing.
		let s = s.max(1e-6);

		let min_edge = w.min(h);
		let rmax = 0.5 * min_edge;
		let r0 = r.hypot(s * 1.15).min(rmax);
		let r1 = r.hypot(s * 2.0).min(rmax);

		let exponent = 2.0 * r1 / r0;

		let s_inv = s.max(1e-6).recip(); // blur size reciprocal

		// Pull in long end (make less eccentric).
		let delta = 1.25 * s * ((-(0.5 * s_inv * w).powi(2)).exp() - (-(0.5 * s_inv * h).powi(2)).exp());
		let w = w + delta.min(0.0);
		let h = h - delta.max(0.0);

		let recip_exponent = exponent.recip();
		let mut buf = vec![0u8; width * height];
		let scale = 0.5 * compute_erf7(s_inv * 0.5 * (w.max(h) - 0.5 * r));
		for j in 0..height {
				let y = (j as f64) + 0.5 - 0.5 * (height as f64);
				let y0 = y.abs() - (h * 0.5 - r1);
				let y1 = y0.max(0.0);
				for i in 0..width {
						let x = (i as f64) + 0.5 - 0.5 * (width as f64);
						let x0 = x.abs() - (w * 0.5 - r1);
						let x1 = x0.max(0.0);
						let d_pos = (x1.powf(exponent) + y1.powf(exponent)).powf(recip_exponent);
						let d_neg = x0.max(y0).min(0.0);
						let d = d_pos + d_neg - r1;
						let z = scale * (compute_erf7(s_inv * (min_edge + d)) - compute_erf7(s_inv * d));
						buf[j * width + i] = (z * 255.0).round() as u8;
				}
		}
		buf
}