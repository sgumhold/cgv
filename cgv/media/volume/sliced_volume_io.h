#pragma once

#include "sliced_volume.h"

#include "../lib_begin.h"

namespace cgv {
	namespace media {
		namespace volume {

			enum FlipType
			{
				FT_NO_FLIP,
				FT_VERTICAL,
				FT_HORIZONTAL,
				FT_VERTICAL_AND_HORIZONTAL
			};
			
			/// <summary>
			/// read volume from a video file with commands ffprobe and ffmpeg which must be found in system path
			/// </summary>
			/// <param name="V"> volume data structure into which video is read</param>
			/// <param name="file_name">file name of video</param>
			/// <param name="dims">vector with (width,height,nr_frames), which can all be -1 and are then detected with ffprobe from video file </param>
			/// <param name="extent">spatial extent of volume in each dimension with negative value for dimensions i where dims(i) == -1; these extent dimensions are multiplied with dims(i) after determined from file </param>
			/// <param name="cf">component format of volume into which video is converted automatically</param>
			/// <param name="offset">frame offset after which to start reading; offset == 0 has no effect, offset == 1 skips first frame</param>
			/// <param name="flip_t">allows to flip frames vertically and or horizontally, defaults to no flipping</param>
			/// <param name="on_progress_update">callback function to reports on progress updates starting 0 after volume dimensions and extent is set and then counting read slices up to total number of slices in volume, what marks termination</param>
			/// <returns>returns whether volume reading was successful </returns>
			extern CGV_API bool read_volume_from_video_with_ffmpeg(volume& V, const std::string& file_name,
				volume::dimension_type dims, volume::extent_type extent, const cgv::data::component_format& cf,
				size_t offset = 0, FlipType flip_t = FT_NO_FLIP, void (*on_progress_update)(int,void*) = 0, void* user_data = 0, bool cycle_till_eof = false);
			/// <summary>
			/// read volume from .svx header in which dimensions, extent, file name pattern and frame offset are specified; uses read_volume_from_video_with_ffmpeg in case file name pattern corresponds to video format for which no video reader has been registered
			/// </summary>
			/// <param name="file_name">name of svx file</param>
			/// <param name="V">volume into which slices are read</param>
			/// <returns></returns>
			extern CGV_API bool read_from_sliced_volume(const std::string& file_name, volume& V);

			extern CGV_API bool write_as_sliced_volume(const std::string& file_name, const std::string& file_name_pattern, const volume& V);
		}
	}
}

#include <cgv/config/lib_end.h>