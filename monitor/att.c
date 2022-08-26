// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>
#include <linux/limits.h>

#include "lib/bluetooth.h"
#include "lib/uuid.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "src/shared/att.h"
#include "src/shared/gatt-db.h"
#include "src/textfile.h"
#include "src/settings.h"
#include "bt.h"
#include "packet.h"
#include "display.h"
#include "l2cap.h"
#include "att.h"

static void print_uuid(const char *label, const void *data, uint16_t size)
{
	const char *str;
	char uuidstr[MAX_LEN_UUID_STR];

	switch (size) {
	case 2:
		str = bt_uuid16_to_str(get_le16(data));
		print_field("%s: %s (0x%4.4x)", label, str, get_le16(data));
		break;
	case 4:
		str = bt_uuid32_to_str(get_le32(data));
		print_field("%s: %s (0x%8.8x)", label, str, get_le32(data));
		break;
	case 16:
		sprintf(uuidstr, "%8.8x-%4.4x-%4.4x-%4.4x-%8.8x%4.4x",
				get_le32(data + 12), get_le16(data + 10),
				get_le16(data + 8), get_le16(data + 6),
				get_le32(data + 2), get_le16(data + 0));
		str = bt_uuidstr_to_str(uuidstr);
		print_field("%s: %s (%s)", label, str, uuidstr);
		break;
	default:
		packet_hexdump(data, size);
		break;
	}
}

static void print_handle_range(const char *label, const void *data)
{
	print_field("%s: 0x%4.4x-0x%4.4x", label,
				get_le16(data), get_le16(data + 2));
}

static void print_data_list(const char *label, uint8_t length,
					const void *data, uint16_t size)
{
	uint8_t count;

	if (length == 0)
		return;

	count = size / length;

	print_field("%s: %u entr%s", label, count, count == 1 ? "y" : "ies");

	while (size >= length) {
		print_field("Handle: 0x%4.4x", get_le16(data));
		print_hex_field("Value", data + 2, length - 2);

		data += length;
		size -= length;
	}

	packet_hexdump(data, size);
}

static void print_attribute_info(uint16_t type, const void *data, uint16_t len)
{
	const char *str = bt_uuid16_to_str(type);

	print_field("%s: %s (0x%4.4x)", "Attribute type", str, type);

	switch (type) {
	case 0x2800:	/* Primary Service */
	case 0x2801:	/* Secondary Service */
		print_uuid("  UUID", data, len);
		break;
	case 0x2802:	/* Include */
		if (len < 4) {
			print_hex_field("  Value", data, len);
			break;
		}
		print_handle_range("  Handle range", data);
		print_uuid("  UUID", data + 4, len - 4);
		break;
	case 0x2803:	/* Characteristic */
		if (len < 3) {
			print_hex_field("  Value", data, len);
			break;
		}
		print_field("  Properties: 0x%2.2x", *((uint8_t *) data));
		print_field("  Handle: 0x%2.2x", get_le16(data + 1));
		print_uuid("  UUID", data + 3, len - 3);
		break;
	default:
		print_hex_field("Value", data, len);
		break;
	}
}

static const char *att_opcode_to_str(uint8_t opcode);

static void att_error_response(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_error_response *pdu = frame->data;
	const char *str;

	switch (pdu->error) {
	case 0x01:
		str = "Invalid Handle";
		break;
	case 0x02:
		str = "Read Not Permitted";
		break;
	case 0x03:
		str = "Write Not Permitted";
		break;
	case 0x04:
		str = "Invalid PDU";
		break;
	case 0x05:
		str = "Insufficient Authentication";
		break;
	case 0x06:
		str = "Request Not Supported";
		break;
	case 0x07:
		str = "Invalid Offset";
		break;
	case 0x08:
		str = "Insufficient Authorization";
		break;
	case 0x09:
		str = "Prepare Queue Full";
		break;
	case 0x0a:
		str = "Attribute Not Found";
		break;
	case 0x0b:
		str = "Attribute Not Long";
		break;
	case 0x0c:
		str = "Insufficient Encryption Key Size";
		break;
	case 0x0d:
		str = "Invalid Attribute Value Length";
		break;
	case 0x0e:
		str = "Unlikely Error";
		break;
	case 0x0f:
		str = "Insufficient Encryption";
		break;
	case 0x10:
		str = "Unsupported Group Type";
		break;
	case 0x11:
		str = "Insufficient Resources";
		break;
	case 0x12:
		str = "Database Out of Sync";
		break;
	case 0x13:
		str = "Value Not Allowed";
		break;
	case 0xfd:
		str = "CCC Improperly Configured";
		break;
	case 0xfe:
		str = "Procedure Already in Progress";
		break;
	case 0xff:
		str = "Out of Range";
		break;
	default:
		str = "Reserved";
		break;
	}

	print_field("%s (0x%2.2x)", att_opcode_to_str(pdu->request),
							pdu->request);
	print_field("Handle: 0x%4.4x", le16_to_cpu(pdu->handle));
	print_field("Error: %s (0x%2.2x)", str, pdu->error);
}

static const struct bitfield_data ccc_value_table[] = {
	{  0, "Notification (0x01)"		},
	{  1, "Indication (0x02)"		},
	{ }
};

static void print_ccc_value(const struct l2cap_frame *frame)
{
	uint8_t value;
	uint8_t mask;

	if (!l2cap_frame_get_u8((void *)frame, &value)) {
		print_text(COLOR_ERROR, "invalid size");
		return;
	}

	mask = print_bitfield(4, value, ccc_value_table);
	if (mask)
		print_text(COLOR_WHITE_BG, "    Unknown fields (0x%2.2x)",
								mask);
}

static void ccc_read(const struct l2cap_frame *frame)
{
	print_ccc_value(frame);
}

static void ccc_write(const struct l2cap_frame *frame)
{
	print_ccc_value(frame);
}

static bool print_ase_codec(const struct l2cap_frame *frame)
{
	uint8_t codec_id;
	uint16_t codec_cid, codec_vid;

	if (!l2cap_frame_get_u8((void *)frame, &codec_id)) {
		print_text(COLOR_ERROR, "Codec: invalid size");
		return false;
	}

	packet_print_codec_id("    Codec", codec_id);

	if (!l2cap_frame_get_le16((void *)frame, &codec_cid)) {
		print_text(COLOR_ERROR, "Codec Company ID: invalid size");
		return false;
	}

	if (!l2cap_frame_get_le16((void *)frame, &codec_vid)) {
		print_text(COLOR_ERROR, "Codec Vendor ID: invalid size");
		return false;
	}

	if (codec_id == 0xff) {
		print_field("    Codec Company ID: %s (0x%04x)",
						bt_compidtostr(codec_cid),
						codec_cid);
		print_field("    Codec Vendor ID: 0x%04x", codec_vid);
	}

	return true;
}

static bool print_ase_lv(const struct l2cap_frame *frame, const char *label,
			struct packet_ltv_decoder *decoder, size_t decoder_len)
{
	struct bt_hci_lv_data *lv;

	lv = l2cap_frame_pull((void *)frame, frame, sizeof(*lv));
	if (!lv) {
		print_text(COLOR_ERROR, "%s: invalid size", label);
		return false;
	}

	if (!l2cap_frame_pull((void *)frame, frame, lv->len)) {
		print_text(COLOR_ERROR, "%s: invalid size", label);
		return false;
	}

	packet_print_ltv(label, lv->data, lv->len, decoder, decoder_len);

	return true;
}

static bool print_ase_cc(const struct l2cap_frame *frame, const char *label,
			struct packet_ltv_decoder *decoder, size_t decoder_len)
{
	return print_ase_lv(frame, label, decoder, decoder_len);
}

static const struct bitfield_data pac_context_table[] = {
	{  0, "Unspecified (0x0001)"			},
	{  1, "Conversational (0x0002)"			},
	{  2, "Media (0x0004)"				},
	{  3, "Game (0x0008)"				},
	{  4, "Instructional (0x0010)"			},
	{  5, "Voice Assistants (0x0020)"		},
	{  6, "Live (0x0040)"				},
	{  7, "Sound Effects (0x0080)"			},
	{  8, "Notifications (0x0100)"			},
	{  9, "Ringtone (0x0200)"			},
	{  10, "Alerts (0x0400)"			},
	{  11, "Emergency alarm (0x0800)"		},
	{  12, "RFU (0x1000)"				},
	{  13, "RFU (0x2000)"				},
	{  14, "RFU (0x4000)"				},
	{  15, "RFU (0x8000)"				},
	{ }
};

static void print_context(const struct l2cap_frame *frame, const char *label)
{
	uint16_t value;
	uint16_t mask;

	if (!l2cap_frame_get_le16((void *)frame, &value)) {
		print_text(COLOR_ERROR, "    value: invalid size");
		goto done;
	}

	print_field("%s: 0x%4.4x", label, value);

	mask = print_bitfield(8, value, pac_context_table);
	if (mask)
		print_text(COLOR_WHITE_BG, "    Unknown fields (0x%4.4x)",
								mask);

done:
	if (frame->size)
		print_hex_field("    Data", frame->data, frame->size);
}

static void ase_decode_preferred_context(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	print_context(&frame, "      Preferred Context");
}

static void ase_decode_context(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	print_context(&frame, "      Context");
}

static void ase_decode_program_info(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;
	const char *str;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	str = l2cap_frame_pull(&frame, &frame, len);
	if (!str) {
		print_text(COLOR_ERROR, "    value: invalid size");
		goto done;
	}

	print_field("      Program Info: %s", str);

done:
	if (frame.size)
		print_hex_field("    Data", frame.data, frame.size);
}

static void ase_decode_language(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;
	uint32_t value;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	if (!l2cap_frame_get_le24(&frame, &value)) {
		print_text(COLOR_ERROR, "    value: invalid size");
		goto done;
	}

	print_field("      Language: 0x%6.6x", value);

done:
	if (frame.size)
		print_hex_field("    Data", frame.data, frame.size);
}

struct packet_ltv_decoder ase_metadata_table[] = {
	LTV_DEC(0x01, ase_decode_preferred_context),
	LTV_DEC(0x02, ase_decode_context),
	LTV_DEC(0x03, ase_decode_program_info),
	LTV_DEC(0x04, ase_decode_language)
};

static bool print_ase_metadata(const struct l2cap_frame *frame)
{
	return print_ase_lv(frame, "    Metadata", NULL, 0);
}

static const struct bitfield_data pac_freq_table[] = {
	{  0, "8 Khz (0x0001)"				},
	{  1, "11.25 Khz (0x0002)"			},
	{  2, "16 Khz (0x0004)"				},
	{  3, "22.05 Khz (0x0008)"			},
	{  4, "24 Khz (0x0010)"				},
	{  5, "32 Khz (0x0020)"				},
	{  6, "44.1 Khz (0x0040)"			},
	{  7, "48 Khz (0x0080)"				},
	{  8, "88.2 Khz (0x0100)"			},
	{  9, "96 Khz (0x0200)"				},
	{  10, "176.4 Khz (0x0400)"			},
	{  11, "192 Khz (0x0800)"			},
	{  12, "384 Khz (0x1000)"			},
	{  13, "RFU (0x2000)"				},
	{  14, "RFU (0x4000)"				},
	{  15, "RFU (0x8000)"				},
	{ }
};

static void pac_decode_freq(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;
	uint16_t value;
	uint16_t mask;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	if (!l2cap_frame_get_le16(&frame, &value)) {
		print_text(COLOR_ERROR, "    value: invalid size");
		goto done;
	}

	print_field("      Sampling Frequencies: 0x%4.4x", value);

	mask = print_bitfield(8, value, pac_freq_table);
	if (mask)
		print_text(COLOR_WHITE_BG, "    Unknown fields (0x%4.4x)",
								mask);

done:
	if (frame.size)
		print_hex_field("    Data", frame.data, frame.size);
}

static const struct bitfield_data pac_duration_table[] = {
	{  0, "7.5 ms (0x01)"				},
	{  1, "10 ms (0x02)"				},
	{  2, "RFU (0x04)"				},
	{  3, "RFU (0x08)"				},
	{  4, "7.5 ms preferred (0x10)"			},
	{  5, "10 ms preferred (0x20)"			},
	{  6, "RFU (0x40)"				},
	{  7, "RFU (0x80)"				},
	{ }
};

static void pac_decode_duration(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;
	uint8_t value;
	uint8_t mask;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	if (!l2cap_frame_get_u8(&frame, &value)) {
		print_text(COLOR_ERROR, "    value: invalid size");
		goto done;
	}

	print_field("      Frame Duration: 0x%4.4x", value);

	mask = print_bitfield(8, value, pac_duration_table);
	if (mask)
		print_text(COLOR_WHITE_BG, "    Unknown fields (0x%2.2x)",
								mask);

done:
	if (frame.size)
		print_hex_field("    Data", frame.data, frame.size);
}

static const struct bitfield_data pac_channel_table[] = {
	{  0, "1 channel (0x01)"			},
	{  1, "2 channels (0x02)"			},
	{  2, "3 channels (0x04)"			},
	{  3, "4 chanenls (0x08)"			},
	{  4, "5 channels (0x10)"			},
	{  5, "6 channels (0x20)"			},
	{  6, "7 channels (0x40)"			},
	{  7, "8 channels (0x80)"			},
	{ }
};

static void pac_decode_channels(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;
	uint8_t value;
	uint8_t mask;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	if (!l2cap_frame_get_u8(&frame, &value)) {
		print_text(COLOR_ERROR, "    value: invalid size");
		goto done;
	}

	print_field("      Audio Channel Count: 0x%2.2x", value);

	mask = print_bitfield(8, value, pac_channel_table);
	if (mask)
		print_text(COLOR_WHITE_BG, "    Unknown fields (0x%2.2x)",
								mask);

done:
	if (frame.size)
		print_hex_field("    Data", frame.data, frame.size);
}

static void pac_decode_frame_length(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;
	uint16_t min, max;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	if (!l2cap_frame_get_le16(&frame, &min)) {
		print_text(COLOR_ERROR, "    min: invalid size");
		goto done;
	}

	if (!l2cap_frame_get_le16(&frame, &max)) {
		print_text(COLOR_ERROR, "    min: invalid size");
		goto done;
	}

	print_field("      Frame Length: %u (0x%4.4x) - %u (0x%4.4x)",
							min, min, max, max);

done:
	if (frame.size)
		print_hex_field("    Data", frame.data, frame.size);
}

static void pac_decode_sdu(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;
	uint8_t value;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	if (!l2cap_frame_get_u8(&frame, &value)) {
		print_text(COLOR_ERROR, "    value: invalid size");
		goto done;
	}

	print_field("      Max SDU: %u (0x%2.2x)", value, value);

done:
	if (frame.size)
		print_hex_field("    Data", frame.data, frame.size);
}

struct packet_ltv_decoder pac_cap_table[] = {
	LTV_DEC(0x01, pac_decode_freq),
	LTV_DEC(0x02, pac_decode_duration),
	LTV_DEC(0x03, pac_decode_channels),
	LTV_DEC(0x04, pac_decode_frame_length),
	LTV_DEC(0x05, pac_decode_sdu)
};

static void print_pac(const struct l2cap_frame *frame)
{
	uint8_t num = 0, i;

	if (!l2cap_frame_get_u8((void *)frame, &num)) {
		print_text(COLOR_ERROR, "Number of PAC(s): invalid size");
		goto done;
	}

	print_field("  Number of PAC(s): %u", num);

	for (i = 0; i < num; i++) {
		print_field("  PAC #%u:", i);

		if (!print_ase_codec(frame))
			goto done;

		if (!print_ase_cc(frame, "    Codec Specific Capabilities",
				pac_cap_table, ARRAY_SIZE(pac_cap_table)))
			break;

		if (!print_ase_metadata(frame))
			break;
	}

done:
	if (frame->size)
		print_hex_field("  Data", frame->data, frame->size);
}

static void pac_read(const struct l2cap_frame *frame)
{
	print_pac(frame);
}

static void pac_notify(const struct l2cap_frame *frame)
{
	print_pac(frame);
}

static bool print_prefer_framing(const struct l2cap_frame *frame)
{
	uint8_t framing;

	if (!l2cap_frame_get_u8((void *)frame, &framing)) {
		print_text(COLOR_ERROR, "    Framing: invalid size");
		return false;
	}

	switch (framing) {
	case 0x00:
		print_field("    Framing: Unframed PDUs supported (0x00)");
		break;
	case 0x01:
		print_field("    Framing: Unframed PDUs not supported (0x01)");
		break;
	default:
		print_field("    Framing: Reserved (0x%2.2x)", framing);
		break;
	}

	return true;
}

static const struct bitfield_data prefer_phy_table[] = {
	{  0, "LE 1M PHY preffered (0x01)"		},
	{  1, "LE 2M PHY preffered (0x02)"		},
	{  2, "LE Codec PHY preffered (0x04)"		},
	{ }
};

static bool print_prefer_phy(const struct l2cap_frame *frame)
{
	uint8_t phy, mask;

	if (!l2cap_frame_get_u8((void *)frame, &phy)) {
		print_text(COLOR_ERROR, "PHY: invalid size");
		return false;
	}

	print_field("    PHY: 0x%2.2x", phy);

	mask = print_bitfield(4, phy, prefer_phy_table);
	if (mask)
		print_text(COLOR_WHITE_BG, "    Unknown fields (0x%2.2x)",
								mask);

	return true;
}

static bool print_ase_rtn(const struct l2cap_frame *frame, const char *label)
{
	uint8_t rtn;

	if (!l2cap_frame_get_u8((void *)frame, &rtn)) {
		print_text(COLOR_ERROR, "%s: invalid size", label);
		return false;
	}

	print_field("%s: %u", label, rtn);

	return true;
}

static bool print_ase_latency(const struct l2cap_frame *frame,
						const char *label)
{
	uint16_t latency;

	if (!l2cap_frame_get_le16((void *)frame, &latency)) {
		print_text(COLOR_ERROR, "%s: invalid size", label);
		return false;
	}

	print_field("%s: %u", label, latency);

	return true;
}

static bool print_ase_pd(const struct l2cap_frame *frame, const char *label)
{
	uint32_t pd;

	if (!l2cap_frame_get_le24((void *)frame, &pd)) {
		print_text(COLOR_ERROR, "%s: invalid size", label);
		return false;
	}

	print_field("%s: %u us", label, pd);

	return true;
}

static void ase_decode_freq(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;
	uint8_t value;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	if (!l2cap_frame_get_u8(&frame, &value)) {
		print_text(COLOR_ERROR, "    value: invalid size");
		goto done;
	}

	switch (value) {
	case 0x01:
		print_field("      Sampling Frequency: 8 Khz (0x01)");
		break;
	case 0x02:
		print_field("      Sampling Frequency: 11.25 Khz (0x02)");
		break;
	case 0x03:
		print_field("      Sampling Frequency: 16 Khz (0x03)");
		break;
	case 0x04:
		print_field("      Sampling Frequency: 22.05 Khz (0x04)");
		break;
	case 0x05:
		print_field("      Sampling Frequency: 24 Khz (0x04)");
		break;
	case 0x06:
		print_field("      Sampling Frequency: 32 Khz (0x04)");
		break;
	case 0x07:
		print_field("      Sampling Frequency: 44.1 Khz (0x04)");
		break;
	case 0x08:
		print_field("      Sampling Frequency: 48 Khz (0x04)");
		break;
	case 0x09:
		print_field("      Sampling Frequency: 88.2 Khz (0x04)");
		break;
	case 0x0a:
		print_field("      Sampling Frequency: 96 Khz (0x04)");
		break;
	case 0x0b:
		print_field("      Sampling Frequency: 176.4 Khz (0x04)");
		break;
	case 0x0c:
		print_field("      Sampling Frequency: 192 Khz (0x04)");
		break;
	case 0x0d:
		print_field("      Sampling Frequency: 384 Khz (0x04)");
		break;
	default:
		print_field("      Sampling Frequency: RFU (0x%2.2x)", value);
		break;
	}

done:
	if (frame.size)
		print_hex_field("    Data", frame.data, frame.size);
}

static void ase_decode_duration(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;
	uint8_t value;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	if (!l2cap_frame_get_u8(&frame, &value)) {
		print_text(COLOR_ERROR, "    value: invalid size");
		goto done;
	}

	switch (value) {
	case 0x00:
		print_field("      Frame Duration: 7.5 ms (0x00)");
		break;
	case 0x01:
		print_field("      Frame Duration: 10 ms (0x01)");
		break;
	default:
		print_field("      Frame Duration: RFU (0x%2.2x)", value);
		break;
	}

done:
	if (frame.size)
		print_hex_field("    Data", frame.data, frame.size);
}

static const struct bitfield_data channel_location_table[] = {
	{  0, "Front Left (0x00000001)"			},
	{  1, "Front Right (0x00000002)"		},
	{  2, "Front Center (0x00000004)"		},
	{  3, "Low Frequency Effects 1 (0x00000008)"	},
	{  4, "Back Left (0x00000010)"			},
	{  5, "Back Right (0x00000020)"			},
	{  6, "Front Left of Center (0x00000040)"	},
	{  7, "Front Right of Center (0x00000080)"	},
	{  8, "Back Center (0x00000100)"		},
	{  9, "Low Frequency Effects 2 (0x00000200)"	},
	{  10, "Side Left (0x00000400)"			},
	{  11, "Side Right (0x00000800)"		},
	{  12, "Top Front Left (0x00001000)"		},
	{  13, "Top Front Right (0x00002000)"		},
	{  14, "Top Front Center (0x00004000)"		},
	{  15, "Top Center (0x00008000)"		},
	{  16, "Top Back Left (0x00010000)"		},
	{  17, "Top Back Right (0x00020000)"		},
	{  18, "Top Side Left (0x00040000)"		},
	{  19, "Top Side Right (0x00080000)"		},
	{  20, "Top Back Center (0x00100000)"		},
	{  21, "Bottom Front Center (0x00200000)"	},
	{  22, "Bottom Front Left (0x00400000)"		},
	{  23, "Bottom Front Right (0x00800000)"	},
	{  24, "Front Left Wide (0x01000000)"		},
	{  25, "Front Right Wide (0x02000000)"		},
	{  26, "Left Surround (0x04000000)"		},
	{  27, "Right Surround (0x08000000)"		},
	{  28, "RFU (0x10000000)"			},
	{  29, "RFU (0x20000000)"			},
	{  30, "RFU (0x40000000)"			},
	{  31, "RFU (0x80000000)"			},
	{ }
};

static void print_location(const struct l2cap_frame *frame)
{
	uint32_t value;
	uint32_t mask;

	if (!l2cap_frame_get_le32((void *)frame, &value)) {
		print_text(COLOR_ERROR, "    value: invalid size");
		goto done;
	}

	print_field("   Location: 0x%8.8x", value);

	mask = print_bitfield(6, value, channel_location_table);
	if (mask)
		print_text(COLOR_WHITE_BG, "    Unknown fields (0x%8.8x)",
								mask);

done:
	if (frame->size)
		print_hex_field("  Data", frame->data, frame->size);
}

static void ase_decode_location(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	print_location(&frame);
}

static void ase_decode_frame_length(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;
	uint16_t value;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	if (!l2cap_frame_get_le16(&frame, &value)) {
		print_text(COLOR_ERROR, "    value: invalid size");
		goto done;
	}

	print_field("      Frame Length: %u (0x%4.4x)", value, value);

done:
	if (frame.size)
		print_hex_field("    Data", frame.data, frame.size);
}

static void ase_decode_blocks(const uint8_t *data, uint8_t len)
{
	struct l2cap_frame frame;
	uint8_t value;

	l2cap_frame_init(&frame, 0, 0, 0, 0, 0, 0, data, len);

	if (!l2cap_frame_get_u8(&frame, &value)) {
		print_text(COLOR_ERROR, "    value: invalid size");
		goto done;
	}

	print_field("      Frame Blocks per SDU: %u (0x%2.2x)", value, value);

done:
	if (frame.size)
		print_hex_field("    Data", frame.data, frame.size);
}

struct packet_ltv_decoder ase_cc_table[] = {
	LTV_DEC(0x01, ase_decode_freq),
	LTV_DEC(0x02, ase_decode_duration),
	LTV_DEC(0x03, ase_decode_location),
	LTV_DEC(0x04, ase_decode_frame_length),
	LTV_DEC(0x05, ase_decode_blocks)
};

static void print_ase_config(const struct l2cap_frame *frame)
{
	if (!print_prefer_framing(frame))
		return;

	if (!print_prefer_phy(frame))
		return;

	if (!print_ase_rtn(frame, "    RTN"))
		return;

	if (!print_ase_latency(frame, "    Max Transport Latency"))
		return;

	if (!print_ase_pd(frame, "    Presentation Delay Min"))
		return;

	if (!print_ase_pd(frame, "    Presentation Delay Max"))
		return;

	if (!print_ase_pd(frame, "    Preferred Presentation Delay Min"))
		return;

	if (!print_ase_pd(frame, "    Preferred Presentation Delay Max"))
		return;

	if (!print_ase_codec(frame))
		return;

	print_ase_cc(frame, "    Codec Specific Configuration",
			ase_cc_table, ARRAY_SIZE(ase_cc_table));
}

static bool print_ase_framing(const struct l2cap_frame *frame,
						const char *label)
{
	uint8_t framing;

	if (!l2cap_frame_get_u8((void *)frame, &framing)) {
		print_text(COLOR_ERROR, "%s: invalid size", label);
		return false;
	}

	switch (framing) {
	case 0x00:
		print_field("%s: Unframed (0x00)", label);
		break;
	case 0x01:
		print_field("%s: Framed (0x01)", label);
		break;
	default:
		print_field("%s: Reserved (0x%2.2x)", label, framing);
	}

	return true;
}

static const struct bitfield_data phy_table[] = {
	{  0, "LE 1M PHY (0x01)"		},
	{  1, "LE 2M PHY (0x02)"		},
	{  2, "LE Codec PHY (0x04)"		},
	{ }
};

static bool print_ase_phy(const struct l2cap_frame *frame, const char *label)
{
	uint8_t phy, mask;

	if (!l2cap_frame_get_u8((void *)frame, &phy)) {
		print_text(COLOR_ERROR, "%s: invalid size", label);
		return false;
	}

	print_field("%s: 0x%2.2x", label, phy);

	mask = print_bitfield(4, phy, phy_table);
	if (mask)
		print_text(COLOR_WHITE_BG, "    Unknown fields (0x%2.2x)",
								mask);

	return true;
}

static bool print_ase_interval(const struct l2cap_frame *frame,
						const char *label)
{
	uint32_t interval;

	if (!l2cap_frame_get_le24((void *)frame, &interval)) {
		print_text(COLOR_ERROR, "%s: invalid size", label);
		return false;
	}

	print_field("%s: %u usec", label, interval);

	return true;
}

static bool print_ase_sdu(const struct l2cap_frame *frame, const char *label)
{
	uint16_t sdu;

	if (!l2cap_frame_get_le16((void *)frame, &sdu)) {
		print_text(COLOR_ERROR, "%s: invalid size", label);
		return false;
	}

	print_field("%s: %u", label, sdu);

	return true;
}

static void print_ase_qos(const struct l2cap_frame *frame)
{
	if (!l2cap_frame_print_u8((void *)frame, "    CIG ID"))
		return;

	if (!l2cap_frame_print_u8((void *)frame, "    CIS ID"))
		return;

	if (!print_ase_interval(frame, "    SDU Interval"))
		return;

	if (!print_ase_framing(frame, "    Framing"))
		return;

	if (!print_ase_phy(frame, "    PHY"))
		return;

	if (!print_ase_sdu(frame, "    Max SDU"))
		return;

	if (!print_ase_rtn(frame, "    RTN"))
		return;

	if (!print_ase_latency(frame, "    Max Transport Latency"))
		return;

	print_ase_pd(frame, "    Presentation Delay");
}

static void print_ase_metadata_status(const struct l2cap_frame *frame)
{
	if (!l2cap_frame_print_u8((void *)frame, "    CIG ID"))
		return;

	if (!l2cap_frame_print_u8((void *)frame, "    CIS ID"))
		return;

	print_ase_metadata(frame);
}

static void print_ase_status(const struct l2cap_frame *frame)
{
	uint8_t id, state;

	if (!l2cap_frame_get_u8((void *)frame, &id)) {
		print_text(COLOR_ERROR, "ASE ID: invalid size");
		goto done;
	}

	print_field("    ASE ID: %u", id);

	if (!l2cap_frame_get_u8((void *)frame, &state)) {
		print_text(COLOR_ERROR, "ASE State: invalid size");
		goto done;
	}

	switch (state) {
	/* ASE_State = 0x00 (Idle) */
	case 0x00:
		print_field("    State: Idle (0x00)");
		break;
	/* ASE_State = 0x01 (Codec Configured) */
	case 0x01:
		print_field("    State: Codec Configured (0x01)");
		print_ase_config(frame);
		break;
	/* ASE_State = 0x02 (QoS Configured) */
	case 0x02:
		print_field("    State: QoS Configured (0x02)");
		print_ase_qos(frame);
		break;
	/* ASE_Status = 0x03 (Enabling) */
	case 0x03:
		print_field("    State: Enabling (0x03)");
		print_ase_metadata_status(frame);
		break;
	/* ASE_Status = 0x04 (Streaming) */
	case 0x04:
		print_field("    State: Streaming (0x04)");
		print_ase_metadata_status(frame);
		break;
	/* ASE_Status = 0x05 (Disabling) */
	case 0x05:
		print_field("    State: Disabling (0x05)");
		print_ase_metadata_status(frame);
		break;
	/* ASE_Status = 0x06 (Releasing) */
	case 0x06:
		print_field("    State: Releasing (0x06)");
		break;
	default:
		print_field("    State: Reserved (0x%2.2x)", state);
		break;
	}

done:
	if (frame->size)
		print_hex_field("  Data", frame->data, frame->size);
}

static void ase_read(const struct l2cap_frame *frame)
{
	print_ase_status(frame);
}

static void ase_notify(const struct l2cap_frame *frame)
{
	print_ase_status(frame);
}

static bool print_ase_target_latency(const struct l2cap_frame *frame)
{
	uint8_t latency;

	if (!l2cap_frame_get_u8((void *)frame, &latency)) {
		print_text(COLOR_ERROR, "    Target Latency: invalid size");
		return false;
	}

	switch (latency) {
	case 0x01:
		print_field("    Target Latency: Low Latency (0x01)");
		break;
	case 0x02:
		print_field("    Target Latency: Balance Latency/Reliability "
								"(0x02)");
		break;
	case 0x03:
		print_field("    Target Latency: High Reliability (0x03)");
		break;
	default:
		print_field("    Target Latency: Reserved (0x%2.2x)", latency);
		break;
	}

	return true;
}

static bool ase_config_cmd(const struct l2cap_frame *frame)
{
	if (!l2cap_frame_print_u8((void *)frame, "    ASE ID"))
		return false;

	if (!print_ase_target_latency(frame))
		return false;

	if (!print_ase_phy(frame, "    PHY"))
		return false;

	if (!print_ase_codec(frame))
		return false;

	if (!print_ase_cc(frame, "    Codec Specific Configuration",
				ase_cc_table, ARRAY_SIZE(ase_cc_table)))
		return false;

	return true;
}

static bool ase_qos_cmd(const struct l2cap_frame *frame)
{
	if (!l2cap_frame_print_u8((void *)frame, "    ASE ID"))
		return false;

	if (!l2cap_frame_print_u8((void *)frame, "    CIG ID"))
		return false;

	if (!l2cap_frame_print_u8((void *)frame, "    CIS ID"))
		return false;

	if (!print_ase_interval(frame, "    SDU Interval"))
		return false;

	if (!print_ase_framing(frame, "    Framing"))
		return false;

	if (!print_ase_phy(frame, "    PHY"))
		return false;

	if (!print_ase_sdu(frame, "    Max SDU"))
		return false;

	if (!print_ase_rtn(frame, "    RTN"))
		return false;

	if (!print_ase_latency(frame, "    Max Transport Latency"))
		return false;

	if (!print_ase_pd(frame, "    Presentation Delay"))
		return false;

	return true;
}

static bool ase_enable_cmd(const struct l2cap_frame *frame)
{
	if (!l2cap_frame_print_u8((void *)frame, "    ASE ID"))
		return false;

	if (!print_ase_metadata(frame))
		return false;

	return true;
}

static bool ase_start_cmd(const struct l2cap_frame *frame)
{
	if (!l2cap_frame_print_u8((void *)frame, "    ASE ID"))
		return false;

	return true;
}

static bool ase_disable_cmd(const struct l2cap_frame *frame)
{
	if (!l2cap_frame_print_u8((void *)frame, "    ASE ID"))
		return false;

	return true;
}

static bool ase_stop_cmd(const struct l2cap_frame *frame)
{
	if (!l2cap_frame_print_u8((void *)frame, "    ASE ID"))
		return false;

	return true;
}

static bool ase_metadata_cmd(const struct l2cap_frame *frame)
{
	if (!l2cap_frame_print_u8((void *)frame, "    ASE ID"))
		return false;

	if (!print_ase_metadata(frame))
		return false;

	return true;
}

static bool ase_release_cmd(const struct l2cap_frame *frame)
{
	if (!l2cap_frame_print_u8((void *)frame, "    ASE ID"))
		return false;

	return true;
}

#define ASE_CMD(_op, _desc, _func) \
[_op] = { \
	.desc = _desc, \
	.func = _func, \
}

struct ase_cmd {
	const char *desc;
	bool (*func)(const struct l2cap_frame *frame);
} ase_cmd_table[] = {
	/* Opcode = 0x01 (Codec Configuration) */
	ASE_CMD(0x01, "Codec Configuration", ase_config_cmd),
	/* Opcode = 0x02 (QoS Configuration) */
	ASE_CMD(0x02, "QoS Configuration", ase_qos_cmd),
	/* Opcode = 0x03 (Enable) */
	ASE_CMD(0x03, "Enable", ase_enable_cmd),
	/* Opcode = 0x04 (Receiver Start Ready) */
	ASE_CMD(0x04, "Receiver Start Ready", ase_start_cmd),
	/* Opcode = 0x05 (Disable) */
	ASE_CMD(0x05, "Disable", ase_disable_cmd),
	/* Opcode = 0x06 (Receiver Stop Ready) */
	ASE_CMD(0x06, "Receiver Stop Ready", ase_stop_cmd),
	/* Opcode = 0x07 (Update Metadata) */
	ASE_CMD(0x07, "Update Metadata", ase_metadata_cmd),
	/* Opcode = 0x08 (Release) */
	ASE_CMD(0x08, "Release", ase_release_cmd),
};

static struct ase_cmd *ase_get_cmd(uint8_t op)
{
	if (op > ARRAY_SIZE(ase_cmd_table))
		return NULL;

	return &ase_cmd_table[op];
}

static void print_ase_cmd(const struct l2cap_frame *frame)
{
	uint8_t op, num, i;
	struct ase_cmd *cmd;

	if (!l2cap_frame_get_u8((void *)frame, &op)) {
		print_text(COLOR_ERROR, "opcode: invalid size");
		goto done;
	}

	if (!l2cap_frame_get_u8((void *)frame, &num)) {
		print_text(COLOR_ERROR, "num: invalid size");
		goto done;
	}

	cmd = ase_get_cmd(op);
	if (!cmd) {
		print_field("    Opcode: Reserved (0x%2.2x)", op);
		goto done;
	}

	print_field("    Opcode: %s (0x%2.2x)", cmd->desc, op);
	print_field("    Number of ASE(s): %u", num);

	for (i = 0; i < num && frame->size; i++) {
		print_field("    ASE: #%u", i);

		if (!cmd->func(frame))
			break;
	}

done:
	if (frame->size)
		print_hex_field("  Data", frame->data, frame->size);
}

static void ase_cp_write(const struct l2cap_frame *frame)
{
	print_ase_cmd(frame);
}

static bool print_ase_cp_rsp_code(const struct l2cap_frame *frame)
{
	uint8_t code;

	if (!l2cap_frame_get_u8((void *)frame, &code)) {
		print_text(COLOR_ERROR, "    ASE Response Code: invalid size");
		return false;
	}

	switch (code) {
	case 0x00:
		print_field("    ASE Response Code: Success (0x00)");
		break;
	case 0x01:
		print_field("    ASE Response Code: Unsupported Opcode (0x01)");
		break;
	case 0x02:
		print_field("    ASE Response Code: Invalid Length (0x02)");
		break;
	case 0x03:
		print_field("    ASE Response Code: Invalid ASE ID (0x03)");
		break;
	case 0x04:
		print_field("    ASE Response Code: Invalid ASE State (0x04)");
		break;
	case 0x05:
		print_field("    ASE Response Code: Invalid ASE Direction "
								"(0x05)");
		break;
	case 0x06:
		print_field("    ASE Response Code: Unsupported Audio "
							"Capabilities (0x06)");
		break;
	case 0x07:
		print_field("    ASE Response Code: Unsupported Configuration "
								"(0x07)");
		break;
	case 0x08:
		print_field("    ASE Response Code: Rejected Configuration "
								"(0x08)");
		break;
	case 0x09:
		print_field("    ASE Response Code: Invalid Configuration "
								"(0x09)");
		break;
	case 0x0a:
		print_field("    ASE Response Code: Unsupported Metadata "
								"(0x0a)");
		break;
	case 0x0b:
		print_field("    ASE Response Code: Rejected Metadata (0x0b)");
		break;
	case 0x0c:
		print_field("    ASE Response Code: Invalid Metadata (0x0c)");
		break;
	case 0x0d:
		print_field("    ASE Response Code: Insufficient Resources "
								"(0x0d)");
		break;
	case 0x0e:
		print_field("    ASE Response Code: Unspecified Error (0x0e)");
		break;
	default:
		print_field("    ASE Response Code: Reserved (0x%2.2x)", code);
		break;
	}

	return true;
}

static bool print_ase_cp_rsp_reason(const struct l2cap_frame *frame)
{
	uint8_t reason;

	if (!l2cap_frame_get_u8((void *)frame, &reason)) {
		print_text(COLOR_ERROR,
				"    ASE Response Reason: invalid size");
		return false;
	}

	switch (reason) {
	case 0x00:
		print_field("    ASE Response Reason: None (0x00)");
		break;
	case 0x01:
		print_field("    ASE Response Reason: ASE ID (0x01)");
		break;
	case 0x02:
		print_field("    ASE Response Reason: Codec Specific "
						"Configuration (0x02)");
		break;
	case 0x03:
		print_field("    ASE Response Reason: SDU Interval (0x03)");
		break;
	case 0x04:
		print_field("    ASE Response Reason: Framing (0x04)");
		break;
	case 0x05:
		print_field("    ASE Response Reason: PHY (0x05)");
		break;
	case 0x06:
		print_field("    ASE Response Reason: Max SDU (0x06)");
		break;
	case 0x07:
		print_field("    ASE Response Reason: RTN (0x07)");
		break;
	case 0x08:
		print_field("    ASE Response Reason: Max Transport Latency "
								"(0x08)");
		break;
	case 0x09:
		print_field("    ASE Response Reason: Presentation Delay "
								"(0x09)");
		break;
	case 0x0a:
		print_field("    ASE Response Reason: Invalid ASE/CIS Mapping "
								"(0x0a)");
		break;
	default:
		print_field("    ASE Response Reason: Reserved (0x%2.2x)",
								reason);
		break;
	}

	return true;
}

static void print_ase_cp_rsp(const struct l2cap_frame *frame)
{
	uint8_t op, num, i;
	struct ase_cmd *cmd;

	if (!l2cap_frame_get_u8((void *)frame, &op)) {
		print_text(COLOR_ERROR, "    opcode: invalid size");
		goto done;
	}

	if (!l2cap_frame_get_u8((void *)frame, &num)) {
		print_text(COLOR_ERROR, "    Number of ASE(s): invalid size");
		goto done;
	}

	cmd = ase_get_cmd(op);
	if (!cmd) {
		print_field("    Opcode: Reserved (0x%2.2x)", op);
		goto done;
	}

	print_field("    Opcode: %s (0x%2.2x)", cmd->desc, op);
	print_field("    Number of ASE(s): %u", num);

	for (i = 0; i < num && frame->size; i++) {
		print_field("    ASE: #%u", i);

		if (!l2cap_frame_print_u8((void *)frame, "    ASE ID"))
			break;

		if (!print_ase_cp_rsp_code(frame))
			break;

		if (!print_ase_cp_rsp_reason(frame))
			break;
	}

done:
	if (frame->size)
		print_hex_field("  Data", frame->data, frame->size);
}

static void ase_cp_notify(const struct l2cap_frame *frame)
{
	print_ase_cp_rsp(frame);
}

static void pac_loc_read(const struct l2cap_frame *frame)
{
	print_location(frame);
}

static void pac_loc_notify(const struct l2cap_frame *frame)
{
	print_location(frame);
}

static void print_pac_context(const struct l2cap_frame *frame)
{
	uint16_t snk, src;
	uint16_t mask;

	if (!l2cap_frame_get_le16((void *)frame, &snk)) {
		print_text(COLOR_ERROR, "  sink: invalid size");
		goto done;
	}

	print_field("  Sink Context: 0x%4.4x", snk);

	mask = print_bitfield(4, snk, pac_context_table);
	if (mask)
		print_text(COLOR_WHITE_BG, "  Unknown fields (0x%4.4x)",
								mask);

	if (!l2cap_frame_get_le16((void *)frame, &src)) {
		print_text(COLOR_ERROR, "  source: invalid size");
		goto done;
	}

	print_field("  Source Context: 0x%4.4x", src);

	mask = print_bitfield(4, src, pac_context_table);
	if (mask)
		print_text(COLOR_WHITE_BG, "  Unknown fields (0x%4.4x)",
								mask);

done:
	if (frame->size)
		print_hex_field("  Data", frame->data, frame->size);
}

static void pac_context_read(const struct l2cap_frame *frame)
{
	print_pac_context(frame);
}

static void pac_context_notify(const struct l2cap_frame *frame)
{
	print_pac_context(frame);
}

#define GATT_HANDLER(_uuid, _read, _write, _notify) \
{ \
	.uuid = { \
		.type = BT_UUID16, \
		.value.u16 = _uuid, \
	}, \
	.read = _read, \
	.write = _write, \
	.notify = _notify \
}

struct gatt_handler {
	bt_uuid_t uuid;
	void (*read)(const struct l2cap_frame *frame);
	void (*write)(const struct l2cap_frame *frame);
	void (*notify)(const struct l2cap_frame *frame);
} gatt_handlers[] = {
	GATT_HANDLER(0x2902, ccc_read, ccc_write, NULL),
	GATT_HANDLER(0x2bc4, ase_read, NULL, ase_notify),
	GATT_HANDLER(0x2bc5, ase_read, NULL, ase_notify),
	GATT_HANDLER(0x2bc6, NULL, ase_cp_write, ase_cp_notify),
	GATT_HANDLER(0x2bc9, pac_read, NULL, pac_notify),
	GATT_HANDLER(0x2bca, pac_loc_read, NULL, pac_loc_notify),
	GATT_HANDLER(0x2bcb, pac_read, NULL, pac_notify),
	GATT_HANDLER(0x2bcc, pac_loc_read, NULL, pac_loc_notify),
	GATT_HANDLER(0x2bcd, pac_context_read, NULL, pac_context_notify),
	GATT_HANDLER(0x2bce, pac_context_read, NULL, pac_context_notify),
};

static struct gatt_handler *get_handler(struct gatt_db_attribute *attr)
{
	const bt_uuid_t *uuid = gatt_db_attribute_get_type(attr);
	size_t i;

	for (i = 0; i < ARRAY_SIZE(gatt_handlers); i++) {
		struct gatt_handler *handler = &gatt_handlers[i];

		if (!bt_uuid_cmp(&handler->uuid, uuid))
			return handler;
	}

	return NULL;
}

static void att_exchange_mtu_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_exchange_mtu_req *pdu = frame->data;

	print_field("Client RX MTU: %d", le16_to_cpu(pdu->mtu));
}

static void att_exchange_mtu_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_exchange_mtu_rsp *pdu = frame->data;

	print_field("Server RX MTU: %d", le16_to_cpu(pdu->mtu));
}

static void att_find_info_req(const struct l2cap_frame *frame)
{
	print_handle_range("Handle range", frame->data);
}

static const char *att_format_str(uint8_t format)
{
	switch (format) {
	case 0x01:
		return "UUID-16";
	case 0x02:
		return "UUID-128";
	default:
		return "unknown";
	}
}

static uint16_t print_info_data_16(const void *data, uint16_t len)
{
	while (len >= 4) {
		print_field("Handle: 0x%4.4x", get_le16(data));
		print_uuid("UUID", data + 2, 2);
		data += 4;
		len -= 4;
	}

	return len;
}

static uint16_t print_info_data_128(const void *data, uint16_t len)
{
	while (len >= 18) {
		print_field("Handle: 0x%4.4x", get_le16(data));
		print_uuid("UUID", data + 2, 16);
		data += 18;
		len -= 18;
	}

	return len;
}

static void att_find_info_rsp(const struct l2cap_frame *frame)
{
	const uint8_t *format = frame->data;
	uint16_t len;

	print_field("Format: %s (0x%2.2x)", att_format_str(*format), *format);

	if (*format == 0x01)
		len = print_info_data_16(frame->data + 1, frame->size - 1);
	else if (*format == 0x02)
		len = print_info_data_128(frame->data + 1, frame->size - 1);
	else
		len = frame->size - 1;

	packet_hexdump(frame->data + (frame->size - len), len);
}

static void att_find_by_type_val_req(const struct l2cap_frame *frame)
{
	uint16_t type;

	print_handle_range("Handle range", frame->data);

	type = get_le16(frame->data + 4);
	print_attribute_info(type, frame->data + 6, frame->size - 6);
}

static void att_find_by_type_val_rsp(const struct l2cap_frame *frame)
{
	const uint8_t *ptr = frame->data;
	uint16_t len = frame->size;

	while (len >= 4) {
		print_handle_range("Handle range", ptr);
		ptr += 4;
		len -= 4;
	}

	packet_hexdump(ptr, len);
}

static void att_read_type_req(const struct l2cap_frame *frame)
{
	print_handle_range("Handle range", frame->data);
	print_uuid("Attribute type", frame->data + 4, frame->size - 4);
}

static void att_read_type_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_read_group_type_rsp *pdu = frame->data;

	print_field("Attribute data length: %d", pdu->length);
	print_data_list("Attribute data list", pdu->length,
					frame->data + 1, frame->size - 1);
}

struct att_read {
	struct gatt_db_attribute *attr;
	bool in;
	uint16_t chan;
	void (*func)(const struct l2cap_frame *frame);
};

struct att_conn_data {
	struct gatt_db *ldb;
	struct gatt_db *rdb;
	struct queue *reads;
};

static void att_conn_data_free(void *data)
{
	struct att_conn_data *att_data = data;

	gatt_db_unref(att_data->rdb);
	gatt_db_unref(att_data->ldb);
	queue_destroy(att_data->reads, free);
	free(att_data);
}

static void load_gatt_db(struct packet_conn_data *conn)
{
	struct att_conn_data *data = conn->data;
	char filename[PATH_MAX];
	char local[18];
	char peer[18];

	if (!data) {
		data = new0(struct att_conn_data, 1);
		data->rdb = gatt_db_new();
		data->ldb = gatt_db_new();
		conn->data = data;
		conn->destroy = att_conn_data_free;
	}

	if (!gatt_db_isempty(data->ldb) && !gatt_db_isempty(data->rdb))
		return;

	ba2str((bdaddr_t *)conn->src, local);
	ba2str((bdaddr_t *)conn->dst, peer);

	if (gatt_db_isempty(data->ldb)) {
		create_filename(filename, PATH_MAX, "/%s/attributes", local);
		btd_settings_gatt_db_load(data->ldb, filename);
	}

	if (gatt_db_isempty(data->rdb)) {
		create_filename(filename, PATH_MAX, "/%s/cache/%s", local,
								peer);
		btd_settings_gatt_db_load(data->rdb, filename);
	}
}

static struct gatt_db_attribute *get_attribute(const struct l2cap_frame *frame,
						uint16_t handle, bool rsp)
{
	struct packet_conn_data *conn;
	struct att_conn_data *data;
	struct gatt_db *db;

	conn = packet_get_conn_data(frame->handle);
	if (!conn)
		return NULL;

	/* Try loading local and remote gatt_db if not loaded yet */
	load_gatt_db(conn);

	data = conn->data;
	if (!data)
		return NULL;

	if (frame->in) {
		if (rsp)
			db = data->rdb;
		else
			db = data->ldb;
	} else {
		if (rsp)
			db = data->ldb;
		else
			db = data->rdb;
	}

	return gatt_db_get_attribute(db, handle);
}

static void print_attribute(struct gatt_db_attribute *attr)
{
	uint16_t handle = gatt_db_attribute_get_handle(attr);
	const bt_uuid_t *uuid;
	char label[21];

	uuid = gatt_db_attribute_get_type(attr);
	if (!uuid)
		goto done;

	switch (uuid->type) {
	case BT_UUID16:
		sprintf(label, "Handle: 0x%4.4x Type", handle);
		print_field("%s: %s (0x%4.4x)", label,
				bt_uuid16_to_str(uuid->value.u16),
				uuid->value.u16);
		return;
	case BT_UUID128:
		sprintf(label, "Handle: 0x%4.4x Type", handle);
		print_uuid(label, &uuid->value.u128, 16);
		return;
	case BT_UUID_UNSPEC:
	case BT_UUID32:
		break;
	}

done:
	print_field("Handle: 0x%4.4x", handle);
}

static void print_handle(const struct l2cap_frame *frame, uint16_t handle,
								bool rsp)
{
	struct gatt_db_attribute *attr;

	attr = get_attribute(frame, handle, rsp);
	if (!attr) {
		print_field("Handle: 0x%4.4x", handle);
		return;
	}

	print_attribute(attr);
}

static void att_read_req(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_read_req *pdu = frame->data;
	uint16_t handle;
	struct packet_conn_data *conn;
	struct att_conn_data *data;
	struct att_read *read;
	struct gatt_db_attribute *attr;
	struct gatt_handler *handler;

	l2cap_frame_pull((void *)frame, frame, sizeof(*pdu));

	handle = le16_to_cpu(pdu->handle);
	print_handle(frame, handle, false);

	attr = get_attribute(frame, handle, false);
	if (!attr)
		return;

	handler = get_handler(attr);
	if (!handler || !handler->read)
		return;

	conn = packet_get_conn_data(frame->handle);
	data = conn->data;

	if (!data->reads)
		data->reads = queue_new();

	read = new0(struct att_read, 1);
	read->attr = attr;
	read->in = frame->in;
	read->chan = frame->chan;
	read->func = handler->read;

	queue_push_tail(data->reads, read);
}

static bool match_read_frame(const void *data, const void *match_data)
{
	const struct att_read *read = data;
	const struct l2cap_frame *frame = match_data;

	/* Read frame and response frame shall be in the opposite direction to
	 * match.
	 */
	if (read->in == frame->in)
		return false;

	return read->chan == frame->chan;
}

static void att_read_rsp(const struct l2cap_frame *frame)
{
	struct packet_conn_data *conn;
	struct att_conn_data *data;
	struct att_read *read;

	print_hex_field("Value", frame->data, frame->size);

	conn = packet_get_conn_data(frame->handle);
	if (!conn)
		return;

	data = conn->data;

	read = queue_remove_if(data->reads, match_read_frame, (void *)frame);
	if (!read)
		return;

	print_attribute(read->attr);

	read->func(frame);

	free(read);
}

static void att_read_blob_req(const struct l2cap_frame *frame)
{
	print_handle(frame, get_le16(frame->data), false);
	print_field("Offset: 0x%4.4x", get_le16(frame->data + 2));
}

static void att_read_blob_rsp(const struct l2cap_frame *frame)
{
	packet_hexdump(frame->data, frame->size);
}

static void att_read_multiple_req(const struct l2cap_frame *frame)
{
	int i, count;

	count = frame->size / 2;

	for (i = 0; i < count; i++)
		print_handle(frame, get_le16(frame->data + (i * 2)), false);
}

static void att_read_group_type_req(const struct l2cap_frame *frame)
{
	print_handle_range("Handle range", frame->data);
	print_uuid("Attribute group type", frame->data + 4, frame->size - 4);
}

static void print_group_list(const char *label, uint8_t length,
					const void *data, uint16_t size)
{
	uint8_t count;

	if (length == 0)
		return;

	count = size / length;

	print_field("%s: %u entr%s", label, count, count == 1 ? "y" : "ies");

	while (size >= length) {
		print_handle_range("Handle range", data);
		print_uuid("UUID", data + 4, length - 4);

		data += length;
		size -= length;
	}

	packet_hexdump(data, size);
}

static void att_read_group_type_rsp(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_read_group_type_rsp *pdu = frame->data;

	print_field("Attribute data length: %d", pdu->length);
	print_group_list("Attribute group list", pdu->length,
					frame->data + 1, frame->size - 1);
}

static void print_write(const struct l2cap_frame *frame, uint16_t handle,
							size_t len)
{
	struct gatt_db_attribute *attr;
	struct gatt_handler *handler;

	print_handle(frame, handle, false);
	print_hex_field("  Data", frame->data, frame->size);

	if (len > frame->size) {
		print_text(COLOR_ERROR, "invalid size");
		return;
	}

	attr = get_attribute(frame, handle, false);
	if (!attr)
		return;

	handler = get_handler(attr);
	if (!handler)
		return;

	handler->write(frame);
}

static void att_write_req(const struct l2cap_frame *frame)
{
	uint16_t handle;

	if (!l2cap_frame_get_le16((void *)frame, &handle)) {
		print_text(COLOR_ERROR, "invalid size");
		return;
	}

	print_write(frame, handle, frame->size);
}

static void att_write_rsp(const struct l2cap_frame *frame)
{
}

static void att_prepare_write_req(const struct l2cap_frame *frame)
{
	print_handle(frame, get_le16(frame->data), false);
	print_field("Offset: 0x%4.4x", get_le16(frame->data + 2));
	print_hex_field("  Data", frame->data + 4, frame->size - 4);
}

static void att_prepare_write_rsp(const struct l2cap_frame *frame)
{
	print_handle(frame, get_le16(frame->data), true);
	print_field("Offset: 0x%4.4x", get_le16(frame->data + 2));
	print_hex_field("  Data", frame->data + 4, frame->size - 4);
}

static void att_execute_write_req(const struct l2cap_frame *frame)
{
	uint8_t flags = *(uint8_t *) frame->data;
	const char *flags_str;

	switch (flags) {
	case 0x00:
		flags_str = "Cancel all prepared writes";
		break;
	case 0x01:
		flags_str = "Immediately write all pending values";
		break;
	default:
		flags_str = "Unknown";
		break;
	}

	print_field("Flags: %s (0x%02x)", flags_str, flags);
}

static void print_notify(const struct l2cap_frame *frame, uint16_t handle,
								size_t len)
{
	struct gatt_db_attribute *attr;
	struct gatt_handler *handler;
	struct l2cap_frame clone;

	print_handle(frame, handle, false);
	print_hex_field("  Data", frame->data, len);

	if (len > frame->size) {
		print_text(COLOR_ERROR, "invalid size");
		return;
	}

	attr = get_attribute(frame, handle, true);
	if (!attr)
		return;

	handler = get_handler(attr);
	if (!handler)
		return;

	/* Use a clone if the callback is not expected to parse the whole
	 * frame.
	 */
	if (len != frame->size) {
		l2cap_frame_clone(&clone, frame);
		clone.size = len;
		frame = &clone;
	}

	handler->notify(frame);
}

static void att_handle_value_notify(const struct l2cap_frame *frame)
{
	uint16_t handle;
	const struct bt_l2cap_att_handle_value_notify *pdu = frame->data;

	l2cap_frame_pull((void *)frame, frame, sizeof(*pdu));

	handle = le16_to_cpu(pdu->handle);
	print_notify(frame, handle, frame->size);
}

static void att_handle_value_ind(const struct l2cap_frame *frame)
{
	const struct bt_l2cap_att_handle_value_ind *pdu = frame->data;

	l2cap_frame_pull((void *)frame, frame, sizeof(*pdu));

	print_notify(frame, le16_to_cpu(pdu->handle), frame->size);
}

static void att_handle_value_conf(const struct l2cap_frame *frame)
{
}

static void att_multiple_vl_rsp(const struct l2cap_frame *frame)
{
	struct l2cap_frame *f = (void *) frame;

	while (frame->size) {
		uint16_t handle;
		uint16_t len;

		if (!l2cap_frame_get_le16(f, &handle))
			return;

		if (!l2cap_frame_get_le16(f, &len))
			return;

		print_field("Length: 0x%4.4x", len);

		print_notify(frame, handle, len);

		l2cap_frame_pull(f, f, len);
	}
}

static void att_write_command(const struct l2cap_frame *frame)
{
	uint16_t handle;

	if (!l2cap_frame_get_le16((void *)frame, &handle)) {
		print_text(COLOR_ERROR, "invalid size");
		return;
	}

	print_write(frame, handle, frame->size);
}

static void att_signed_write_command(const struct l2cap_frame *frame)
{
	uint16_t handle;

	if (!l2cap_frame_get_le16((void *)frame, &handle)) {
		print_text(COLOR_ERROR, "invalid size");
		return;
	}

	print_write(frame, handle, frame->size - 12);
	print_hex_field("  Data", frame->data, frame->size - 12);
	print_hex_field("  Signature", frame->data + frame->size - 12, 12);
}

struct att_opcode_data {
	uint8_t opcode;
	const char *str;
	void (*func) (const struct l2cap_frame *frame);
	uint8_t size;
	bool fixed;
};

static const struct att_opcode_data att_opcode_table[] = {
	{ 0x01, "Error Response",
			att_error_response, 4, true },
	{ 0x02, "Exchange MTU Request",
			att_exchange_mtu_req, 2, true },
	{ 0x03, "Exchange MTU Response",
			att_exchange_mtu_rsp, 2, true },
	{ 0x04, "Find Information Request",
			att_find_info_req, 4, true },
	{ 0x05, "Find Information Response",
			att_find_info_rsp, 5, false },
	{ 0x06, "Find By Type Value Request",
			att_find_by_type_val_req, 6, false },
	{ 0x07, "Find By Type Value Response",
			att_find_by_type_val_rsp, 4, false },
	{ 0x08, "Read By Type Request",
			att_read_type_req, 6, false },
	{ 0x09, "Read By Type Response",
			att_read_type_rsp, 3, false },
	{ 0x0a, "Read Request",
			att_read_req, 2, true },
	{ 0x0b, "Read Response",
			att_read_rsp, 0, false },
	{ 0x0c, "Read Blob Request",
			att_read_blob_req, 4, true },
	{ 0x0d, "Read Blob Response",
			att_read_blob_rsp, 0, false },
	{ 0x0e, "Read Multiple Request",
			att_read_multiple_req, 4, false },
	{ 0x0f, "Read Multiple Response"	},
	{ 0x10, "Read By Group Type Request",
			att_read_group_type_req, 6, false },
	{ 0x11, "Read By Group Type Response",
			att_read_group_type_rsp, 4, false },
	{ 0x12, "Write Request"	,
			att_write_req, 2, false	},
	{ 0x13, "Write Response",
			att_write_rsp, 0, true	},
	{ 0x16, "Prepare Write Request",
			att_prepare_write_req, 4, false },
	{ 0x17, "Prepare Write Response",
			att_prepare_write_rsp, 4, false },
	{ 0x18, "Execute Write Request",
			att_execute_write_req, 1, true },
	{ 0x19, "Execute Write Response"	},
	{ 0x1b, "Handle Value Notification",
			att_handle_value_notify, 2, false },
	{ 0x1d, "Handle Value Indication",
			att_handle_value_ind, 2, false },
	{ 0x1e, "Handle Value Confirmation",
			att_handle_value_conf, 0, true },
	{ 0x20, "Read Multiple Request Variable Length",
			att_read_multiple_req, 4, false },
	{ 0x21, "Read Multiple Response Variable Length",
			att_multiple_vl_rsp, 4, false },
	{ 0x23, "Handle Multiple Value Notification",
			att_multiple_vl_rsp, 4, false },
	{ 0x52, "Write Command",
			att_write_command, 2, false },
	{ 0xd2, "Signed Write Command", att_signed_write_command, 14, false },
	{ }
};

static const char *att_opcode_to_str(uint8_t opcode)
{
	int i;

	for (i = 0; att_opcode_table[i].str; i++) {
		if (att_opcode_table[i].opcode == opcode)
			return att_opcode_table[i].str;
	}

	return "Unknown";
}

void att_packet(uint16_t index, bool in, uint16_t handle, uint16_t cid,
					const void *data, uint16_t size)
{
	struct l2cap_frame frame;
	uint8_t opcode = *((const uint8_t *) data);
	const struct att_opcode_data *opcode_data = NULL;
	const char *opcode_color, *opcode_str;
	int i;

	if (size < 1) {
		print_text(COLOR_ERROR, "malformed attribute packet");
		packet_hexdump(data, size);
		return;
	}

	for (i = 0; att_opcode_table[i].str; i++) {
		if (att_opcode_table[i].opcode == opcode) {
			opcode_data = &att_opcode_table[i];
			break;
		}
	}

	if (opcode_data) {
		if (opcode_data->func) {
			if (in)
				opcode_color = COLOR_MAGENTA;
			else
				opcode_color = COLOR_BLUE;
		} else
			opcode_color = COLOR_WHITE_BG;
		opcode_str = opcode_data->str;
	} else {
		opcode_color = COLOR_WHITE_BG;
		opcode_str = "Unknown";
	}

	print_indent(6, opcode_color, "ATT: ", opcode_str, COLOR_OFF,
				" (0x%2.2x) len %d", opcode, size - 1);

	if (!opcode_data || !opcode_data->func) {
		packet_hexdump(data + 1, size - 1);
		return;
	}

	if (opcode_data->fixed) {
		if (size - 1 != opcode_data->size) {
			print_text(COLOR_ERROR, "invalid size");
			packet_hexdump(data + 1, size - 1);
			return;
		}
	} else {
		if (size - 1 < opcode_data->size) {
			print_text(COLOR_ERROR, "too short packet");
			packet_hexdump(data + 1, size - 1);
			return;
		}
	}

	l2cap_frame_init(&frame, index, in, handle, 0, cid, 0,
						data + 1, size - 1);
	opcode_data->func(&frame);
}
