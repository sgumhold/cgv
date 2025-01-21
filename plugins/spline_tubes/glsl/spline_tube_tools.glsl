#version 330 core

//////
//
// Shader module interface definition
//

///***** begin interface of spline_tube_tools.glsl **********************************/

// --- defines -----------------------
#define POS_INF 3e+37

// --- functions -----------------------

float Pow2(float x);
float Pow3(float x);
// Raises a quadratic polynomial to the power of 2
void Pow2(in float c[3], out float o_c[5]);
void Sub(in float a[5], in float b[5], out float o_c[5]);

void SplinePointsToPolyCoeffs(float p0, float h, float p1, out float o_c[3]);

float EvalPoly(float x, float c0, float c1, float c2, float c3, float c4, float c5, float c6); 
float EvalPoly(float x, float c0, float c1, float c2, float c3, float c4, float c5);
float EvalPoly(float x, float c0, float c1, float c2, float c3, float c4);
float EvalPoly(float x, float c0, float c1, float c2, float c3);
float EvalPoly(float x, float c0, float c1, float c2);
float EvalPoly(float x, float c0, float c1);
float EvalPoly(float x, float c0);

float EvalPolyD0(float x, float c[3]);
float EvalPolyD1(float x, float c[3]);
float EvalPolyD2(float x, float c[3]);
float EvalPolyD0(float x, float c[5]);
float EvalPolyD1(float x, float c[5]);
float EvalPolyD2(float x, float c[5]);
float EvalPolyD3(float x, float c[5]);

vec3 qSplineEval(float l, float curveX[3], float curveY[3], float curveZ[3]);

vec3 EvalCSpline(vec3 p1, vec3 t1, vec3 p2, vec3 t2, float l);

// Evaluate a quadratic spline(and its first, second and third dervatives) at parameter t
float qSplineIDistEval(float t, float curveX[3], float polyB_C[5]);
float qSplineD1Eval(float t, float curveX[3], float polyB_C[5]);
float qSplineD2Eval(float t, float curveX[3], float polyB_C[5]);
float qSplineD3Eval(float t, float polyB_C[5]);

// Bisection method for finding roots for 'f(t)' and its first, second, and third derivatives
float qSplineIDistEval_BinRootFinder_Eval(float n, float p, float curveX[3], float polyB_C[5], int max_iter);
float qSplineD1_BinRootFinder_Eval(float n, float p, float curveX[3], float polyB_C[5], int max_iter);
float qSplineD2_BinRootFinder_Eval(float n, float p, float curveX[3], float polyB_C[5], int max_iter);
float qSplineD3_BinRootFinder_Eval(float n, float p, float polyB_C[5], int max_iter);

// Newtons method for finding roots for 'f(t)' and its first and second derivatives
float qSplineIDistEval_NewtonRootFinder_Eval(float n, float p, float curveX[3], float polyB_C[5], int max_iter);
float qSplineD1_NewtonRootFinder_Eval(float n, float p, float curveX[3], float polyB_C[5], int max_iter);
float qSplineD2_NewtonRootFinder_Eval(float n, float p, float curveX[3], float polyB_C[5], int max_iter);

// Bisection method for finding roots of a polynomial
// c: coefficients of the polynomial
// n: parameter for which function evaluated to a negative value
// p: parameter for which function evaluated to a positive value
// max_iter: maximum number of iterations
float BisectionMethodPolyD0(float c[5], float n, float p, int max_iter);
float BisectionMethodPolyD1(float c[5], float n, float p, int max_iter);

// Newton's method for finding roots of a polynomial
// c: coefficients of the polynomial
// x: initial guess
// max_iter: maximum number of iterations
float NewtonsMethodPolyD0(float c[5], float x, int max_iter);
float NewtonsMethodPolyD1(float c[5], float x, int max_iter);
// Similar to above, but with an additional epsilon parameter
// epsilon: maximum error
float NewtonsMethodPolyD0(float c[5], float x, float epsilon, int max_iter);
float NewtonsMethodPolyD1(float c[5], float x, float epsilon, int max_iter);

// Find the roots of a polynomial given the roots of its derivative (in order words, given its extrema)
void FindRootsPolyD0(float poly_C[5], float x_i[5], int m_i[5], out float x_o[6], out int m_o[6], int max_iter);
void FindRootsPolyD1(float poly_C[5], float x_i[4], int m_i[4], out float x_o[5], out int m_o[5], int max_iter);
///***** end interface of curve_tools.glsl ************************************/



//****************************************************************************/
//******** Function Definitions***********************************************/
//****************************************************************************/
float Pow2(float x) { 
	return x * x; 
}

float Pow3(float x) { 
	return x * x * x; 
}

void Pow2(in float c[3], out float o_c[5]) {
	o_c[0] = c[0] * c[0]; 
	o_c[1] =  2.0 * c[0] * c[1];
	o_c[2] = c[1] * c[1] +  2.0 * c[0] * c[2];
	o_c[3] =  2.0 * c[2] * c[1];
	o_c[4] = c[2] * c[2];
}

void Sub(in float a[5], in float b[5], out float o_c[5]) {
	o_c[0] = a[0] - b[0];
	o_c[1] = a[1] - b[1];
	o_c[2] = a[2] - b[2];
	o_c[3] = a[3] - b[3];
	o_c[4] = a[4] - b[4];
}

void SplinePointsToPolyCoeffs(float p0, float h, float p1, out float o_c[3]) {
	o_c[0] = p0;
	o_c[1] = -2.0 * p0 + 2.0 * h;
	o_c[2] =   p0 + p1 - 2.0 * h;
}

float EvalPoly(float x, float c0, float c1, float c2, float c3, float c4, float c5, float c6) { return x * (x * (x * (x * (x * (x * c6 + c5) + c4) + c3) + c2) + c1) + c0; }
float EvalPoly(float x, float c0, float c1, float c2, float c3, float c4, float c5) { return EvalPoly(x, c0,c1,c2,c3,c4,c5,0.0); }
float EvalPoly(float x, float c0, float c1, float c2, float c3, float c4) { return EvalPoly(x, c0,c1,c2,c3,c4,0.0,0.0); }
float EvalPoly(float x, float c0, float c1, float c2, float c3) { return EvalPoly(x, c0,c1,c2,c3,0.0,0.0,0.0); }
float EvalPoly(float x, float c0, float c1, float c2) { return EvalPoly(x, c0,c1,c2,0.0,0.0,0.0,0.0); }
float EvalPoly(float x, float c0, float c1) { return EvalPoly(x, c0,c1,0.0,0.0,0.0,0.0,0.0); }
float EvalPoly(float x, float c0) { return EvalPoly(x, c0,0.0,0.0,0.0,0.0,0.0,0.0); }

//float EvalPolyD0(float x, float c[3]) { return EvalPoly(x, c[0], c[1], c[2]); }
//float EvalPolyD1(float x, float c[3]) { return EvalPoly(x, c[1], c[2] * 2.0); }
//float EvalPolyD2(float x, float c[3]) { return EvalPoly(x, c[2] * 2.0);       }
//float EvalPolyD0(float x, float c[5]) { return EvalPoly(x, c[0], c[1], c[2], c[3], c[4]);             }
//float EvalPolyD1(float x, float c[5]) { return EvalPoly(x, c[1], c[2] * 2.0, c[3] * 3.0, c[4] * 4.0); }
//float EvalPolyD2(float x, float c[5]) { return EvalPoly(x, c[2] * 2.0, c[3] * 6.0, c[4] * 12.0);      }
//float EvalPolyD3(float x, float c[5]) { return EvalPoly(x, c[3] * 6.0, c[4] * 24.0);                  }

// same as above, but multiplication wit 2, 3, or 4 is replaced with addition (gives a small performance boost)
float EvalPolyD0(float x, float c[3]) { return EvalPoly(x, c[0], c[1], c[2]); }
float EvalPolyD1(float x, float c[3]) { return EvalPoly(x, c[1], c[2] + c[2]); }
float EvalPolyD2(float x, float c[3]) { return EvalPoly(x, c[2] + c[2]); }
float EvalPolyD0(float x, float c[5]) { return EvalPoly(x, c[0], c[1], c[2], c[3], c[4]); }
float EvalPolyD1(float x, float c[5]) { return EvalPoly(x, c[1], c[2] + c[2], c[3] + c[3] + c[3], c[4] + c[4] + c[4] + c[4]); }
float EvalPolyD2(float x, float c[5]) { return EvalPoly(x, c[2] + c[2], c[3] * 6.0, c[4] * 12.0); }
float EvalPolyD3(float x, float c[5]) { return EvalPoly(x, c[3] * 6.0, c[4] * 24.0); }

vec3 qSplineEval(float l, float curveX[3], float curveY[3], float curveZ[3]) {
	return vec3(EvalPolyD0(l, curveX), EvalPolyD0(l, curveY), EvalPolyD0(l, curveZ));
}

vec3 EvalCSpline(vec3 p1, vec3 t1, vec3 p2, vec3 t2, float l) {
	vec3 h1 = p1 + t1 / 3.0;
	vec3 h2 = p2 - t2 / 3.0;
	vec3 a1 = mix(p1, h1, l);
	vec3 a2 = mix(h1, h2, l);
	vec3 a3 = mix(h2, p2, l);
	vec3 b1 = mix(a1, a2, l);
	vec3 b2 = mix(a2, a3, l);
	return mix(b1, b2, l);
}

float qSplineIDistEval(float t, float curveX[3], float polyB_C[5]) {		
	float term  = EvalPolyD0(t, curveX);
	float discr = EvalPolyD0(t, polyB_C);
	if(discr < 0.0) return POS_INF;
	else return term - sqrt(discr);
}

float qSplineD1Eval(float t, float curveX[3], float polyB_C[5]) {	 		
	float f1D1 = EvalPolyD1(t, curveX);	
	float f2D0 = EvalPolyD0(t, polyB_C);
	float f2D1 = EvalPolyD1(t, polyB_C);
	return f1D1 - f2D1 * 0.5 * inversesqrt(max(0.0, f2D0));
}

float qSplineD2Eval(float t, float curveX[3], float polyB_C[5]) {		
	float f1D1 = EvalPolyD1(t, curveX);
	float f1D2 = EvalPolyD2(t, curveX);
	float f2D0 = EvalPolyD0(t, polyB_C);
	float f2D1 = EvalPolyD1(t, polyB_C);
	float f2D2 = EvalPolyD2(t, polyB_C);
	f2D0 = max(0.0, f2D0);
	return (Pow2(f2D1) / f2D0 * 0.25 - f2D2 * 0.5) * inversesqrt(f2D0) + f1D2;
}

float qSplineD3Eval(float t, float polyB_C[5]) {
	float f2D0 = EvalPolyD0(t, polyB_C);
	float f2D1 = EvalPolyD1(t, polyB_C);
	float f2D2 = EvalPolyD2(t, polyB_C);
	float f2D3 = EvalPolyD3(t, polyB_C);
	f2D0 = max(0.0, f2D0);
	return (-3.0 * Pow3(f2D1) + 6.0 * f2D0 * f2D1 * f2D2 - 4.0 * Pow2(f2D0) * f2D3) / Pow2(f2D0) * inversesqrt(f2D0);
}

float qSplineIDistEval_BinRootFinder_Eval(float n, float p, float curveX[3], float polyB_C[5], int max_iter) {
	if(qSplineIDistEval(n, curveX, polyB_C) > 0.0) return n;
	if(qSplineIDistEval(p, curveX, polyB_C) < 0.0) return p;

	for(int i = 0; i < max_iter; ++i) {
		float m = (n + p) * 0.5;
		float f = qSplineIDistEval(m, curveX, polyB_C);
		if(f < 0.0) n = m;
		else p = m;
	}

	return (n + p) * 0.5;
}

float qSplineD1_BinRootFinder_Eval(float n, float p, float curveX[3], float polyB_C[5], int max_iter) {
	if(qSplineD1Eval(n, curveX, polyB_C) > 0.0) return n;
    if(qSplineD1Eval(p, curveX, polyB_C) < 0.0) return p;

    for(int i = 0; i < max_iter; ++i) {
        float m = (n + p) * 0.5;
        float f = qSplineD1Eval(m, curveX, polyB_C);
        if(f < 0.0) n = m;
        else p = m;
    }

    return (n + p) * 0.5;
}

float qSplineD2_BinRootFinder_Eval(float n, float p, float curveX[3], float polyB_C[5], int max_iter) {
	if(qSplineD2Eval(n, curveX, polyB_C) > 0.0) return n;
	if(qSplineD2Eval(p, curveX, polyB_C) < 0.0) return p;

	for(int i = 0; i < max_iter; ++i) {
		float m = (n + p) * 0.5;
		float f = qSplineD2Eval(m, curveX, polyB_C);
		if(f < 0.0) n = m;
		else p = m;
	}

	return (n + p) * 0.5;
}

float qSplineD3_BinRootFinder_Eval(float n, float p, float polyB_C[5], int max_iter) {
	if(qSplineD3Eval(n, polyB_C) > 0.0) return n;
	if(qSplineD3Eval(p, polyB_C) < 0.0) return p;

	for(int i = 0; i < max_iter; ++i) {
		float m = (n + p) * 0.5;
		float f = qSplineD3Eval(m, polyB_C);
		if(f < 0.0) n = m;
		else p = m;
	}

	return (n + p) * 0.5;
}

float qSplineIDistEval_NewtonRootFinder_Eval(float n, float p, float curveX[3], float polyB_C[5], int max_iter) {
	if(qSplineIDistEval(n, curveX, polyB_C) > 0.0) return n;
	if(qSplineIDistEval(p, curveX, polyB_C) < 0.0) return p;
	
	float x0 = (n + p) * 0.5;
    for (int i = 0; i < max_iter; i++) {
        float f = qSplineIDistEval(x0, curveX, polyB_C);
        float df = qSplineD1Eval(x0, curveX, polyB_C);
        x0 -= f / df;
    }
    return x0;
}

float qSplineD1_NewtonRootFinder_Eval(float n, float p, float curveX[3], float polyB_C[5], int max_iter) {
	if(qSplineD1Eval(n, curveX, polyB_C) > 0.0) return n;
	if(qSplineD1Eval(p, curveX, polyB_C) < 0.0) return p;
	
	float x0 = (n + p) * 0.5;
    for (int i = 0; i < max_iter; i++) {
        float f = qSplineD1Eval(x0, curveX, polyB_C);
        float df = qSplineD2Eval(x0, curveX, polyB_C);
        x0 -= f / df;
    }
    return x0;
}
float qSplineD2_NewtonRootFinder_Eval(float n, float p, float curveX[3], float polyB_C[5], int max_iter) {
	if(qSplineD2Eval(n, curveX, polyB_C) > 0.0) return n;
	if(qSplineD2Eval(p, curveX, polyB_C) < 0.0) return p;
	
	float x0 = (n + p) * 0.5;
    for (int i = 0; i < max_iter; i++) {
        float f = qSplineD2Eval(x0, curveX, polyB_C);
        float df = qSplineD3Eval(x0, polyB_C);
        x0 -= f / df;
    }
    return x0;
}

float BisectionMethodPolyD0(float c[5], float n, float p, int max_iter) {
    for(int j = 0; j < max_iter; ++j) {                                           
		float m = (n + p) * 0.5;                                                          
		float f = EvalPolyD0(m, c);												  
		if(f < 0.0) n = m;                                                                
		else p = m;                                                                       
	}                                                                                     
	return (n + p) * 0.5;
}

float BisectionMethodPolyD1(float c[5], float n, float p, int max_iter) {
    for(int j = 0; j < max_iter; ++j) {                                           
		float m = (n + p) * 0.5;                                                          
		float f = EvalPolyD1(m, c);												  
		if(f < 0.0) n = m;                                                                
		else p = m;                                                                       
	}                                                                                     
	return (n + p) * 0.5;
}

float NewtonsMethodPolyD0(float c[5], float x, int max_iter) {
    float x0 = x;
    for (int i = 0; i < max_iter; i++) {
        float f = EvalPolyD0(x0, c);
        float df = EvalPolyD1(x0, c);
        x0 -= f / df;
    }
    return x0;
}

float NewtonsMethodPolyD1(float c[5], float x, int max_iter) {
    float x0 = x;
    for (int i = 0; i < max_iter; i++) {
        float f = EvalPolyD1(x0, c);
        float df = EvalPolyD2(x0, c);
        x0 -= f / df;
    }
    return x0;
}

float NewtonsMethodPolyD0(float c[5], float x, float epsilon, int max_iter) {
    float x0 = x;
    for (int i = 0; i < max_iter; i++) {
        float f = EvalPolyD0(x0, c);
        if (abs(f) < epsilon) break;
        float df = EvalPolyD1(x0, c);
        x0 -= f / df;
    }
    return x0;
}

float NewtonsMethodPolyD1(float c[5], float x, float epsilon, int max_iter) {
    float x0 = x;
    for (int i = 0; i < max_iter; i++) {
        float f = EvalPolyD1(x0, c);
        if (abs(f) < epsilon) break;
        float df = EvalPolyD2(x0, c);
        x0 -= f / df;
    }
    return x0;
}

float HybridMethodPolyD0(float c[5], float n, float p, int max_iter) {
	float x0 = (n + p) * 0.5;
	float f;
    for (int i = 0; i < max_iter; i++) {
        f = EvalPolyD0(x0, c);
        float df = EvalPolyD1(x0, c);
        x0 -= f / df;
    }
    return (abs(f) < 1e-7) ? x0 : BisectionMethodPolyD0(c, n, p, max_iter * 2);
}

float HybridMethodPolyD1(float c[5], float n, float p, int max_iter) {
	float x0 = (n + p) * 0.5;
	float f;
	for (int i = 0; i < max_iter; i++) {
		f = EvalPolyD1(x0, c);
		float df = EvalPolyD2(x0, c);
		x0 -= f / df;
	}
	return (abs(f) < 1e-7) ? x0 : BisectionMethodPolyD1(c, n, p, max_iter * 2);
}

void FindRootsPolyD0(float poly_C[5], float x_i[5], int m_i[5], out float x_o[6], out int m_o[6], int max_iter) {
	m_o[0] = m_o[5] = 1;
	x_o[0] = x_i[0];
	float x_l = x_i[0];
	float y_l = EvalPolyD0(x_l, poly_C);
	float sy_l = sign(y_l);
	for(int i = 1; i < 5; ++i) {
		if(m_i[i] == 1) {
			float x_r = x_i[i];
			float y_r = EvalPolyD0(x_r, poly_C);
			float sy_r = sign(y_r);
			x_o[i] = 0.0;
			if(sy_l != sy_r) {
				float n = x_l;
				float p = x_r;
				float ny = EvalPolyD0(n, poly_C);
				float py = EvalPolyD0(p, poly_C);
				if(ny > 0.0 && py < 0.0) {
					float t = n;
					n = p; p = t;
				}
				x_o[i] = BisectionMethodPolyD0(poly_C, n, p, max_iter);
				//x_o[i] = NewtonsMethodPolyD0(poly_C, (n+p) * 0.5, 5);
				m_o[i] = 1;
			} else {
				m_o[i] = 0;
			}
			x_l = x_r;
			y_l = y_r;
			sy_l = sy_r;
		} else {
			m_o[i] = 0;
		}
	}
	x_o[5] = x_i[4];
}

void FindRootsPolyD1(float poly_C[5], float x_i[4], int m_i[4], out float x_o[5], out int m_o[5], int max_iter) {
	m_o[0] = m_o[4] = 1;
	x_o[0] = x_i[0];
	float x_l = x_i[0];
	float y_l = EvalPolyD1(x_l, poly_C);
	float sy_l = sign(y_l);
	for(int i = 1; i < 4; ++i) {
		if(m_i[i] == 1) {
			float x_r = x_i[i];
			float y_r = EvalPolyD1(x_r, poly_C);
			float sy_r = sign(y_r);
			x_o[i] = 0.0;
			if(sy_l != sy_r) {
				float n = x_l;
				float p = x_r;
				float ny = EvalPolyD1(n, poly_C);
				float py = EvalPolyD1(p, poly_C);
				if(ny > 0.0 && py < 0.0) {
					float t = n;
					n = p; p = t;
				}
				x_o[i] = BisectionMethodPolyD1(poly_C, n, p, max_iter);
				//x_o[i] = NewtonsMethodPolyD1(poly_C, (n+ p) * 0.5, 5);
				m_o[i] = 1;
			} else {
				m_o[i] = 0;
			}
			x_l = x_r;
			y_l = y_r;
			sy_l = sy_r;
		} else {
			m_o[i] = 0;
		}
	}
	x_o[4] = x_i[3];
}