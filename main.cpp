#include <iostream>
#include <lua.hpp>
#include <thread>
#include <utility>
#include <cassert>

using namespace std;

struct cppthread {
    int status; //0 = OK, 1 = closed, -1 = error
    thread th;

    template <typename F>
    cppthread(F fn) : status(0), th(fn) {}
};

char const *const typenames[] = {
    "LUA_TNIL",
    "LUA_TBOOLEAN",
    "LUA_TLIGHTUSERDATA",
    "LUA_TNUMBER",
    "LUA_TSTRING",
    "LUA_TTABLE",
    "LUA_TFUNCTION",
    "LUA_TUSERDATA",
    "LUA_TTHREAD",
};

static int thrun(lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    
    void *cth = lua_newuserdatauv(L, sizeof(cppthread), 0);
    assert(cth);
    lua_insert(L,1); //Move this underneath function and arguments
    
    lua_State *th = lua_newthread(L);
    //Prevent thread from being garbage-collected
    lua_pushlightuserdata(th, cth);
    lua_pushthread(th);
    lua_rawset(th, LUA_REGISTRYINDEX);

    //Remove newthread from old stack
    lua_pop(L,1);
    //Move function and arguments to new stack (but not userdata ret val)
    lua_xmove(L, th, lua_gettop(L) - 1);
    
    auto thread_fn = [th, cth]() {
        int rc = lua_pcall(th, lua_gettop(th) - 1, 0, 0);
        if (rc != LUA_OK) {
            if (lua_type(th,-1) == LUA_TSTRING) {
                printf("\nError: %s\n", lua_tostring(th,-1));
            }
        }

        //Allow thread to be garbage-collected
        lua_pushlightuserdata(th, cth);
        lua_pushnil(th);
        lua_rawset(th, LUA_REGISTRYINDEX);
    };
    
    //printf("Allocated thread %p\n", cth);
    new (cth) cppthread(thread_fn);
    
    luaL_getmetatable(L, "cppthread");
    lua_setmetatable(L, -2);

    
    
    return 1;
}

static int thjoin(lua_State *L) {
    cppthread *th = reinterpret_cast<cppthread*>(
        luaL_checkudata(L,1,"cppthread")
    );

    if (th->status != 0) {
        luaL_error(L, "Cannot join dead thread");
    }
    
    //printf("Joining thread %p\n", th);
    th->th.join();
    th->status = 1;
    
    return 0;
}

int what_am_i(lua_State *L) {
    for (int i = 1; i <= lua_gettop(L); i++) {
        int tp = lua_type(L, i);
        printf("Argument %d: %s\n", i, typenames[tp]);
    }
    return 0;
}

static int cppthread_gc(lua_State *L) {
    cppthread *th = reinterpret_cast<cppthread*>(
        luaL_checkudata(L,1,"cppthread")
    );

    if (th->status == 0) {
        //printf("Detaching thread %p\n", th);
        th->th.detach();
    }
    //printf("Destroying thread %p\n", th);
    th->th.~thread();

    return 0;
}

static luaL_Reg const mylib[] = {
    {"run", &thrun},
    {"join", &thjoin},
    {"w", &what_am_i},
    {NULL, NULL}
};

extern "C"
int luaopen_thing(lua_State *L) {
    //Table with all the thread functions
    luaL_newlib(L, mylib);

    //Metatable for thread handles
    int tp = luaL_newmetatable(L, "cppthread");
    
    lua_pushlstring(L, "__index", sizeof("__index") -1);
    lua_pushvalue(L, -3); //Copy thread function table to top of stack
    lua_settable(L, -3);  //Set __index for the metatable
    
    lua_pushlstring(L,"__gc",4);
    lua_pushcfunction(L, &cppthread_gc);
    lua_rawset(L, -3);

    //No touchy
    lua_pushlstring(L,"__metatable",11);
    lua_pushlstring(L,"cppthread",9);
    lua_rawset(L, -3);

    lua_pop(L, 1); //Pop the metatable
    
    return 1;
}