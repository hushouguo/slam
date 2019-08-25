/*
 * \file: protocol.h
 * \brief: Created by hushouguo at 15:19:02 Jul 29 2019
 */
 
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unordered_map>

//luajit-2.0.5
#include "lua.hpp"

// google protobuf-3.6.1
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/message.h>
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

using namespace google::protobuf;
using namespace google::protobuf::compiler;

#define SafeFree(P)						do { if(P) { ::free((void*)P); (P) = nullptr; } } while(0)
#define SafeDelete(P)					do { if(P) { delete (P); (P) = nullptr; } } while(0)
#define SafeClose(S)					do { if(S > 0) { ::close(S); S = -1; } } while(0)

#define ENABLE_DEBUG					1
#if ENABLE_DEBUG
#define Debug(MESSAGE, ...)	fprintf(stdout, "Debug:" MESSAGE "\n", ##__VA_ARGS__); fflush(stdout)
#else
#define Debug(MESSAGE, ...)
#endif

#define Trace(MESSAGE, ...)	fprintf(stderr, "Trace:" MESSAGE "\n", ##__VA_ARGS__); fflush(stderr)
#define Alarm(MESSAGE, ...)	fprintf(stderr, "Alarm:" MESSAGE "\n", ##__VA_ARGS__); fflush(stderr)
#define Error(MESSAGE, ...)	fprintf(stderr, "Error:" MESSAGE "\n", ##__VA_ARGS__); fflush(stderr)

#define CHECK_RETURN(RC, RESULT, MESSAGE, ...) do { if (!(RC)) { Error(MESSAGE, ##__VA_ARGS__); return RESULT; } } while (false)
#define CHECK_GOTO(RC, SYMBOL, MESSAGE, ...) do { if (!(RC)) { Error(MESSAGE, ##__VA_ARGS__); goto SYMBOL; } } while (false)
#define CHECK_BREAK(RC, MESSAGE, ...) if (!(RC)) { Error(MESSAGE, ##__VA_ARGS__); break; } }
#define CHECK_CONTINUE(RC, MESSAGE, ...) if (!(RC)) { Error(MESSAGE, ##__VA_ARGS__); continue; } }
#define CHECK_DEBUG(RC, MESSAGE, ...) if (!(RC)) { Debug(MESSAGE, ##__VA_ARGS__); }
#define CHECK_ALARM(RC, MESSAGE, ...) if (!(RC)) { Alarm(MESSAGE, ##__VA_ARGS__); }
#define CHECK_ERROR(RC, MESSAGE, ...) if (!(RC)) { Error(MESSAGE, ##__VA_ARGS__); }

// This function does not distinguish between a missing key and a key mapped
// to a NULL value.
template <class Collection>
	typename Collection::value_type::second_type
	FindOrNull(const Collection& collection, const typename Collection::value_type::first_type& key) 
	{
		typename Collection::const_iterator i = collection.find(key);
		return i == collection.end() ? typename Collection::value_type::second_type() : i->second;
	}

// Same as above, except takes non-const reference to collection.
template <class Collection>
	typename Collection::value_type::second_type
	FindOrNull(Collection& collection, const typename Collection::value_type::first_type& key) 
	{
		typename Collection::iterator i = collection.find(key);
		return i == collection.end() ? typename Collection::value_type::second_type() : i->second;
	}


// Returns true if and only if the given collection contains the given key.
template <class Collection, class Key>
	bool ContainsKey(const Collection& collection, const Key& key) {
		return collection.find(key) != collection.end();
	}

// Returns a pointer to the const value associated with the given key if it
// exists, or NULL otherwise.
template <class Collection>
	const typename Collection::value_type::second_type*
	FindPtrOrNull(const Collection& collection, const typename Collection::value_type::first_type& key) 
	{
		typename Collection::const_iterator i = collection.find(key);
		return i == collection.end() ? nullptr : &i->second;
	}

// Same as above but returns a pointer to the non-const value.
template <class Collection>
	typename Collection::value_type::second_type*
	FindPtrOrNull(Collection& collection, const typename Collection::value_type::first_type& key) 
	{
		typename Collection::iterator i = collection.find(key);
		return i == collection.end() ? nullptr : &i->second;
	}

#include "MessageBuilder.h"
#include "MessageParser.h"
#include "luaT_message_parser.h"

#endif
