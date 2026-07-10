# Ellipsoidal Case

input:

- center curve $\mathbf c(t)$ and symmetric matrix polynomial with  $\mathbf T(t)$

- ray: $\mathbf x(t)=\mathbf o+s\cdot\hat v$ with $t\geq 0$

output

- $s_{\mathrm{fst}}$ ray parameter and curve parameter $t$ of first intersection with $g(x,t)=(x\hat v-c(t))^t\mathbf T(t)(x\hat v-c(t))-1=0$

$\frac{\partial g}{\partial x}=2\mathbf T(t)\left(x-c(t)\right)$

$\frac{\partial g}{\partial t}=(x-c(t))^t\mathbf{\dot T}(t)(x-c(t))-2(x-c(t))^t\mathbf T(t)\dot c(t)$

$\frac{dx}{dt}=-\left[\left(c_x(t)-x\right)\dot c_x(t)+c_y(t)\dot c_y(t)+c_z(t)\dot c_z(t)-r(t)\dot r(t)\right]/\left(x-c_x(t)\right)$

# Sphere Case

input:

* center curve $\mathbf c(t)$ and radius polynomial $r(t)$ 

* ray: $\mathbf x(t)=\mathbf o+s\cdot\hat v$ with $t\geq 0$

output

* $s_{\mathrm{fst}}$ ray parameter of first intersection

transform such that ray origin $\mathbf o$ is in coordinate origin and $\hat v$ coincides with x-direction. The ray sphere-tube entering intersection is found at ray-parameters that minimize the x-coordinate and the leaving intersections at maxima in the x-coordinate. Both can be found from $\frac{dx}{dt}=0$.

The sphere surface constraint implicitly relates $x$ with $t$:

 $g(x,t)=0=(c_x(t)-x)^2+c^2_y(t)+c_z^2(t)-r^2(t)$

and the Implicit Function Theorem allows computation of $\frac{dx}{dt}=-\frac{\partial g}{\partial t}/\frac{\partial g}{\partial x}$:

$\frac{\partial g}{\partial t}=2\left[\left(c_x(t)-x\right)\dot c_x(t)+2c_y(t)\dot c_y(t)+2c_z(t)\dot c_z(t)-2r(t)\dot r(t)\right]$ 

$\frac{\partial g}{\partial x}=2\left(x-c_x(t)\right)$ 

Thus we get 
$\frac{dx}{dt}=-\left[\left(c_x(t)-x\right)\dot c_x(t)+c_y(t)\dot c_y(t)+c_z(t)\dot c_z(t)-r(t)\dot r(t)\right]/\left(x-c_x(t)\right)$

Here we assume $x\not=c_x(t)$. From $\frac{dx}{dt}=0$ we get:

$\left(c_x(t)-x\right)\dot c_x(t)+c_y(t)\dot c_y(t)+c_z(t)\dot c_z(t)-r(t)\dot r(t)=0$

To eliminate dependency from $x$, we separate term containing it, square it and eliminate square form constraint:

$\left(c_x(t)-x\right)\dot c_x(t)= r(t)\dot r(t)-c_y(t)\dot c_y(t)-c_z(t)\dot c_z(t)$

$\left(c_x(t)-x\right)^2\dot c^2_x(t)= \left[r(t)\dot r(t)-c_y(t)\dot c_y(t)-c_z(t)\dot c_z(t)\right]^2$

$\left[r^2(t)-c^2_y(t)-c_z^2(t)\right]\dot c^2_x(t)= \left[r(t)\dot r(t)-c_y(t)\dot c_y(t)-c_z(t)\dot c_z(t)\right]^2$

The final result is polynomial of degree 10 in $t$.

Alternatively, we can directly solve $g(x,t)=0$ for $x$:

$x=c_x(t)\pm\sqrt{r^2(t)-c_y^2(t)-c_z^2(t)}=h(t)\pm\sqrt{q(t)}$

Next compute derivative

$\dot x(t)=\dot h(t)\pm\frac{\dot q(t)}{2\sqrt{q(t)}}$

Choose first - thus - negative solution and set equal to zero:

$\dot h(t)=\frac{\dot q(t)}{2\sqrt{q(t)}} \Rightarrow 2\sqrt{q(t)}\dot h(t)=\dot q(t)$

Finally, we square both sides

$4q(t)\dot h^2(t)=\dot q^2(t)$

Plugin $h(t)$ and $q(t)$ back in:

$4\left[r^2(t)-c_y^2(t)-c_z^2(t)\right]\dot c_x^2(t)=\left[2r(t)\dot r(t)-2c_y(t)\dot c_y(t)-2c_z(t)\dot c_z(t)\right]^2$

Again this is polynomial of degree 10 - actually the same as with the Langrangian multiplier method.

# Circle Case

input:

- center curve $\mathbf c(t)$ and radius polynomial $r(t)$

- ray: $\mathbf x(t)=\mathbf o+s\cdot\hat v$ with $t\geq 0$

output

- $s_{\mathrm{fst}}$ ray parameter of first intersection

At every center curve point we define a plane from the center curve point and tangent:

$\left<\mathbf o+s\cdot\hat v-\mathrm c(t),\dot c(t)\right>=0$

We can solve for $s$:

$s = \frac{\left<\mathrm c(t)-\mathrm o,\dot c(t)\right>}{\left<\hat v,\dot c(t)\right>}=
\frac{\left<\mathrm c(t),\dot c(t)\right>}{\dot c_x(t)}$

This becomes numerically unstable if viewing ray becomes parallel to plane.

Now the resulting point should be at distance $r$ from $c(t)$:
$\left[\mathrm o + \frac{\left<\mathrm c(t),\dot c(t)\right>}{\dot c_x(t)}\hat v - \mathbf c(t)\right]^2=r^2(t)$

Exploiting that $\hat v$ points in x-direction and that the ray starts in coordinate system origin, we get:

$\left(\frac{\left<\mathrm c(t),\dot c(t)\right>}{\dot c_x(t)}-c_x(t)\right)^2+c_y^2(t)+c_z^2(t)=r^2(t)$

Multiplying with $\dot c_x^2(t)$ yields

$\left[\left<\mathrm c(t),\dot c(t)\right>-c_x(t)\dot c_x(t)\right]^2=\dot c_x^2(t)\left[r^2(t)-c_y^2(t)-c_z^2(t)\right]^2$

# Comparison of Both Cases

Both cases can be written in the form

$\lambda_{\mathrm case}(t)=\rho(t)$

furthermore defining the orthogonal square distance $d(t)$ of the curve to the ray helps shortening the terms:

$D(t):=c_y^2(t)+c_z^2(t),\qquad \dot D(t)=2c_y(t)\dot c_y(t)+2c_z(t)\dot c_z(t)$

The right hand side for both cases is

$\rho(t)=\dot c_x^2(t)\left[r^2(t)-D(t)\right]$

the differing left hand sides are:

$\lambda_{\mathrm sphere}(t)=\frac14\left[\frac{\partial}{\partial t}\left(r^2(t)-D(t)\right)\right]^2 = \left[r(t)\dot r(t)-\frac12\dot D(t)\right]^2$

$\lambda_{\mathrm circle}(t)=\left[\left<\mathrm c(t),\dot c(t)\right>-c_x(t)\dot c_x(t)\right]^2=\left[c_y(t)\dot c_y(t)+c_z(t)\dot c_z(t)\right]^2 = \frac14\left[D(t)\dot D(t)\right]^2$

And finally combining both into one double equation

$\frac14\left[\dot D(t)\right]^2 {\mathrm circle\atop=}\dot c_x^2(t)\left[r^2(t)-D(t)\right]{\mathrm sphere\atop=}\left[r(t)\dot r(t)-\frac12\dot D(t)\right]^2$

In both cases we have polynomials of degree 10 on both sides or in case of degree two curves degree 6.

# View Aligned Ribbon

input:

- center curve $\mathbf c(t)$ and completely arbitrary (no continuity is needed) radius function $r(t)$

- ray: $\mathbf x(t)=\mathbf o+s\cdot\hat v$ with $t\geq 0$

output

- $s_{\mathrm{fst}}$ ray parameter of first intersection with view aligned ribbon

The view aligned ribbon is defined by specifying a bi-tangent direction $b(t)$ for each curve parameter $t$ that is computed from the cross product of the viewing vector that coincides with the positional direction to the curve center $c(t)$ and the curve tangent direction $\dot c(t)$:

$b(t)=c(t)\times\dot c(t)$

The curve point and bi-tangent direction span the bi-tangent line. The idea of computing the ray-ribbon intersection is to exploit the sparsity of line-line intersections in 3D and first solve the ray-bi-tangent intersection problem yielding up to 8 potential solutions $t_i$ and in a second step test for each intersection if the radius is smaller than $r(t_i)$. The problem is solved in eye coordinates where the ray origin is in the eye point and the ray points along the x-direction. 

Plücker Coordinates are used to check for each $t$ whether the ray intersects the ribbon bi-tangent line:

ray: $[d_1,m_1]=\left[\hat v,0\right]$

bi-tanget: $[d_2,m_2]=\left[b(t),c(t)\times b(t)\right]$

Ray and bi-tangent intersection, iff $d_1\cdot m_2+d_2\cdot m_1=0$:

$\hat v\cdot \left[c(t)\times b(t)\right] = 0$

$\left.c(t)\times\left[c(t)\times\dot c(t)\right]\right|_x=0$

$\left.\left<c(t),\dot c(t)\right>c(t)-\left<c(t),c(t)\right>\dot c(t)\right|_x=0$

$\left<c(t),\dot c(t)\right>c_x(t)-\left<c(t),c(t)\right>\dot c_x(t)=0$

Now this seems as a polynomial of degree 8 but in practice this is only of degree 7. The reason is that the leading coefficient of a cubic polynomial derivative is just 3 times the leading coefficient of the polynomial itself. This if $\gamma_{\alpha,i}$ are the monomial coefficients $c_{\alpha}$, i.e. $c_{\alpha}(t)=\sum_{i=0}^3\gamma_{\alpha,i}t^i$ and $\dot\gamma_{\alpha,i}$ of $\dot c_{\alpha}$ then we have $\dot\gamma_{\alpha,2}=3\gamma_{\alpha,3}$. As a result the coefficient $\pi_8$ of $t^8$ in the last equation above computes to

$\pi_8=\gamma_{x,3}\left(\gamma_{x,3}\cdot3\gamma_{x,3}+\gamma_{y,3}\cdot 3\gamma_{y,3}+\gamma_{z,3}\cdot 3\gamma_{z,3}\right)-3\gamma_{x,3}\left(\gamma_{x,3}^2+\gamma_{y,3}^2+\gamma_{z,3}^2\right)=0$

For the coefficient of $t^7$ we have more contributions. In each term of each triple product one can decrease maximum degree to one below individually. For example:

$c_x(t)\dot c_x(t)c_x(t)\Rightarrow\gamma_{x,2}3\gamma_{x,3}\gamma_{x,3}+\gamma_{x,3}2\gamma_{x,2}\gamma_{x,3}+\gamma_{x,3}3\gamma_{x,3}\gamma_{x,2}=8\gamma_{x,3}^2\gamma_{x,2}$

$c_y(t)\dot c_y(t)c_x(t)\Rightarrow\gamma_{y,2}3\gamma_{y,3}\gamma_{x,3}+\gamma_{y,3}2\gamma_{y,2}\gamma_{x,3}+\gamma_{y,3}3\gamma_{y,3}\gamma_{x,2}=5\gamma_{x,3}\gamma_{y,3}\gamma_{y,2}+3\gamma_{x,2}\gamma_{y,3}^2$

The negative terms look like
$c_x(t)^2\dot c_x(t)\Rightarrow\gamma_{x,2}\gamma_{x,3}3\gamma_{x,3}+\gamma_{x,3}\gamma_{x,2}3\gamma_{x,3}+\gamma_{x,3}^22\gamma_{x,2}=8\gamma_{x,3}^2\gamma_{x,2}$

$c_y(t)^2\dot c_x(t)\Rightarrow\gamma_{y,2}\gamma_{y,3}3\gamma_{x,3}+\gamma_{y,3}\gamma_{y,2}3\gamma_{x,3}+\gamma_{y,3}^22\gamma_{x,2}=6\gamma_{x,3}\gamma_{y,3}\gamma_{y,2}+2\gamma_{x,2}\gamma_{y,3}^2$

Again a lot cancels, but not all. It remains

$\pi_7=\gamma_{x,2}\left(\gamma_{y,3}^2+\gamma_{z,3}^2\right)-\gamma_{x,3}\left(\gamma_{y,3}\gamma_{y,2}+\gamma_{z,3}\gamma_{z,2}\right)\not=0$

Probably, the other coefficients could also be computed in a simplified manner.

The intersection point itself can be computed from: $p=\left(\left<m_1,d_2\right>\mathbf 1+d_1m_2^T-d_2m_1^T\right)\frac{d_1\times d_2}{\left\|d_1\times d_2\right\|}$

With $m_1=0$ this simplifies to $p=\left(d_1m_2^T\right)\frac{d_1\times d_2}{\left|d_1\times d_2\right|}$ or $p=\frac{\left<m_2,d_1\times d_2\right>}{\left|d_1\times d_2\right|}d_1$

As $d_1$ is just the x-direction, we can compute the ray parameter to

$s=\frac{\left<m_2,d_1\times d_2\right>}{\left|d_1\times d_2\right|}=\frac{\left<c(t)\times\left(c(t)\times\dot c(t)\right),\hat x\times \left(c(t)\times\dot c(t)\right)\right>}{\left|d_1\times d_2\right|}$
