#version 330 core

//////
//
// Shader module interface definition
//

///***** begin interface of curve_tools.glsl **********************************/
// --- link dependencies ---------------
/* transform.glsl */

// --- functions -----------------------

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

float BisectionMethodPolyD0(float c[5], float n, float p, int max_iter);
float BisectionMethodPolyD1(float c[5], float n, float p, int max_iter);

float NewtonsMethodPolyD0(float c[5], float x, int max_iter);
float NewtonsMethodPolyD1(float c[5], float x, int max_iter);
///***** end interface of curve_tools.glsl ************************************/


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
