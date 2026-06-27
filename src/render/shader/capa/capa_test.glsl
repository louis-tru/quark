
float capa_edge_cross_x(float sampleY, CAPAEdge edge) {
	return fma(sampleY - edge.p0.y, edge.dxdy, edge.p0.x);
}

float capa_right_area_segment(float y0, float y1, float x0, float x1, float pixelX) {
	float localX0 = x0 - pixelX;
	float localX1 = x1 - pixelX;
	float mid = (localX0 + localX1) * 0.5;
	if (mid <= 0.0)
		return y1 - y0;
	if (mid >= 1.0)
		return 0.0;
	return (y1 - y0) * (1.0 - mid);
}

float capa_right_area(float y0, float y1, CAPAEdge edge, float pixelX) {
	float x0 = capa_edge_cross_x(y0, edge);
	float x1 = capa_edge_cross_x(y1, edge);
	float dy = y1 - y0;
	float dx = x1 - x0;
	if (dy <= 0.0)
		return 0.0;
	if (abs(dx) < 1e-6)
		return capa_right_area_segment(y0, y1, x0, x1, pixelX);

	float split0 = y0 + (pixelX - x0) * dy / dx;
	float split1 = y0 + (pixelX + 1.0 - x0) * dy / dx;
	if (split0 > split1) {
		float tmp = split0;
		split0 = split1;
		split1 = tmp;
	}
	split0 = clamp(split0, y0, y1);
	split1 = clamp(split1, y0, y1);

	float area = 0.0;
	float a = y0;
	float b = split0;
	if (b > a)
		area += capa_right_area_segment(a, b, capa_edge_cross_x(a, edge), capa_edge_cross_x(b, edge), pixelX);
	a = split0;
	b = split1;
	if (b > a)
		area += capa_right_area_segment(a, b, capa_edge_cross_x(a, edge), capa_edge_cross_x(b, edge), pixelX);
	a = split1;
	b = y1;
	if (b > a)
		area += capa_right_area_segment(a, b, capa_edge_cross_x(a, edge), capa_edge_cross_x(b, edge), pixelX);
	return area;
}
