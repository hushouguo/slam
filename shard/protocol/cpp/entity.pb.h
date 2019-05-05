// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: entity.proto

#ifndef PROTOBUF_INCLUDED_entity_2eproto
#define PROTOBUF_INCLUDED_entity_2eproto

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
#include <google/protobuf/map.h>  // IWYU pragma: export
#include <google/protobuf/map_entry.h>
#include <google/protobuf/map_field_inl.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#define PROTOBUF_INTERNAL_EXPORT_protobuf_entity_2eproto 

namespace protobuf_entity_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[3];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
}  // namespace protobuf_entity_2eproto
namespace protocol {
class Entity;
class EntityDefaultTypeInternal;
extern EntityDefaultTypeInternal _Entity_default_instance_;
class Entity_ValuesEntry_DoNotUse;
class Entity_ValuesEntry_DoNotUseDefaultTypeInternal;
extern Entity_ValuesEntry_DoNotUseDefaultTypeInternal _Entity_ValuesEntry_DoNotUse_default_instance_;
class Value;
class ValueDefaultTypeInternal;
extern ValueDefaultTypeInternal _Value_default_instance_;
}  // namespace protocol
namespace google {
namespace protobuf {
template<> ::protocol::Entity* Arena::CreateMaybeMessage<::protocol::Entity>(Arena*);
template<> ::protocol::Entity_ValuesEntry_DoNotUse* Arena::CreateMaybeMessage<::protocol::Entity_ValuesEntry_DoNotUse>(Arena*);
template<> ::protocol::Value* Arena::CreateMaybeMessage<::protocol::Value>(Arena*);
}  // namespace protobuf
}  // namespace google
namespace protocol {

enum ValueType {
  valuetype_nil = 0,
  valuetype_int64 = 1,
  valuetype_string = 2,
  valuetype_float = 3,
  valuetype_bool = 4,
  valuetype_bytes = 5,
  ValueType_INT_MIN_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32min,
  ValueType_INT_MAX_SENTINEL_DO_NOT_USE_ = ::google::protobuf::kint32max
};
bool ValueType_IsValid(int value);
const ValueType ValueType_MIN = valuetype_nil;
const ValueType ValueType_MAX = valuetype_bytes;
const int ValueType_ARRAYSIZE = ValueType_MAX + 1;

const ::google::protobuf::EnumDescriptor* ValueType_descriptor();
inline const ::std::string& ValueType_Name(ValueType value) {
  return ::google::protobuf::internal::NameOfEnum(
    ValueType_descriptor(), value);
}
inline bool ValueType_Parse(
    const ::std::string& name, ValueType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<ValueType>(
    ValueType_descriptor(), name, value);
}
// ===================================================================

class Value : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:protocol.Value) */ {
 public:
  Value();
  virtual ~Value();

  Value(const Value& from);

  inline Value& operator=(const Value& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Value(Value&& from) noexcept
    : Value() {
    *this = ::std::move(from);
  }

  inline Value& operator=(Value&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Value& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Value* internal_default_instance() {
    return reinterpret_cast<const Value*>(
               &_Value_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(Value* other);
  friend void swap(Value& a, Value& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Value* New() const final {
    return CreateMaybeMessage<Value>(NULL);
  }

  Value* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<Value>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const Value& from);
  void MergeFrom(const Value& from);
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
  void InternalSwap(Value* other);
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

  // string value_string = 3;
  void clear_value_string();
  static const int kValueStringFieldNumber = 3;
  const ::std::string& value_string() const;
  void set_value_string(const ::std::string& value);
  #if LANG_CXX11
  void set_value_string(::std::string&& value);
  #endif
  void set_value_string(const char* value);
  void set_value_string(const char* value, size_t size);
  ::std::string* mutable_value_string();
  ::std::string* release_value_string();
  void set_allocated_value_string(::std::string* value_string);

  // bytes value_bytes = 6;
  void clear_value_bytes();
  static const int kValueBytesFieldNumber = 6;
  const ::std::string& value_bytes() const;
  void set_value_bytes(const ::std::string& value);
  #if LANG_CXX11
  void set_value_bytes(::std::string&& value);
  #endif
  void set_value_bytes(const char* value);
  void set_value_bytes(const void* value, size_t size);
  ::std::string* mutable_value_bytes();
  ::std::string* release_value_bytes();
  void set_allocated_value_bytes(::std::string* value_bytes);

  // int64 value_int64 = 2;
  void clear_value_int64();
  static const int kValueInt64FieldNumber = 2;
  ::google::protobuf::int64 value_int64() const;
  void set_value_int64(::google::protobuf::int64 value);

  // .protocol.ValueType type = 1;
  void clear_type();
  static const int kTypeFieldNumber = 1;
  ::protocol::ValueType type() const;
  void set_type(::protocol::ValueType value);

  // float value_float = 4;
  void clear_value_float();
  static const int kValueFloatFieldNumber = 4;
  float value_float() const;
  void set_value_float(float value);

  // bool value_bool = 5;
  void clear_value_bool();
  static const int kValueBoolFieldNumber = 5;
  bool value_bool() const;
  void set_value_bool(bool value);

  // @@protoc_insertion_point(class_scope:protocol.Value)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr value_string_;
  ::google::protobuf::internal::ArenaStringPtr value_bytes_;
  ::google::protobuf::int64 value_int64_;
  int type_;
  float value_float_;
  bool value_bool_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_entity_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class Entity_ValuesEntry_DoNotUse : public ::google::protobuf::internal::MapEntry<Entity_ValuesEntry_DoNotUse, 
    ::std::string, ::protocol::Value,
    ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
    ::google::protobuf::internal::WireFormatLite::TYPE_MESSAGE,
    0 > {
public:
  typedef ::google::protobuf::internal::MapEntry<Entity_ValuesEntry_DoNotUse, 
    ::std::string, ::protocol::Value,
    ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
    ::google::protobuf::internal::WireFormatLite::TYPE_MESSAGE,
    0 > SuperType;
  Entity_ValuesEntry_DoNotUse();
  Entity_ValuesEntry_DoNotUse(::google::protobuf::Arena* arena);
  void MergeFrom(const Entity_ValuesEntry_DoNotUse& other);
  static const Entity_ValuesEntry_DoNotUse* internal_default_instance() { return reinterpret_cast<const Entity_ValuesEntry_DoNotUse*>(&_Entity_ValuesEntry_DoNotUse_default_instance_); }
  void MergeFrom(const ::google::protobuf::Message& other) final;
  ::google::protobuf::Metadata GetMetadata() const;
};

// -------------------------------------------------------------------

class Entity : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:protocol.Entity) */ {
 public:
  Entity();
  virtual ~Entity();

  Entity(const Entity& from);

  inline Entity& operator=(const Entity& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Entity(Entity&& from) noexcept
    : Entity() {
    *this = ::std::move(from);
  }

  inline Entity& operator=(Entity&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Entity& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Entity* internal_default_instance() {
    return reinterpret_cast<const Entity*>(
               &_Entity_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    2;

  void Swap(Entity* other);
  friend void swap(Entity& a, Entity& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Entity* New() const final {
    return CreateMaybeMessage<Entity>(NULL);
  }

  Entity* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<Entity>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const Entity& from);
  void MergeFrom(const Entity& from);
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
  void InternalSwap(Entity* other);
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

  // map<string, .protocol.Value> values = 2;
  int values_size() const;
  void clear_values();
  static const int kValuesFieldNumber = 2;
  const ::google::protobuf::Map< ::std::string, ::protocol::Value >&
      values() const;
  ::google::protobuf::Map< ::std::string, ::protocol::Value >*
      mutable_values();

  // uint64 id = 1;
  void clear_id();
  static const int kIdFieldNumber = 1;
  ::google::protobuf::uint64 id() const;
  void set_id(::google::protobuf::uint64 value);

  // @@protoc_insertion_point(class_scope:protocol.Entity)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::MapField<
      Entity_ValuesEntry_DoNotUse,
      ::std::string, ::protocol::Value,
      ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
      ::google::protobuf::internal::WireFormatLite::TYPE_MESSAGE,
      0 > values_;
  ::google::protobuf::uint64 id_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_entity_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Value

// .protocol.ValueType type = 1;
inline void Value::clear_type() {
  type_ = 0;
}
inline ::protocol::ValueType Value::type() const {
  // @@protoc_insertion_point(field_get:protocol.Value.type)
  return static_cast< ::protocol::ValueType >(type_);
}
inline void Value::set_type(::protocol::ValueType value) {
  
  type_ = value;
  // @@protoc_insertion_point(field_set:protocol.Value.type)
}

// int64 value_int64 = 2;
inline void Value::clear_value_int64() {
  value_int64_ = GOOGLE_LONGLONG(0);
}
inline ::google::protobuf::int64 Value::value_int64() const {
  // @@protoc_insertion_point(field_get:protocol.Value.value_int64)
  return value_int64_;
}
inline void Value::set_value_int64(::google::protobuf::int64 value) {
  
  value_int64_ = value;
  // @@protoc_insertion_point(field_set:protocol.Value.value_int64)
}

// string value_string = 3;
inline void Value::clear_value_string() {
  value_string_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Value::value_string() const {
  // @@protoc_insertion_point(field_get:protocol.Value.value_string)
  return value_string_.GetNoArena();
}
inline void Value::set_value_string(const ::std::string& value) {
  
  value_string_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:protocol.Value.value_string)
}
#if LANG_CXX11
inline void Value::set_value_string(::std::string&& value) {
  
  value_string_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:protocol.Value.value_string)
}
#endif
inline void Value::set_value_string(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  value_string_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:protocol.Value.value_string)
}
inline void Value::set_value_string(const char* value, size_t size) {
  
  value_string_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:protocol.Value.value_string)
}
inline ::std::string* Value::mutable_value_string() {
  
  // @@protoc_insertion_point(field_mutable:protocol.Value.value_string)
  return value_string_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Value::release_value_string() {
  // @@protoc_insertion_point(field_release:protocol.Value.value_string)
  
  return value_string_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Value::set_allocated_value_string(::std::string* value_string) {
  if (value_string != NULL) {
    
  } else {
    
  }
  value_string_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value_string);
  // @@protoc_insertion_point(field_set_allocated:protocol.Value.value_string)
}

// float value_float = 4;
inline void Value::clear_value_float() {
  value_float_ = 0;
}
inline float Value::value_float() const {
  // @@protoc_insertion_point(field_get:protocol.Value.value_float)
  return value_float_;
}
inline void Value::set_value_float(float value) {
  
  value_float_ = value;
  // @@protoc_insertion_point(field_set:protocol.Value.value_float)
}

// bool value_bool = 5;
inline void Value::clear_value_bool() {
  value_bool_ = false;
}
inline bool Value::value_bool() const {
  // @@protoc_insertion_point(field_get:protocol.Value.value_bool)
  return value_bool_;
}
inline void Value::set_value_bool(bool value) {
  
  value_bool_ = value;
  // @@protoc_insertion_point(field_set:protocol.Value.value_bool)
}

// bytes value_bytes = 6;
inline void Value::clear_value_bytes() {
  value_bytes_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Value::value_bytes() const {
  // @@protoc_insertion_point(field_get:protocol.Value.value_bytes)
  return value_bytes_.GetNoArena();
}
inline void Value::set_value_bytes(const ::std::string& value) {
  
  value_bytes_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:protocol.Value.value_bytes)
}
#if LANG_CXX11
inline void Value::set_value_bytes(::std::string&& value) {
  
  value_bytes_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:protocol.Value.value_bytes)
}
#endif
inline void Value::set_value_bytes(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  value_bytes_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:protocol.Value.value_bytes)
}
inline void Value::set_value_bytes(const void* value, size_t size) {
  
  value_bytes_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:protocol.Value.value_bytes)
}
inline ::std::string* Value::mutable_value_bytes() {
  
  // @@protoc_insertion_point(field_mutable:protocol.Value.value_bytes)
  return value_bytes_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Value::release_value_bytes() {
  // @@protoc_insertion_point(field_release:protocol.Value.value_bytes)
  
  return value_bytes_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Value::set_allocated_value_bytes(::std::string* value_bytes) {
  if (value_bytes != NULL) {
    
  } else {
    
  }
  value_bytes_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value_bytes);
  // @@protoc_insertion_point(field_set_allocated:protocol.Value.value_bytes)
}

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// Entity

// uint64 id = 1;
inline void Entity::clear_id() {
  id_ = GOOGLE_ULONGLONG(0);
}
inline ::google::protobuf::uint64 Entity::id() const {
  // @@protoc_insertion_point(field_get:protocol.Entity.id)
  return id_;
}
inline void Entity::set_id(::google::protobuf::uint64 value) {
  
  id_ = value;
  // @@protoc_insertion_point(field_set:protocol.Entity.id)
}

// map<string, .protocol.Value> values = 2;
inline int Entity::values_size() const {
  return values_.size();
}
inline void Entity::clear_values() {
  values_.Clear();
}
inline const ::google::protobuf::Map< ::std::string, ::protocol::Value >&
Entity::values() const {
  // @@protoc_insertion_point(field_map:protocol.Entity.values)
  return values_.GetMap();
}
inline ::google::protobuf::Map< ::std::string, ::protocol::Value >*
Entity::mutable_values() {
  // @@protoc_insertion_point(field_mutable_map:protocol.Entity.values)
  return values_.MutableMap();
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace protocol

namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::protocol::ValueType> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::protocol::ValueType>() {
  return ::protocol::ValueType_descriptor();
}

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_INCLUDED_entity_2eproto
