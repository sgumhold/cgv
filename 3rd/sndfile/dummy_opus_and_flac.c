#include "sfconfig.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#else
#include "sf_unistd.h"
#endif

#include "sndfile.h"
#include "sfendian.h"
#include "common.h"

int ogg_opus_open(SF_PRIVATE* psf)
{
	psf_log_printf(psf, "This version of libsndfile was compiled without Ogg/Opus support.\n");
	return SFE_UNIMPLEMENTED;
} /* ogg_opus_open */

#define FLAC_API_EXPORTS

#include <flac/export.h>
#include <flac/stream_decoder.h>
#include <flac/stream_encoder.h>

FLAC_API void FLAC__stream_decoder_delete(FLAC__StreamDecoder* decoder) {}
FLAC_API FLAC__bool FLAC__stream_decoder_set_metadata_respond_all(FLAC__StreamDecoder* decoder) { return false; }
FLAC_API FLAC__StreamDecoderState FLAC__stream_decoder_get_state(const FLAC__StreamDecoder* decoder) { return FLAC__STREAM_DECODER_ABORTED; }
FLAC_API FLAC__bool FLAC__stream_decoder_get_decode_position(const FLAC__StreamDecoder* decoder, FLAC__uint64* position) { return false; }
FLAC_API FLAC__StreamDecoderInitStatus FLAC__stream_decoder_init_stream(
	FLAC__StreamDecoder* decoder,
	FLAC__StreamDecoderReadCallback read_callback,
	FLAC__StreamDecoderSeekCallback seek_callback,
	FLAC__StreamDecoderTellCallback tell_callback,
	FLAC__StreamDecoderLengthCallback length_callback,
	FLAC__StreamDecoderEofCallback eof_callback,
	FLAC__StreamDecoderWriteCallback write_callback,
	FLAC__StreamDecoderMetadataCallback metadata_callback,
	FLAC__StreamDecoderErrorCallback error_callback,
	void* client_data
) { return FLAC__STREAM_DECODER_INIT_STATUS_ERROR_OPENING_FILE; }
FLAC_API FLAC__bool FLAC__stream_decoder_finish(FLAC__StreamDecoder* decoder) { return false; }
FLAC_API FLAC__bool FLAC__stream_decoder_process_single(FLAC__StreamDecoder* decoder) { return false; }
FLAC_API FLAC__bool FLAC__stream_decoder_process_until_end_of_metadata(FLAC__StreamDecoder* decoder) { return false; }
FLAC_API FLAC__bool FLAC__stream_decoder_seek_absolute(FLAC__StreamDecoder* decoder, FLAC__uint64 sample) { return false; }
FLAC_API FLAC__StreamDecoder* FLAC__stream_decoder_new(void) { return 0; }
FLAC_API FLAC__StreamEncoder* FLAC__stream_encoder_new(void) { return 0; }
FLAC_API void FLAC__stream_encoder_delete(FLAC__StreamEncoder* encoder) {}
FLAC_API FLAC__bool FLAC__stream_encoder_set_channels(FLAC__StreamEncoder* encoder, uint32_t value) { return false; }
FLAC_API FLAC__bool FLAC__stream_encoder_set_bits_per_sample(FLAC__StreamEncoder* encoder, uint32_t value) { return false; }
FLAC_API FLAC__bool FLAC__stream_encoder_set_sample_rate(FLAC__StreamEncoder* encoder, uint32_t value) { return false; }
FLAC_API FLAC__bool FLAC__stream_encoder_set_compression_level(FLAC__StreamEncoder* encoder, uint32_t value) { return false; }
FLAC_API FLAC__bool FLAC__stream_encoder_set_metadata(FLAC__StreamEncoder* encoder, FLAC__StreamMetadata** metadata, uint32_t num_blocks) { return false; }
FLAC_API FLAC__StreamEncoderInitStatus FLAC__stream_encoder_init_stream(
	FLAC__StreamEncoder* encoder,
	FLAC__StreamEncoderWriteCallback write_callback,
	FLAC__StreamEncoderSeekCallback seek_callback,
	FLAC__StreamEncoderTellCallback tell_callback,
	FLAC__StreamEncoderMetadataCallback metadata_callback,
	void* client_data
) { return FLAC__STREAM_ENCODER_INIT_STATUS_ENCODER_ERROR; }
FLAC_API FLAC__bool FLAC__stream_encoder_finish(FLAC__StreamEncoder* encoder) { return false; }
FLAC_API FLAC__bool FLAC__stream_encoder_process_interleaved(FLAC__StreamEncoder* encoder, const FLAC__int32 buffer[], uint32_t samples) { return false; }
FLAC_API FLAC__StreamMetadata* FLAC__metadata_object_new(FLAC__MetadataType type) { return 0; }
FLAC_API void FLAC__metadata_object_delete(FLAC__StreamMetadata* object) {}
FLAC_API FLAC__bool FLAC__metadata_object_vorbiscomment_append_comment(FLAC__StreamMetadata* object, FLAC__StreamMetadata_VorbisComment_Entry entry, FLAC__bool copy) { return false; }
FLAC_API FLAC__bool FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(FLAC__StreamMetadata_VorbisComment_Entry* entry, const char* field_name, const char* field_value) { return false; }
FLAC_API int FLAC__metadata_object_vorbiscomment_find_entry_from(const FLAC__StreamMetadata* object, uint32_t offset, const char* field_name) { return 0; }
FLAC_API const char* const FLAC__StreamDecoderStateString[] = {
	"FLAC__STREAM_DECODER_SEARCH_FOR_METADATA",
	"FLAC__STREAM_DECODER_READ_METADATA",
	"FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC",
	"FLAC__STREAM_DECODER_READ_FRAME",
	"FLAC__STREAM_DECODER_END_OF_STREAM",
	"FLAC__STREAM_DECODER_OGG_ERROR",
	"FLAC__STREAM_DECODER_SEEK_ERROR",
	"FLAC__STREAM_DECODER_ABORTED",
	"FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR",
	"FLAC__STREAM_DECODER_UNINITIALIZED",
	"FLAC__STREAM_DECODER_END_OF_LINK"
};
FLAC_API const char* const FLAC__StreamDecoderErrorStatusString[] = {
	"FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC",
	"FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER",
	"FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH",
	"FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM",
	"FLAC__STREAM_DECODER_ERROR_STATUS_BAD_METADATA",
	"FLAC__STREAM_DECODER_ERROR_STATUS_OUT_OF_BOUNDS",
	"FLAC__STREAM_DECODER_ERROR_STATUS_MISSING_FRAME"
};
FLAC_API const char* const FLAC__StreamEncoderInitStatusString[] = {
	"FLAC__STREAM_ENCODER_INIT_STATUS_OK",
	"FLAC__STREAM_ENCODER_INIT_STATUS_ENCODER_ERROR",
	"FLAC__STREAM_ENCODER_INIT_STATUS_UNSUPPORTED_CONTAINER",
	"FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_CALLBACKS",
	"FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_NUMBER_OF_CHANNELS",
	"FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_BITS_PER_SAMPLE",
	"FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_SAMPLE_RATE",
	"FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_BLOCK_SIZE",
	"FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_MAX_LPC_ORDER",
	"FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_QLP_COEFF_PRECISION",
	"FLAC__STREAM_ENCODER_INIT_STATUS_BLOCK_SIZE_TOO_SMALL_FOR_LPC_ORDER",
	"FLAC__STREAM_ENCODER_INIT_STATUS_NOT_STREAMABLE",
	"FLAC__STREAM_ENCODER_INIT_STATUS_INVALID_METADATA",
	"FLAC__STREAM_ENCODER_INIT_STATUS_ALREADY_INITIALIZED"
};
