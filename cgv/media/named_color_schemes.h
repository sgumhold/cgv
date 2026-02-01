/* Sources:

[1] d3js https://d3js.org/
A subset of color schemes was ported from d3js-scale-chromatic (https://github.com/d3/d3-scale-chromatic/).
These include:
// sequential schemes: blues_data, greens_data, grays_data, oranges_data, purples_data, reds_data
// diverging schemes: BrBG_data, PiYG_data, PRGn_data, PuOr_data, RdBu_data, RdGy_data, RdYlBu_data, RdYlGn_data, spectral_data
// cyclic schemes: turbo_fn, viridis_data, magma_data, inferno_data, plasma_data, cividis_fn
// categorical schemes: category10_data, accent8_data, dark8_data, observable10_data, paired12_data, pastel8_1_data, pastel8_2_data, set8_data, set9_data, set12_data, tableau10_data

License:
d3js copyright notice
Copyright 2010-2024 Mike Bostock

Permission to use, copy, modify, and/or distribute this software for any purpose
with or without fee is hereby granted, provided that the above copyright notice
and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.

Apache-Style Software License for ColorBrewer software and ColorBrewer Color Schemes

Copyright 2002 Cynthia Brewer, Mark Harrower, and The Pennsylvania State University

Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

==============================================================================

[2] Matplotlib https://matplotlib.org/
A subset of color schemes was ported from Matpotlib (https://github.com/matplotlib/matplotlib/tree/main/lib/matplotlib/_cm.py).
These include: bone_data, jet_data, hot_data, twilight_data

License:
License agreement for matplotlib versions 1.3.0 and later
=========================================================

1. This LICENSE AGREEMENT is between the Matplotlib Development Team
("MDT"), and the Individual or Organization ("Licensee") accessing and
otherwise using matplotlib software in source or binary form and its
associated documentation.

2. Subject to the terms and conditions of this License Agreement, MDT
hereby grants Licensee a nonexclusive, royalty-free, world-wide license
to reproduce, analyze, test, perform and/or display publicly, prepare
derivative works, distribute, and otherwise use matplotlib
alone or in any derivative version, provided, however, that MDT's
License Agreement and MDT's notice of copyright, i.e., "Copyright (c)
2012- Matplotlib Development Team; All Rights Reserved" are retained in
matplotlib  alone or in any derivative version prepared by
Licensee.

3. In the event Licensee prepares a derivative work that is based on or
incorporates matplotlib or any part thereof, and wants to
make the derivative work available to others as provided herein, then
Licensee hereby agrees to include in any such work a brief summary of
the changes made to matplotlib .

4. MDT is making matplotlib available to Licensee on an "AS
IS" basis.  MDT MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR
IMPLIED.  BY WAY OF EXAMPLE, BUT NOT LIMITATION, MDT MAKES NO AND
DISCLAIMS ANY REPRESENTATION OR WARRANTY OF MERCHANTABILITY OR FITNESS
FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF MATPLOTLIB
WILL NOT INFRINGE ANY THIRD PARTY RIGHTS.

5. MDT SHALL NOT BE LIABLE TO LICENSEE OR ANY OTHER USERS OF MATPLOTLIB
 FOR ANY INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES OR
LOSS AS A RESULT OF MODIFYING, DISTRIBUTING, OR OTHERWISE USING
MATPLOTLIB , OR ANY DERIVATIVE THEREOF, EVEN IF ADVISED OF
THE POSSIBILITY THEREOF.

6. This License Agreement will automatically terminate upon a material
breach of its terms and conditions.

7. Nothing in this License Agreement shall be deemed to create any
relationship of agency, partnership, or joint venture between MDT and
Licensee.  This License Agreement does not grant permission to use MDT
trademarks or trade name in a trademark sense to endorse or promote
products or services of Licensee, or any third party.

8. By copying, installing or otherwise using matplotlib ,
Licensee agrees to be bound by the terms and conditions of this License
Agreement.

License agreement for matplotlib versions prior to 1.3.0
========================================================

1. This LICENSE AGREEMENT is between John D. Hunter ("JDH"), and the
Individual or Organization ("Licensee") accessing and otherwise using
matplotlib software in source or binary form and its associated
documentation.

2. Subject to the terms and conditions of this License Agreement, JDH
hereby grants Licensee a nonexclusive, royalty-free, world-wide license
to reproduce, analyze, test, perform and/or display publicly, prepare
derivative works, distribute, and otherwise use matplotlib
alone or in any derivative version, provided, however, that JDH's
License Agreement and JDH's notice of copyright, i.e., "Copyright (c)
2002-2011 John D. Hunter; All Rights Reserved" are retained in
matplotlib  alone or in any derivative version prepared by
Licensee.

3. In the event Licensee prepares a derivative work that is based on or
incorporates matplotlib  or any part thereof, and wants to
make the derivative work available to others as provided herein, then
Licensee hereby agrees to include in any such work a brief summary of
the changes made to matplotlib.

4. JDH is making matplotlib  available to Licensee on an "AS
IS" basis.  JDH MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR
IMPLIED.  BY WAY OF EXAMPLE, BUT NOT LIMITATION, JDH MAKES NO AND
DISCLAIMS ANY REPRESENTATION OR WARRANTY OF MERCHANTABILITY OR FITNESS
FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF MATPLOTLIB
WILL NOT INFRINGE ANY THIRD PARTY RIGHTS.

5. JDH SHALL NOT BE LIABLE TO LICENSEE OR ANY OTHER USERS OF MATPLOTLIB
 FOR ANY INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES OR
LOSS AS A RESULT OF MODIFYING, DISTRIBUTING, OR OTHERWISE USING
MATPLOTLIB , OR ANY DERIVATIVE THEREOF, EVEN IF ADVISED OF
THE POSSIBILITY THEREOF.

6. This License Agreement will automatically terminate upon a material
breach of its terms and conditions.

7. Nothing in this License Agreement shall be deemed to create any
relationship of agency, partnership, or joint venture between JDH and
Licensee.  This License Agreement does not grant permission to use JDH
trademarks or trade name in a trademark sense to endorse or promote
products or services of Licensee, or any third party.

8. By copying, installing or otherwise using matplotlib,
Licensee agrees to be bound by the terms and conditions of this License
Agreement.

==============================================================================

[3] Cynthia A. Brewer’s ColorBrewer https://colorbrewer2.org/
Many of the color schemes listed under [1] and [2] originate from Cynthia A. Brewer's ColorBrewer

License:
Apache-Style Software License for ColorBrewer software and ColorBrewer Color Schemes

Copyright (c) 2002 Cynthia Brewer, Mark Harrower, and The Pennsylvania State University.

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.
*/

#pragma once

#include <array>
#include <set>

#include <cgv/math/interpolate.h>

#include "color.h"
#include "color_scheme.h"

#include "lib_begin.h"

namespace cgv {
namespace media {
namespace schemes {

/// @brief Implementation of a specialized interpolator for rgb colors that allows separate piecewise linear interpolation of non-uniform breakpoints per color channel.
template<typename ParamT = float>
class rgb_per_channel_piecewise_interpolator : public cgv::math::interpolator<cgv::rgb, ParamT> {
public:
	using point_type = std::pair<ParamT, float>;

	rgb_per_channel_piecewise_interpolator() {}

	rgb_per_channel_piecewise_interpolator(const std::vector<point_type>& red_points, const std::vector<point_type>& green_points, const std::vector<point_type>& blue_points) {
		points[0] = red_points;
		points[1] = green_points;
		points[2] = blue_points;
	}

	std::unique_ptr<interpolator> clone() const override {
		return std::unique_ptr<interpolator>(new rgb_per_channel_piecewise_interpolator(*this));
	};

	cgv::rgb at(ParamT t) const override {
		return {
			cgv::math::interpolate_linear(points[0], t),
			cgv::math::interpolate_linear(points[1], t),
			cgv::math::interpolate_linear(points[2], t)
		};
	}

	std::vector<point_type> points[3];
};

// sequential (single-hue)
constexpr std::array<const char*, 7> blues_data = {
	"deebf79ecae13182bd",
	"eff3ffbdd7e76baed62171b5",
	"eff3ffbdd7e76baed63182bd08519c",
	"eff3ffc6dbef9ecae16baed63182bd08519c",
	"eff3ffc6dbef9ecae16baed64292c62171b5084594",
	"f7fbffdeebf7c6dbef9ecae16baed64292c62171b5084594",
	"f7fbffdeebf7c6dbef9ecae16baed64292c62171b508519c08306b"
};
constexpr std::array<const char*, 7> greens_data = {
	"e5f5e0a1d99b31a354",
	"edf8e9bae4b374c476238b45",
	"edf8e9bae4b374c47631a354006d2c",
	"edf8e9c7e9c0a1d99b74c47631a354006d2c",
	"edf8e9c7e9c0a1d99b74c47641ab5d238b45005a32",
	"f7fcf5e5f5e0c7e9c0a1d99b74c47641ab5d238b45005a32",
	"f7fcf5e5f5e0c7e9c0a1d99b74c47641ab5d238b45006d2c00441b"
};
constexpr std::array<const char*, 7> grays_data = {
	"f0f0f0bdbdbd636363",
	"f7f7f7cccccc969696525252",
	"f7f7f7cccccc969696636363252525",
	"f7f7f7d9d9d9bdbdbd969696636363252525",
	"f7f7f7d9d9d9bdbdbd969696737373525252252525",
	"fffffff0f0f0d9d9d9bdbdbd969696737373525252252525",
	"fffffff0f0f0d9d9d9bdbdbd969696737373525252252525000000"
};
constexpr std::array<const char*, 7> oranges_data = {
	"fee6cefdae6be6550d",
	"feeddefdbe85fd8d3cd94701",
	"feeddefdbe85fd8d3ce6550da63603",
	"feeddefdd0a2fdae6bfd8d3ce6550da63603",
	"feeddefdd0a2fdae6bfd8d3cf16913d948018c2d04",
	"fff5ebfee6cefdd0a2fdae6bfd8d3cf16913d948018c2d04",
	"fff5ebfee6cefdd0a2fdae6bfd8d3cf16913d94801a636037f2704"
};
constexpr std::array<const char*, 7> purples_data = {
	"efedf5bcbddc756bb1",
	"f2f0f7cbc9e29e9ac86a51a3",
	"f2f0f7cbc9e29e9ac8756bb154278f",
	"f2f0f7dadaebbcbddc9e9ac8756bb154278f",
	"f2f0f7dadaebbcbddc9e9ac8807dba6a51a34a1486",
	"fcfbfdefedf5dadaebbcbddc9e9ac8807dba6a51a34a1486",
	"fcfbfdefedf5dadaebbcbddc9e9ac8807dba6a51a354278f3f007d"
};
static constexpr std::array<const char*, 7> reds_data = {
	"fee0d2fc9272de2d26",
	"fee5d9fcae91fb6a4acb181d",
	"fee5d9fcae91fb6a4ade2d26a50f15",
	"fee5d9fcbba1fc9272fb6a4ade2d26a50f15",
	"fee5d9fcbba1fc9272fb6a4aef3b2ccb181d99000d",
	"fff5f0fee0d2fcbba1fc9272fb6a4aef3b2ccb181d99000d",
	"fff5f0fee0d2fcbba1fc9272fb6a4aef3b2ccb181da50f1567000d"
};

static auto interpolateBlues() { return continuous_color_scheme::smooth(to_colors(blues_data.back()), ColorSchemeType::kSequential); }
static auto interpolateGreens() { return continuous_color_scheme::smooth(to_colors(greens_data.back()), ColorSchemeType::kSequential); }
static auto interpolateGrays() { return continuous_color_scheme::smooth(to_colors(grays_data.back()), ColorSchemeType::kSequential); }
static auto interpolateOranges() { return continuous_color_scheme::smooth(to_colors(oranges_data.back()), ColorSchemeType::kSequential); }
static auto interpolatePurples() { return continuous_color_scheme::smooth(to_colors(purples_data.back()), ColorSchemeType::kSequential); }
static auto interpolateReds() { return continuous_color_scheme::smooth(to_colors(reds_data.back()), ColorSchemeType::kSequential); }

static auto schemeBlues() { return discrete_color_scheme(to_colors(blues_data), ColorSchemeType::kSequential); }
static auto schemeGreens() { return discrete_color_scheme(to_colors(greens_data), ColorSchemeType::kSequential); }
static auto schemeGrays() { return discrete_color_scheme(to_colors(grays_data), ColorSchemeType::kSequential); }
static auto schemeOranges() { return discrete_color_scheme(to_colors(oranges_data), ColorSchemeType::kSequential); }
static auto schemePurples() { return discrete_color_scheme(to_colors(purples_data), ColorSchemeType::kSequential); }
static auto schemeReds() { return discrete_color_scheme(to_colors(reds_data), ColorSchemeType::kSequential); }

// sequential (multi-hue)
// Viridis turbo
static cgv::rgb turbo_fn(float t) {
	cgv::rgb color = {
		0.1357f + t * (4.5974f - t * (42.3277f - t * (130.5887f - t * (150.5666f - t * 58.1375f)))),
		0.0914f + t * (2.1856f + t * (4.8052f - t * (14.0195f - t * (4.2109f + t * 2.7747f)))),
		0.1067f + t * (12.5925f - t * (60.1097f - t * (109.0745f - t * (88.5066f - t * 26.8183f))))
	};
	color.clamp();
	return color;
}

// Cividis from Nunez, Anderton, and Renslow (https://journals.plos.org/plosone/article?id=10.1371/journal.pone.0199239)
static cgv::rgb cividis_fn(float t) {
	cgv::rgb color = {
		-0.0178f - t * (0.1386f - t * (9.3401f - t * (25.1086f - t * (27.5479f - t * 10.6297f)))),
		0.1274f + t * (0.6695f + t * (0.2071f - t * (0.5155f - t * (0.6925f - t * 0.2642f)))),
		0.3186f + t * (1.7347f - t * (9.735f - t * (24.1853f - t * (25.9409f - t * 9.7085f))))
	};
	color.clamp();
	return color;
}
constexpr std::array<int32_t, 256> viridis_data = { 0x440154, 0x440256, 0x450457, 0x450559, 0x46075a, 0x46085c, 0x460a5d, 0x460b5e, 0x470d60, 0x470e61, 0x471063, 0x471164, 0x471365, 0x481467, 0x481668, 0x481769, 0x48186a, 0x481a6c, 0x481b6d, 0x481c6e, 0x481d6f, 0x481f70, 0x482071, 0x482173, 0x482374, 0x482475, 0x482576, 0x482677, 0x482878, 0x482979, 0x472a7a, 0x472c7a, 0x472d7b, 0x472e7c, 0x472f7d, 0x46307e, 0x46327e, 0x46337f, 0x463480, 0x453581, 0x453781, 0x453882, 0x443983, 0x443a83, 0x443b84, 0x433d84, 0x433e85, 0x423f85, 0x424086, 0x424186, 0x414287, 0x414487, 0x404588, 0x404688, 0x3f4788, 0x3f4889, 0x3e4989, 0x3e4a89, 0x3e4c8a, 0x3d4d8a, 0x3d4e8a, 0x3c4f8a, 0x3c508b, 0x3b518b, 0x3b528b, 0x3a538b, 0x3a548c, 0x39558c, 0x39568c, 0x38588c, 0x38598c, 0x375a8c, 0x375b8d, 0x365c8d, 0x365d8d, 0x355e8d, 0x355f8d, 0x34608d, 0x34618d, 0x33628d, 0x33638d, 0x32648e, 0x32658e, 0x31668e, 0x31678e, 0x31688e, 0x30698e, 0x306a8e, 0x2f6b8e, 0x2f6c8e, 0x2e6d8e, 0x2e6e8e, 0x2e6f8e, 0x2d708e, 0x2d718e, 0x2c718e, 0x2c728e, 0x2c738e, 0x2b748e, 0x2b758e, 0x2a768e, 0x2a778e, 0x2a788e, 0x29798e, 0x297a8e, 0x297b8e, 0x287c8e, 0x287d8e, 0x277e8e, 0x277f8e, 0x27808e, 0x26818e, 0x26828e, 0x26828e, 0x25838e, 0x25848e, 0x25858e, 0x24868e, 0x24878e, 0x23888e, 0x23898e, 0x238a8d, 0x228b8d, 0x228c8d, 0x228d8d, 0x218e8d, 0x218f8d, 0x21908d, 0x21918c, 0x20928c, 0x20928c, 0x20938c, 0x1f948c, 0x1f958b, 0x1f968b, 0x1f978b, 0x1f988b, 0x1f998a, 0x1f9a8a, 0x1e9b8a, 0x1e9c89, 0x1e9d89, 0x1f9e89, 0x1f9f88, 0x1fa088, 0x1fa188, 0x1fa187, 0x1fa287, 0x20a386, 0x20a486, 0x21a585, 0x21a685, 0x22a785, 0x22a884, 0x23a983, 0x24aa83, 0x25ab82, 0x25ac82, 0x26ad81, 0x27ad81, 0x28ae80, 0x29af7f, 0x2ab07f, 0x2cb17e, 0x2db27d, 0x2eb37c, 0x2fb47c, 0x31b57b, 0x32b67a, 0x34b679, 0x35b779, 0x37b878, 0x38b977, 0x3aba76, 0x3bbb75, 0x3dbc74, 0x3fbc73, 0x40bd72, 0x42be71, 0x44bf70, 0x46c06f, 0x48c16e, 0x4ac16d, 0x4cc26c, 0x4ec36b, 0x50c46a, 0x52c569, 0x54c568, 0x56c667, 0x58c765, 0x5ac864, 0x5cc863, 0x5ec962, 0x60ca60, 0x63cb5f, 0x65cb5e, 0x67cc5c, 0x69cd5b, 0x6ccd5a, 0x6ece58, 0x70cf57, 0x73d056, 0x75d054, 0x77d153, 0x7ad151, 0x7cd250, 0x7fd34e, 0x81d34d, 0x84d44b, 0x86d549, 0x89d548, 0x8bd646, 0x8ed645, 0x90d743, 0x93d741, 0x95d840, 0x98d83e, 0x9bd93c, 0x9dd93b, 0xa0da39, 0xa2da37, 0xa5db36, 0xa8db34, 0xaadc32, 0xaddc30, 0xb0dd2f, 0xb2dd2d, 0xb5de2b, 0xb8de29, 0xbade28, 0xbddf26, 0xc0df25, 0xc2df23, 0xc5e021, 0xc8e020, 0xcae11f, 0xcde11d, 0xd0e11c, 0xd2e21b, 0xd5e21a, 0xd8e219, 0xdae319, 0xdde318, 0xdfe318, 0xe2e418, 0xe5e419, 0xe7e419, 0xeae51a, 0xece51b, 0xefe51c, 0xf1e51d, 0xf4e61e, 0xf6e620, 0xf8e621, 0xfbe723, 0xfde725 };
constexpr std::array<int32_t, 256> magma_data = { 0x000004, 0x010005, 0x010106, 0x010108, 0x020109, 0x02020b, 0x02020d, 0x03030f, 0x030312, 0x040414, 0x050416, 0x060518, 0x06051a, 0x07061c, 0x08071e, 0x090720, 0x0a0822, 0x0b0924, 0x0c0926, 0x0d0a29, 0x0e0b2b, 0x100b2d, 0x110c2f, 0x120d31, 0x130d34, 0x140e36, 0x150e38, 0x160f3b, 0x180f3d, 0x19103f, 0x1a1042, 0x1c1044, 0x1d1147, 0x1e1149, 0x20114b, 0x21114e, 0x221150, 0x241253, 0x251255, 0x271258, 0x29115a, 0x2a115c, 0x2c115f, 0x2d1161, 0x2f1163, 0x311165, 0x331067, 0x341069, 0x36106b, 0x38106c, 0x390f6e, 0x3b0f70, 0x3d0f71, 0x3f0f72, 0x400f74, 0x420f75, 0x440f76, 0x451077, 0x471078, 0x491078, 0x4a1079, 0x4c117a, 0x4e117b, 0x4f127b, 0x51127c, 0x52137c, 0x54137d, 0x56147d, 0x57157e, 0x59157e, 0x5a167e, 0x5c167f, 0x5d177f, 0x5f187f, 0x601880, 0x621980, 0x641a80, 0x651a80, 0x671b80, 0x681c81, 0x6a1c81, 0x6b1d81, 0x6d1d81, 0x6e1e81, 0x701f81, 0x721f81, 0x732081, 0x752181, 0x762181, 0x782281, 0x792282, 0x7b2382, 0x7c2382, 0x7e2482, 0x802582, 0x812581, 0x832681, 0x842681, 0x862781, 0x882781, 0x892881, 0x8b2981, 0x8c2981, 0x8e2a81, 0x902a81, 0x912b81, 0x932b80, 0x942c80, 0x962c80, 0x982d80, 0x992d80, 0x9b2e7f, 0x9c2e7f, 0x9e2f7f, 0xa02f7f, 0xa1307e, 0xa3307e, 0xa5317e, 0xa6317d, 0xa8327d, 0xaa337d, 0xab337c, 0xad347c, 0xae347b, 0xb0357b, 0xb2357b, 0xb3367a, 0xb5367a, 0xb73779, 0xb83779, 0xba3878, 0xbc3978, 0xbd3977, 0xbf3a77, 0xc03a76, 0xc23b75, 0xc43c75, 0xc53c74, 0xc73d73, 0xc83e73, 0xca3e72, 0xcc3f71, 0xcd4071, 0xcf4070, 0xd0416f, 0xd2426f, 0xd3436e, 0xd5446d, 0xd6456c, 0xd8456c, 0xd9466b, 0xdb476a, 0xdc4869, 0xde4968, 0xdf4a68, 0xe04c67, 0xe24d66, 0xe34e65, 0xe44f64, 0xe55064, 0xe75263, 0xe85362, 0xe95462, 0xea5661, 0xeb5760, 0xec5860, 0xed5a5f, 0xee5b5e, 0xef5d5e, 0xf05f5e, 0xf1605d, 0xf2625d, 0xf2645c, 0xf3655c, 0xf4675c, 0xf4695c, 0xf56b5c, 0xf66c5c, 0xf66e5c, 0xf7705c, 0xf7725c, 0xf8745c, 0xf8765c, 0xf9785d, 0xf9795d, 0xf97b5d, 0xfa7d5e, 0xfa7f5e, 0xfa815f, 0xfb835f, 0xfb8560, 0xfb8761, 0xfc8961, 0xfc8a62, 0xfc8c63, 0xfc8e64, 0xfc9065, 0xfd9266, 0xfd9467, 0xfd9668, 0xfd9869, 0xfd9a6a, 0xfd9b6b, 0xfe9d6c, 0xfe9f6d, 0xfea16e, 0xfea36f, 0xfea571, 0xfea772, 0xfea973, 0xfeaa74, 0xfeac76, 0xfeae77, 0xfeb078, 0xfeb27a, 0xfeb47b, 0xfeb67c, 0xfeb77e, 0xfeb97f, 0xfebb81, 0xfebd82, 0xfebf84, 0xfec185, 0xfec287, 0xfec488, 0xfec68a, 0xfec88c, 0xfeca8d, 0xfecc8f, 0xfecd90, 0xfecf92, 0xfed194, 0xfed395, 0xfed597, 0xfed799, 0xfed89a, 0xfdda9c, 0xfddc9e, 0xfddea0, 0xfde0a1, 0xfde2a3, 0xfde3a5, 0xfde5a7, 0xfde7a9, 0xfde9aa, 0xfdebac, 0xfcecae, 0xfceeb0, 0xfcf0b2, 0xfcf2b4, 0xfcf4b6, 0xfcf6b8, 0xfcf7b9, 0xfcf9bb, 0xfcfbbd, 0xfcfdbf };
constexpr std::array<int32_t, 256> inferno_data = { 0x000004, 0x010005, 0x010106, 0x010108, 0x02010a, 0x02020c, 0x02020e, 0x030210, 0x040312, 0x040314, 0x050417, 0x060419, 0x07051b, 0x08051d, 0x09061f, 0x0a0722, 0x0b0724, 0x0c0826, 0x0d0829, 0x0e092b, 0x10092d, 0x110a30, 0x120a32, 0x140b34, 0x150b37, 0x160b39, 0x180c3c, 0x190c3e, 0x1b0c41, 0x1c0c43, 0x1e0c45, 0x1f0c48, 0x210c4a, 0x230c4c, 0x240c4f, 0x260c51, 0x280b53, 0x290b55, 0x2b0b57, 0x2d0b59, 0x2f0a5b, 0x310a5c, 0x320a5e, 0x340a5f, 0x360961, 0x380962, 0x390963, 0x3b0964, 0x3d0965, 0x3e0966, 0x400a67, 0x420a68, 0x440a68, 0x450a69, 0x470b6a, 0x490b6a, 0x4a0c6b, 0x4c0c6b, 0x4d0d6c, 0x4f0d6c, 0x510e6c, 0x520e6d, 0x540f6d, 0x550f6d, 0x57106e, 0x59106e, 0x5a116e, 0x5c126e, 0x5d126e, 0x5f136e, 0x61136e, 0x62146e, 0x64156e, 0x65156e, 0x67166e, 0x69166e, 0x6a176e, 0x6c186e, 0x6d186e, 0x6f196e, 0x71196e, 0x721a6e, 0x741a6e, 0x751b6e, 0x771c6d, 0x781c6d, 0x7a1d6d, 0x7c1d6d, 0x7d1e6d, 0x7f1e6c, 0x801f6c, 0x82206c, 0x84206b, 0x85216b, 0x87216b, 0x88226a, 0x8a226a, 0x8c2369, 0x8d2369, 0x8f2469, 0x902568, 0x922568, 0x932667, 0x952667, 0x972766, 0x982766, 0x9a2865, 0x9b2964, 0x9d2964, 0x9f2a63, 0xa02a63, 0xa22b62, 0xa32c61, 0xa52c60, 0xa62d60, 0xa82e5f, 0xa92e5e, 0xab2f5e, 0xad305d, 0xae305c, 0xb0315b, 0xb1325a, 0xb3325a, 0xb43359, 0xb63458, 0xb73557, 0xb93556, 0xba3655, 0xbc3754, 0xbd3853, 0xbf3952, 0xc03a51, 0xc13a50, 0xc33b4f, 0xc43c4e, 0xc63d4d, 0xc73e4c, 0xc83f4b, 0xca404a, 0xcb4149, 0xcc4248, 0xce4347, 0xcf4446, 0xd04545, 0xd24644, 0xd34743, 0xd44842, 0xd54a41, 0xd74b3f, 0xd84c3e, 0xd94d3d, 0xda4e3c, 0xdb503b, 0xdd513a, 0xde5238, 0xdf5337, 0xe05536, 0xe15635, 0xe25734, 0xe35933, 0xe45a31, 0xe55c30, 0xe65d2f, 0xe75e2e, 0xe8602d, 0xe9612b, 0xea632a, 0xeb6429, 0xeb6628, 0xec6726, 0xed6925, 0xee6a24, 0xef6c23, 0xef6e21, 0xf06f20, 0xf1711f, 0xf1731d, 0xf2741c, 0xf3761b, 0xf37819, 0xf47918, 0xf57b17, 0xf57d15, 0xf67e14, 0xf68013, 0xf78212, 0xf78410, 0xf8850f, 0xf8870e, 0xf8890c, 0xf98b0b, 0xf98c0a, 0xf98e09, 0xfa9008, 0xfa9207, 0xfa9407, 0xfb9606, 0xfb9706, 0xfb9906, 0xfb9b06, 0xfb9d07, 0xfc9f07, 0xfca108, 0xfca309, 0xfca50a, 0xfca60c, 0xfca80d, 0xfcaa0f, 0xfcac11, 0xfcae12, 0xfcb014, 0xfcb216, 0xfcb418, 0xfbb61a, 0xfbb81d, 0xfbba1f, 0xfbbc21, 0xfbbe23, 0xfac026, 0xfac228, 0xfac42a, 0xfac62d, 0xf9c72f, 0xf9c932, 0xf9cb35, 0xf8cd37, 0xf8cf3a, 0xf7d13d, 0xf7d340, 0xf6d543, 0xf6d746, 0xf5d949, 0xf5db4c, 0xf4dd4f, 0xf4df53, 0xf4e156, 0xf3e35a, 0xf3e55d, 0xf2e661, 0xf2e865, 0xf2ea69, 0xf1ec6d, 0xf1ed71, 0xf1ef75, 0xf1f179, 0xf2f27d, 0xf2f482, 0xf3f586, 0xf3f68a, 0xf4f88e, 0xf5f992, 0xf6fa96, 0xf8fb9a, 0xf9fc9d, 0xfafda1, 0xfcffa4 };
constexpr std::array<int32_t, 256> plasma_data = { 0x0d0887, 0x100788, 0x130789, 0x16078a, 0x19068c, 0x1b068d, 0x1d068e, 0x20068f, 0x220690, 0x240691, 0x260591, 0x280592, 0x2a0593, 0x2c0594, 0x2e0595, 0x2f0596, 0x310597, 0x330597, 0x350498, 0x370499, 0x38049a, 0x3a049a, 0x3c049b, 0x3e049c, 0x3f049c, 0x41049d, 0x43039e, 0x44039e, 0x46039f, 0x48039f, 0x4903a0, 0x4b03a1, 0x4c02a1, 0x4e02a2, 0x5002a2, 0x5102a3, 0x5302a3, 0x5502a4, 0x5601a4, 0x5801a4, 0x5901a5, 0x5b01a5, 0x5c01a6, 0x5e01a6, 0x6001a6, 0x6100a7, 0x6300a7, 0x6400a7, 0x6600a7, 0x6700a8, 0x6900a8, 0x6a00a8, 0x6c00a8, 0x6e00a8, 0x6f00a8, 0x7100a8, 0x7201a8, 0x7401a8, 0x7501a8, 0x7701a8, 0x7801a8, 0x7a02a8, 0x7b02a8, 0x7d03a8, 0x7e03a8, 0x8004a8, 0x8104a7, 0x8305a7, 0x8405a7, 0x8606a6, 0x8707a6, 0x8808a6, 0x8a09a5, 0x8b0aa5, 0x8d0ba5, 0x8e0ca4, 0x8f0da4, 0x910ea3, 0x920fa3, 0x9410a2, 0x9511a1, 0x9613a1, 0x9814a0, 0x99159f, 0x9a169f, 0x9c179e, 0x9d189d, 0x9e199d, 0xa01a9c, 0xa11b9b, 0xa21d9a, 0xa31e9a, 0xa51f99, 0xa62098, 0xa72197, 0xa82296, 0xaa2395, 0xab2494, 0xac2694, 0xad2793, 0xae2892, 0xb02991, 0xb12a90, 0xb22b8f, 0xb32c8e, 0xb42e8d, 0xb52f8c, 0xb6308b, 0xb7318a, 0xb83289, 0xba3388, 0xbb3488, 0xbc3587, 0xbd3786, 0xbe3885, 0xbf3984, 0xc03a83, 0xc13b82, 0xc23c81, 0xc33d80, 0xc43e7f, 0xc5407e, 0xc6417d, 0xc7427c, 0xc8437b, 0xc9447a, 0xca457a, 0xcb4679, 0xcc4778, 0xcc4977, 0xcd4a76, 0xce4b75, 0xcf4c74, 0xd04d73, 0xd14e72, 0xd24f71, 0xd35171, 0xd45270, 0xd5536f, 0xd5546e, 0xd6556d, 0xd7566c, 0xd8576b, 0xd9586a, 0xda5a6a, 0xda5b69, 0xdb5c68, 0xdc5d67, 0xdd5e66, 0xde5f65, 0xde6164, 0xdf6263, 0xe06363, 0xe16462, 0xe26561, 0xe26660, 0xe3685f, 0xe4695e, 0xe56a5d, 0xe56b5d, 0xe66c5c, 0xe76e5b, 0xe76f5a, 0xe87059, 0xe97158, 0xe97257, 0xea7457, 0xeb7556, 0xeb7655, 0xec7754, 0xed7953, 0xed7a52, 0xee7b51, 0xef7c51, 0xef7e50, 0xf07f4f, 0xf0804e, 0xf1814d, 0xf1834c, 0xf2844b, 0xf3854b, 0xf3874a, 0xf48849, 0xf48948, 0xf58b47, 0xf58c46, 0xf68d45, 0xf68f44, 0xf79044, 0xf79143, 0xf79342, 0xf89441, 0xf89540, 0xf9973f, 0xf9983e, 0xf99a3e, 0xfa9b3d, 0xfa9c3c, 0xfa9e3b, 0xfb9f3a, 0xfba139, 0xfba238, 0xfca338, 0xfca537, 0xfca636, 0xfca835, 0xfca934, 0xfdab33, 0xfdac33, 0xfdae32, 0xfdaf31, 0xfdb130, 0xfdb22f, 0xfdb42f, 0xfdb52e, 0xfeb72d, 0xfeb82c, 0xfeba2c, 0xfebb2b, 0xfebd2a, 0xfebe2a, 0xfec029, 0xfdc229, 0xfdc328, 0xfdc527, 0xfdc627, 0xfdc827, 0xfdca26, 0xfdcb26, 0xfccd25, 0xfcce25, 0xfcd025, 0xfcd225, 0xfbd324, 0xfbd524, 0xfbd724, 0xfad824, 0xfada24, 0xf9dc24, 0xf9dd25, 0xf8df25, 0xf8e125, 0xf7e225, 0xf7e425, 0xf6e626, 0xf6e826, 0xf5e926, 0xf5eb27, 0xf4ed27, 0xf3ee27, 0xf3f027, 0xf2f227, 0xf1f426, 0xf1f525, 0xf0f724, 0xf0f921 };

// Matplotlib bone. A grayscale color map with a blue tint, useful for adding an "electronic" look to grayscale images.
const std::array<std::vector<std::pair<float, float>>, 3> bone_data = { {
	{
		{ 0.0f, 0.0f },
		{ 0.746032f, 0.652778f },
		{ 1.0f, 1.0f },
	},
	{
		{ 0.0f, 0.0f },
		{ 0.365079f, 0.319444f },
		{ 0.746032f, 0.777778f },
		{ 1.0f, 1.0f }
	},
	{
		{ 0.0f, 0.0f },
		{ 0.365079f, 0.444444f },
		{ 1.0f, 1.0f }
	}
} };
// Matplotlib jet. A sequential rainbow-like color map from dark blue over green and yellow to dark red. Use with caution; prefer turbo instead.
const std::array<std::vector<std::pair<float, float>>, 3> jet_data = { {
	{
		{ 0.00f, 0.0f },
		{ 0.35f, 0.0f },
		{ 0.66f, 1.0f },
		{ 0.89f, 1.0f },
		{ 1.00f, 0.5f }
	},
	{
		{ 0.000f, 0.0f },
		{ 0.125f, 0.0f },
		{ 0.375f, 1.0f },
		{ 0.640f, 1.0f },
		{ 0.910f, 0.0f },
		{ 1.000f, 0.0f }
	},
	{
		{ 0.00f, 0.5f },
		{ 0.11f, 1.0f },
		{ 0.34f, 1.0f },
		{ 0.65f, 0.0f },
		{ 1.00f, 0.0f }
	}
} };
// Matplotlib hot.
const std::array<std::vector<std::pair<float, float>>, 3> hot_data = { {
	{
		{ 0.0f, 0.0416f },
		{ 0.365079f, 1.0f },
		{ 1.0f, 1.0f },
	},
	{
		{ 0.0f, 0.0f },
		{ 0.365079f, 0.0f },
		{ 0.746032f, 1.0f },
		{ 1.0f, 1.0f }
	},
	{
		{ 0.0f, 0.0f },
		{ 0.746032f, 0.0f },
		{ 1.0f, 1.0f }
	}
} };

static continuous_color_scheme interpolateBone() { return { rgb_per_channel_piecewise_interpolator<float>(bone_data[0], bone_data[1], bone_data[2]) }; }
static continuous_color_scheme interpolateJet() { return { rgb_per_channel_piecewise_interpolator<float>(jet_data[0], jet_data[1], jet_data[2]) }; }
static continuous_color_scheme interpolateHot() { return { rgb_per_channel_piecewise_interpolator<float>(hot_data[0], hot_data[1], hot_data[2]) }; }
static auto interpolateTurbo() { return continuous_color_scheme::function(turbo_fn); }
static auto interpolateViridis() { return continuous_color_scheme::linear(to_colors(viridis_data)); }
static auto interpolateMagma() { return continuous_color_scheme::linear(to_colors(magma_data)); }
static auto interpolateInferno() { return continuous_color_scheme::linear(to_colors(inferno_data)); }
static auto interpolatePlasma() { return continuous_color_scheme::linear(to_colors(plasma_data)); }
static auto interpolateCividis() { return continuous_color_scheme::function(cividis_fn); }

// diverging
constexpr std::array<const char*, 9> BrBG_data = {
	"d8b365f5f5f55ab4ac",
	"a6611adfc27d80cdc1018571",
	"a6611adfc27df5f5f580cdc1018571",
	"8c510ad8b365f6e8c3c7eae55ab4ac01665e",
	"8c510ad8b365f6e8c3f5f5f5c7eae55ab4ac01665e",
	"8c510abf812ddfc27df6e8c3c7eae580cdc135978f01665e",
	"8c510abf812ddfc27df6e8c3f5f5f5c7eae580cdc135978f01665e",
	"5430058c510abf812ddfc27df6e8c3c7eae580cdc135978f01665e003c30",
	"5430058c510abf812ddfc27df6e8c3f5f5f5c7eae580cdc135978f01665e003c30"
};
constexpr std::array<const char*, 9> PiYG_data = {
	"e9a3c9f7f7f7a1d76a",
	"d01c8bf1b6dab8e1864dac26",
	"d01c8bf1b6daf7f7f7b8e1864dac26",
	"c51b7de9a3c9fde0efe6f5d0a1d76a4d9221",
	"c51b7de9a3c9fde0eff7f7f7e6f5d0a1d76a4d9221",
	"c51b7dde77aef1b6dafde0efe6f5d0b8e1867fbc414d9221",
	"c51b7dde77aef1b6dafde0eff7f7f7e6f5d0b8e1867fbc414d9221",
	"8e0152c51b7dde77aef1b6dafde0efe6f5d0b8e1867fbc414d9221276419",
	"8e0152c51b7dde77aef1b6dafde0eff7f7f7e6f5d0b8e1867fbc414d9221276419"
};
constexpr std::array<const char*, 9> PRGn_data = {
	"af8dc3f7f7f77fbf7b",
	"7b3294c2a5cfa6dba0008837",
	"7b3294c2a5cff7f7f7a6dba0008837",
	"762a83af8dc3e7d4e8d9f0d37fbf7b1b7837",
	"762a83af8dc3e7d4e8f7f7f7d9f0d37fbf7b1b7837",
	"762a839970abc2a5cfe7d4e8d9f0d3a6dba05aae611b7837",
	"762a839970abc2a5cfe7d4e8f7f7f7d9f0d3a6dba05aae611b7837",
	"40004b762a839970abc2a5cfe7d4e8d9f0d3a6dba05aae611b783700441b",
	"40004b762a839970abc2a5cfe7d4e8f7f7f7d9f0d3a6dba05aae611b783700441b"
};
constexpr std::array<const char*, 9> PuOr_data = {
	"998ec3f7f7f7f1a340",
	"5e3c99b2abd2fdb863e66101",
	"5e3c99b2abd2f7f7f7fdb863e66101",
	"542788998ec3d8daebfee0b6f1a340b35806",
	"542788998ec3d8daebf7f7f7fee0b6f1a340b35806",
	"5427888073acb2abd2d8daebfee0b6fdb863e08214b35806",
	"5427888073acb2abd2d8daebf7f7f7fee0b6fdb863e08214b35806",
	"2d004b5427888073acb2abd2d8daebfee0b6fdb863e08214b358067f3b08",
	"2d004b5427888073acb2abd2d8daebf7f7f7fee0b6fdb863e08214b358067f3b08"
};
constexpr std::array<const char*, 9> RdBu_data = {
	"ef8a62f7f7f767a9cf",
	"ca0020f4a58292c5de0571b0",
	"ca0020f4a582f7f7f792c5de0571b0",
	"b2182bef8a62fddbc7d1e5f067a9cf2166ac",
	"b2182bef8a62fddbc7f7f7f7d1e5f067a9cf2166ac",
	"b2182bd6604df4a582fddbc7d1e5f092c5de4393c32166ac",
	"b2182bd6604df4a582fddbc7f7f7f7d1e5f092c5de4393c32166ac",
	"67001fb2182bd6604df4a582fddbc7d1e5f092c5de4393c32166ac053061",
	"67001fb2182bd6604df4a582fddbc7f7f7f7d1e5f092c5de4393c32166ac053061"
};
constexpr std::array<const char*, 9> RdGy_data = {
	"ef8a62ffffff999999",
	"ca0020f4a582bababa404040",
	"ca0020f4a582ffffffbababa404040",
	"b2182bef8a62fddbc7e0e0e09999994d4d4d",
	"b2182bef8a62fddbc7ffffffe0e0e09999994d4d4d",
	"b2182bd6604df4a582fddbc7e0e0e0bababa8787874d4d4d",
	"b2182bd6604df4a582fddbc7ffffffe0e0e0bababa8787874d4d4d",
	"67001fb2182bd6604df4a582fddbc7e0e0e0bababa8787874d4d4d1a1a1a",
	"67001fb2182bd6604df4a582fddbc7ffffffe0e0e0bababa8787874d4d4d1a1a1a"
};
constexpr std::array<const char*, 9> RdYlBu_data = {
	"fc8d59ffffbf91bfdb",
	"d7191cfdae61abd9e92c7bb6",
	"d7191cfdae61ffffbfabd9e92c7bb6",
	"d73027fc8d59fee090e0f3f891bfdb4575b4",
	"d73027fc8d59fee090ffffbfe0f3f891bfdb4575b4",
	"d73027f46d43fdae61fee090e0f3f8abd9e974add14575b4",
	"d73027f46d43fdae61fee090ffffbfe0f3f8abd9e974add14575b4",
	"a50026d73027f46d43fdae61fee090e0f3f8abd9e974add14575b4313695",
	"a50026d73027f46d43fdae61fee090ffffbfe0f3f8abd9e974add14575b4313695"
};
constexpr std::array<const char*, 9> RdYlGn_data = {
	"fc8d59ffffbf91cf60",
	"d7191cfdae61a6d96a1a9641",
	"d7191cfdae61ffffbfa6d96a1a9641",
	"d73027fc8d59fee08bd9ef8b91cf601a9850",
	"d73027fc8d59fee08bffffbfd9ef8b91cf601a9850",
	"d73027f46d43fdae61fee08bd9ef8ba6d96a66bd631a9850",
	"d73027f46d43fdae61fee08bffffbfd9ef8ba6d96a66bd631a9850",
	"a50026d73027f46d43fdae61fee08bd9ef8ba6d96a66bd631a9850006837",
	"a50026d73027f46d43fdae61fee08bffffbfd9ef8ba6d96a66bd631a9850006837"
};

static auto interpolateBrBG() { return continuous_color_scheme::smooth(to_colors(BrBG_data.back()), ColorSchemeType::kDiverging); }
static auto interpolatePiYG() { return continuous_color_scheme::smooth(to_colors(PiYG_data.back()), ColorSchemeType::kDiverging); }
static auto interpolatePRGn() { return continuous_color_scheme::smooth(to_colors(PRGn_data.back()), ColorSchemeType::kDiverging); }
static auto interpolatePuOr() { return continuous_color_scheme::smooth(to_colors(PuOr_data.back()), ColorSchemeType::kDiverging); }
static auto interpolateRdBu() { return continuous_color_scheme::smooth(to_colors(RdBu_data.back()), ColorSchemeType::kDiverging); }
static auto interpolateRdGy() { return continuous_color_scheme::smooth(to_colors(RdGy_data.back()), ColorSchemeType::kDiverging); }
static auto interpolateRdYlBu() { return continuous_color_scheme::smooth(to_colors(RdYlBu_data.back()), ColorSchemeType::kDiverging); }
static auto interpolateRdYlGn() { return continuous_color_scheme::smooth(to_colors(RdYlGn_data.back()), ColorSchemeType::kDiverging); }

static auto schemeBrBG() { return discrete_color_scheme(to_colors(BrBG_data), ColorSchemeType::kDiverging); }
static auto schemePRGn() { return discrete_color_scheme(to_colors(PRGn_data), ColorSchemeType::kDiverging); }
static auto schemePiYG() { return discrete_color_scheme(to_colors(PiYG_data), ColorSchemeType::kDiverging); }
static auto schemePuOr() { return discrete_color_scheme(to_colors(PuOr_data), ColorSchemeType::kDiverging); }
static auto schemeRdBu() { return discrete_color_scheme(to_colors(RdBu_data), ColorSchemeType::kDiverging); }
static auto schemeRdGy() { return discrete_color_scheme(to_colors(RdGy_data), ColorSchemeType::kDiverging); }
static auto schemeRdYlBu() { return discrete_color_scheme(to_colors(RdYlBu_data), ColorSchemeType::kDiverging); }
static auto schemeRdYlGn() { return discrete_color_scheme(to_colors(RdYlGn_data), ColorSchemeType::kDiverging); }

constexpr std::array<int32_t, 11> spectral_data = { 0x9e0142, 0xd53e4f, 0xf46d43, 0xfdae61, 0xfee08b, 0xffffbf, 0xe6f598, 0xabdda4, 0x66c2a5, 0x3288bd, 0x5e4fa2 };
static auto interpolateSpectral() { return continuous_color_scheme::smooth(to_colors(spectral_data)); }

// cyclical
constexpr std::array<int32_t, 7> hue_data = { 0xff0000, 0xffff00, 0x00ff00, 0x00ffff, 0x0000ff, 0xff00ff, 0xff0000 };
constexpr const char* twilight_data = "e1d8e2e0d9e2dfd9e1ded9e0ddd9e0dbd8dfd9d8ded8d7ddd6d6dcd4d6dbd2d5dacfd4d9cdd2d8cad1d7c7d0d6c5cfd4c2cdd3bfccd2bccbd1b9c9d0b6c8cfb3c6ceb0c4cdadc3ccaac1cba7c0caa4becaa1bcc99ebbc89bb9c898b7c796b5c793b4c690b2c58eb0c58baec589adc487abc484a9c382a7c380a5c37ea3c27ca1c27a9fc2789ec1769cc1749ac17398c07196c07094c06e92bf6d90bf6b8ebf6a8cbe698abe6888be6786bd6683bd6581bd647fbc647dbc637bbb6279bb6277ba6174ba6172b96070b8606eb8606bb75f69b65f67b65f65b55f62b45f60b35e5eb25e5bb15e59b05e57af5e54ae5e52ad5e4fac5e4daa5e4ba95d48a85d46a65d43a55d41a35d3ea15c3c9f5c3a9e5c379c5b359a5b32975a30955a2e93592b9059298e58278b572588562386552183541f7f531d7c521c79511a764f19724e186f4c176b4b166849156547146146135e44135b4212574012543f11513d114e3c114c3a104939104637104436104235104034113e33113c32113a3112393013372f13363012363212373311373411373611383711393911393a113a3c113b3e113c40113d42113e44123f4712404912414c13424e13435014445314455615465815475b16485d174960174a63184b65194b68194c6b1a4d6d1b4d701c4e721d4e751e4f781f4f7a204f7d214f7f22508124508425508626508828508b29508d2b508f2d50912e4f93304f95324f97344f99364f9b374f9d394f9e3b4fa03d4fa23f4fa4414fa5444fa7464fa84850aa4a50ab4c50ad4e50ae5051b05351b15551b25752b45952b55c53b65e54b76054b86355b96556ba6757bc6a58bc6c59bd6f5abe715cbf735dc0765ec17860c27b61c27d63c38065c48267c48569c5876bc68a6dc68c6fc78e71c79174c89376c99679c9987cca9b7eca9d81cba084cba287cca48acda78dcda991ceab94ceae97cfb09ad0b29ed1b5a1d1b7a4d2b9a8d3bbabd4bdafd5bfb2d6c1b6d7c3b9d8c5bcd8c7c0d9c9c3dacbc6dbcdc9dcceccddd0cfddd1d2ded3d4dfd4d6dfd5d8e0d6dae0d7dce1d7dde1d8dfe1d8e0e1d8e1";

static auto interpolateHue() { return continuous_color_scheme::linear(to_colors(hue_data), ColorSchemeType::kCyclical); }
static auto interpolateTwilight() { return continuous_color_scheme::linear(to_colors(twilight_data), ColorSchemeType::kCyclical); }

// categorical
constexpr const char* category10_data = "1f77b4ff7f0e2ca02cd627289467bd8c564be377c27f7f7fbcbd2217becf";
constexpr const char* accent8_data = "7fc97fbeaed4fdc086ffff99386cb0f0027fbf5b17666666";
constexpr const char* dark8_data = "1b9e77d95f027570b3e7298a66a61ee6ab02a6761d666666";
constexpr const char* observable10_data = "4269d0efb118ff725c6cc5b03ca951ff8ab7a463f297bbf59c6b4e9498a0";
constexpr const char* paired12_data = "a6cee31f78b4b2df8a33a02cfb9a99e31a1cfdbf6fff7f00cab2d66a3d9affff99b15928";
constexpr const char* pastel8_1_data = "fbb4aeb3cde3ccebc5decbe4fed9a6ffffcce5d8bdfddaecf2f2f2";
constexpr const char* pastel8_2_data = "b3e2cdfdcdaccbd5e8f4cae4e6f5c9fff2aef1e2cccccccc";
constexpr const char* set8_data = "66c2a5fc8d628da0cbe78ac3a6d854ffd92fe5c494b3b3b3";
constexpr const char* set9_data = "e41a1c377eb84daf4a984ea3ff7f00ffff33a65628f781bf999999";
constexpr const char* set12_data = "8dd3c7ffffb3bebadafb807280b1d3fdb462b3de69fccde5d9d9d9bc80bdccebc5ffed6f";
constexpr const char* tableau10_data = "4e79a7f28e2ce1575976b7b259a14fedc949af7aa1ff9da79c755fbab0ab";

static auto schemeCategory10() { return discrete_color_scheme(to_colors(category10_data), ColorSchemeType::kCategorical);}
static auto schemeAccent8() { return discrete_color_scheme(to_colors(accent8_data), ColorSchemeType::kCategorical); }
static auto schemeDark8() { return discrete_color_scheme(to_colors(dark8_data), ColorSchemeType::kCategorical); }
static auto schemeObservable10() { return discrete_color_scheme(to_colors(observable10_data), ColorSchemeType::kCategorical); }
static auto schemePaired12() { return discrete_color_scheme(to_colors(paired12_data), ColorSchemeType::kCategorical); }
static auto schemePastel8_1() { return discrete_color_scheme(to_colors(pastel8_1_data), ColorSchemeType::kCategorical); }
static auto schemePastel8_2() { return discrete_color_scheme(to_colors(pastel8_2_data), ColorSchemeType::kCategorical); }
static auto schemeSet8() { return discrete_color_scheme(to_colors(set8_data), ColorSchemeType::kCategorical); }
static auto schemeSet9() { return discrete_color_scheme(to_colors(set9_data), ColorSchemeType::kCategorical); }
static auto schemeSet12() { return discrete_color_scheme(to_colors(set12_data), ColorSchemeType::kCategorical); }
static auto schemeTableau10() { return discrete_color_scheme(to_colors(tableau10_data), ColorSchemeType::kCategorical); }

} // namespace schemes

extern CGV_API size_t load_continuous_color_scheme_presets(continuous_color_scheme_registry& registry, const std::set<ColorSchemeType>& types = {});
extern CGV_API size_t load_discrete_color_scheme_presets(discrete_color_scheme_registry& registry, ColorSchemeType type = ColorSchemeType::kUndefined);

} // namespace media
} // namespace cgv

#include <cgv/config/lib_end.h>
