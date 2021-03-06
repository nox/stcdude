#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include "uart.h"
#include "stcdude.h"
#include "lualib.h"
#include "lauxlib.h"


//#define CHECK() printf("Current stack top:  %d (%s:%d)\n", lua_gettop(L),__FILE__, __LINE__);
#define CHECK() ;;
#define get_str_member(var, member, failcode)		\
	lua_pushstring(L, member);			\
	lua_gettable(L, -2);				\
	if (!lua_isstring(L, -1)) { failcode } else {	\
		var = lua_tostring(L, -1);		\
		lua_pop(L, 1); }

void report_errors(lua_State *L, int status) {
	if ( status!=0 ) {
		printf("ooops: %s \n", lua_tostring(L, -1));
		lua_pop(L, 1); // remove error message
	}
}

lua_State* mcudb_open(lua_State* L, char* file)
{
	CHECK();
	printf("Loading lua script: %s\n", file);
	int s = luaL_loadfile(L, file);
	if ( s==0 ) {
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
		if (s!=0) report_errors(L, s);
	}
	report_errors(L, s);
	CHECK();
	return L;
}


void print_mcuinfo(struct mcuinfo *mi) {
	printf("MCU DB information\n");
	printf("Part name:\t %s\n", mi->name);
	printf("Magic bytes:\t %hhX%hhX\n", mi->magic[0],mi->magic[1]);
	printf("IRAM size:\t %d (0x%x) bytes\n", mi->iramsz+1, mi->iramsz+1);
	printf("XRAM size:\t %d (0x%x) bytes\n", mi->xramsz+1, mi->xramsz+1);
	printf("IROM size:\t %d (0x%x) bytes\n", mi->iromsz+1, mi->iromsz+1);
	printf("Tested ops: FixMe: implement reading of tested ops\n");
	if (mi->descr) {
		printf("Description:\n");
		printf(mi->descr);
	}
}

struct mcuinfo* mcudb_query_magic(void* L, char* magic) {
	CHECK();
	char mstr[8];
	sprintf(mstr, "%hhX%hhX", magic[0],magic[1]);
	lua_getglobal(L, "get_mcu_by_magic");  /* function to be called */
	lua_pushstring(L, mstr);   /* push 1st argument */
	/* do the call (2 arguments, 1 result) */
	if (lua_pcall(L, 1, 1, 0) != 0)
		error(L, "error running function `f': %s",
			lua_tostring(L, -1));
	if (!lua_istable(L,-1)) {
		lua_pop(L,1);
		CHECK();
		return 0;
	}
	CHECK();
	char* tmp;
	get_str_member(tmp,"name", lua_pop(L,1); return 0;);
	struct mcuinfo *mi = calloc(1,sizeof(struct mcuinfo));
	memcpy(mi->magic,magic,2);
        mi->name = strdup(tmp);
	CHECK();
        get_str_member(tmp,"descr", lua_pop(L,1); tmp=NULL;);
	if (tmp) mi->descr = strdup(tmp);
	get_str_member(tmp,"speed", lua_pop(L,1); tmp=NULL;);
	if (tmp) {
		sscanf(tmp,"%d",&mi->speed);
	}
	get_str_member(tmp,"speed", lua_pop(L,1); tmp=NULL;);
	if (tmp) {
		sscanf(tmp,"%d",&mi->speed);
	}
	get_str_member(tmp,"iram", lua_pop(L,1); tmp=NULL;);
	if (tmp) {
		sscanf(tmp,"0x%X",&mi->iramsz);
	}
	get_str_member(tmp,"irom", lua_pop(L,1); tmp=NULL;);
	if (tmp) {
		sscanf(tmp,"0x%X",&mi->iromsz);
	}
	get_str_member(tmp,"xram", lua_pop(L,1); tmp=NULL;);
	if (tmp) {
		sscanf(tmp,"0x%X",&mi->xramsz);
	}
	/* TODO: Parse tested ops here */
	lua_pop(L,1);
	
	CHECK();
	return mi;
}
