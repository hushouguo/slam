// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: msgid.proto

#ifndef PROTOBUF_INCLUDED_msgid_2eproto
#define PROTOBUF_INCLUDED_msgid_2eproto

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3006001
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
// @@protoc_insertion_point(includes)
#define PROTOBUF_INTERNAL_EXPORT_protobuf_msgid_2eproto 

namespace protobuf_msgid_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[1];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
}  // namespace protobuf_msgid_2eproto
namespace protocol {
}  // namespace protocol
namespace protocol {

enum MSGID {
  MSGID_BASE = 0,
  MSGID_HEARTBEAT = 1,
  MSGID_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  MSGID_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool MSGID_IsValid(int value);
const MSGID MSGID_MIN = MSGID_BASE;
const MSGID MSGID_MAX = MSGID_HEARTBEAT;
const int MSGID_ARRAYSIZE = MSGID_MAX + 1;

const ::google::protobuf::EnumDescriptor* MSGID_descriptor();
inline const ::std::string& MSGID_Name(MSGID value) {
  return ::google::protobuf::internal::NameOfEnum(
    MSGID_descriptor(), value);
}
inline bool MSGID_Parse(
    const ::std::string& name, MSGID* value) {
  return ::google::protobuf::internal::ParseNamedEnum<MSGID>(
    MSGID_descriptor(), name, value);
}
// ===================================================================


// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace protocol

namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::protocol::MSGID> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::protocol::MSGID>() {
  return ::protocol::MSGID_descriptor();
}

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_INCLUDED_msgid_2eproto