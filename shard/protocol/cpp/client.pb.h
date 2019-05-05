// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: client.proto

#ifndef PROTOBUF_INCLUDED_client_2eproto
#define PROTOBUF_INCLUDED_client_2eproto

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
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#define PROTOBUF_INTERNAL_EXPORT_protobuf_client_2eproto 

namespace protobuf_client_2eproto {
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
}  // namespace protobuf_client_2eproto
namespace protocol {
class Heartbeat;
class HeartbeatDefaultTypeInternal;
extern HeartbeatDefaultTypeInternal _Heartbeat_default_instance_;
}  // namespace protocol
namespace google {
namespace protobuf {
template<> ::protocol::Heartbeat* Arena::CreateMaybeMessage<::protocol::Heartbeat>(Arena*);
}  // namespace protobuf
}  // namespace google
namespace protocol {

// ===================================================================

class Heartbeat : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:protocol.Heartbeat) */ {
 public:
  Heartbeat();
  virtual ~Heartbeat();

  Heartbeat(const Heartbeat& from);

  inline Heartbeat& operator=(const Heartbeat& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Heartbeat(Heartbeat&& from) noexcept
    : Heartbeat() {
    *this = ::std::move(from);
  }

  inline Heartbeat& operator=(Heartbeat&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Heartbeat& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Heartbeat* internal_default_instance() {
    return reinterpret_cast<const Heartbeat*>(
               &_Heartbeat_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(Heartbeat* other);
  friend void swap(Heartbeat& a, Heartbeat& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Heartbeat* New() const final {
    return CreateMaybeMessage<Heartbeat>(NULL);
  }

  Heartbeat* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<Heartbeat>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const Heartbeat& from);
  void MergeFrom(const Heartbeat& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Heartbeat* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // uint64 milliseconds = 1;
  void clear_milliseconds();
  static const int kMillisecondsFieldNumber = 1;
  ::google::protobuf::uint64 milliseconds() const;
  void set_milliseconds(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:protocol.Heartbeat)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint64 milliseconds_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_client_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Heartbeat

// uint64 milliseconds = 1;
inline void Heartbeat::clear_milliseconds() {
  milliseconds_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Heartbeat::milliseconds() const {
  // @@protoc_insertion_point(field_get:protocol.Heartbeat.milliseconds)
  return milliseconds_;
}
inline void Heartbeat::set_milliseconds(::google::protobuf::uint64 value) {
  
  milliseconds_ = value;
  // @@protoc_insertion_point(field_set:protocol.Heartbeat.milliseconds)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace protocol

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_INCLUDED_client_2eproto