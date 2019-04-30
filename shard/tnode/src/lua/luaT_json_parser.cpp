/*
 * \file: luaT_json_parser.cpp
 * \brief: Created by hushouguo at Nov 06 2014 18:20:14
 */

#include "tnode.h"
#include "tools/ByteBuffer.h"
#include "lua/luaT.h"
#include "json/json_parser.h"
#include "lua/luaT_json_parser.h"

BEGIN_NAMESPACE_TNODE {

#define UTF8_ERROR 	-2
	typedef enum {
		JPS_NONE = 0,
		JPS_IN_ARRAY,
		JPS_IN_TABLE
	} json_paser_state;

	typedef struct luaT_json_parser_ctx {
		lua_State*					L;
		int                     	key_lvl;
		json_paser_state        	cur_state;
		std::stack<char>*			state_stack;
		int							array_idx;
		std::stack<char>*			array_stack;
	} luaT_json_parser_ctx_t;

	luaT_json_parser_ctx_t* luaT_json_parser_ctx_create(lua_State* _L) {
		luaT_json_parser_ctx_t* ctx = (luaT_json_parser_ctx_t*)malloc(sizeof(luaT_json_parser_ctx_t));
		ctx->L = _L;
		ctx->key_lvl = 0;
		ctx->cur_state = JPS_NONE;
		ctx->state_stack = new std::stack<char>();
		ctx->array_idx = 1;
		ctx->array_stack = new std::stack<char>();
		return ctx;
	}

	void luaT_json_parser_ctx_delete(luaT_json_parser_ctx_t* ctx) {
		SafeDelete(ctx->state_stack);
		SafeDelete(ctx->array_stack);
		SafeFree(ctx);
	}

	//parse lua userdata, function
	void _json_parser_special_type(lua_State* L, const char* value, size_t len) {
		struct {
			int type;
			const char* name;
			size_t len;
		} lua_types[] = {
			//	{ LUA_TUSERDATA, "userdata", 8},
			{ LUA_TFUNCTION, "function", 8},
		};

		int type = LUA_TSTRING;
		for (auto& i : lua_types) {
			if (len < i.len) { continue; }
			if (memcmp(value, i.name, i.len) == 0) {
				type = i.type;
				break;
			}/* compare 8 bytes */
		}

		switch (type) {
			case LUA_TFUNCTION:	{
					char* p = (char*)strchr(value, ':');
					CHECK_BREAK(p, "illegal function string:%s", value);
					u64 address = strtoull(p + 1, nullptr, 16);
					CHECK_BREAK(address != 0 && address != ULONG_MAX, "illegal function address:%lu, %s", address, value);
					lua_pushcfunction(L, (lua_CFunction)address);
					lua_settable(L, -3);
					return;
				}
				break;

			case LUA_TUSERDATA:
			case LUA_TSTRING:
			default: break;
		}

		lua_pushlstring(L, value, len);
		lua_settable(L, -3);
	}

	int _json_parser_cb(void* ctx, int type, const JSON_value* value) {
		int ret = 1;
		luaT_json_parser_ctx_t* jpctx = (luaT_json_parser_ctx_t*)ctx;

		//Trace("begin state_stack size:%ld, array_stack size:%ld, type:%d", jpctx->state_stack->size(), jpctx->array_stack->size(), type);

		switch(type) {
			case JSON_T_ARRAY_BEGIN:
				if (jpctx->cur_state == JPS_IN_ARRAY) {
					lua_pushinteger(jpctx->L, jpctx->array_idx++);
				}

				lua_newtable(jpctx->L);
				if (jpctx->state_stack->empty()) {
					/* first array, root object */
				}
				jpctx->state_stack->push(jpctx->cur_state);
				jpctx->cur_state = JPS_IN_ARRAY;

				jpctx->array_stack->push(jpctx->array_idx);
				jpctx->array_idx = 1;			
				break;

			case JSON_T_ARRAY_END:
				jpctx->cur_state = (json_paser_state)jpctx->state_stack->top(); jpctx->state_stack->pop();
				jpctx->array_idx = jpctx->array_stack->top(); jpctx->array_stack->pop();
				if (jpctx->cur_state == JPS_IN_TABLE) {
					if (jpctx->key_lvl > 0) {
						lua_settable(jpctx->L, -3);
						--jpctx->key_lvl;
					}
					else {
						ret = 0;
						Error << "json object value without key";
					}
				}
				else if (jpctx->cur_state == JPS_IN_ARRAY) {
					lua_settable(jpctx->L, -3);
				}
				else {
					/* lua_pop(jpctx->L, 1); */
					/* root object, stay in stack, don't pop */
				}
				break;

			case JSON_T_OBJECT_BEGIN:
				if (jpctx->cur_state == JPS_IN_ARRAY) {
					lua_pushinteger(jpctx->L, jpctx->array_idx++);
				}

				lua_newtable(jpctx->L);
				if (jpctx->state_stack->empty()) {
					/* first table, root object */
				}
				jpctx->state_stack->push(jpctx->cur_state);
				jpctx->cur_state = JPS_IN_TABLE;
				break;

			case JSON_T_OBJECT_END:
				jpctx->cur_state = (json_paser_state)jpctx->state_stack->top(); jpctx->state_stack->pop();
				if (jpctx->cur_state == JPS_IN_TABLE) {
					if(jpctx->key_lvl > 0) {
						lua_settable(jpctx->L, -3);
						--jpctx->key_lvl;
					}
					else {
						ret = 0;
						Error << "json object value without key";
					}
				}
				else if (jpctx->cur_state == JPS_IN_ARRAY) {
					lua_settable(jpctx->L, -3);
				}
				else if (jpctx->cur_state == JPS_NONE) {
					/* root object, stay in stack */
				}
				else {
					/* lua_pop(jpctx->L, 1); */
				}
				break;

			case JSON_T_INTEGER:
				if (jpctx->cur_state == JPS_IN_TABLE) {
					if (jpctx->key_lvl > 0) {
						lua_pushinteger(jpctx->L, value->vu.integer_value);
						lua_settable(jpctx->L, -3);
						--jpctx->key_lvl;
					}
					else {
						ret = 0;
						Error << "json object value without key";
					}
				}
				else if (jpctx->cur_state == JPS_IN_ARRAY) {
					lua_pushinteger(jpctx->L, jpctx->array_idx++);
					lua_pushinteger(jpctx->L, value->vu.integer_value);
					lua_settable(jpctx->L, -3);
				}
				else {
					ret = 0;
					Error << "json no root object: " << __LINE__;
				}
				break;

			case JSON_T_FLOAT:
				if (jpctx->cur_state == JPS_IN_TABLE) {
					if (jpctx->key_lvl > 0) {
						lua_pushnumber(jpctx->L, value->vu.float_value);
						lua_settable(jpctx->L, -3);
						--jpctx->key_lvl;
					}
					else {
						ret = 0;
						Error << "json object value without key";
					}
				}
				else if (jpctx->cur_state == JPS_IN_ARRAY) {
					lua_pushinteger(jpctx->L, jpctx->array_idx++);
					lua_pushnumber(jpctx->L, value->vu.float_value);
					lua_settable(jpctx->L, -3);
				}
				else {
					ret = 0;
					Error << "json no root object: " << __LINE__;
				}
				break;

			case JSON_T_NULL:
				if (jpctx->cur_state == JPS_IN_TABLE) {
					if (jpctx->key_lvl > 0) {
						lua_pushnil(jpctx->L);
						lua_settable(jpctx->L, -3);
						--jpctx->key_lvl;
					}
					else {
						ret = 0;
						Error << "json object value without key";
					}
				}
				else if (jpctx->cur_state == JPS_IN_ARRAY) {
					lua_pushinteger(jpctx->L, jpctx->array_idx++);
					lua_pushnil(jpctx->L);
					lua_settable(jpctx->L, -3);
				}
				else {
					ret = 0;
					Error << "json no root object:" << __LINE__;
				}
				break;

			case JSON_T_TRUE:
				if (jpctx->cur_state == JPS_IN_TABLE) {
					if (jpctx->key_lvl > 0) {
						lua_pushboolean(jpctx->L, true);
						lua_settable(jpctx->L, -3);
						--jpctx->key_lvl;
					}
					else {
						ret = 0;
						Error << "json object value without key";
					}
				}
				else if (jpctx->cur_state == JPS_IN_ARRAY) {
					lua_pushinteger(jpctx->L, jpctx->array_idx++);
					lua_pushboolean(jpctx->L, true);
					lua_settable(jpctx->L, -3);
				}
				else {
					ret = 0;
					Error << "json no root object: " << __LINE__;
				}
				break;

			case JSON_T_FALSE:
				if (jpctx->cur_state == JPS_IN_TABLE) {
					if (jpctx->key_lvl > 0) {
						lua_pushboolean(jpctx->L, false);
						lua_settable(jpctx->L, -3);
						--jpctx->key_lvl;
					}
					else {
						ret = 0;
						Error << "json object value without key";
					}
				}
				else if (jpctx->cur_state == JPS_IN_ARRAY) {
					lua_pushinteger(jpctx->L, jpctx->array_idx++);
					lua_pushboolean(jpctx->L, false);
					lua_settable(jpctx->L, -3);
				}
				else {
					ret = 0;
					Error << "json no root object: " << __LINE__;
				}
				break;

			case JSON_T_STRING:
				if (jpctx->cur_state == JPS_IN_TABLE) {
					if (jpctx->key_lvl > 0) {
						//lua_pushlstring(jpctx->L, value->vu.str.value, value->vu.str.length);
						//lua_settable(jpctx->L, -3);
						_json_parser_special_type(jpctx->L, value->vu.str.value, value->vu.str.length);

						--jpctx->key_lvl;
					}
					else {
						ret = 0;
						Error << "json object value without key";
					}
				}
				else if (jpctx->cur_state == JPS_IN_ARRAY) {
					lua_pushinteger(jpctx->L, jpctx->array_idx++);

					//lua_pushlstring(jpctx->L, value->vu.str.value, value->vu.str.length);
					//lua_settable(jpctx->L, -3);
					_json_parser_special_type(jpctx->L, value->vu.str.value, value->vu.str.length);
				}
				else {
					ret = 0;
					Error << "json no root object: " << __LINE__;
				}
				break;

			case JSON_T_KEY:
				lua_pushlstring(jpctx->L, value->vu.str.value, value->vu.str.length);
				++jpctx->key_lvl;
				break;
		}
		//Debug("end state_stack size:%ld, array_stack size:%ld, ret:%d", jpctx->state_stack->size(), jpctx->array_stack->size(), ret);
		return ret;
	}

	bool luaT_json_parser_decode(lua_State* L, const char* jsonstr, unsigned int len) {
		unsigned int i;
		int oldtop = lua_gettop(L);
		luaT_json_parser_ctx_t* ctx = luaT_json_parser_ctx_create(L);
		JSON_config config;
		struct JSON_parser_struct* jc = NULL;

		init_JSON_config(&config);

		config.depth                  = -1;
		config.callback               = &_json_parser_cb;
		config.callback_ctx           = ctx;
		config.allow_comments         = 0;
		config.handle_floats_manually = 0;

		/* setlocale(LC_ALL, argv[1]) */

		jc = new_JSON_parser(&config);
		for(i = 0; i < len; ++i) {
			if(jsonstr[i] <= 0) {
				break;
			}

			if (!JSON_parser_char(jc, jsonstr[i])) {
				Error("JSON_parser_char: syntax error, byte: %d, %s", i, jsonstr);
				delete_JSON_parser(jc);
				luaT_json_parser_ctx_delete(ctx);
				lua_settop(L, oldtop);
				return false;
			}
		}

		if (!JSON_parser_done(jc)) {
			Error << "JSON_parser_end: syntax error: " << jsonstr;
			delete_JSON_parser(jc);
			luaT_json_parser_ctx_delete(ctx);
			lua_settop(L, oldtop);
			return false;
		}

		delete_JSON_parser(jc);
		luaT_json_parser_ctx_delete(ctx);

		return true;
	}

	int json_utf8_get(const char* buf, int idx) {
		return buf[idx] & 0xFF;
	}
	/*
	 *	Get the 6-bit payload of the next continuation byte.
	 *	Return UTF8_ERROR if it is not a continuation byte.
	 */
	int json_utf8_cont(const char* buf, int idx) {
		int c = json_utf8_get(buf, idx);
		return ((c & 0xC0) == 0x80) ? (c & 0x3F) : UTF8_ERROR;
	}

	bool json_utf8_decode_single(const char* buf, int buflen_left, unsigned int* o_char, int* o_len) {
		int c;
		if(buflen_left < 1) {
			return false;
		}
		c = json_utf8_get(buf, 0);  /* the first byte of the character */

		/* Zero continuation (0 to 127) */
		if ((c & 0x80) == 0) {
			*o_char = c;
			*o_len = 1;
			return true;
		}

		/* One continuation (128 to 2047) */
		if ((c & 0xE0) == 0xC0) {
			int c1;
			if(buflen_left < 2) {
				return false;
			}
			c1 = json_utf8_cont(buf, 1);
			if (c1 < 0) {
				return false;
			}
			*o_char = ((c & 0x1F) << 6) | c1;
			*o_len = 2;
			return *o_char >= 128 ? true : false;
		}

		/* Two continuation (2048 to 55295 and 57344 to 65535) */
		if ((c & 0xF0) == 0xE0) {
			int c1, c2;
			if(buflen_left < 3) {
				return false;
			}
			c1 = json_utf8_cont(buf, 1);
			c2 = json_utf8_cont(buf, 2);
			if (c1 < 0 || c2 < 0) {
				return false;
			}
			*o_char = ((c & 0x0F) << 12) | (c1 << 6) | c2;
			*o_len = 3;
			return *o_char >= 2048 && (*o_char < 55296 || *o_char > 57343) ? true : false;
		}

		/* Three continuation (65536 to 1114111) */
		if ((c & 0xF8) == 0xF0) {
			int c1, c2, c3;
			if (buflen_left < 4) {
				return false;
			}
			c1 = json_utf8_cont(buf, 1);
			c2 = json_utf8_cont(buf, 2);
			c3 = json_utf8_cont(buf, 3);
			if (c1 < 0 || c2 < 0 || c3 < 0) {
				return false;
			}
			*o_char = ((c & 0x0F) << 18) | (c1 << 12) | (c2 << 6) | c3;
			*o_len = 4;
			return *o_char >= 65536 && *o_char <= 1114111 ? true : false;
		}
		return false;
	}

	void create_json_escapes(const char* s, ByteBuffer* byteBuffer, int srclen) {
		const char* b = s;
		int srclen_left = srclen < 0 ? strlen(s) : srclen;
		byteBuffer->append('"');
		while (b && *b) {
			/*
			 * This assumes an ASCII superset. But so does everything in PTree.
			 * We escape everything outside ASCII, because this code can't
			 * handle high unicode characters.
			 */
			if (
					*b == 0x20 
					|| 
					*b == 0x21 
					|| 
					(*b >= 0x23 && *b <= 0x2E) 
					||
					(*b >= 0x30 && *b <= 0x5B) 
					|| 
					(*b >= 0x5D) /* && *b <= 0xFF */
			   )
			{
				byteBuffer->append(*b);
			}
			else if (*b == '\b') { byteBuffer->append('\\'); byteBuffer->append('b'); }
			else if (*b == '\f') { byteBuffer->append('\\'); byteBuffer->append('f'); }
			else if (*b == '\n') { byteBuffer->append('\\'); byteBuffer->append('n'); }
			else if (*b == '\r') { byteBuffer->append('\\'); byteBuffer->append('r'); }
			else if (*b == '/')  { byteBuffer->append('\\'); byteBuffer->append('/'); }
			else if (*b == '"')  { byteBuffer->append('\\'); byteBuffer->append('"'); }
			else if (*b == '\\') { byteBuffer->append('\\'); byteBuffer->append('\\');}
			else
			{
				unsigned int o_char = 0;
				int o_len = 0;
				unsigned int u = *b;
				unsigned int u_n = 0;
				const char* hexdigits = NULL;

				if (json_utf8_decode_single(b, srclen_left, &o_char, &o_len)) {
					u = o_char & 0x0000ffff;
					u_n = o_char >> 16;

					b += (o_len-1);
					srclen_left -= (o_len-1);
				}

				hexdigits = "0123456789ABCDEF";
				if (u_n != 0) {
					int d1 = u_n / 4096; u_n %= 4096;
					int d2 = u_n / 256; u_n %= 256;
					int d3 = u_n / 16; u_n %= 16;
					int d4 = u_n;
					byteBuffer->append('\\'); byteBuffer->append('u');
					byteBuffer->append(hexdigits[d1]); byteBuffer->append(hexdigits[d2]);
					byteBuffer->append(hexdigits[d3]); byteBuffer->append(hexdigits[d4]);
				}

				{
					int d1 = u / 4096; u %= 4096;
					int d2 = u / 256; u %= 256;
					int d3 = u / 16; u %= 16;
					int d4 = u;
					byteBuffer->append('\\'); byteBuffer->append('u');
					byteBuffer->append(hexdigits[d1]); byteBuffer->append(hexdigits[d2]);
					byteBuffer->append(hexdigits[d3]); byteBuffer->append(hexdigits[d4]);
				}
			}
			++b;
			--srclen_left;
		}
		byteBuffer->append('"');
		//byteBuffer->append('\0');
	}

	bool luaT_json_parser_dump_object(lua_State* L, ByteBuffer* byteBuffer) {
		switch (lua_type(L, -1)) {
			case LUA_TNIL:
				byteBuffer->append((Byte*)"null", 4);
				break;

			case LUA_TNUMBER: {
					size_t len = 0;
					const char* s = lua_tolstring(L, -1, &len);/* compatible lua_Integer & lua_Number */
					byteBuffer->append((Byte*)s, len);
				}
				break;

			case LUA_TBOOLEAN: {
					int b = lua_toboolean(L, -1);
					if (b) {
						byteBuffer->append((Byte*)"true", 4);
					}
					else {
						byteBuffer->append((Byte*)"false", 5);
					}
				}
				break;

			case LUA_TSTRING: {
					size_t len = 0;
					const char* s = lua_tolstring(L, -1, &len);
					create_json_escapes(s, byteBuffer, len);
				}
				break;

			case LUA_TTABLE: {
					int idx = 0;
					byteBuffer->append('{');

					lua_pushnil(L);
					while (lua_next(L, -2)) {
						size_t len = 0;
						const char* key = NULL;

						/* -1 is the value and -2 is the key */
						if (idx++ > 0) {
							byteBuffer->append(',');
						}

						/* fetch key, don't lua_tostring(L, -1); */
						lua_pushvalue(L, -2);
						key = lua_tolstring(L, -1, &len);
						lua_pop(L, 1);

						create_json_escapes(key, byteBuffer, len);

						byteBuffer->append(':');

						if (!luaT_json_parser_dump_object(L, byteBuffer)) { return false; }

						lua_pop(L, 1); /* removes 'value'; keeps 'key' for next iteration */
					}
					byteBuffer->append('}');
				}
				break;

			case LUA_TFUNCTION: {
					lua_CFunction func = lua_tocfunction(L, -1);
					char buffer[128];
					snprintf(buffer, sizeof(buffer), "function:%p", func);
					create_json_escapes(buffer, byteBuffer, strlen(buffer));
				}
				break;

			case LUA_TUSERDATA:
			case LUA_TTHREAD:
			case LUA_TLIGHTUSERDATA:
			default: 
				Error << "Unable dump object type: " << lua_typename(L, lua_type(L, -1));
				return false;
		}
		return true;
	}

	bool luaT_json_parser_encode(lua_State* L, ByteBuffer* byteBuffer) {
		int oldtop = lua_gettop(L);
		bool rc = luaT_json_parser_dump_object(L, byteBuffer);
		lua_settop(L, oldtop);
		if (rc) {
			byteBuffer->append('\0');
		}
		return rc;
	}
}
