#include <cgv/signal/rebind.h>
#include <cgv/base/node.h>
#include <cgv/base/register.h>
#include <cgv/math/fvec.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/arrow_renderer.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/box_wire_renderer.h>
#include <random>

template <typename T>
struct rigid_body 
{
	typename cgv::math::fvec<T,3> X;
	typename cgv::math::quaternion<T> O;
	typename cgv::math::fvec<T,3> P;
	typename cgv::math::fvec<T,3> L;
	T iM;
	typename cgv::math::fvec<T,3> iI;
	typename cgv::math::fvec<T,3> extent;
	cgv::rgb color;
	rigid_body(
		const cgv::math::fvec<T, 3>& _X, const cgv::math::quaternion<T>& _O,
		const cgv::math::fvec<T, 3>& _P, const cgv::math::fvec<T, 3>& _L,
		T _iM, const cgv::math::fvec<T, 3>& _iI,
		const cgv::math::fvec<T, 3>& _extent, const cgv::rgb& _color) : X(_X), O(_O), P(_P), L(_L), iM(_iM), iI(_iI), extent(_extent), color(_color) {}
	template <typename S>
	rigid_body(const rigid_body<S>& b) : X(b.X), O(b.O), P(b.P), L(b.L), iM(b.iM), iI(b.iI), extent(b.extent), color(b.color) {}
	typename cgv::math::fmat<T, 3, 3> compute_iI() const
	{
		typename cgv::math::fmat<T, 3, 3> R, Rt;
		O.put_matrix(R);
		Rt = transpose(R);
		typename cgv::math::fmat<T, 3, 3> invI(T(0));
		invI(0, 0) = iI[0];
		invI(1, 1) = iI[1];
		invI(2, 2) = iI[2];
		return R * invI * Rt;
	}
	typename cgv::math::fmat<T, 3, 3> compute_I() const
	{
		typename cgv::math::fmat<T, 3, 3> R, Rt;
		O.put_matrix(R);
		Rt = transpose(R);
		typename cgv::math::fmat<T, 3, 3> I(T(0));
		I(0, 0) = T(1) / iI[0];
		I(1, 1) = T(1) / iI[1];
		I(2, 2) = T(1) / iI[2];
		return R * I * Rt;
	}
	typename cgv::math::fvec<T, 3> compute_omega() const
	{
		return compute_iI() * L;
	}
	void integrate(T dt, const cgv::math::fvec<T, 3>& Fo, const cgv::math::fvec<T, 3>& To, bool use_matrix, bool do_normalize, bool integrate_omega)
	{
		X += dt * iM * P;
		typename cgv::math::fvec<T, 3> omega = compute_omega();
		if (use_matrix) {
			typename cgv::math::fmat<T, 3, 3> R, dR;
			O.put_matrix(R);
			dR.set_col(0, cross(omega, R.col(0)));
			dR.set_col(1, cross(omega, R.col(1)));
			dR.set_col(2, cross(omega, R.col(2)));
			R = R + dt * dR;
			cgv::math::mat<T> A(3,3,&R(0,0)), U, V;
			cgv::math::diag_mat<T> w;
			cgv::math::svd(A, U, w, V);
			A = U * transpose(V);
			R = cgv::math::fmat<T,3,3>(3,3,&A(0,0));
			O.set(R);
			if (do_normalize)
				O.normalize();
		}
		else {
			T angular_velocity = omega.length();
			if (angular_velocity > std::numeric_limits<float>::epsilon()) {
				O = typename cgv::math::quaternion<T>(omega / angular_velocity, dt * angular_velocity) * O;
				if (do_normalize)
					O.normalize();
			}
		}
		P += dt * Fo;
		if (integrate_omega) {
			// we can use \dot\omega = torque - omega x I omega;
			auto I = compute_I();
			auto alpha = compute_iI() * (To - cross(omega, I * omega));
			omega += dt * alpha;
			L = I * omega;
		}
		else
			L += dt * To;
	}
};

class rigid_body_test :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider
{
public:
protected:
	// simulation data
	bool animate = true;
	double t;
	double h = 0.001f;
	float gravity = 9.81f;
	bool apply_gravity = false;
	bool use_matrix = false;
	bool do_normalize = true;
	bool integrate_omega = false;
	double time_scale = 1.0f;
	std::vector<rigid_body<float>>   B;
	std::vector<rigid_body<double>> dB;
	// visualization data
	bool wireframe = true;
	bool use_double = true;
	bool show_boxes = true;
	bool show_centers = true;
	bool show_linear_momentum = false;
	bool show_angular_momentum = true;
	bool show_linear_velocity = false;
	bool show_angular_velocity = true;
	bool show_Y = true;
	// render data
	cgv::render::view* view_ptr;
	cgv::render::box_render_style box_style;
	cgv::render::box_wire_render_style box_wire_style;
	cgv::render::arrow_render_style arrow_style;
	cgv::render::sphere_render_style sphere_style;
	cgv::render::attribute_array_manager b_manager;
	cgv::render::attribute_array_manager bw_manager;
	cgv::render::attribute_array_manager a_manager;
	cgv::render::attribute_array_manager s_manager;
	// simulation methods
	void compute_omegas(std::vector<cgv::vec3>& omegas)
	{
		omegas.clear();
		if (use_double)
			for (const auto& b : dB)
				omegas.push_back(b.compute_omega());
		else
			for (const auto& b : B)
				omegas.push_back(b.compute_omega());
	}
	void integrate(double dt)
	{
		if (use_double) {
			cgv::dvec3 g(0.0, apply_gravity ? double(-gravity) : 0.0, 0.0);
			cgv::dvec3 T(0.0);
			for (auto& b : dB)
				b.integrate(dt, g / b.iM, T, use_matrix, do_normalize, integrate_omega);
		}
		else {
			cgv::vec3 g(0.0f, apply_gravity ? -gravity : 0.0f, 0.0f);
			cgv::vec3 T(0.0f);
			for (auto& b : B)
				b.integrate(float(dt), g / b.iM, T, use_matrix, do_normalize, integrate_omega);
		}
	}
	void timer_event(double, double dt)
	{
		if (!animate)
			return;
		double T = 0.0;
		dt *= time_scale;
		while (T < dt) {
			integrate(h);
			T += h;
			t += T;
		}
		update_member(&t);
		post_redraw();
	}
public:
	rigid_body<float> construct_body(const cgv::vec3 X, const cgv::quat& O, const cgv::vec3& P, const cgv::vec3& L,
		float rho, const cgv::vec3& E, const cgv::rgb& color)
	{
		float M = rho * E[0] * E[1] * E[2];
		cgv::vec3 I = M/12*cgv::vec3(E[1]*E[1]+ E[2]*E[2], E[2]*E[2]+ E[0]*E[0], E[0]*E[0]+ E[1]*E[1]);
		float iM = 1.0f / M;
		cgv::vec3 iI(1.0f/I[0],1.0f/I[1],1.0f/I[2]);
		return
			{ X,O,P,L,iM,iI,E,color };
	}
	void construct_scene()
	{
		B.clear();
		B.push_back(
			construct_body(
				{ -8.0f,0.0f,0.0f }, { 1.0f,0.0f,0.0f,0.0f },    // X, O
				{ 0.0f,0.0f,0.0f }, { 0.03f,50.0f,0.0f },          // P, L
				1.0f, { 1.0f,4.0f,8.0f }, { 1.0f,0.7f,0.3f }));  // rho, extent, color
		B.push_back(
			construct_body(
				{ 0.0f,0.0f,0.0f }, { 1.0f,0.0f,0.0f,0.0f },     // X, O
				{ 0.0f,0.0f,0.0f }, { 0.03f,50.0f,0.0f },          // P, L
				1.0f, { 4.0f,8.0f,1.0f }, { 1.0f,0.7f,0.3f }));  // rho, extent, color
		B.push_back(
			construct_body(
				{ 8.0f,0.0f,0.0f }, { 1.0f,0.0f,0.0f,0.0f },    // X, O
				{ 0.0f,0.0f,0.0f }, { 0.03f,50.0f,0.0f },          // P, L
				1.0f, { 8.0f,1.0f,4.0f }, { 1.0f,0.7f,0.3f }));  // rho, extent, color

		dB.clear();
		for (const auto& b : B)
			dB.push_back(rigid_body<double>(b));
	}
	/// define format and texture filters in constructor
	rigid_body_test() : cgv::base::node("Rigid Body")
	{
		construct_scene();
		t = 0.0f;
		arrow_style.length_scale = 5.0f;
		arrow_style.radius_lower_bound = 0.1f;
		arrow_style.radius_relative_to_length = 0.0f;
		sphere_style.radius = 0.01f;
		sphere_style.use_group_color = false;
		sphere_style.map_color_to_material = cgv::render::CM_COLOR_AND_OPACITY;
		connect(cgv::gui::get_animation_trigger().shoot, this, &rigid_body_test::timer_event);
	}
	std::string get_type_name() const
	{
		return "rigid_body";
	}
	void on_set(void* member_ptr)
	{
		update_member(member_ptr);
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{
		if ((view_ptr = find_view_as_node())) {
			view_ptr->set_focus(0.0, 0.0, 0.0);
			view_ptr->set_y_extent_at_focus(40.0);
		}
		ctx.set_bg_clr_idx(4);
		// create attribute managers
		if (!b_manager.init(ctx))
			return false;
		if (!bw_manager.init(ctx))
			return false;
		if (!a_manager.init(ctx))
			return false;
		if (!s_manager.init(ctx))
			return false;
		// increase reference counts of used renderer singletons
		cgv::render::ref_box_renderer			(ctx, 1);
		cgv::render::ref_box_wire_renderer		(ctx, 1);
		cgv::render::ref_arrow_renderer			(ctx, 1);
		cgv::render::ref_sphere_renderer		(ctx, 1);
		return true;
	}
	void draw(cgv::render::context& ctx)
	{
		std::vector<cgv::vec3> X;
		std::vector<cgv::quat> O;
		std::vector<cgv::vec3> E;
		std::vector<cgv::rgb>  C;
		std::vector<cgv::vec3> L;
		std::vector<cgv::vec3> Y;
		if (use_double) {
			for (const auto& b : dB) {
				X.push_back(b.X);
				O.push_back(b.O);
				E.push_back(b.extent);
				C.push_back(b.color);
				L.push_back(b.L);
				cgv::dmat3 R;
				b.O.put_matrix(R);
				Y.push_back(R.col(1));
			}
		}
		else {
			for (const auto& b : B) {
				X.push_back(b.X);
				O.push_back(b.O);
				E.push_back(b.extent);
				C.push_back(b.color);
				L.push_back(b.L);
				cgv::mat3 R;
				b.O.put_matrix(R);
				Y.push_back(R.col(1));
			}
		}
		std::vector<cgv::vec3> omegas;
		compute_omegas(omegas);
		if (show_boxes) {
			if (wireframe) {
				auto& r = cgv::render::ref_box_wire_renderer(ctx);
				r.set_render_style(box_wire_style);
				r.set_position_array(ctx, X);
				r.set_rotation_array(ctx, O);
				r.set_color_array(ctx, C);
				r.set_extent_array(ctx, E);
				r.render(ctx, 0, B.size());
			}
			else {
				auto& r = cgv::render::ref_box_renderer(ctx);
				r.set_render_style(box_style);
				r.set_position_array(ctx, X);
				r.set_rotation_array(ctx, O);
				r.set_color_array(ctx, C);
				r.set_extent_array(ctx, E);
				r.render(ctx, 0, B.size());
			}
		}
		if (show_angular_momentum) {
			auto& r = cgv::render::ref_arrow_renderer(ctx);
			r.set_render_style(arrow_style);
			r.set_color(ctx, cgv::rgba(0.0f, 0.3f, 1.0f,1.0f));
			r.set_position_array(ctx, X);
			r.set_direction_array(ctx, L);
			r.render(ctx, 0, B.size());
		}
		if (show_angular_velocity) {
			auto& r = cgv::render::ref_arrow_renderer(ctx);
			r.set_render_style(arrow_style);
			r.set_color(ctx, cgv::rgba(1.0f, 0.3f, 0.0f,1.0f));
			r.set_position_array(ctx, X);
			r.set_direction_array(ctx, omegas);
			r.render(ctx, 0, B.size());
		}
		if (show_Y) {
			auto& r = cgv::render::ref_arrow_renderer(ctx);
			r.set_render_style(arrow_style);
			r.set_color(ctx, cgv::rgba(1.0f, 0.3f, 0.0f,1.0f));
			r.set_position_array(ctx, X);
			r.set_direction_array(ctx, Y);
			r.render(ctx, 0, B.size());
		}
	}
	void clear(cgv::render::context& ctx)
	{
		// clear attribute managers
		b_manager.destruct(ctx);
		bw_manager.destruct(ctx);
		a_manager.destruct(ctx);
		s_manager.destruct(ctx);

		// decrease reference counts of used renderer singletons
		cgv::render::ref_box_renderer			(ctx, -1);
		cgv::render::ref_box_wire_renderer		(ctx, -1);
		cgv::render::ref_arrow_renderer			(ctx, -1);
		cgv::render::ref_sphere_renderer		(ctx, -1);
	}
	void on_restart()
	{
		t = 0;
		construct_scene();
		update_member(&t);
	}
	void create_gui()
	{
		add_decorator("Rigid Body Test", "heading");
		add_view("time", t);
		add_member_control(this, "Use Double", use_double, "toggle", "shortcut='D'");
		add_member_control(this, "Use Matrix", use_matrix, "toggle", "shortcut='M'");
		add_member_control(this, "Do Normalize", do_normalize, "toggle", "shortcut='N'");
		add_member_control(this, "Integrate Omega", integrate_omega, "toggle", "shortcut='I'");
		connect_copy(add_button("Restart", "shortcut='R'")->click, cgv::signal::rebind(this, &rigid_body_test::on_restart));
		add_member_control(this, "Animate", animate, "toggle", "shortcut='A'");
		add_member_control(this, "Time Scale", time_scale, "value_slider", "min=0.1;max=100.0;log=true;step=0.01");
		add_member_control(this, "Step Size", h, "value_slider", "min=0.00001;max=0.1;log=true;step=0.000001");
		add_member_control(this, "Apply Gravity", apply_gravity, "check", "shortcut='G'");
		add_member_control(this, "Show Boxes", show_boxes, "check", "shortcut='B'");
		add_member_control(this, "Wireframe", wireframe, "check", "shortcut='W'");
		add_member_control(this, "Show Centers", show_centers, "check", "shortcut='C'");
		add_member_control(this, "Show Linear Momenta", show_linear_momentum, "check", "shortcut='P'");
		add_member_control(this, "Show Linear Velocity", show_linear_momentum, "check", "shortcut='V'");
		add_member_control(this, "Show Angular Momenta", show_angular_momentum, "check", "shortcut='L'");
		add_member_control(this, "Show Angular Velocity", show_angular_velocity, "check", "shortcut='O'");
		add_member_control(this, "Show Y", show_angular_velocity, "check", "shortcut='Y'");
		if (begin_tree_node("Box Style", box_style, false)) {
			align("\a");
			add_gui("box_style", box_style);
			align("\b");
			end_tree_node(box_style);
		}
		if (begin_tree_node("Box Wire Style", box_wire_style, false)) {
			align("\a");
			add_gui("box_wire_style", box_wire_style);
			align("\b");
			end_tree_node(box_wire_style);
		}
		if (begin_tree_node("Arrow Style", arrow_style, false)) {
			align("\a");
			add_gui("arrow_style", arrow_style);
			align("\b");
			end_tree_node(arrow_style);
		}
		if (begin_tree_node("Sphere Style", sphere_style, false)) {
			align("\a");
			add_gui("sphere_style", sphere_style);
			align("\b");
			end_tree_node(sphere_style);
		}
	}
};

cgv::base::factory_registration<rigid_body_test> fr_renderer_test("New/Demo/Rigid Body", 'R', true);
