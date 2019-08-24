/*
 * \file: slam_lua_db.c
 * \brief: Created by hushouguo at 13:33:19 Aug 22 2019
 */

#include "slam.h"

//
// key
static bool slam_lua_db_encode_key(lua_State* L, int t, slam_byte_buffer_t* byte_buffer) {
    size_t len = 0;
    const char* key = lua_tolstring(L, t, &len);
	log_trace("encode key: %s", key);
    slam_byte_buffer_write(byte_buffer, key, len);
    return true;
}

//
// false, 0, '', 1.23
static bool slam_lua_db_encode_value(lua_State* L, int t, slam_byte_buffer_t* byte_buffer) {
	log_trace("encode value: %d, %s", t, lua_tostring(L, t));
    if (lua_isnone(L, t) || lua_isnil(L, t)) {
        slam_byte_buffer_write(byte_buffer, "null", 4);
    }
    else if (lua_isboolean(L, t) || lua_isnumber(L, t)) {
        size_t len = 0;
        const char* value = lua_tolstring(L, t, &len);
        slam_byte_buffer_write(byte_buffer, value, len);
    }
    else if (lua_isstring(L, t)) {
        size_t len = 0;
        const char* value = lua_tolstring(L, t, &len);
        slam_byte_buffer_write(byte_buffer, "'", 1);
        slam_byte_buffer_write(byte_buffer, value, len);
        slam_byte_buffer_write(byte_buffer, "'", 1);
    }
	else if (lua_isfunction(L, t)) {
		//log_alarm("t: %d, type: %s", t, lua_typename(L, lua_type(L, t)));
	}
    else {
        CHECK_RETURN(false, false, "value is illegal type: %s", lua_typename(L, lua_type(L, t)));
    }
	log_trace("encode value: %d, %s end", t, lua_tostring(L, t));
    return true;
}

//
// Key = Value, ...
static bool slam_lua_db_encode_table(lua_State* L, int t, slam_byte_buffer_t* byte_buffer) {
	log_alarm("func: %s, t: %d, %s begin", __FUNCTION__, t, lua_typename(L, lua_type(L, t)));
    bool rc = false;
    /* table is in the stack at index 't' */
    lua_pushnil(L); /* first key */
    while(lua_next(L, t)) {
        if (rc) {
            slam_byte_buffer_write(byte_buffer, ",", 1);
        }
        /* -2 : key, -1 : value */
		log_trace("\tt: %d, %s, %s", -1, lua_typename(L, lua_type(L, -1)), lua_tostring(L, -1));
		log_trace("\tt: %d, %s, %s", -2, lua_typename(L, lua_type(L, -2)), lua_tostring(L, -2));

        rc = slam_lua_db_encode_key(L, -2, byte_buffer);
        CHECK_RETURN(rc, false, "encode key error");
        slam_byte_buffer_write(byte_buffer, "=", 1);
        rc = slam_lua_db_encode_value(L, -1, byte_buffer);
        CHECK_RETURN(rc, false, "encode value error");
        lua_pop(L, 1); /* removes 'value'; keeps 'key' for next iteration */
    }
	log_alarm("func: %s, t: %d, %s end", __FUNCTION__, t, lua_typename(L, lua_type(L, t)));
    return true;
}


//
// decode db_result to lua table
static bool slam_lua_db_decode_result(lua_State* L, slam_db_result_t* result) {
    uint64_t i, row_number = 1, field_number = slam_db_result_field_number(result);
    MYSQL_FIELD* fields = slam_db_result_fetch_field(result);
    lua_newtable(L);
    while (true) {
        MYSQL_ROW row = slam_db_result_fetch_row(result);
        if (!row) {
            break;
        }
		lua_pushinteger(L, row_number++);
		lua_newtable(L);
        unsigned long* lengths = slam_db_result_fetch_length(result);
        for (i = 0; i < field_number; ++i) {
            lua_pushstring(L, fields[i].org_name); /* key */
            switch (fields[i].type) {
                case MYSQL_TYPE_TINY: case MYSQL_TYPE_SHORT: 
                case MYSQL_TYPE_INT24: case MYSQL_TYPE_BIT:                
                    lua_pushinteger(L, atoi(row[i]));
                    break;
                case MYSQL_TYPE_LONG: 
                    lua_pushinteger(L, atol(row[i])); 
                    break;
                case MYSQL_TYPE_LONGLONG: 
                    lua_pushinteger(L, atoll(row[i])); 
                    break;
                case MYSQL_TYPE_DECIMAL: case MYSQL_TYPE_NEWDECIMAL:
                case MYSQL_TYPE_FLOAT: case MYSQL_TYPE_DOUBLE:
                    lua_pushnumber(L, atof(row[i]));
                    break;                    
                case MYSQL_TYPE_TIMESTAMP: case MYSQL_TYPE_DATE: case MYSQL_TYPE_TIME:
                case MYSQL_TYPE_DATETIME: case MYSQL_TYPE_YEAR:                 
                case MYSQL_TYPE_STRING: case MYSQL_TYPE_VAR_STRING:
                    lua_pushlstring(L, row[i], lengths[i]);
                    break;                    
                case MYSQL_TYPE_BLOB:
                    lua_pushlstring(L, row[i], lengths[i]);
                    break;                                     
                default: 
					log_error("ignore type: %d, table: %s, field: %s", fields[i].type, fields[i].org_table, fields[i].org_name);
					lua_pushnil(L);
					break;
            }
			lua_settable(L, -3);
        }
		lua_settable(L, -3);
    }
    return true;
}


//
// userdata newdb(host, username, password, port)
static int cc_newdb(lua_State* L) {
    int args = lua_gettop(L);
    
    CHECK_RETURN(args == 4, 0, "`%s` lack args:%d", __FUNCTION__, args);
    CHECK_RETURN(lua_isstring(L, -args), 0, "[%s]", lua_typename(L, lua_type(L, -args)));
    CHECK_RETURN(lua_isstring(L, -(args - 1)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 1))));
    CHECK_RETURN(lua_isstring(L, -(args - 2)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 2))));
    CHECK_RETURN(lua_isnumber(L, -(args - 3)), 0, "[%s]", lua_typename(L, lua_type(L, -(args - 3))));
    
    const char* host = lua_tostring(L, -args);
    const char* username = lua_tostring(L, -(args - 1));
    const char* password = lua_tostring(L, -(args - 2));
    int port = lua_tointeger(L, -(args - 3));
    const char* database = nullptr;

    //
    // create slam_db instance
    slam_db_t* db = slam_db_new();
    bool rc = slam_db_connect(db, host, username, password, port, database);
    if (!rc) {
        slam_db_delete(db);
        lua_pushnil(L);
        return 1;
    }
    
    //
    // create userdata and setup metatable into userdata
    slam_db_t** db_userdata = (slam_db_t **) lua_newuserdata(L, sizeof(slam_db_t*));
    *db_userdata = db;
    
    luaL_getmetatable(L, SLAM_LUA_METATABLE_DB_NAME);
    lua_setmetatable(L, -2);
    
    return 1;
}

static int __cc_db_gc(lua_State *L) {
    slam_db_t** db = (slam_db_t **) luaL_checkudata(L, 1, SLAM_LUA_METATABLE_DB_NAME);      
    luaL_argcheck(L, db != nullptr, 1, "invalid `db` userdata");
    if (db) {
        slam_db_delete(*db); // destroy db instance
    }
    return 0;
}

static int __cc_db_tostring(lua_State *L) {
    lua_pushstring(L, "db");
    return 1;
}

#if 0
//
// uint64_t db:insert_id()
static int cc_db_insert_id(lua_State* L) {    
    int args = lua_gettop(L);// this:userdata in bottom of lua stack
    CHECK_RETURN(args == 1, 0, "`%s` lack args:%d", __FUNCTION__, args);

    slam_db_t** db = (slam_db_t **) luaL_checkudata(L, 1, SLAM_LUA_METATABLE_DB_NAME);
    luaL_argcheck(L, db != nullptr, 1, "invalid `db` userdata");

    uint64_t insertid = slam_db_insert_id(*db);
    lua_pushinteger(L, insertid);
    return 1;
}
#endif

//
// bool db:execute(sql)
static int cc_db_execute(lua_State* L) {
    bool rc = false;
    int args = lua_gettop(L);// this:userdata in bottom of lua stack
    if (args != 2) {
        lua_pushboolean(L, rc);
        CHECK_RETURN(false, 1, "`%s` lack args: %d", __FUNCTION__, args);
    }

    slam_db_t** db = (slam_db_t **) luaL_checkudata(L, 1, SLAM_LUA_METATABLE_DB_NAME);
    if (!db) {
        lua_pushboolean(L, rc);
        CHECK_RETURN(false, 1, "invalid userdata: %s", SLAM_LUA_METATABLE_DB_NAME);
    }

    if (!lua_isstring(L, -(args - 1))) {
        lua_pushboolean(L, rc);
        CHECK_RETURN(false, 1, "args error: %s", lua_typename(L, lua_type(L, -(args - 1))));
    }
    
    const char* sql = lua_tostring(L, -(args - 1));
    rc = slam_db_execute(*db, sql);
    lua_pushboolean(L, rc);
    CHECK_RETURN(rc, 1, "execute sql: %s error", sql);
    return 1;
}

//
// [lua_table | nil] db:query(sql)
static int cc_db_query(lua_State* L) {
    int args = lua_gettop(L);// this:userdata in bottom of lua stack
    if (args != 2) {
        lua_pushnil(L);
        CHECK_RETURN(false, 1, "`%s` lack args: %d", __FUNCTION__, args);
    }

    slam_db_t** db = (slam_db_t **) luaL_checkudata(L, 1, SLAM_LUA_METATABLE_DB_NAME);
    if (!db) {
        lua_pushnil(L);
        CHECK_RETURN(false, 1, "invalid userdata: %s", SLAM_LUA_METATABLE_DB_NAME);
    }

    if (!lua_isstring(L, -(args - 1))) {
        lua_pushnil(L);
        CHECK_RETURN(false, 1, "args error: %s", lua_typename(L, lua_type(L, -(args - 1))));
    }

    const char* sql = lua_tostring(L, -(args - 1));
    slam_db_result_t* result = slam_db_query(*db, sql);
    if (!result) {
        lua_pushnil(L);
        CHECK_RETURN(false, 1, "query sql: %s error", sql);
    }

    slam_lua_db_decode_result(L, result);
    slam_db_result_delete(result);
    
    return 1;
}

//
// bool db:insert(table_name, record)
static int cc_db_insert(lua_State* L) {    
    int args = lua_gettop(L);// this:userdata in bottom of lua stack
    if (args != 3) {
        lua_pushboolean(L, false);
        CHECK_RETURN(false, 1, "`%s` lack args: %d", __FUNCTION__, args);
    }
    
    slam_db_t** db = (slam_db_t **) luaL_checkudata(L, 1, SLAM_LUA_METATABLE_DB_NAME);
    if (!db) {
        lua_pushboolean(L, false);
        CHECK_RETURN(false, 1, "invalid userdata: %s", SLAM_LUA_METATABLE_DB_NAME);
    }

    if (!lua_isstring(L, -(args - 1))) {
        lua_pushboolean(L, false);
        CHECK_RETURN(false, 1, "args error: %s", lua_typename(L, lua_type(L, -(args - 1))));
    }

    if (!lua_istable(L, -(args - 2))) {
        lua_pushboolean(L, false);
        CHECK_RETURN(false, 1, "args error: %s", lua_typename(L, lua_type(L, -(args - 2))));
    }
	
	slam_lua_dump_table(L, -1, "stack top");
	return false;

    size_t len = 0;
    const char* table_name = lua_tolstring(L, -(args - 1), &len);

	log_trace("func: %s, args: %d, table: %s", __FUNCTION__, args, table_name);

    slam_byte_buffer_t* byte_buffer = slam_byte_buffer_new(4096);
    slam_byte_buffer_write(byte_buffer, "insert into `", 13);
    slam_byte_buffer_write(byte_buffer, table_name, len);
    slam_byte_buffer_write(byte_buffer, "` set ", 6);
    //bool rc = slam_lua_db_encode_table(L, -(args - 2), byte_buffer);
	bool rc = true;
    if (rc) {
        slam_byte_buffer_write(byte_buffer, "\0", 1);
        const char* sql = (const char*) slam_byte_buffer_readbuffer(byte_buffer);
        log_debug("insert sql: %s", sql);
        rc = slam_db_execute(*db, sql);
    }
    slam_byte_buffer_delete(byte_buffer);
    
    lua_pushboolean(L, rc);
    return 1;
}

//
// bool db:update(table_name, record, [where])
static int cc_db_update(lua_State* L) {    
    int args = lua_gettop(L);// this:userdata in bottom of lua stack
    if (args != 3 && args != 4) {
        lua_pushboolean(L, false);
        CHECK_RETURN(false, 1, "`%s` lack args: %d", __FUNCTION__, args);
    }
    
    slam_db_t** db = (slam_db_t **) luaL_checkudata(L, 1, SLAM_LUA_METATABLE_DB_NAME);
    if (!db) {
        lua_pushboolean(L, false);
        CHECK_RETURN(false, 1, "invalid userdata: %s", SLAM_LUA_METATABLE_DB_NAME);
    }

    if (!lua_isstring(L, -(args - 1))) {
        lua_pushboolean(L, false);
        CHECK_RETURN(false, 1, "args error: %s", lua_typename(L, lua_type(L, -(args - 1))));
    }

    size_t table_len = 0;
    const char* table = lua_tolstring(L, -(args - 1), &table_len);

    if (!lua_istable(L, -(args - 2))) {
        lua_pushboolean(L, false);
        CHECK_RETURN(false, 1, "args error: %s", lua_typename(L, lua_type(L, -(args - 2))));
    }

    size_t where_len = 0;
    const char* where = nullptr;
    
    if (args == 4) {
        if (!lua_isstring(L, -(args - 3))) {
            lua_pushboolean(L, false);
            CHECK_RETURN(false, 1, "args error: %s", lua_typename(L, lua_type(L, -(args - 3))));
        }
        where = lua_tolstring(L, -(args - 3), &where_len);
    }
    
    slam_byte_buffer_t* byte_buffer = slam_byte_buffer_new(4096);
    slam_byte_buffer_write(byte_buffer, "update `", 8);
    slam_byte_buffer_write(byte_buffer, table, table_len);
    slam_byte_buffer_write(byte_buffer, "` set ", 6);
    bool rc = slam_lua_db_encode_table(L, -(args - 2), byte_buffer);
    if (rc) {
        if (where) {
            slam_byte_buffer_write(byte_buffer, " where ", 7);
            slam_byte_buffer_write(byte_buffer, where, where_len);
        }
        slam_byte_buffer_write(byte_buffer, "\0", 1);
        const char* sql = (const char*) slam_byte_buffer_readbuffer(byte_buffer);
        log_debug("update sql: %s", sql);
        rc = slam_db_execute(*db, sql);
    }
    slam_byte_buffer_delete(byte_buffer);
    
    lua_pushboolean(L, rc);
    return 1;
}

void slam_lua_reg_db_func(slam_lua_t* lua) {
	lua_State* L = lua->L;

    slam_lua_begin_namespace(lua, SLAM_LUA_REGISTER_NAMESPACE);
    
    //
    // db
    luaL_newmetatable(L, SLAM_LUA_METATABLE_DB_NAME); //db metatable
    
    /* metatable.__index = metatable */
    lua_pushvalue(L, -1);  /* duplicates the metatable */
    lua_setfield(L, -2, "__index"); /* __index ref self */


    //
    // bool db:execute(sql)
    LUA_REGISTER(L, "execute", cc_db_execute);
    //
    // [lua_table | nil] db:query(sql)
    LUA_REGISTER(L, "query", cc_db_query);
    //
    // bool db:insert(table_name, record)
    LUA_REGISTER(L, "insert", cc_db_insert);
    //
    // bool db:update(table_name, record, where)
    LUA_REGISTER(L, "update", cc_db_update);
    //
    // uint64_t db:insert_id()
    //LUA_REGISTER(L, "insert_id", cc_db_insert_id);


    //
    // gc & tostring
    LUA_REGISTER(L, "__gc", __cc_db_gc);
    LUA_REGISTER(L, "__tostring", __cc_db_tostring);
    
    slam_lua_end_namespace(lua);
    
    //
    // userdata newdb(host, username, password, port)
    slam_lua_begin_namespace(lua, SLAM_LUA_REGISTER_NAMESPACE);
    LUA_REGISTER(L, "newdb", cc_newdb);
    slam_lua_end_namespace(lua);
}

