@func(gen_vector; :>comp="", :>post="", :>offset="", :>type_id="", :>type_name="", :>gl_suf="", :>gl_type="", :>cast=0, :>return="")@{
@define(:>conv="")
@if(gl_type)@{
	@define(conv="(".gl_type.")")
@}
	case @(type_id)+@(offset) : 
		{
			const @(comp)<@(type_name)@(post)>& v = *reinterpret_cast<const @(comp)<@(type_name)@(post)>*>(value_ptr);
			if (force_array) {
@if(gl_type)@{
	@if(cast)@{
				glUniform1@(gl_suf)v(loc, v.size(), reinterpret_cast<const @(gl_type)*>(&v[0]));
	@}@else@{
				@(comp)<@(gl_type)@(post)> vi = v; glUniform1@(gl_suf)v(loc, vi.size(), &vi[0]);
	@}
@}@else@{
				glUniform1@(gl_suf)v(loc, v.size(), &v[0]);
@}
			}
			else {
				switch (v.size()) {
				case 2 : glUniform2@(gl_suf)(loc, @(conv)v[0], @(conv)v[1]); break;
				case 3 : glUniform3@(gl_suf)(loc, @(conv)v[0], @(conv)v[1], @(conv)v[2]); break;
				case 4 : glUniform4@(gl_suf)(loc, @(conv)v[0], @(conv)v[1], @(conv)v[2], @(conv)v[3]); break;
@if(gl_type)@{
	@if(cast)@{
				default: glUniform1@(gl_suf)v(loc, v.size(), reinterpret_cast<const @(gl_type)*>(&v[0])); break;
	@}@else@{
				default: { @(comp)<@(gl_type)@(post)> vi = v; glUniform1@(gl_suf)v(loc, vi.size(), &vi[0]); } break;
	@}
@}@else@{
				default: glUniform1@(gl_suf)v(loc, v.size(), &v[0]); break;
@}
				}
			}
			break;
		}
@}
@func(gen_matrices; :>comp="", :>offset="", :>post="", :>return="")@{
	case TI_FLT32+@(offset):
		{
			const @(comp)<flt32_type@(post)>& m = *reinterpret_cast<const @(comp)<flt32_type@(post)>*>(value_ptr);
			switch (m.size()) {
			case  4 : glUniformMatrix2fv(loc, 1, 0, &m(0,0)); break;
			case  9 : glUniformMatrix3fv(loc, 1, 0, &m(0,0)); break;
			case 16 : glUniformMatrix4fv(loc, 1, 0, &m(0,0)); break;
			}
			break;
		}
	case TI_FLT64+@(offset):
		{
			const @(comp)<flt64_type@(post)>& md = *reinterpret_cast<const @(comp)<flt32_type@(post)>*>(value_ptr);
			@(comp)<flt32_type@(post)> m = md;
			switch (m.size()) {
			case  4 : glUniformMatrix2fv(loc, 1, 0, &m(0,0)); break;
			case  9 : glUniformMatrix3fv(loc, 1, 0, &m(0,0)); break;
			case 16 : glUniformMatrix4fv(loc, 1, 0, &m(0,0)); break;
			}
			break;
		}
@}

@func(gen_vectors; :>comp="", :>offset="", :>post="", :>return="")@{
	@skip(gen_vector(comp, post, offset, "TI_INT8",   "int8_type",   "i", "int32_type"))
	@skip(gen_vector(comp, post, offset, "TI_INT16",  "int16_type",  "i", "int32_type"))
	@skip(gen_vector(comp, post, offset, "TI_INT32",  "int32_type",  "i"))
	@skip(gen_vector(comp, post, offset, "TI_UINT8",  "uint8_type",  "i", "int32_type"))
	@skip(gen_vector(comp, post, offset, "TI_UINT16", "uint16_type", "i", "int32_type"))
	@skip(gen_vector(comp, post, offset, "TI_UINT32", "uint32_type", "i", "int32_type", 1))
	@skip(gen_vector(comp, post, offset, "TI_FLT32",  "flt32_type",  "f"))
	@skip(gen_vector(comp, post, offset, "TI_FLT64",  "flt64_type",  "f", "flt32_type"))
@}
@skip(gen_vectors("vec", "TO_VEC"))
@//skip(gen_vectors("std::vector", "TO_VECTOR"))
@skip(gen_matrices("mat", "TO_MAT"))
@skip(gen_vectors("fvec", "TO_FVEC2", ",2"))
@skip(gen_vectors("fvec", "TO_FVEC3", ",3"))
@skip(gen_vectors("fvec", "TO_FVEC4", ",4"))
@skip(gen_matrices("fmat", "TO_FMAT2", ",2,2"))
@skip(gen_matrices("fmat", "TO_FMAT3", ",3,3"))
@skip(gen_matrices("fmat", "TO_FMAT4", ",4,4"))
