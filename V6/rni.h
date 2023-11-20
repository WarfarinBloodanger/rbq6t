#ifndef RNI
#define RNI
#define RBQ_VERSION 6.1
#include<stdio.h>

#ifndef bool
#define bool char
#define true 1
#define false 0
#endif

char excpbuf[1024];
#define FORMAT(str,...) (sprintf(excpbuf,str,__VA_ARGS__),excpbuf)

typedef enum{
	TYPE_UNDEF=0,
	TYPE_NULL,
	TYPE_NUM,
	TYPE_STR,
	TYPE_ARR,
	TYPE_MAP,
	TYPE_FUNC,
	TYPE_PTR,
} ValueType;

const char* typenames[]={
	"undefined",
	"null",
	"number",
	"string",
	"array",
	"object",
	"function",
	"pointer",
};

typedef struct{
	ValueType type;
	union{
		void* str;
		void* arr;
		void* dict;
		void* fn;
	};
	bool isBlack,released;
}Object;

typedef struct{
	ValueType type;
	union{
		double num;
		Object* obj;
	    void* ptr;
	};
}Value;

typedef Object* ObjectRef;
typedef Value* ValueRef;
typedef Value (*NativeFunction)(void* env,ValueRef thisObject,Value* argv,int argc);

typedef struct{
	Value (*NewNumber)(double x);
	Value (*NewString)(const char* buf);
	Value (*NewStringWithGBK)(const char* buf);
	Value (*NewPointer)(void* ptr);
	
	Value (*NewArray)();
	Value (*GetArrayIndex)(Value value,int index);
	void (*ResizeArray)(Value value,int size);
	void (*AppendItem)(Value value,Value v);
	void (*SetArrayIndex)(Value value,int index,Value v);
	int (*GetArrayLength)(Value value);
	
	Value (*NewMap)();
	Value (*GetMapIndex)(Value value,Value index);
	Value (*Keys)(Value value);
	int (*HasKey)(Value value,Value key);
	void (*SetMapIndex)(Value value,Value key,Value v);
	int (*GetMapLength)(Value value);
	
	Value (*NewFunction)(NativeFunction native);
	Value (*GetAttribute)(Value value,const char*attribute);
	Value (*CallFunction)(Value value,Value thisObject,Value* args,int argc);
	
	const char* (*ToStr)(Value value);
	const char* (*ToGBKStr)(Value value);
	Value (*JSON)(const char*json);
	
	void (*Throw)(const char*type,const char*reason);
}rbq_env;

#define rbq_inner_native_func(name)\
Value __declspec(dllexport) name(void* __env,ValueRef _this,Value* args,int argc)

#define env ((rbq_env*)(__env))

#define rbq_native_func(name)\
Value __declspec(dllexport) name(rbq_env* __env,ValueRef _this,Value* args,int argc)


#define CHECK_ARGS_TYPE(argno,_type)\
if(argc<argno)env->Throw("ArgumentError",FORMAT("expected argument #%d, given: %d",(argno),argc));\
if(args[(argno)-1].type!=_type)env->Throw("TypeError",FORMAT("expected '%s' for argument %d, given: '%s'",typenames[_type],(argno),typenames[args[(argno)-1].type]))

#define PTROF(type,value)\
((type)(value.ptr))

#endif
