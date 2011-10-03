/*
 * Generated by asn1c-0.9.23 (http://lionet.info/asn1c)
 * From ASN.1 module "POKERTH-PROTOCOL"
 * 	found in "../../../docs/pokerth.asn1"
 * 	`asn1c -fskeletons-copy`
 */

#include "ReportAvatarAckMessage.h"

static int
reportResult_3_constraint(asn_TYPE_descriptor_t *td, const void *sptr,
                          asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	/* Replace with underlying type checker */
	td->check_constraints = asn_DEF_NativeEnumerated.check_constraints;
	return td->check_constraints(td, sptr, ctfailcb, app_key);
}

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static void
reportResult_3_inherit_TYPE_descriptor(asn_TYPE_descriptor_t *td) {
	td->free_struct    = asn_DEF_NativeEnumerated.free_struct;
	td->print_struct   = asn_DEF_NativeEnumerated.print_struct;
	td->ber_decoder    = asn_DEF_NativeEnumerated.ber_decoder;
	td->der_encoder    = asn_DEF_NativeEnumerated.der_encoder;
	td->xer_decoder    = asn_DEF_NativeEnumerated.xer_decoder;
	td->xer_encoder    = asn_DEF_NativeEnumerated.xer_encoder;
	td->uper_decoder   = asn_DEF_NativeEnumerated.uper_decoder;
	td->uper_encoder   = asn_DEF_NativeEnumerated.uper_encoder;
	if(!td->per_constraints)
		td->per_constraints = asn_DEF_NativeEnumerated.per_constraints;
	td->elements       = asn_DEF_NativeEnumerated.elements;
	td->elements_count = asn_DEF_NativeEnumerated.elements_count;
	/* td->specifics      = asn_DEF_NativeEnumerated.specifics;	// Defined explicitly */
}

static void
reportResult_3_free(asn_TYPE_descriptor_t *td,
                    void *struct_ptr, int contents_only) {
	reportResult_3_inherit_TYPE_descriptor(td);
	td->free_struct(td, struct_ptr, contents_only);
}

static int
reportResult_3_print(asn_TYPE_descriptor_t *td, const void *struct_ptr,
                     int ilevel, asn_app_consume_bytes_f *cb, void *app_key) {
	reportResult_3_inherit_TYPE_descriptor(td);
	return td->print_struct(td, struct_ptr, ilevel, cb, app_key);
}

static asn_dec_rval_t
reportResult_3_decode_ber(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
                          void **structure, const void *bufptr, size_t size, int tag_mode) {
	reportResult_3_inherit_TYPE_descriptor(td);
	return td->ber_decoder(opt_codec_ctx, td, structure, bufptr, size, tag_mode);
}

static asn_enc_rval_t
reportResult_3_encode_der(asn_TYPE_descriptor_t *td,
                          void *structure, int tag_mode, ber_tlv_tag_t tag,
                          asn_app_consume_bytes_f *cb, void *app_key) {
	reportResult_3_inherit_TYPE_descriptor(td);
	return td->der_encoder(td, structure, tag_mode, tag, cb, app_key);
}

static asn_dec_rval_t
reportResult_3_decode_xer(asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td,
                          void **structure, const char *opt_mname, const void *bufptr, size_t size) {
	reportResult_3_inherit_TYPE_descriptor(td);
	return td->xer_decoder(opt_codec_ctx, td, structure, opt_mname, bufptr, size);
}

static asn_enc_rval_t
reportResult_3_encode_xer(asn_TYPE_descriptor_t *td, void *structure,
                          int ilevel, enum xer_encoder_flags_e flags,
                          asn_app_consume_bytes_f *cb, void *app_key) {
	reportResult_3_inherit_TYPE_descriptor(td);
	return td->xer_encoder(td, structure, ilevel, flags, cb, app_key);
}

static asn_INTEGER_enum_map_t asn_MAP_reportResult_value2enum_3[] = {
	{ 0,	20,	"avatarReportAccepted" },
	{ 1,	21,	"avatarReportDuplicate" },
	{ 2,	19,	"avatarReportInvalid" }
};
static unsigned int asn_MAP_reportResult_enum2value_3[] = {
	0,	/* avatarReportAccepted(0) */
	1,	/* avatarReportDuplicate(1) */
	2	/* avatarReportInvalid(2) */
};
static asn_INTEGER_specifics_t asn_SPC_reportResult_specs_3 = {
	asn_MAP_reportResult_value2enum_3,	/* "tag" => N; sorted by tag */
	asn_MAP_reportResult_enum2value_3,	/* N => "tag"; sorted by N */
	3,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static ber_tlv_tag_t asn_DEF_reportResult_tags_3[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_reportResult_3 = {
	"reportResult",
	"reportResult",
	reportResult_3_free,
	reportResult_3_print,
	reportResult_3_constraint,
	reportResult_3_decode_ber,
	reportResult_3_encode_der,
	reportResult_3_decode_xer,
	reportResult_3_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_reportResult_tags_3,
	sizeof(asn_DEF_reportResult_tags_3)
	/sizeof(asn_DEF_reportResult_tags_3[0]), /* 1 */
	asn_DEF_reportResult_tags_3,	/* Same as above */
	sizeof(asn_DEF_reportResult_tags_3)
	/sizeof(asn_DEF_reportResult_tags_3[0]), /* 1 */
	0,	/* No PER visible constraints */
	0, 0,	/* Defined elsewhere */
	&asn_SPC_reportResult_specs_3	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_ReportAvatarAckMessage_1[] = {
	{	ATF_NOFLAGS, 0, offsetof(struct ReportAvatarAckMessage, reportedPlayerId),
		(ASN_TAG_CLASS_UNIVERSAL | (2 << 2)),
		0,
		&asn_DEF_NonZeroId,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"reportedPlayerId"
	},
	{	ATF_NOFLAGS, 0, offsetof(struct ReportAvatarAckMessage, reportResult),
		(ASN_TAG_CLASS_UNIVERSAL | (10 << 2)),
		0,
		&asn_DEF_reportResult_3,
		0,	/* Defer constraints checking to the member type */
		0,	/* PER is not compiled, use -gen-PER */
		0,
		"reportResult"
	},
};
static ber_tlv_tag_t asn_DEF_ReportAvatarAckMessage_tags_1[] = {
	(ASN_TAG_CLASS_APPLICATION | (137 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_TYPE_tag2member_t asn_MAP_ReportAvatarAckMessage_tag2el_1[] = {
	{ (ASN_TAG_CLASS_UNIVERSAL | (2 << 2)), 0, 0, 0 }, /* reportedPlayerId at 759 */
	{ (ASN_TAG_CLASS_UNIVERSAL | (10 << 2)), 1, 0, 0 } /* reportResult at 761 */
};
static asn_SEQUENCE_specifics_t asn_SPC_ReportAvatarAckMessage_specs_1 = {
	sizeof(struct ReportAvatarAckMessage),
	offsetof(struct ReportAvatarAckMessage, _asn_ctx),
	asn_MAP_ReportAvatarAckMessage_tag2el_1,
	2,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* Start extensions */
	3	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_ReportAvatarAckMessage = {
	"ReportAvatarAckMessage",
	"ReportAvatarAckMessage",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	0, 0,	/* No PER support, use "-gen-PER" to enable */
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_ReportAvatarAckMessage_tags_1,
	sizeof(asn_DEF_ReportAvatarAckMessage_tags_1)
	/sizeof(asn_DEF_ReportAvatarAckMessage_tags_1[0]) - 1, /* 1 */
	asn_DEF_ReportAvatarAckMessage_tags_1,	/* Same as above */
	sizeof(asn_DEF_ReportAvatarAckMessage_tags_1)
	/sizeof(asn_DEF_ReportAvatarAckMessage_tags_1[0]), /* 2 */
	0,	/* No PER visible constraints */
	asn_MBR_ReportAvatarAckMessage_1,
	2,	/* Elements count */
	&asn_SPC_ReportAvatarAckMessage_specs_1	/* Additional specs */
};

