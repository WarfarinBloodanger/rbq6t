#pragma GCC optimize(3)
#pragma GCC target("avx")
#pragma GCC optimize("Ofast")
#pragma GCC optimize("inline")
#pragma GCC optimize("-fgcse")
#pragma GCC optimize("-fgcse-lm")
#pragma GCC optimize("-fipa-sra")
#pragma GCC optimize("-ftree-pre")
#pragma GCC optimize("-ftree-vrp")
#pragma GCC optimize("-fpeephole2")
#pragma GCC optimize("-ffast-math")
#pragma GCC optimize("-fsched-spec")
#pragma GCC optimize("unroll-loops")
#pragma GCC optimize("-falign-jumps")
#pragma GCC optimize("-falign-loops")
#pragma GCC optimize("-falign-labels")
#pragma GCC optimize("-fdevirtualize")
#pragma GCC optimize("-fcaller-saves")
#pragma GCC optimize("-fcrossjumping")
#pragma GCC optimize("-fthread-jumps")
#pragma GCC optimize("-funroll-loops")
#pragma GCC optimize("-fwhole-program")
#pragma GCC optimize("-freorder-blocks")
#pragma GCC optimize("-fschedule-insns")
#pragma GCC optimize("inline-functions")
#pragma GCC optimize("-ftree-tail-merge")
#pragma GCC optimize("-fschedule-insns2")
#pragma GCC optimize("-fstrict-aliasing")
#pragma GCC optimize("-fstrict-overflow")
#pragma GCC optimize("-falign-functions")
#pragma GCC optimize("-fcse-skip-blocks")
#pragma GCC optimize("-fcse-follow-jumps")
#pragma GCC optimize("-fsched-interblock")
#pragma GCC optimize("-fpartial-inlining")
#pragma GCC optimize("no-stack-protector")
#pragma GCC optimize("-freorder-functions")
#pragma GCC optimize("-findirect-inlining")
#pragma GCC optimize("-fhoist-adjacent-loads")
#pragma GCC optimize("-frerun-cse-after-loop")
#pragma GCC optimize("inline-small-functions")
#pragma GCC optimize("-finline-small-functions")
#pragma GCC optimize("-ftree-switch-conversion")
#pragma GCC optimize("-foptimize-sibling-calls")
#pragma GCC optimize("-fexpensive-optimizations")
#pragma GCC optimize("-funsafe-loop-optimizations")
#pragma GCC optimize("inline-functions-called-once")
#pragma GCC optimize("-fdelete-null-pointer-checks")
#pragma GCC optimize(2)
#include<iostream>

#include<cstring>
#include<cstdlib>
#include<cmath>
#include<ctime>

#include<string>
#include<sstream>
#include<algorithm>
#include<vector>
#include<map>
#include<set>
#include<fstream>
#include<windows.h>
using namespace std;

char excpbuf[1024];
#define SYNTAX_ERR(str,...) do{sprintf(excpbuf,str"\n",__VA_ARGS__);throw (string)excpbuf;}while(0)
#define FORMAT(str,...) (sprintf(excpbuf,str,__VA_ARGS__),(string)excpbuf)
void THROW(const string&type,const string&reason);

typedef void* 				Val;
typedef Val*				Ptr;
typedef unsigned char 		u8;
typedef char 				i8;
typedef int16_t 			i16;
typedef uint16_t 			u16;
typedef int32_t				i32;
typedef uint32_t			u32;
typedef int64_t				i64;
typedef uint64_t			u64;
typedef string*				StrRef;
typedef vector<Val>			Arr;
typedef map<string,Val>		Map;
typedef double Num;
typedef u8	uchar;
typedef u16 ushort;
typedef u32 uint;
typedef u64 ull;
typedef const uchar OPCODE;
typedef vector<uchar> Bytes;

const double PI=acos(-1);
bool DEBUG_MODE=false;
bool cliMode=false;
const uint MAGIC_NUMBER=0X01140514;
const uint VERSION_CODE=0X60001020;
uint runstackSize=1024*8;

#define Add emplace_back
#include "bytecode.cpp" 

struct Instr{
	uchar type;
	union{
		ushort _o[2];
		uint x;
		uchar _b[4];
	};
	#define op1 _o[0]
	#define op2 _o[1]
	#define getIntOp() x
	Instr(){type=NOP;}
	Instr(uchar t){op1=op2=0;type=t;}
	Instr(uchar t,ushort o1){op1=op2=0;type=t;x=o1;}
	Instr(uchar t,ushort o1,ushort o2){op1=op2=0;type=t;op1=o1,op2=o2;}
	Instr(uchar t,uint o1){type=t;x=o1;}
	Bytes serialize()const{
		Bytes b;
		b.Add(type);
		for(int i=0;i<4;i++)b.Add(_b[i]);
		return b;
	}
	bool operator==(const Instr&instr)const{return type==instr.type&&x==instr.x;}
	bool operator<(const Instr&instr)const{
		if(type!=instr.type)return type<instr.type;
		return x<instr.x;
	}
};

uint usedLabelCount=0;
typedef Instr Label;
#define newLabel() Instr(LABEL,++usedLabelCount)

ostream& operator<<(ostream&out,const Instr&ins){
	if(ins.type==LINE)out<<"L"<<ins.op1<<":"<<ins.op2;
	else out<<"\t"<<opcodeName[ins.type]<<"\t["<<ins.op2<<"] ["<<ins.op1<<"]";
	return out;
}
#undef op1
#undef op2

typedef vector<Instr> CodeSet;
ostream& operator<<(ostream&out,const CodeSet&s){
	uint i=0;
	for(auto a:s){
		out<<"["<<i<<"] "<<a<<endl;
		i++;
	} 
	return out;
}
namespace utils{
long long hex2dec(const string&s){
	long long x=0,v=0;
	uint i=0;
	if(s.size()&&s[0]=='-')v=1;
	if(s.size()<=(uint)(2+v)||(s[i]!='0'&&toupper(s[i+1]!='X')))SYNTAX_ERR("invalid hex number: '%s'",s.c_str());
	else i+=2;
	for(;i<s.size();i++){
		if(!isdigit(s[i])&&(toupper(s[i])<'A'||toupper(s[i])>'F'))SYNTAX_ERR("invalid hex number: '%s'",s.c_str());
		x=(x<<4)^(isdigit(s[i])?s[i]&15:10+(toupper(s[i])-'A'));
	}
	return x*(v?-1:1);
}
string encd(ull v){
	string s="";
	if(v<0)SYNTAX_ERR("invalid codepoint %lld",v);
	if(v<=0x7f)s+=(char)(v&0x7f);
	else if(v<=0x7ff)s+=(char)(0xc0|((v&0x7c0)>>6)),s+=char((0x80|(v&0x3f)));
	else if(v<=0xffff)s+=char(0xe0|((v&0xf000)>>12)),s+=char(0x80|((v&0xfc0)>>6)),s+=char(0x80|(v&0x3f));
	else if(v<=0x10ffff)s+=char(0xf0|((v&0x1c0000)>>18)),s+=char(0x80|((v&0x3f00)>>12)),s+=char(0x80|((v&0xfc0)>>6)),s+=char(0x80|(v&0x3f));
	else SYNTAX_ERR("invalid codepoint %lld",v);
	return s;
}
inline string int2str(ull x,ull base){
	string r="";int n=0;
	if(x==0)return (string)"0";
	if(x<0)n=1,x=-x;
	if(abs(base)>36||abs(base)<=1)SYNTAX_ERR("invalid number parsing base: %lld",base);
	while(x){
		int v=x%base;
		if(v<0)v-=base,x+=base;
		if(v>=0&&v<=9)r=r+char('0'+v);
		else r=r+char('A'+v-10);
		x/=base;
	}
	reverse(r.begin(),r.end());
	if(n)r="-"+r;
	return r;
}
inline string strictStr(const string&str){
	string r="";
	for(auto a:str){
		switch(a){
			case '\a':r+="\\a";break;
			case '\t':r+="\\t";break;
			case '\n':r+="\\n";break;
			case '\r':r+="\\r";break;
			case '\b':r+="\\b";break;
			case '\\':r+="\\\\";break;
			case '"':r+="\"";break;
			default:{
				if(!isprint(a))r+="\\u00"+utils::int2str((unsigned char)a,16);
				else r+=a;
				break;
			}
		}
	}
	return "\""+r+"\"";
}
inline string stringExpr(const string&x){
	uint len=x.size();string r="";
	#define gen(v) r+=(v)
	for(uint i=0;i<len;i++)if(x[i]=='\\'){
		if(i+1<len){
			switch(x[i+1]){
				case 'a':gen('\a');break;
				case 'n':gen('\n');break;
				case 't':gen('\t');break;
				case 'r':gen('\r');break;
				case 'b':gen('\b');break;
				case '\\':gen('\\');break;
				case '\'':gen('\'');break;
				case '\"':gen('\"');break;
				case '0':gen('\0');break;
				case 'u':{
					i+=2;
					if(i+3>=len)SYNTAX_ERR("uncompleted unicode escape sequence in string '%s'",x.c_str());
					string e=encd(hex2dec((string)"0x"+x[i]+x[i+1]+x[i+2]+x[i+3]));
					r+=e;
					i+=2;
					break;
				}
				default:SYNTAX_ERR("unknown escaped char '\\%c' in string '%s'",x[i+1],x.c_str());
			}
			i++;
		}
	}else gen(x[i]);
	return r;
}
inline void concat(CodeSet &a,const CodeSet &b){
	for(auto x:b)a.push_back(x);
}
inline vector<string> splitBySpace(const string&s){
	vector<string> vs;
	string token="";
	uint i=0;
	bool inQuote=false;
	char terminator=' ';
	while(i<s.size()){
		if(inQuote){
			if(s[i]==terminator){
				token+=s[i];
				inQuote=false;
				vs.Add(token);
				token="";
			}
			else{
				token+=s[i];
				if(s[i]=='\\'){
					i++;
				}
			}
		}
		else if(s[i]=='\''||s[i]=='\"'){
			if(token.size()){
				vs.Add(token);
			}
			token=(string)""+s[i];
			terminator=s[i];
			inQuote=true;
		}
		else if(s[i]==' '){
			if(token.size()){
				vs.Add(token);
				token="";
			}
		}
		else token+=s[i];
		i++;
	}
	vs.Add(token);
	return vs;
}
CodeSet operator+(const CodeSet&a,const CodeSet&b){
	CodeSet c;
	for(auto x:a)c.Add(x);
	for(auto x:b)c.Add(x);
	return c;
}
Bytes operator+(const Bytes&a,const Bytes&b){
	Bytes c;
	for(auto x:a)c.Add(x);
	for(auto x:b)c.Add(x);
	return c;
}
Num str2num(const string&s){
	Num v;stringstream ss("");ss<<s;ss>>v;return v;
}
char buf[48];
string num2str(const Num&d){
	sprintf(buf,"%.14g",d);return buf;
}
string ptr2str(void*ptr){
	sprintf(buf,"0x%p",ptr);return buf;
}
inline bool isInteger(Num d){
	return ceil(d)==floor(d);
}
inline Bytes BYTE(uchar g){return {uchar(g&0xffu)};}
inline Bytes SHORT(ushort g){return BYTE((g>>8)&0xffu)+BYTE(g&0xffu);}
inline Bytes INT(uint g){return SHORT((g>>16)&0xffffu)+SHORT(g&0xffffu);}

struct BitParser{
	union{
		Num x;
		uchar bits[16];
	};
};
BitParser parser;
const string HEX_CHARS="0123456789ABCDEF";
ostream& operator<<(ostream&out,const Bytes&b){
	uint i=0;
	for(auto a:b)out<<HEX_CHARS[a/16]<<HEX_CHARS[a%16]<<" \n"[(i++)%8==7];
	return out;
}
string ReplaceExt(const string&fileName){
	string x=fileName;
	while(x.size()&&x.back()!='.')x.pop_back();
	if(x.size())return x+"rbq6";
	return fileName+".rbq6";
}
}
using namespace utils;

const uint MAX_FILE_CNT=1024*4;
namespace file_manager{
	char fbuffer[2048];
	FILE *file_ptrs[MAX_FILE_CNT];
	int size;
	inline int file_open(string file,string mode="r") {
		bool write=false;
		if(strchr(mode.c_str(),'w')!=NULL)write=true;
		FILE *ptr=fopen(file.c_str(),mode.c_str());
		if(ptr==NULL){
			if(write){ofstream fcout(file.c_str());fcout.close();}
			else THROW("IOError",FORMAT("%s",(("File doesn't exist: "+file).c_str())));
		}
		file_ptrs[size++]=ptr;
		return size-1;
	}
	inline void check_handle(int handle){if(handle<0||handle>=size)THROW("IOError",FORMAT("%s",(("Invalid handle: "+num2str(handle)).c_str())));}
	inline bool file_close(int handle){check_handle(handle);return fclose(file_ptrs[handle]);}
	inline bool file_eof(int handle){check_handle(handle);return feof(file_ptrs[handle]);}
	inline string fread_string(int handle){check_handle(handle);fscanf(file_ptrs[handle],"%s",fbuffer);return fbuffer;}
	inline string fread_line(int handle){
		check_handle(handle);string result="";
		char c=fgetc(file_ptrs[handle]);
		while(c!='\n'&&!feof(file_ptrs[handle]))result+=c,c=fgetc(file_ptrs[handle]);
		return result;
	}
	int fwrite_string(int handle,string text){check_handle(handle);int ret=fprintf(file_ptrs[handle],"%s",text.c_str());fflush(file_ptrs[handle]);return ret;}
	inline double fread_number(int handle){check_handle(handle);return atof(fread_string(handle).c_str());}
	inline unsigned int fread_char(int handle){
		check_handle(handle);
		unsigned char c=fgetc(file_ptrs[handle]);
		return c;
	}
	inline void fwrite_char(int handle,unsigned char x){check_handle(handle);fwrite(&x,sizeof(x),1,file_ptrs[handle]);fflush(file_ptrs[handle]);}
	inline double read_number(){double d;cin>>d;return d;}
	inline string read_string(){string s;cin>>s;return s;}
	inline string read_line(){string s;getline(cin,s);return s;}
	inline string read_getchar(){char c=getchar();return (string)""+c;}
	inline bool read_eof(){return cin.eof();}
};
using namespace file_manager;

namespace tokenizer{
enum {
	TOK_NUM,TOK_STR,TOK_ID,TOK_HEX,
	TOK_ADD,TOK_SUB,
	TOK_MUL,TOK_DIV,TOK_MOD,
	TOK_EQL,TOK_NEQ,TOK_BIG,TOK_SML,TOK_GE,TOK_LE,
	TOK_AND,TOK_OR,TOK_NOT,
	TOK_BITAND,TOK_BITOR,TOK_BITNOT,TOK_XOR,TOK_LSHF,TOK_RSHF,
	TOK_COM,TOK_ASS,TOK_DOT,TOK_QUEZ,TOK_COL,
	TOK_LPR,TOK_RPR,TOK_LBK,TOK_RBK,TOK_LBR,TOK_RBR,TOK_FEN,
	TOK_FUNC,TOK_IF,TOK_ELSE,TOK_WHILE,TOK_RET,TOK_FOR,TOK_VAR,TOK_BREAK,TOK_CTN,TOK_ALL,
	TOK_TRUE,TOK_FALSE,TOK_NULL,TOK_UNDEFINED,TOK_INCLUDE,TOK_THIS,TOK_CLASS,TOK_NEW,TOK_HAS,TOK_TYPEOF,TOK_CHOOSE,TOK_IS,
	TOK_OP,TOK_CONSTRUCTOR,TOK_PUB,TOK_PROT,TOK_PRIV,TOK_SUPER,TOK_STATIC,TOK_ARR,
	TOK_TRY,TOK_CATCH,TOK_FINALLY,
	TOK_ADDE,TOK_SUBE,
	TOK_MULE,TOK_DIVE,TOK_MODE,
	TOK_BITANDE,TOK_BITORE,TOK_XORE,TOK_LSHFE,TOK_RSHFE,TOK_REF,
};
const string tokenName[]={
	"number","string","identifier","hexnumber",
	"'+'","'-'","'*'","'/'","'%'","'=='","'!='","'>'","'<'","'>='","'<='",
	"'&&'","'||'","'!'","'&'","'|'","'~'","'^'","'<<'","'>>'",
	"','","'='","'.'","'?'","':'","'('","')'","'['","']'","'{'","'}'","';'",
	"'fn'/'function'","'if'","'else'","'while'","'return'","'for'","'var'","'break'","'continue'","'all'",
	"'true'","'false'","'null'","'undef'","'include'","'this'","'class'","'new'","'has'","'typeof'","'or'","'is'",
	"'operator'","'constructor'","'public'","'protected'","'private'","'super'","'static'","'arr'",
	"try","catch","finally",
	"+=","-=",
	"*=","/=","%=",
	"&=","|=","^=","<<=",">>=","ref"
};
inline uint priority(char tok){
	switch(tok){
		case TOK_COM:return 10;
		case TOK_ASS:
		case TOK_ADDE:case TOK_SUBE:case TOK_MULE:case TOK_DIVE:case TOK_MODE:
		case TOK_BITANDE:case TOK_BITORE:case TOK_XORE:case TOK_LSHFE:case TOK_RSHFE:return 20;
		case TOK_CHOOSE:return 24;
		case TOK_QUEZ:return 25;
		case TOK_OR:return 30;
		case TOK_AND:return 40;
		case TOK_BITOR:return 50;
		case TOK_XOR:return 60;
		case TOK_BITAND:return 70;
		case TOK_EQL:case TOK_NEQ:return 80;
		case TOK_LE:case TOK_SML:case TOK_GE:case TOK_BIG:case TOK_HAS:case TOK_TYPEOF:case TOK_IS:return 90;
		case TOK_LSHF:case TOK_RSHF:return 100;
		case TOK_ADD:case TOK_SUB:return 110;
		case TOK_MUL:case TOK_DIV:case TOK_MOD:return 120;
		case TOK_NOT:case TOK_BITNOT:return 130;
		case TOK_LPR:case TOK_LBK:case TOK_DOT:case TOK_REF:return 140; 
		default:return 1;
	}
}
struct Token{
	string file;
	uchar type;
	uint line,column;
	string val;
};
ostream& operator<<(ostream&out,const Token&t){
	out<<t.file<<" "<<t.line<<":"<<t.column<<" <"<<tokenName[t.type]<<"> "<<strictStr(t.val);
	return out;
}
const Token END_TOKEN=Token();
vector<Token> tokens;
char getIdType(const string&s){
	if(s=="fn")return TOK_FUNC;
	if(s=="function")return TOK_FUNC;
	if(s=="if")return TOK_IF;
	if(s=="else")return TOK_ELSE;
	if(s=="while")return TOK_WHILE;
	if(s=="for")return TOK_FOR;
	if(s=="return")return TOK_RET;
	if(s=="var")return TOK_VAR;
	if(s=="break")return TOK_BREAK;
	if(s=="continue")return TOK_CTN;
	if(s=="all")return TOK_ALL;
	if(s=="true")return TOK_TRUE;
	if(s=="false")return TOK_FALSE;
	if(s=="null")return TOK_NULL;
	if(s=="undef")return TOK_UNDEFINED;
	if(s=="include")return TOK_INCLUDE;
	if(s=="this")return TOK_THIS;
	if(s=="class")return TOK_CLASS;
	if(s=="new")return TOK_NEW;
	if(s=="has")return TOK_HAS;
	if(s=="typeof")return TOK_TYPEOF;
	if(s=="or")return TOK_CHOOSE;
	if(s=="is")return TOK_IS;
	if(s=="operator")return TOK_OP;
	if(s=="constructor")return TOK_CONSTRUCTOR;
	if(s=="static")return TOK_STATIC;
	if(s=="super")return TOK_SUPER;
	if(s=="ref")return TOK_REF;
	if(s=="arr")return TOK_ARR;
	if(s=="try")return TOK_TRY;
	if(s=="catch")return TOK_CATCH;
	if(s=="finally")return TOK_FINALLY;
	return TOK_ID; 
}
void tokenize(const string&file,const string&src,vector<Token>&tokens=tokens,int strict=1){
	uint line=1,column=0;
	uint len=src.size();
	uint loc=0;
	auto nextchar=[&]{
		line+=src[loc]=='\n';
		column=src[loc]=='\n'?1:column+(src[loc]=='\t'?4:1);
		loc++;
	};
	while(loc<len){
		Token cur;
		cur.line=line,cur.column=column,cur.file=file;
		if(isalpha(src[loc])||src[loc]=='_'){
			while((loc<len)&&(isalpha(src[loc])||isdigit(src[loc])||src[loc]=='_'))cur.val.push_back(src[loc]),nextchar();
			cur.type=getIdType(cur.val),tokens.push_back(cur); 
		}
		else if(isdigit(src[loc])||src[loc]=='.'){
			bool d=0,e=0,n=0;
			if(src[loc]=='0'&&loc+1<len&&(src[loc+1]=='x'||src[loc+1]=='X')){
				cur.val.push_back(src[loc]),nextchar();
				cur.val.push_back(src[loc]),nextchar();
				while((loc<len&&isdigit(src[loc]))||(src[loc]>='a'&&src[loc]<='f')||(src[loc]>='A'&&src[loc]<='F'))cur.val.push_back(tolower(src[loc])),nextchar();
				cur.type=TOK_HEX,tokens.push_back(cur);
			}
			else{
				while((loc<len&&isdigit(src[loc]))||src[loc]=='.'||src[loc]=='E'||src[loc]=='e'||src[loc]=='-'){
					if(src[loc]=='.'){if(!d)d=1;else break;}
					if(src[loc]=='e'||src[loc]=='E'){if(n&&!e)e=1;else break;}
					if(src[loc]=='-'){if(loc-1>=0&&(src[loc-1]=='e'||src[loc-1]=='E'));else break;}
					if(isdigit(src[loc]))n=1;
					cur.val.push_back(src[loc]),nextchar();
				}
				if(cur.val.size()==1&&cur.val[0]=='.')cur.type=TOK_DOT;
				else cur.type=TOK_NUM;
				tokens.push_back(cur);
			}
		}
		else if(src[loc]=='\''||src[loc]=='\"'){
			if(loc+2<len&&src[loc+1]==src[loc]&&src[loc+2]==src[loc]){
				char e=src[loc];
				nextchar(),nextchar();
				while(loc+2<len&&!(src[loc]==e&&src[loc+1]==e&&src[loc+2]==e)){
					if(src[loc]=='\\')cur.val.push_back('\\');
					cur.val.push_back(src[loc]),nextchar();
				}
				if(strict&&loc+2>=len)SYNTAX_ERR("uncompleted multi-line string, expected '%c%c%c' at end",e,e,e);
				if(src[loc]==e)cur.val.push_back(src[loc]),nextchar();
				nextchar(),nextchar();
				cur.type=TOK_STR,tokens.push_back(cur);
			}
			else{
				char s=src[loc];cur.val.push_back(src[loc]),nextchar();
				while(loc<len&&src[loc]!=s&&src[loc]!='\r'&&src[loc]!='\n'){
					if(src[loc]=='\\')cur.val.push_back(src[loc]),nextchar();
					cur.val.push_back(src[loc]),nextchar();
				}
				if(src[loc]==s)cur.val.push_back(src[loc]),nextchar();
				cur.type=TOK_STR,tokens.push_back(cur);
			}
		}
		else if(src[loc]=='`'){
			char s=src[loc];cur.val.push_back(src[loc]),nextchar();
			while(loc<len&&src[loc]!=s&&src[loc]!='\r'&&src[loc]!='\n'){
				if(src[loc]=='\\')cur.val.push_back('\\');
				cur.val.push_back(src[loc]),nextchar();
			}
			if(src[loc]==s)cur.val.push_back(src[loc]),nextchar();
			cur.type=TOK_STR,tokens.push_back(cur);
		}
		else if(isspace(src[loc]))nextchar();
		#define checkeql_(tokname)\
			if(loc<len&&src[loc]=='=')cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_##tokname##E;
		#define checkeql(tokname)\
		{\
			cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_##tokname;\
			checkeql_(tokname);\
			tokens.push_back(cur);\
			break;\
		}
		else switch(src[loc]){
			case('+'):checkeql(ADD);
			case('-'):{
				cur.val.push_back(src[loc]),nextchar();
				if(loc<len&&src[loc]=='=')cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_SUBE;
				else cur.type=TOK_SUB;
				tokens.push_back(cur);
				break;
			}
			case('*'):checkeql(MUL);
			case('/'):{
				if(loc+1<len&&src[loc+1]=='/'){
					nextchar();
					while(loc<len&&src[loc]!='\n')nextchar();
					nextchar();
				}
				else if(loc+1<len&&src[loc+1]=='*'){
					nextchar();
					while(loc+1<len&&!(src[loc]=='*'&&src[loc+1]=='/'))nextchar();
					if(loc+1>=len)SYNTAX_ERR("uncompleted comment symbol, expected '*/' at end%c",' ');
					nextchar(),nextchar();
				}
				else checkeql(DIV);
				break;
			}
			case('%'):checkeql(MOD);
			case('.'):cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_DOT,tokens.push_back(cur);break;
			case('('):cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_LPR,tokens.push_back(cur);break;
			case('['):cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_LBK,tokens.push_back(cur);break;
			case('{'):cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_LBR,tokens.push_back(cur);break;
			case(')'):cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_RPR,tokens.push_back(cur);break;
			case(']'):cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_RBK,tokens.push_back(cur);break;
			case('}'):cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_RBR,tokens.push_back(cur);break;
			case('^'):checkeql(XOR);
			case(';'):cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_FEN,tokens.push_back(cur);break;
			case(','):cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_COM,tokens.push_back(cur);break;
			case(':'):cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_COL,tokens.push_back(cur);break;
			case('?'):cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_QUEZ,tokens.push_back(cur);break;
			case('='):{
				cur.val.push_back(src[loc]),nextchar();
				if(loc<len&&src[loc]=='=')cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_EQL;
				else cur.type=TOK_ASS;
				tokens.push_back(cur);
				break;
			}
			case('>'):{
				cur.val.push_back(src[loc]),nextchar();
				if(loc<len&&src[loc]=='=')cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_GE;
				else if(loc<len&&src[loc]=='>'){
					cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_RSHF;
					checkeql_(RSHF);
				}
				else cur.type=TOK_BIG;
				tokens.push_back(cur);
				break;
			}
			case('<'):{
				cur.val.push_back(src[loc]),nextchar();
				if(loc<len&&src[loc]=='=')cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_LE;
				else if(loc<len&&src[loc]=='<'){
					cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_LSHF;
					checkeql_(LSHF);
				}
				else cur.type=TOK_SML;
				tokens.push_back(cur);
				break;
			}
			case('!'):{
				cur.val.push_back(src[loc]),nextchar();
				if(loc<len&&src[loc]=='=')cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_NEQ;
				else cur.type=TOK_NOT;
				tokens.push_back(cur);
				break;
			}
			case('&'):{
				cur.val.push_back(src[loc]),nextchar();
				if(loc<len&&src[loc]=='&')cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_AND;
				else{
					cur.type=TOK_BITAND;
					checkeql_(BITAND);
				}
				tokens.push_back(cur);
				break;
			}
			case('|'):{
				cur.val.push_back(src[loc]),nextchar();
				if(loc<len&&src[loc]=='|')cur.val.push_back(src[loc]),nextchar(),cur.type=TOK_OR;
				else{
					cur.type=TOK_BITOR;
					checkeql_(BITOR);
				}
				tokens.push_back(cur);
				break;
			}
			default:if(!strict)break;SYNTAX_ERR("unknown char '%c' at line %d, column %d",src[loc],line,column);nextchar();break;
		}
	}
}
void startTokenize(const string&file,bool starter=false){
	stringstream src("");
	string line;
	ifstream fcin(file);
	if(starter)src<<"{";
	while(getline(fcin,line)){
		vector<string>t=splitBySpace(line);
		if(t.size()&&t[0]=="#include"){
			for(uint i=1;i<t.size();i++){
				if(t[i][0]=='\''||t[i][0]=='"')t[i]=stringExpr(t[i].substr(1,t[i].size()-2));
				startTokenize(t[i]);
			}
			src<<endl;
		}
	else src<<line<<'\n';
	}
	if(starter)src<<"}";
	tokenize(file,src.str(),tokens,1);
	fcin.close();
}
uint currentTokenIndex;
#define EOT() (currentTokenIndex>=tokens.size())
#define tok (EOT()?END_TOKEN:tokens[currentTokenIndex])
Token lastToken(){
	if(currentTokenIndex==0)return END_TOKEN;
	return tokens[currentTokenIndex-1];
}
Token nextToken(){
	if(currentTokenIndex==tokens.size())return END_TOKEN;
	return tokens[currentTokenIndex++];
}
Token readToken(uchar type){
	if(tok.type!=type)SYNTAX_ERR("File %s, %d:%d, expected '%s', got '%s'",tok.file.c_str(),tok.line,tok.column,tokenName[type].c_str(),tok.val.c_str());
	if(currentTokenIndex==tokens.size())return END_TOKEN;
	return tokens[currentTokenIndex++];
}
}
using namespace tokenizer;

template <typename T> 
struct Table{
	uint usedIdx;
	map<T,uint> tables;
	map<uint,T> values;
	Table(){
		usedIdx=0;
	}
	bool Has(const T&key){
		return tables.find(key)!=tables.end();
	}
	void Bind(const T&key,uint id){
		tables[key]=id;
		values[id]=key;
	}
	uint Ensure(const T&key){
		if(!Has(key)){
			Bind(key,usedIdx);
			usedIdx++;
		}
		return tables[key];
	}
	uint Id(const string&key){
		return tables[key];
	}
	T& Get(uint id){
		return values[id];
	}
};

typedef Table<string> SymTable;

map<string,SymTable> names;

namespace Compiler{

typedef enum {
	VT_UNKNOWN,
	VT_GLOBAL,
	VT_LOCAL,
	VT_UPVALUE,
	VT_BUILTIN,
} VariableType;

const string VT_NAME[]={
	"UNKNOWN",
	"GLOBAL",
	"LOCAL",
	"UPVALUE",
	"BUILTIN",
};

typedef enum {
	UT_UNKNOWN,
	UT_AT_LOCAL,
	UT_AT_UPVALUE,
} UpvalType;

const string UT_NAME[]={
	"UNKNOWN",
	"AT_LOCAL",
	"AT_UPVALUE",
};

struct Variable{
	bool isUpvalue;
	string name;
	uint id;
	VariableType type;
	
	Variable(VariableType _type){
		type=_type;
	}
	Variable(VariableType _type,const string&_name,uint _id){
		type=_type;
		name=_name;
		id=_id;
	}
	Variable(VariableType _type,const string&_name,uint _id,bool _upvalue){
		type=_type;
		name=_name;
		id=_id;
		isUpvalue=_upvalue;
	}
	
	string ToStr(){
		return VT_NAME[type]+" '"+name+"'";
	}
};

struct UpvalInfo{
	UpvalType type;
	uint args;
	UpvalInfo(UpvalType _type,uint _args){
		type=_type;
		args=_args;
	}
	UpvalInfo(){
		type=UT_UNKNOWN;
	}
};

uint usedUniqueId;
struct FnInfo{
	string fnName;
	vector<UpvalInfo> upvalueInfo;
	uint localCnt;
	uint argsCnt;
	CodeSet instr;
	uint uniqueId;
	FnInfo(){}
	FnInfo(const string&_fnName,const map<uint,UpvalInfo>&_upvalueInfo,uint _localCnt,uint _argsCnt,const CodeSet&_instr){
		upvalueInfo.resize(_upvalueInfo.size());
		for(const auto&a:_upvalueInfo){
			upvalueInfo[a.first]=a.second;
		}
		fnName=_fnName;
		localCnt=_localCnt;
		argsCnt=_argsCnt;
		instr=_instr;
		uniqueId=usedUniqueId;
		usedUniqueId++;
	}
	bool operator<(const FnInfo&fnInfo)const{return uniqueId<fnInfo.uniqueId;}
};

Table<string> STRING_CONSTANT;
Table<double> NUMBER_CONSTANT;
Table<FnInfo> FUNC_CONSTANT;

struct SingleScope{
	uint usedLocal;
	uint usedUpvalue;
	
	vector<map<string,uint>> vars;
	map<string,uint> upvalues;
	map<uint,UpvalInfo> upvalueInfo;
	
	SingleScope(){
		usedLocal=0;
		_New();
	}
	Variable _Find(const string&name){
		for(auto scope=vars.rbegin();scope!=vars.rend();scope++){
			if(scope->find(name)!=scope->end()){
				return Variable(VT_LOCAL,name,(*scope)[name]);
			}
		}
		if(upvalues.find(name)!=upvalues.end()){
			return Variable(VT_UPVALUE,name,upvalues[name]); 
		}
		return Variable(VT_UNKNOWN,name,0);
	}
	void _Add(const string&name){
		map<string,uint>&lastScope=vars.back();
		if(lastScope.find(name)==lastScope.end()){
			lastScope[name]=usedLocal; 
			usedLocal++;
		}
	}
	void _AddUpvalue(const string&name,UpvalType type,uint args){
		if(upvalues.find(name)==upvalues.end()){
			upvalues[name]=usedUpvalue;
			upvalueInfo[usedUpvalue]=UpvalInfo(type,args);
			usedUpvalue++;
		}
	}
	void _New(){
		vars.Add(map<string,uint>());
	}
	void _Pop(){
		vars.pop_back();
	}
};

vector<SingleScope> scope;
SingleScope builtinScope;

SingleScope&CurScope(){
	return scope.back();
}

void NewScope(){
	scope.Add(SingleScope());
}
uint PopScope(){
	uint cnt=scope.back().usedLocal;
	scope.pop_back();
	return cnt;
}

Variable Find(int depth,const string&name){
	if(depth==-1)return Variable(VT_UNKNOWN);
	Variable result=builtinScope._Find(name);
	if(result.type!=VT_UNKNOWN){
		result.type=VT_BUILTIN;
		return result;
	}
	
	result=scope[depth]._Find(name);
	if(result.type==VT_UNKNOWN){
		result=Find(depth-1,name);
		switch(result.type){
			default:return result;
			case VT_UPVALUE:{
				scope[depth]._AddUpvalue(name,UT_AT_UPVALUE,result.id);
				return scope[depth]._Find(name);
			}
			case VT_LOCAL:{
				scope[depth]._AddUpvalue(name,UT_AT_LOCAL,result.id);
				return scope[depth]._Find(name);
			}
		}
	} 
	else{
		if(depth==0&&result.type==VT_LOCAL){
			result.type=VT_GLOBAL;
		}
		return result;
	}
}

Variable Find(const string&name){
	return Find(scope.size()-1,name);
}

void DeclareLocalVar(const string&name){
	CurScope()._Add(name);
}

CodeSet LoadVar(const string&name){
	Variable var=Find(name);
	switch(var.type){
		default:{
			DeclareLocalVar(name);
			return LoadVar(name);
		}
		case VT_BUILTIN:{
			return {Instr(LOADBUILTIN,var.id)};
		}
		case VT_GLOBAL:{
			return {Instr(LOADGLOBAL,var.id)};
		}
		case VT_LOCAL:{
			return {Instr(LOADVAR,var.id)};
		}
		case VT_UPVALUE:{
			return {Instr(LOADUPVALUE,var.id)};
		}
	}
}

CodeSet String(const string&str){
	return {Instr(STRSLOT,STRING_CONSTANT.Ensure(stringExpr(str.substr(1,str.size()-2))))};
}
CodeSet Number(double num){
	if(ceil(num)==floor(num)&&num>=0&&num<=65536ll*65536ll){
		return {Instr(SMI,uint(num))};
	}
	else return {Instr(NUMSLOT,NUMBER_CONSTANT.Ensure(num))};
}

#define CP(code1,code2)case code1:context.pop_back();byte=code2;break;

void Opt_JIF(CodeSet&context,uint offset){
	uchar byte=JUMP_IF_FALSE;
	switch(context.back().type){
		CP(SML,JGE)
		CP(BIG,JLE)
		CP(LE,JBIG)
		CP(GE,JSML)
		CP(EQL,JNEQ)
		CP(NEQ,JEQ)
	}
	context.emplace_back(Instr(byte,offset)); 
}
void Opt_LJIF(CodeSet&context,uint offset){
	uchar byte=LJUMP_IF_FALSE;
	switch(context.back().type){
		CP(SML,LJGE)
		CP(BIG,LJLE)
		CP(LE,LJBIG)
		CP(GE,LJSML)
		CP(EQL,LJNEQ)
		CP(NEQ,LJEQ)
	}
	context.emplace_back(Instr(byte,offset)); 
}
void Opt_Expr(CodeSet&context){
	auto last=context.rbegin();
	if((*last).type==POP){
		last++;
		uchar t=(*last).type;
		if(t==STOREGLOBALSMI||t==STORELOCALSMI||t==STOREADDRSMI||t==STOREATTRSMI){
			context.pop_back();
		}
	}
}
CodeSet FnBody(const string&name="<anonymous>");
CodeSet Expr(uint precd){
//	cout<<"Expression("<<precd<<")"<<endl;
	CodeSet c;
	switch(tok.type){
		case TOK_ID:{
			concat(c,LoadVar(tok.val));
			nextToken();
			break;
		}
		case TOK_STR:{
			concat(c,String(tok.val));
			nextToken();
			break;
		}
		case TOK_THIS:{
			c.Add(Instr(LOADTHIS));
			nextToken();
			break;
		}
		case TOK_NUM:{
			concat(c,Number(str2num(tok.val)));
			nextToken();
			break;
		}
		case TOK_HEX:{
			concat(c,Number(hex2dec(tok.val)));
			nextToken();
			break;
		}
		
		#define MATH_UNARY(op,code,symbol)\
		case TOK_##op:{\
			nextToken();\
			concat(c,Expr(0));\
			c.Add(code);\
			break;\
		}
		
		MATH_UNARY(ADD,POSITIVE,"+")
		MATH_UNARY(SUB,NEGATIVE,"-")
		MATH_UNARY(NOT,NOT,"!")
		MATH_UNARY(BITNOT,BITNOT,"~")
		
		#undef MATH_UNARY
		
		// array
		case TOK_LBK:{
			nextToken();
			uint length=0;
			while(tok.type!=TOK_RBK){
				CodeSet element=Expr(0);
				
				concat(c,element);
				length++;
				
				if(tok.type==TOK_COM)nextToken();
				else break;
			}
			
			readToken(TOK_RBK);
			c.Add(Instr(LOADARR,length));
			break;
		}
		// map
		case TOK_LBR:{
			nextToken();
			uint length=0;
			while(tok.type!=TOK_RBR){
				CodeSet key=Expr(0);
				concat(c,key);
				
				readToken(TOK_COL);
				
				CodeSet value=Expr(0);
				
				concat(c,value);
				length++;
				
				if(tok.type==TOK_COM)nextToken();
				else break;
			}
			
			readToken(TOK_RBR);
			c.Add(Instr(MAKEMAP,length));
			break;
		}
		case TOK_TYPEOF:{
			nextToken();
			concat(c,Expr(priority(TOK_TYPEOF)));
			c.Add(TYPEOF);
			break;
		}
		case TOK_MUL:{
			nextToken();
			concat(c,Expr(140));
			c.Add(DEREF);
			break;
		}
		case TOK_BITAND:{
			nextToken();
			concat(c,Expr(140));
			c.Add(GETREF);
			break;
		}
		case TOK_LPR:{
			nextToken();
			concat(c,Expr(0));
			readToken(TOK_RPR);
			break;
		}
		case TOK_FUNC:{
			nextToken();
			concat(c,FnBody());
			break;
		}
		default:{
			SYNTAX_ERR("unexpected '%s', expected an expression",tok.val.c_str());
			break;
		}
	}
//	cout<<"1st operand LT:"<<LT<<endl;
	while(currentTokenIndex<tokens.size()&&priority(tok.type)>precd){
//		cout<<"LT:"<<LT<<endl;
		switch(tok.type){
			#define MATH_OP(op,code,symbol)\
			case TOK_##op:{\
				nextToken();\
				CodeSet rExpr=Expr(priority(TOK_##op));\
				concat(c,rExpr);\
				c.Add(code);\
				break;\
			}
			
			#define MATH_2_OP(op,code,symbol)\
			case TOK_##op:{\
				nextToken();\
				CodeSet rExpr=Expr(priority(TOK_##op));\
				concat(c,rExpr);\
				uint x=rExpr.back().x;\
				if(rExpr.back().type==SMI&&(x>=0&&x<=65535)){\
					c.pop_back();\
					c.Add(Instr(code##E,x));\
				}\
				else c.Add(code);\
				break;\
			}
			
			MATH_2_OP(BITOR,BITOR,"|")
			MATH_2_OP(BITAND,BITAND,"&")
			MATH_2_OP(ADD,ADD,"+")
			MATH_2_OP(XOR,XOR,"^")
			MATH_2_OP(SUB,SUB,"-")
			MATH_2_OP(MUL,MUL,"*")
			MATH_2_OP(DIV,DIV,"/")
			MATH_2_OP(MOD,MOD,"%")
			MATH_2_OP(LSHF,LSHF,"<<")
			MATH_2_OP(RSHF,RSHF,">>")
			MATH_OP(EQL,EQL,"==")
			MATH_OP(NEQ,NEQ,"!=")
			MATH_OP(GE,GE,">=")
			MATH_OP(LE,LE,"<=")
			MATH_OP(BIG,BIG,">")
			MATH_OP(SML,SML,"<")
			
			#undef MATH_OP
			
			// subscript
			case TOK_LBK:{
				nextToken();
				concat(c,Expr(0));
				c.Add(GETADDR);
				readToken(TOK_RBK);
				break;
			}
			
			// logic and
			case TOK_AND:{
				nextToken();
				CodeSet branch=Expr(priority(TOK_AND));
				c.Add(Instr(AND,(uint)branch.size()));
				concat(c,branch);
				break;
			}
			// logic or
			case TOK_OR:{
				nextToken();
				CodeSet branch=Expr(priority(TOK_OR));
				c.Add(Instr(OR,(uint)branch.size()));
				concat(c,branch);
				break;
			}
			case TOK_ASS:{
				nextToken();
				CodeSet v=Expr(priority(TOK_ASS));
				switch(c.back().type){
					#define REP(code1,code2)\
					case code1:{\
						uint x=c.back().x;\
						c.pop_back();\
						concat(c,v);\
						c.Add(Instr(code2,x));\
						break;\
					}
					REP(LOADGLOBAL,STOREGLOBAL)
					REP(LOADVAR,STORELOCAL)
					REP(LOADUPVALUE,STOREUPVALUE) 
					REP(GETADDR,STOREADDR)
					REP(GETATTR,STOREATTR) 
					default:{
						SYNTAX_ERR("invalid left%cvalue in assignment",' ');
						break;
					}
					#undef REP
				}
				break;
			}
			// function call
			case TOK_LPR:{
				nextToken();
				
				uint argsCnt=0;
				CodeSet args;
				while(tok.type!=TOK_RPR){
					CodeSet arg=Expr(0);
					concat(args,arg);
					
					argsCnt++;
					
					if(tok.type==TOK_COM)nextToken();
					else break;
				}
				
				readToken(TOK_RPR);
				switch(c.back().type){
					case GETADDR:{
						c.pop_back();
						concat(c,args);
						c.Add(Instr(INVOKE,argsCnt));
						break;
					}
					default:{
						concat(c,args);
						c.Add(Instr(CALL,argsCnt));
						break;
					}
				}
				break;
			}
			case TOK_DOT:{
				nextToken();
				
				string attrName=nextToken().val;
				if(!isalpha(attrName[0]))SYNTAX_ERR("'%s' is not an valid attribute name",attrName.c_str());
				concat(c,String("'"+attrName+"'"));
				c.Add(Instr(GETADDR));
				break;
			}
			case TOK_RPR:case TOK_RBK:case TOK_RBR:return c;
			case TOK_FEN:nextToken();return c;
			default:return c;
		}
	}
	return c;
}

vector<pair<uint,uint>>loops;
CodeSet Block(bool top=false);
CodeSet Stmt(bool top=false);

void Patch(CodeSet&c){
	CodeSet origin=c;
	c.clear();
	
	// get all labels
	map<uint,uint> labelPos;
	uint ip=0;
	
	for(uint i=0;i<origin.size();i++){
		if(origin[i].type==LABEL){
			labelPos[origin[i].x]=ip;
		}
		else c.emplace_back(origin[i]),ip++;
	}
	ip=0;
	for(auto&a:c){
		switch(a.type){
			#define PATCH(g)\
			case L##g:{\
				uint offset=labelPos[a.x]-ip-1;\
				a=Instr(g,offset);\
				break;\
			}
			PATCH(JUMP)
			PATCH(JUMP_IF_FALSE)
			PATCH(JSML)
			PATCH(JBIG)
			PATCH(JLE)
			PATCH(JGE)
			PATCH(JEQ)
			PATCH(JNEQ)
			case LLOOP:{
				uint offset=ip-labelPos[a.x]+1;
				a=Instr(LOOP,offset);
				break;
			}
		}
		ip++;
	}
}

CodeSet MkIf(){
	readToken(TOK_IF);
	CodeSet s;
	CodeSet condition=Expr(0);
	CodeSet trueBranch=Stmt();
	CodeSet falseBranch=CodeSet();
	
	if(tok.type==TOK_ELSE){
		readToken(TOK_ELSE);
		falseBranch=Stmt();
		trueBranch.emplace_back(Instr(JUMP,(uint)falseBranch.size()));
	}
	
	concat(s,condition);
	
	Opt_JIF(s,(uint)trueBranch.size());
	concat(s,trueBranch);
	concat(s,falseBranch);
	return s;
}
CodeSet MkWhile(){
	readToken(TOK_WHILE);
	CodeSet s;
	Label checkLabel=newLabel(),stepLabel=newLabel(),endLabel=newLabel();
	s.emplace_back(checkLabel);
	concat(s,Expr(0));
	Opt_LJIF(s,endLabel.x);
	loops.emplace_back(make_pair(stepLabel.x,endLabel.x));
	concat(s,Stmt());
	loops.pop_back();
	s.emplace_back(stepLabel);
	concat(s,CodeSet());
	s.emplace_back(Instr(LLOOP,checkLabel.x)); 
	s.emplace_back(endLabel);
	Patch(s);
	
	return s;
}
CodeSet MkFor(){
	CodeSet s;
	readToken(TOK_FOR);
	readToken(TOK_LPR);
	concat(s,Stmt());
	
	Label checkLabel=newLabel(),stepLabel=newLabel(),endLabel=newLabel();
	s.emplace_back(checkLabel);
	concat(s,Expr(0));
	
	CodeSet step=Stmt();
	
	readToken(TOK_RPR);
	s.emplace_back(Instr(LJUMP_IF_FALSE,endLabel.getIntOp()));
	loops.emplace_back(make_pair(stepLabel.getIntOp(),endLabel.getIntOp()));
	concat(s,Stmt());
	loops.pop_back();
	s.emplace_back(stepLabel);
	concat(s,step);
	s.emplace_back(Instr(LLOOP,checkLabel.getIntOp())); 
	s.emplace_back(endLabel);
	Patch(s);
	return s;
}

CodeSet Continue(){
	CodeSet c;
	return c;
}
CodeSet Break(){
	CodeSet c;
	return c;
}

CodeSet FnBody(const string&name){
	NewScope();
	
	readToken(TOK_LPR);
	
	uint argsCnt=0;
	CodeSet body;
	while(tok.type!=TOK_RPR){
		string varName=readToken(TOK_ID).val;
		
		DeclareLocalVar(varName);
		
		// type definition
		if(tok.type==TOK_COL){
			nextToken();
			string typeName=nextToken().val;
			if(!isalpha(typeName[0])){
				SYNTAX_ERR("'%s' is not a valid type name",typeName.c_str());
			}
			body.Add(Instr(CHECKTYPE,argsCnt));
			body.Add(Instr(CHECKTYPE,STRING_CONSTANT.Ensure(typeName)));
		}
		argsCnt++;
		
		if(tok.type==TOK_COM)nextToken();
		else break;
	}
	
	readToken(TOK_RPR);
	
	readToken(TOK_LBR);
	concat(body,Block(false));
	readToken(TOK_RBR);
	
	SingleScope scope=CurScope();
	PopScope();
	uint id=FUNC_CONSTANT.Ensure(FnInfo(name,scope.upvalueInfo,scope.usedLocal,argsCnt,body));
	
	CodeSet s;
	s.Add(Instr(FUNCSLOT,id));
	return s;
}

CodeSet ClassDeclaration(){
	nextToken();
	
	CodeSet s;
	return s;
}

CodeSet Stmt(bool top){
	CodeSet c;
	switch(tok.type){
		case TOK_FUNC:{
			nextToken();
			string fnName=readToken(TOK_ID).val;
			concat(c,LoadVar(fnName));
			CodeSet v=FnBody(fnName);
			switch(c.back().type){
				#define REP(code1,code2)\
				case code1:{\
					uint x=c.back().x;\
					c.pop_back();\
					concat(c,v);\
					c.Add(Instr(code2,x));\
					break;\
				}
				REP(LOADGLOBAL,STOREGLOBAL)
				REP(LOADVAR,STORELOCAL)
				REP(GETADDR,STOREADDR)
				REP(GETATTR,STOREATTR)
				default:{
					SYNTAX_ERR("invalid left%cvalue in assignment",' ');
					break;
				}
				#undef REP
			}
			c.Add(Instr(POP));
			break;
		}
		case TOK_IF:concat(c,MkIf());break;
		case TOK_WHILE:concat(c,MkWhile());break;
		case TOK_FOR:concat(c,MkFor());break;
		case TOK_BREAK:concat(c,Break());break;
		case TOK_CTN:concat(c,Continue());break;
		case TOK_LBR:nextToken();concat(c,Block());readToken(TOK_RBR);break;
		case TOK_FEN:nextToken();break;
		case TOK_RET:{
			nextToken();
			if(tok.type==TOK_FEN)c.emplace_back(Instr(LOADNULL));
			else c=Expr(0);
			c.emplace_back(Instr(RETURN));
			break;
		}
		case TOK_CLASS:{
			concat(c,ClassDeclaration());
			break;
		}
		default:concat(c,Expr(0));c.emplace_back((top&&cliMode)?PRINT:POP);Opt_Expr(c);break;
	}
	return c;
}

CodeSet Block(bool top){
	CodeSet c;
	CurScope()._New();
	while(!EOT()&&tok.type!=TOK_RBR){
		concat(c,Stmt(top));
	}
	CurScope()._Pop();
	return c;
}
CodeSet CliBlock(bool top){
	CodeSet c;
	while(!EOT()&&tok.type!=TOK_RBR){
		concat(c,Stmt(top));
	}
	return c;
}

void InitCompiler(){
	builtinScope._Add("print");
	builtinScope._Add("GC");
	builtinScope._Add("json");
	builtinScope._Add("open");
	builtinScope._Add("close");
	builtinScope._Add("eof");
	builtinScope._Add("read");
	builtinScope._Add("readLine");
	builtinScope._Add("readNumber");
	builtinScope._Add("readString");
	builtinScope._Add("readChar");
	builtinScope._Add("write");
	builtinScope._Add("sin");
	builtinScope._Add("asin");
	builtinScope._Add("cos");
	builtinScope._Add("acos");
	builtinScope._Add("tan");
	builtinScope._Add("atan");
	builtinScope._Add("atan2");
	builtinScope._Add("abs");
	builtinScope._Add("ceil");
	builtinScope._Add("floor");
	builtinScope._Add("round");
	builtinScope._Add("exp");
	builtinScope._Add("log");
	builtinScope._Add("pow");
	builtinScope._Add("sqrt");
	builtinScope._Add("max");
	builtinScope._Add("min");
	builtinScope._Add("random");
	builtinScope._Add("isNan");
	builtinScope._Add("toDeg");
	builtinScope._Add("toRad");
	builtinScope._Add("PI");
	builtinScope._Add("loadNative");
	builtinScope._Add("exit");
	builtinScope._Add("system");
	builtinScope._Add("clock");
	builtinScope._Add("__args__");
	builtinScope._Add("ascii");
	builtinScope._Add("char");
	builtinScope._Add("toNumber");
}

}
using namespace Compiler;


void GC();

struct rbq_env;
rbq_env* env;

namespace VirtualMachine{

double* NUMBER_CONSTANT_VALUE;
string* STRING_CONSTANT_VALUE;

struct Object;
struct Value;
struct Fn;

typedef Object* ObjectRef;
typedef Value* ValueRef;
typedef Value* RunStack;
RunStack globalEsp;
string Val2Str(const ValueRef&val);
string StrictVal2Str(const ValueRef&val);
string ValType2Str(const ValueRef&val);
Object* NewString(const string&str);
Object* NewArray(uint size);
Value RunCode(RunStack bottom,RunStack esp,Fn* fn,ValueRef thisObject);
Value GetAttribute(const ValueRef&val,const string&key);
Value InnerCall(ValueRef fn,ValueRef arg,ValueRef thisObject);
bool HasAttribute(const ValueRef&val,const string&key);
typedef Value (*NativeFunction)(rbq_env* env,ValueRef thisObject,Value* argv,int argc);

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

const string TYPE_NAME[]={
	"undefined",
	"null",
	"number",
	"string",
	"array",
	"object",
	"function",
	"pointer",
};

struct Value{
	ValueType type;
	union{
		double num;
		ObjectRef obj;
		void* ptr;
	};
	string ToStr()const{
		return Val2Str((const ValueRef)this);
	}
	string GetTypeName()const{
		return ValType2Str((const ValueRef)this);
	}
	Value(){
		type=TYPE_UNDEF;
	}
	Value(double _num){
		type=TYPE_NUM;
		num=_num;
	}
	Value(const string&str){
		type=TYPE_STR;
		obj=NewString(str);
	}
	Value(void* _ptr){
	    type=TYPE_PTR;
	    ptr=_ptr;
	}
	#define CHECK_OP(symbol,attr)\
	if(HasAttribute((const ValueRef)this,(string)#attr)){\
	   const Value&func=GetAttribute((const ValueRef)this,(string)#attr);\
	   if(func.type==TYPE_FUNC)return InnerCall((ValueRef)&func,(ValueRef)&v,(const ValueRef)this); \
	}\
	THROW("TypeError",FORMAT("cannot apply operation '%s %s' to type '%s'",#symbol,v.GetTypeName().c_str(),GetTypeName().c_str()))
	
	#define CHECK_BOOL_OP(symbol,attr)\
	if(HasAttribute((const ValueRef)this,(string)#attr)){\
	   const Value&func=GetAttribute((const ValueRef)this,(string)#attr);\
	   if(func.type==TYPE_FUNC)return InnerCall((ValueRef)&func,(ValueRef)&v,(const ValueRef)this).IsTrue(); \
	}\
	THROW("TypeError",FORMAT("cannot apply operation '%s %s' to type '%s'",#symbol,v.GetTypeName().c_str(),GetTypeName().c_str()))
	
	Value operator+(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return num+v.num;
		if(!HasAttribute((const ValueRef)this,(string)"__add__"))if(type==TYPE_STR||v.type==TYPE_STR)return ToStr()+v.ToStr();
        CHECK_OP(+,__add__);
		return ToStr()+v.ToStr();
	}
	Value operator-(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return num-v.num;
        CHECK_OP(-,__sub__);
		return (string)"[bad: -]";
	}
	Value operator*(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return num*v.num;
        CHECK_OP(*,__mul__);
		return (string)"[bad: *]";
	}
	Value operator/(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return num/v.num;
        CHECK_OP(/,__div__);
		return (string)"[bad: /]";
	}
	Value operator%(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return fmod(num,v.num);
        CHECK_OP(%,__mod__);
		return (string)"[bad: %]";
	}
	Value operator&(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return int64_t(num)&int64_t(v.num);
        CHECK_OP(&,__bitand__);
		return (string)"[bad: &]";
	}
	Value operator|(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return int64_t(num)|int64_t(v.num);
        CHECK_OP(|,__bitor__);
		return (string)"[bad: |]";
	}
	Value operator^(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return int64_t(num)^int64_t(v.num);
        CHECK_OP(^,__xor__);
		return (string)"[bad: ^]";
	}
	Value operator~()const{
		if(type==TYPE_NUM)return ~int64_t(num);
		return (string)"[bad: ~]";
	}
	Value operator<<(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return int64_t(num)<<int64_t(v.num);
        CHECK_OP(<<,__lsht__);
		return (string)"[bad: <<]";
	}
	Value operator>>(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return int64_t(num)>>int64_t(v.num);
        CHECK_OP(>>,__rsht__);
		return (string)"[bad: >>]";
	}
	bool operator>(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return num>v.num;
		if(type==TYPE_STR||v.type==TYPE_STR)return ToStr()>v.ToStr();
        CHECK_BOOL_OP(>,__big__);
		return false;
	}
	bool operator<(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return num<v.num;
		if(type==TYPE_STR||v.type==TYPE_STR)return ToStr()<v.ToStr();
        CHECK_BOOL_OP(<,__sml__);
		return false;
	}
	bool operator>=(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return num>=v.num;
		if(type==TYPE_STR||v.type==TYPE_STR)return ToStr()>=v.ToStr();
        CHECK_BOOL_OP(>=,__bigeql__);
		return false;
	}
	bool operator<=(const Value&v)const{
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return num<=v.num;
		if(type==TYPE_STR||v.type==TYPE_STR)return ToStr()<=v.ToStr();
        CHECK_BOOL_OP(<,__smleql__);
		return false;
	}
	bool operator==(const Value&v)const{
		if(type!=v.type)return false;
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return num==v.num;
		if(type==TYPE_STR&&v.type==TYPE_STR)return ToStr()==v.ToStr();
        CHECK_BOOL_OP(==,__eql__);
		return obj==v.obj;
	}
	bool operator!=(const Value&v)const{
		if(type!=v.type)return true;
		if(type==TYPE_NUM&&v.type==TYPE_NUM)return num!=v.num;
		if(type==TYPE_STR&&v.type==TYPE_STR)return ToStr()!=v.ToStr();
        CHECK_BOOL_OP(!=,__neq__);
		return obj!=v.obj;
	}
	bool IsTrue()const{
		switch(type){
			case TYPE_NUM:return num;
			case TYPE_NULL:return false;
			case TYPE_UNDEF:return false;
			default:return true;
		}
	}
};

Value ENV=Value();

struct Object{
	ValueType type;
	Object(){isBlack=released=false;}
	Object(ValueType _type){
		isBlack=released=false;
		type=_type;
	}
	union{
		string* str;
		vector<Value>* arr;
		map<Value,Value>* dict;
		Fn* fn;
	};
	bool isBlack,released;
};

typedef Value (*Primitive)(ValueRef thisObject,RunStack args,uint argc); 
typedef enum{
	FN_UNKNOWN=0,
	FN_SCRIPT,
	FN_PRIM,
	FN_NATIVE,
} FnType;

struct Upv{
	Value _value;
	ValueRef value;
};

struct Fn{
	FnType type;
	Upv* upvalues;
	union{
		NativeFunction func;
		Primitive prim;
		FnInfo* info;
	};
	Fn(){}
	Fn(Primitive _prim){
		prim=_prim;
		upvalues=NULL;
		type=FN_PRIM;
	}
	Fn(NativeFunction _func){
		func=_func;
		upvalues=NULL;
		type=FN_NATIVE;
	}
	Fn(FnInfo* _info){
		info=_info;
		upvalues=NULL;
		type=FN_SCRIPT;
	}
	Value CallFunc(RunStack args,uint argc,RunStack esp,ValueRef thisObject){
		switch(type){
			case FN_NATIVE:{
				return func(env,thisObject,args,argc);
			}
			case FN_PRIM:{
				return prim(thisObject,args,argc);
			}
			case FN_SCRIPT:{
				if(argc>info->argsCnt)THROW("ArgumentsError",FORMAT("more than %d argument(s) provided",info->argsCnt));
				
				RunStack bottom=esp;
				for(uint i=0;i<info->localCnt;i++){
					if(i<argc){
						*(esp+i)=args[i];
					}
					else{
						*(esp+1)=Value();
					}
				}
				
				return RunCode(bottom,bottom+info->localCnt+1,this,thisObject);
			}
			default:{
				THROW("SystemError",FORMAT("unknown function at %p is called",this));
				return Value();
			}
		}
	}
};

Value JSON(const string&str);

vector<ObjectRef> OBJ_POOL,OBJ_POOL_2;

uint GC_TRIGGER=1024*64;
double GROW_FACTOR=1;
uint ALLOCATED;

#define ADD_MEM(type)\
ALLOCATED+=sizeof(type)

ObjectRef NewString(const string&str){
	if(ALLOCATED>=GC_TRIGGER)GC();
	
	ObjectRef obj=new Object(TYPE_STR);
	OBJ_POOL.Add(obj);
	obj->str=new string(str);
	
	ADD_MEM(string);
	
	return obj;
}

ObjectRef NewFunc(FnInfo* info){
	if(ALLOCATED>=GC_TRIGGER)GC();
	ObjectRef obj=new Object(TYPE_FUNC);
	OBJ_POOL.Add(obj);
	obj->fn=new Fn(info);
	
	ADD_MEM(Fn);
	
	return obj;
}

ObjectRef NewBuiltinFunc(Primitive prim){
	if(ALLOCATED>=GC_TRIGGER)GC();
	ObjectRef obj=new Object(TYPE_FUNC);
	OBJ_POOL.Add(obj);
	obj->fn=new Fn(prim);
	
	ADD_MEM(Fn);
	
	return obj;
}

ObjectRef NewNativeFunc(NativeFunction native){
	if(ALLOCATED>=GC_TRIGGER)GC();
	ObjectRef obj=new Object(TYPE_FUNC);
	OBJ_POOL.Add(obj);
	obj->fn=new Fn(native);
	
	ADD_MEM(Fn);
	
	return obj;
}

ObjectRef NewArray(uint size){
	if(ALLOCATED>=GC_TRIGGER)GC();
	ObjectRef obj=new Object(TYPE_ARR);
	OBJ_POOL.Add(obj);
	obj->arr=new vector<Value>(size);
	
	ADD_MEM(vector<Value>);
	
	return obj;
}

ObjectRef NewMap(){
	if(ALLOCATED>=GC_TRIGGER)GC();
	ObjectRef obj=new Object(TYPE_MAP);
	OBJ_POOL.Add(obj);
	obj->dict=new map<Value,Value>();
	
	ALLOCATED+=sizeof(map<Value,Value>);
	
	return obj;
}

set<void*> inStack;

string array2str(vector<Value>*arr){
	if(inStack.find((void*)arr)!=inStack.end())return "...";
	inStack.insert((void*)arr);
	
	string ret="[";
	bool first=false;
	
	for(const auto&a:*arr){
		if(!first)first=true;
		else ret+=", ";
		
		ret+=StrictVal2Str((const ValueRef)&a);
	}
	inStack.erase(inStack.find((void*)arr));
	return ret+"]";
}

string map2str(map<Value,Value>*dict){
	if(inStack.find((void*)dict)!=inStack.end())return "...";
	inStack.insert((void*)dict);
	
	string ret="{";
	bool first=false;
	
	for(const auto&a:*dict){
		if(!first)first=true;
		else ret+=", ";
		
		ret+=StrictVal2Str((const ValueRef)&a.first);
		ret+=": ";
		ret+=StrictVal2Str((const ValueRef)&a.second);
	}
	inStack.erase(inStack.find((void*)dict));
	return ret+"}";
}
		
string Val2Str(const ValueRef&val){
	switch(val->type){
		case TYPE_NUM:return num2str(val->num);
		case TYPE_STR:return *(val->obj->str);
		case TYPE_ARR:return array2str(val->obj->arr); 
		case TYPE_MAP:return map2str(val->obj->dict); 
		case TYPE_PTR:return "(ptr @ "+ptr2str(val->ptr)+")";
		default:return TYPE_NAME[val->type];
	}
}	
	
string StrictVal2Str(const ValueRef&val){
	switch(val->type){
		case TYPE_NUM:return num2str(val->num);
		case TYPE_STR:return strictStr(*(val->obj->str));
		case TYPE_ARR:return array2str(val->obj->arr); 
		case TYPE_MAP:return map2str(val->obj->dict); 
		case TYPE_PTR:return "(ptr @ "+ptr2str(val->ptr)+")";
		default:return TYPE_NAME[val->type];
	}
}

string ValType2Str(const ValueRef&val){
	if(HasAttribute(val,"__type__"))return GetAttribute(val,"__type__").ToStr();
	return TYPE_NAME[val->type]; 
}

struct Frame{
	Fn* fn;
	RunStack bottom; 
};

uint GLB_CNT;
uint BUILTIN_CNT;
Value* GLBV;

#define BUILTIN_ATTRIBUTES(klass,count)\
Table<string> klass##_ATTR;\
Value* klass##_ATTR_FN;\
const uint klass##_ATTR_COUNT=count;
BUILTIN_ATTRIBUTES(STRING,13)
BUILTIN_ATTRIBUTES(ARRAY,9)
BUILTIN_ATTRIBUTES(MAP,4)

Value* BUILTIN_VAR;
vector<Frame> CALL_STACK;

Value* STACK;

void DebugOutput(RunStack bottom,RunStack esp,const Instr&instr){
	cout<<instr<<endl;
	for(auto a=bottom;a!=esp;a++){
		cout<<"["<<a<<"] "<<a->ToStr()<<endl;
	}
	system("pause > nul");
}

Value GetBuiltinAttribute(const Value&a,const string&attrName){
	switch(a.type){
		case TYPE_MAP:{
			if(MAP_ATTR.Has(attrName))return MAP_ATTR_FN[MAP_ATTR.Id(attrName)];
			THROW("type 'map' does not own any attribute named '%s'",attrName.c_str());
			break; 
		}
		case TYPE_STR:{
			if(STRING_ATTR.Has(attrName))return STRING_ATTR_FN[STRING_ATTR.Id(attrName)];
			THROW("type 'string' does not own any attribute named '%s'",attrName.c_str());
			break; 
		}
		case TYPE_ARR:{
			if(ARRAY_ATTR.Has(attrName))return ARRAY_ATTR_FN[ARRAY_ATTR.Id(attrName)];
			THROW("type 'array' does not own any attribute named '%s'",attrName.c_str());
			break; 
		}
		default:return Value();
	}
	return Value();
}

bool HasBuiltinAttribute(const Value&a,const string&attrName){
	switch(a.type){
		case TYPE_MAP:return MAP_ATTR.Has(attrName);
		case TYPE_STR:return STRING_ATTR.Has(attrName);
		case TYPE_ARR:return ARRAY_ATTR.Has(attrName);
		default:return false;
	}
}

Value GetAttribute(const ValueRef&a,const string&attr){
	if(HasBuiltinAttribute(*a,attr))return GetBuiltinAttribute(*a,attr);
	if(a->type!=TYPE_MAP)THROW("AttributeError",FORMAT("type '%s' does not own a '%s' attribute",a->GetTypeName().c_str(),attr.c_str()));
	return (*a->obj->dict)[attr];
}
bool HasAttribute(const ValueRef&a,const string&attr){
	if(HasBuiltinAttribute(*a,attr))return true;
	return a->type==TYPE_MAP&&(a->obj->dict->find(attr)!=a->obj->dict->end());
}

string GetStringIndex(const string&str,const Value&value){
	switch(value.type){
		case TYPE_NUM:{
			int index=(int)value.num;
			if(abs(index)>=str.size())THROW("IndexError",FORMAT("index %d out of string bounds",index));
			if(index<0)index+=str.size();
			return (string)""+str[index];
		}
		default:{
			THROW("IndexError",FORMAT("type '%s' cannot be used as string indices",value.GetTypeName().c_str()));
			return "";
		}
	}
}

Value GetArrayIndex(vector<Value>*arr,const Value&value){
	switch(value.type){
		case TYPE_NUM:{
			int index=(int)value.num;
			if(abs(index)>=arr->size())THROW("IndexError",FORMAT("index %d out of array bounds",index));
			if(index<0)index+=arr->size();
			return (*arr)[index];
		}
		default:{
			THROW("IndexError",FORMAT("type '%s' cannot be used as array indices",value.GetTypeName().c_str()));
			return Value();
		}
	}
}

void SetupUpvalues(Fn* fn){
	Frame&father=CALL_STACK.back();
	
	// Setup upvalues
	vector<UpvalInfo>&upvs=fn->info->upvalueInfo;
	uint upvalueCnt=upvs.size();
	fn->upvalues=new Upv[upvalueCnt];
	Upv* realUpv=fn->upvalues;
	for(uint i=0;i<upvalueCnt;i++){
		switch(upvs[i].type){
			case UT_AT_LOCAL:{
				realUpv[i].value=&father.bottom[upvs[i].args];
				break;
			}
			case UT_AT_UPVALUE:{
				realUpv[i].value=father.fn->upvalues[upvs[i].args].value;
				break;
			}
			default:{
				THROW("SystemError",FORMAT("unknown upvalue type (id=%d)",upvs[i].type));
				break;
			}
		}
	}

}

void CloseUpvalues(Fn* fn){
	vector<UpvalInfo>&upvs=fn->info->upvalueInfo;
	Upv* realUpv=fn->upvalues;
	for(uint i=0;i<upvs.size();i++){
		realUpv[i]._value=*realUpv[i].value;
		realUpv[i].value=&realUpv[i]._value;
	}
}

#define INNER_PUSH(value) ((*globalEsp)=(value),globalEsp++)
Value InnerCall(ValueRef fn,ValueRef arg,ValueRef thisObject){
	RunStack old=globalEsp;
	INNER_PUSH(*arg);
	
	Value ret=fn->obj->fn->CallFunc(globalEsp-1,1,globalEsp,thisObject);
	
	globalEsp=old;
	return ret;
}
Value InnerCall(Fn* fn,Value arg,ValueRef thisObject){
	RunStack old=globalEsp;
	INNER_PUSH(arg);
	
	Value ret=fn->CallFunc(globalEsp-1,1,globalEsp,thisObject);
	
	globalEsp=old;
	return ret;
}
Value InnerCall(Fn* fn,Value arg1,Value arg2,ValueRef thisObject){
	RunStack old=globalEsp;
	INNER_PUSH(arg1);
	INNER_PUSH(arg2);
	Value ret=fn->CallFunc(globalEsp-2,2,globalEsp,thisObject);
	globalEsp=old;
	return ret;
}

Value InnerCall(Fn* fn,Value*args,uint argc,ValueRef thisObject){
	RunStack old=globalEsp;
	for(uint i=0;i<argc;i++)INNER_PUSH(args[i]);
	
	Value ret=fn->CallFunc(globalEsp-argc,argc,globalEsp,thisObject);
	
	globalEsp=old;
	return ret;
}

Value RunCode(RunStack bottom,RunStack esp,Fn* fn,ValueRef thisObject){
	Value retVal;
	
	const CodeSet&instr=fn->info->instr;
	Upv* realUpv=fn->upvalues;
	CALL_STACK.Add((Frame){fn,bottom});
	vector<Fn*> CreatedFunctions; 
	
	
	uint size=instr.size();
	uint ip=0;
	RunStack _bottom=esp;
	
	#define BACK()\
	{\
		ip++;\
		globalEsp=esp;\
		goto LOOP_START;\
	}
	
	#define PUSH(value) ((*esp)=(value),esp++)
	#define PUSH_NUM(value) (PUSH(NUM2PTR(value)))
	#define POPSTACK()	(esp--)
	#define POP_SLOTS(x)(esp-=x)
	#define TOP()		(*(esp-1))
	#define TOP_1()		(*(esp-1))
	#define TOP_2()		(*(esp-2))
	#define TOP_3()		(*(esp-3))
	#define SEEK(argc)	(*(esp-(argc)))
	#define LOCAL(x)	(bottom[x])
	
	#define SEEK_EXTENDED_ARGS(argc)	(instr[++ip].x)
	while(1){
		LOOP_START:
		if(ip>=size)break;
		const Instr&ins=instr[ip];
		switch(ins.type){
			default:{
				cout<<"unknown instr at "<<ip<<endl;
				cout<<ins<<endl;
				break;
			}
			case STRSLOT:{
				PUSH(Value(STRING_CONSTANT_VALUE[ins.x]));
				BACK();
			}
			case NUMSLOT:{
				PUSH(Value(NUMBER_CONSTANT_VALUE[ins.x]));
				BACK();
			}
			case GETADDRSMI:{
				switch(TOP().type){
					case TYPE_STR:{
						TOP()=string("")+(*(TOP().obj->str))[ins.x];
						BACK();
					}
					case TYPE_ARR:{
						TOP()=((*(TOP().obj->arr))[ins.x]);
						BACK();
					}
					default:THROW("IndexError",FORMAT("type '%s' is not index-able",TOP().GetTypeName().c_str()));
				}
			}
			case GETADDR:{
				if(TOP().type==TYPE_STR&&HasBuiltinAttribute(TOP_2(),*TOP().obj->str)){
					TOP_2()=GetBuiltinAttribute(TOP_2(),*TOP().obj->str); 
					POPSTACK();
					BACK();
				}
				else switch(TOP_2().type){
					case TYPE_STR:{
						TOP_2()=GetStringIndex((*(TOP_2().obj->str)),TOP());
						POPSTACK();
						BACK();
					}
					case TYPE_ARR:{
						TOP_2()=GetArrayIndex((TOP_2().obj->arr),TOP());
						POPSTACK();
						BACK();
					}
					case TYPE_MAP:{
						TOP_2()=((*(TOP_2().obj->dict))[TOP()]);
						POPSTACK();
						BACK();
					}
					default:THROW("IndexError",FORMAT("type '%s' is not index-able",TOP_2().GetTypeName().c_str()));
				}
			}
			case INVOKE:{
				uint argc=ins.x;
				
				ValueRef owner=&SEEK(argc+2);
				Value index=SEEK(argc+1);
				Value func;
				if(index.type==TYPE_STR&&HasBuiltinAttribute(*owner,*index.obj->str)){
					func=GetBuiltinAttribute(*owner,*index.obj->str); 
				}
				else switch(owner->type){
					case TYPE_STR:{
						func=GetStringIndex((*(owner->obj->str)),index);
						break;
					}
					case TYPE_ARR:{
						func=GetArrayIndex((owner->obj->arr),index);
						break;
					}
					case TYPE_MAP:{
						func=((*(owner->obj->dict))[index]);
						break;
					}
					default:THROW("AttributeError",FORMAT("type '%s' does not own methods",owner->GetTypeName().c_str()));
				}
				
				if(func.type!=TYPE_FUNC){
					THROW("TypeError",FORMAT("type '%s' is not a method",func.GetTypeName().c_str()));
				}
				
				Value value=func.obj->fn->CallFunc(esp-argc,argc,esp,owner);
				POP_SLOTS(argc+1);
				TOP()=value;
				BACK();
			}
			case STOREADDR:{
				switch(TOP_3().type){
					case TYPE_ARR:{
						((*(TOP_3().obj->arr))[TOP_2().num])=TOP_1();
						TOP_3()=TOP_1(); POP_SLOTS(2); 
						BACK();
					}
					case TYPE_MAP:{
						((*(TOP_3().obj->dict))[TOP_2()])=TOP_1();
						TOP_3()=TOP_1(); POP_SLOTS(2); 
						BACK();
					}
					default:THROW("IndexError",FORMAT("type '%s' is not index-able or is not editable",TOP().GetTypeName().c_str()));
				}
				BACK();
			}
			case LOADARR:{
				Value newArr;
				newArr.type=TYPE_ARR;
				newArr.obj=NewArray(ins.x);
				vector<Value>* arr=newArr.obj->arr;
				uint i=0;
				for(auto ptr=esp-ins.x;ptr!=esp;ptr++){
					(*arr)[i]=*ptr;
					i++;
				}
				esp-=ins.x;
				PUSH(newArr);
				BACK();
			}
			case MAKEMAP:{
				Value newDict;
				newDict.type=TYPE_MAP;
				newDict.obj=NewMap();
				map<Value,Value>* dict=newDict.obj->dict;
				for(auto ptr=esp-ins.x*2;ptr!=esp;ptr+=2){
					(*dict)[*(ptr)]=*(ptr+1);
				}
				esp-=ins.x*2;
				PUSH(newDict);
				BACK();
			}
			case SMI:{
				PUSH(Value(ins.x));
				BACK();
			}
			case LOADTHIS:{
				PUSH(*thisObject);
				BACK();
			}
			case LOADGLOBAL:{
				PUSH(GLBV[ins.x]);
				BACK();
			}
			case LOADVAR:{
				PUSH(bottom[ins.x]);
				BACK();
			}
			case LOADUPVALUE:{
				PUSH(*(realUpv[ins.x].value));
				BACK();
			}
			case LOADBUILTIN:{
				PUSH(BUILTIN_VAR[ins.x]);
				BACK();
			}
			case STOREGLOBALSMI:{
				GLBV[ins._o[0]]=Value(ins._o[1]);
				BACK();
			}
			case STOREGLOBAL:{
				GLBV[ins.x]=TOP_1();
				BACK();
			}
			case STORELOCAL:{
				bottom[ins.x]=TOP_1();
				BACK();
			}
			case STOREUPVALUE:{
				*(realUpv[ins.x].value)=TOP_1();
				BACK();
			}
			case ADD:{
				TOP_2()=TOP_2()+TOP_1(); POPSTACK();
				BACK();
			}
			case SUB:{
				TOP_2()=TOP_2()-TOP_1(); POPSTACK();
				BACK();
			}
			case MUL:{
				TOP_2()=TOP_2()*TOP_1(); POPSTACK();
				BACK();
			}
			case DIV:{
				TOP_2()=TOP_2()/TOP_1(); POPSTACK();
				BACK();
			}
			case MOD:{
				TOP_2()=TOP_2()%TOP_1(); POPSTACK();
				BACK();
			}
			case BITAND:{
				TOP_2()=TOP_2()&TOP_1(); POPSTACK();
				BACK();
			}
			case BITOR:{
				TOP_2()=TOP_2()|TOP_1(); POPSTACK();
				BACK();
			}
			case BITNOT:{
				TOP()=~TOP();
				BACK();
			}
			case LSHF:{
				TOP_2()=TOP_2()<<TOP_1(); POPSTACK();
				BACK();
			}
			case RSHF:{
				TOP_2()=TOP_2()>>TOP_1(); POPSTACK();
				BACK();
			}
			case XOR:{
				TOP_2()=TOP_2()^TOP_1(); POPSTACK();
				BACK();
			}
			case ADDE:{
				TOP_1()=TOP_1()+Value(ins.x);
				BACK();
			}
			case SUBE:{
				TOP_1()=TOP_1()-Value(ins.x);
				BACK();
			}
			case MULE:{
				TOP_1()=TOP_1()*Value(ins.x);
				BACK();
			}
			case DIVE:{
				TOP_1()=TOP_1()/Value(ins.x);
				BACK();
			}
			case MODE:{
				TOP_1()=TOP_1()%Value(ins.x);
				BACK();
			}
			case BITANDE:{
				TOP_1()=TOP_1()&Value(ins.x);
				BACK();
			}
			case BITORE:{
				TOP_1()=TOP_1()|Value(ins.x);
				BACK();
			}
			case LSHFE:{
				TOP_1()=TOP_1()<<Value(ins.x);
				BACK();
			}
			case RSHFE:{
				TOP_1()=TOP_1()>>Value(ins.x);
				BACK();
			}
			case XORE:{
				TOP_1()=TOP_1()^Value(ins.x);
				BACK();
			}
			case SML:{
				TOP_2()=(TOP_2()<TOP_1()); POPSTACK();
				BACK();
			}
			case BIG:{
				TOP_2()=(TOP_2()>TOP_1()); POPSTACK();
				BACK();
			}
			case LE:{
				TOP_2()=(TOP_2()<=TOP_1()); POPSTACK();
				BACK();
			}
			case GE:{
				TOP_2()=(TOP_2()>=TOP_1()); POPSTACK();
				BACK();
			}
			case EQL:{
				TOP_2()=(TOP_2()==TOP_1()); POPSTACK();
				BACK();
			}
			case NEQ:{
				TOP_2()=(TOP_2()!=TOP_1()); POPSTACK();
				BACK();
			}
			case POP:{
				POPSTACK();
				BACK();
			}
			case AND:{
				if(!TOP().IsTrue())ip+=ins.x;
				POPSTACK();
				BACK();
			}
			case OR:{
				if(TOP().IsTrue())ip+=ins.x;
				POPSTACK();
				BACK();
			}
			case NOT:{
				TOP()=!(TOP().IsTrue());
				BACK();
			}
			case JSML:{
				if((TOP_2()<TOP_1()))ip+=ins.x;
				POP_SLOTS(2);
				BACK();
			}
			case JBIG:{
				if((TOP_2()>TOP_1()))ip+=ins.x;
				POP_SLOTS(2);
				BACK();
			}
			case JLE:{
				if((TOP_2()<=TOP_1()))ip+=ins.x;
				POP_SLOTS(2);
				BACK();
			}
			case JGE:{
				if((TOP_2()>=TOP_1()))ip+=ins.x;
				POP_SLOTS(2);
				BACK();
			}
			case JEQ:{
				if((TOP_2()==TOP_1()))ip+=ins.x;
				POP_SLOTS(2);
				BACK();
			}
			case JNEQ:{
				if((TOP_2()!=TOP_1()))ip+=ins.x;
				POP_SLOTS(2);
				BACK();
			}
			case JUMP_IF_FALSE:{
				if(!TOP().IsTrue())ip+=ins.x;
				POPSTACK();
				BACK();
			}
			case JUMP:{
				ip+=ins.x;
				BACK();
			}
			case LOOP:{
				ip-=ins.x;
				BACK();
			}
			case FUNCSLOT:{
				Value value;
				value.type=TYPE_FUNC;
				value.obj=NewFunc(&FUNC_CONSTANT.Get(ins.x));
				SetupUpvalues(value.obj->fn);
				CreatedFunctions.Add(value.obj->fn);
				PUSH(value);
				BACK();
			}
			case CALL:{
				uint argc=ins.x;
				if(SEEK(argc+1).type!=TYPE_FUNC){
					THROW("TypeError",FORMAT("type '%s' is not callable",SEEK(argc+1).GetTypeName().c_str()));
				}
				Value value=SEEK(argc+1).obj->fn->CallFunc(esp-argc,argc,esp,&ENV);
				POP_SLOTS(argc);
				TOP()=value;
				BACK();
			}
			case PRINT:{
				cout<<TOP().ToStr()<<endl;
				POPSTACK();
				BACK();
			}
			case TYPEOF:{
				TOP()=TOP().GetTypeName();
				BACK();
			}
			case CHECKTYPE:{
				uint argnum=ins.x;
				ip++;
				uint typeId=ins.x;
				if(STRING_CONSTANT_VALUE[typeId]!=LOCAL(argnum).GetTypeName()){
					THROW("TypeError",FORMAT("expected type '%s' for argument %d, given: '%s'",STRING_CONSTANT_VALUE[typeId].c_str(),argnum+1,LOCAL(argnum).GetTypeName().c_str()));
				}
				BACK();
			}
			case RETURN:{
				retVal=TOP();
				goto RETURN_STAGE;
				BACK();
			}
		}
	}
	RETURN_STAGE:
	
	for(auto a:CreatedFunctions){
		CloseUpvalues(a);
	}
	
	return retVal;
}

namespace JSONParser{
	vector<Token> tokens;
	uint curIndex;
	const Token&Cur(){
		if(curIndex>tokens.size())return END_TOKEN;
		return tokens[curIndex];
	}
	const Token&Read(){
		return tokens[curIndex++];
	}
	const Token&Read(uchar tokenType){
		const Token&cur=Cur();
		if(cur.type!=tokenType){
			THROW("JSONError",FORMAT("unexpected '%s' in json at line %d, column %d, expected '%s'",cur.val.c_str(),cur.line,cur.column,tokenName[tokenType].c_str()));
		}
		return tokens[curIndex++];
	}
	
	void JSONTokenize(const string&src){
		uint line=1,column=0;
		uint len=src.size();
		uint loc=0;
		auto nextchar=[&]{
			line+=src[loc]=='\n';
			column=src[loc]=='\n'?1:column+(src[loc]=='\t'?4:1);
			loc++;
		};
		while(loc<len){
			Token cur;
			#define CHECK_SYMBOL(sym,tok)\
			if(src[loc]==sym){\
				nextchar();\
				cur.type=TOK_##tok;\
				cur.val.push_back(sym);\
				cur.line=line;\
				cur.column=column;\
				tokens.Add(cur);\
			}
			
			CHECK_SYMBOL('{',LBR)
			else CHECK_SYMBOL('}',RBR)
			else CHECK_SYMBOL('[',LBK)
			else CHECK_SYMBOL(']',RBK)
			else CHECK_SYMBOL(',',COM)
			else CHECK_SYMBOL(':',COL)
			else if(isdigit(src[loc])||src[loc]=='.'){
				bool d=0,e=0,n=0;
				if(src[loc]=='0'&&loc+1<len&&(src[loc+1]=='x'||src[loc+1]=='X')){
					cur.val.push_back(src[loc]),nextchar();
					cur.val.push_back(src[loc]),nextchar();
					while((loc<len&&isdigit(src[loc]))||(src[loc]>='a'&&src[loc]<='f')||(src[loc]>='A'&&src[loc]<='F'))cur.val.push_back(tolower(src[loc])),nextchar();
					cur.type=TOK_HEX,tokens.push_back(cur);
				}
				else{
					while((loc<len&&isdigit(src[loc]))||src[loc]=='.'||src[loc]=='E'||src[loc]=='e'||src[loc]=='-'){
						if(src[loc]=='.'){if(!d)d=1;else break;}
						if(src[loc]=='e'||src[loc]=='E'){if(n&&!e)e=1;else break;}
						if(src[loc]=='-'){if(loc-1>=0&&(src[loc-1]=='e'||src[loc-1]=='E'));else break;}
						if(isdigit(src[loc]))n=1;
						cur.val.push_back(src[loc]),nextchar();
					}
					if(cur.val.size()==1&&cur.val[0]=='.')cur.type=TOK_DOT;
					else cur.type=TOK_NUM;
					tokens.push_back(cur);
				}
			}
			else if(src[loc]=='\''||src[loc]=='\"'){
				char s=src[loc];cur.val.push_back(src[loc]),nextchar();
				while(loc<len&&src[loc]!=s&&src[loc]!='\r'&&src[loc]!='\n'){
					if(src[loc]=='\\')cur.val.push_back(src[loc]),nextchar();
					cur.val.push_back(src[loc]),nextchar();
				}
				if(src[loc]==s)cur.val.push_back(src[loc]),nextchar();
				cur.type=TOK_STR,tokens.push_back(cur);
			}
			else if(isspace(src[loc]))nextchar();
			else THROW("JSONError",FORMAT("unexpected character '%c'(ascii=%d) at line %d, column %d",src[loc],src[loc],line,column));
		}
	}
	
	Value ReadObject(bool isKey){
		switch(Cur().type){
			case TOK_NUM:{
				return Value(str2num(Read().val));
			}
			case TOK_STR:{
				string r=Read().val;
				return Value(stringExpr(r.substr(1,r.size()-2)));
			}
			case TOK_LBR:{
				if(isKey)THROW("JSONError",FORMAT("unexpected '%s' in json at line %d, column %d",Cur().val.c_str(),Cur().line,Cur().column));
				Read();
				Value value;
				value.type=TYPE_MAP;
				value.obj=NewMap();
				map<Value,Value>&dict=*value.obj->dict;
				while(Cur().type!=TOK_RBR){
					const Value&key=ReadObject(true);
					Read(TOK_COL);
					const Value&v=ReadObject(false);
					dict[key]=v;
					if(Cur().type==TOK_COM)Read();
					else break;
				}
				Read(TOK_RBR);
				return value; 
			}
			case TOK_LBK:{
				if(isKey)THROW("JSONError",FORMAT("unexpected '%s' in json at line %d, column %d",Cur().val.c_str(),Cur().line,Cur().column));
				Read();
				Value value;
				value.type=TYPE_ARR;
				value.obj=NewArray(0);
				vector<Value>&arr=*value.obj->arr;
				while(Cur().type!=TOK_RBK){
					const Value&v=ReadObject(false);
					arr.Add(v);
					if(Cur().type==TOK_COM)Read();
					else break;
				}
				Read(TOK_RBK);
				return value; 
			}
			default:{
				THROW("JSONError",FORMAT("unexpected '%s' in json at line %d, column %d",Cur().val.c_str(),Cur().line,Cur().column));
				return Value();
			}
		}
	}
	
	Value Loads(const string&str){
		tokens.clear();
		curIndex=0;
		try{
			JSONTokenize(str);
			return ReadObject(false);
		}
		catch(string&s){
			return s;
		}
	}
};

namespace strutils{
	inline string substring(const string&s,const Value&from,const Value&to){
		if(from.type==TYPE_UNDEF&&to.type==TYPE_UNDEF)return s;
		if(from.type!=TYPE_NUM)THROW("IndexError",FORMAT("should not use '%s' as substring indice",from.ToStr().c_str()));
		int a=from.num,b=0;
		if(to.type==TYPE_UNDEF)b=s.size()-1;
		else if(to.type==TYPE_NUM)b=to.num;
		else THROW("IndexError",FORMAT("should not use '%s' as substring indice",to.ToStr().c_str()));
		a=a<0?0:a,b=b<0?0:b,b=(unsigned int64_t)b>=s.size()?s.size()-1:b;
		string r="";
		for(int i=a;i<=b;i++)r+=s[i];
		return r;
	}
	inline string substr(const string&s,int from,int len){
		from=from<0?0:from,from=(unsigned int64_t)from>=s.size()?s.size()-1:from;
		len=len<0?0:len,len=(unsigned int64_t)len>(s.size()-from)?(s.size()-from):len;
		return s.substr(from,len);
	}
	inline string changecase(const string&s,int lower=0){
		string r="";
		for(auto a:s)r+=char(lower?tolower(a):toupper(a));
		return r;
	}
	inline int startswith(const string&s,const string&pre){
		if(pre.size()>s.size())return 0;
		else return s.substr(0,pre.size())==pre;
	}
	inline int endswith(const string&s,const string&suf){
		if(suf.size()>s.size())return 0;
		else return s.substr(s.size()-suf.size(),suf.size())==suf;
	}
	inline string trim(const string&s){
		int x=0,y=s.size()-1;
		while(isspace(s[x])&&x<=y)x++;
		while(isspace(s[y])&&x<=y)y--;
		return s.substr(x,y-x+1);
	}
	inline string fmtprint(const string&f,const vector<ValueRef>&v){
		uint len=f.size();
		string r="";
		for(uint i=0;i<len;i++){
			if(f[i]=='{'){
				uint idx=0,ok=0;i++;
				while(i<len&&f[i]!='}'){
					if(!isdigit(f[i]))THROW("FormatError",FORMAT("invalid format string '%s': invalid index character '%c'",f.c_str(),f[i]));
					idx=(idx<<3)+(idx<<1)+(f[i]&15),i++,ok=1;
				}
				if(f[i]!='}')THROW("FormatError",FORMAT("invalid format string '%s': unterminated '{'",f.c_str()));
				if(!ok)THROW("FormatError",FORMAT("invalid format string '%s': expected an index at position %d",f.c_str(),i));
				if(idx>=v.size())THROW("FormatError",FORMAT("invalid format string '%s': unprovided value index %d",f.c_str(),idx));
				r+=v[idx]->ToStr();
			}
			else if(f[i]=='}')THROW("FormatError",FORMAT("invalid format string '%s': unmatched '}'",f.c_str()));
			else r+=f[i];
		}
		return r;
	}
	inline string repeatstr(const string&_,int times){
		string f=_,ans="";
		for(;times;times>>=1,f=f+f)if(times&1)ans=ans+f;
		return ans;
	}
	// Its not a good idea to use KMP so I dont
	inline string replacestr(const string&_,const string&rpl,const string&use){
		string f=_;
		uint x=f.find(rpl);
		if(x==string::npos)return f;
		return f.replace(x,rpl.size(),use);
	}
	inline Value splitstr(const string&_,const string&rpl){
		size_t x=_.find(rpl),lst=0;
		
		Value value;
		value.type=TYPE_ARR;
		value.obj=NewArray(0);
		
		vector<Value>*arr=value.obj->arr; 
		
		while(x!=string::npos){
			if(x>lst)arr->Add(_.substr(lst,x-lst));
			lst=x+1,x=_.find(rpl,lst); 
		}
		if(lst!=_.size())arr->Add(_.substr(lst));
		
		return value;
	}
	inline int indexof(const string&_,const string&rpl,const Value&from){
		int start=0;
		if(from.type==TYPE_UNDEF)start=0;
		else if(from.type==TYPE_NUM)start=from.num;
		else THROW("IndexError",FORMAT("should not use '%s' as indice",from.ToStr().c_str()));
		start=start<0?0:start,start=(unsigned int64_t)start>=_.size()?_.size()-1:start; 
		uint x=_.find(rpl,start);
		if(x==string::npos)return -1;
		return x;
	}
}
using namespace strutils;

#define BT_FUNC(name)\
Value Builtin_##name(ValueRef thisObject,Value*args,uint argc)

#define ARG(x) args[x]
#define ARGC_ERR(a,name)\
THROW("ArgumentError",FORMAT("expected %d argument(s) when calling %s, given: %d",a,#name,argc))
#define ARGC_ERR_2(a,name)\
THROW("ArgumentError",FORMAT("expected %s argument(s) when calling %s, given: %d",a,#name,argc))

#define ARG_TYPE_ERR(num,type,name)\
THROW("TypeError",FORMAT("expected type '%s' for argument %d when calling %s, given: %s",type,num+1,#name,ARG(num).GetTypeName().c_str()))

BT_FUNC(print){
	for(uint i=0;i<argc;i++){
		cout<<ARG(i).ToStr()<<' ';
	}
	cout<<endl;
	return argc;
}
BT_FUNC(GC){
	if(argc!=0)ARGC_ERR(0,"GC");
	GC();
	return Value();
}
BT_FUNC(Loads){
	if(argc!=1)ARGC_ERR(0,"json");
	if(ARG(0).type!=TYPE_STR)ARG_TYPE_ERR(0,"string","json");
	return JSONParser::Loads(*ARG(0).obj->str);
}
BT_FUNC(Open){
	if(argc!=2)ARGC_ERR(2,"open");
	if(ARG(0).type!=TYPE_STR)ARG_TYPE_ERR(0,"string","open");
	if(ARG(1).type!=TYPE_STR)ARG_TYPE_ERR(1,"string","open");
	return Value(file_manager::file_open(*ARG(0).obj->str,*ARG(1).obj->str));
}
BT_FUNC(Close){
	if(argc!=1)ARGC_ERR(1,"close");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","close");
	return Value(file_manager::file_close(ARG(0).num));
}
BT_FUNC(EOF){
	if(argc!=1)ARGC_ERR(1,"eof");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","eof");
	return Value(file_manager::file_eof(ARG(0).num));
}
BT_FUNC(Read){
	if(argc!=1)ARGC_ERR(1,"read");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","read");
	return Value(file_manager::file_eof(ARG(0).num));
}
BT_FUNC(ReadLine){
	if(argc!=1)ARGC_ERR(1,"readLine");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","readLine");
	return Value(file_manager::fread_line(ARG(0).num));
}
BT_FUNC(ReadNumber){
	if(argc!=1)ARGC_ERR(1,"readNumber");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","readNumber");
	return Value(file_manager::fread_number(ARG(0).num));
}
BT_FUNC(ReadString){
	if(argc!=1)ARGC_ERR(1,"readString");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","readString");
	return Value(file_manager::fread_string(ARG(0).num));
}
BT_FUNC(ReadChar){
	if(argc!=1)ARGC_ERR(1,"readChar");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","readChar");
	return Value(file_manager::fread_char(ARG(0).num));
}
BT_FUNC(Write){
	if(argc!=2)ARGC_ERR(2,"Write");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","Write");
	return Value(file_manager::fwrite_string(ARG(0).num,ARG(1).ToStr()));
}
BT_FUNC(loadDll){
	if(argc!=2)ARGC_ERR(2,"loadNative");
	if(ARG(0).type!=TYPE_STR)ARG_TYPE_ERR(0,"string","loadNative");
	if(ARG(1).type!=TYPE_STR)ARG_TYPE_ERR(1,"string","loadNative");
	HMODULE hMod=LoadLibrary(ARG(0).obj->str->c_str());
	if(!hMod)THROW("IOError",FORMAT("cannot open dll '%s'",ARG(0).obj->str->c_str()));
	NativeFunction func=(NativeFunction)GetProcAddress(hMod,ARG(1).obj->str->c_str());
	if(!func)THROW("IOError",FORMAT("cannot load function '%s'",ARG(1).obj->str->c_str()));
	
	Value value;
	value.type=TYPE_FUNC;
	value.obj=NewNativeFunc(func);
	return value;
}

#define MATH_FN(name)\
BT_FUNC(name){\
	if(argc!=1)ARGC_ERR(1,#name);\
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","Write");\
	return name(ARG(0).num);\
}

#define MATH_FN_2(name)\
BT_FUNC(name){\
	if(argc!=2)ARGC_ERR(2,#name);\
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","Write");\
	if(ARG(1).type!=TYPE_NUM)ARG_TYPE_ERR(1,"number","Write");\
	return name(ARG(0).num,ARG(1).num);\
}

MATH_FN(sin)
MATH_FN(asin)
MATH_FN(cos)
MATH_FN(acos)
MATH_FN(tan)
MATH_FN(atan)
MATH_FN_2(atan2)
MATH_FN(abs)
MATH_FN(ceil)
MATH_FN(floor)
MATH_FN(round)
MATH_FN(exp)
MATH_FN(log)
MATH_FN(sqrt)
MATH_FN_2(pow)

BT_FUNC(max){
	if(argc==0)return Value();
	ValueRef big=args+0;
	for(uint i=1;i<argc;i++){
		big=(ValueRef)&max(*big,args[i]);
	}
	return *big;
}
BT_FUNC(min){
	if(argc==0)return Value();
	ValueRef sml=args+0;
	for(uint i=1;i<argc;i++){
		sml=(ValueRef)&min(*sml,args[i]);
	}
	return *sml;
}
BT_FUNC(random){
	if(argc!=0)ARGC_ERR(0,"random");
	return 1.0f*rand()/RAND_MAX;
}
BT_FUNC(isNan){
	if(argc!=1)ARGC_ERR(1,"isNan");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","isNan");
	return ARG(0).num==ARG(0).num;
}
BT_FUNC(toDeg){
	if(argc!=1)ARGC_ERR(1,"toDeg");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","toDeg");
	return ARG(0).num/PI*180;
}
BT_FUNC(toRad){
	if(argc!=1)ARGC_ERR(1,"toRad");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","toRad");
	return ARG(0).num/180*PI;
}
BT_FUNC(system){
	if(argc!=1)ARGC_ERR(1,"system");
	if(ARG(0).type!=TYPE_STR)ARG_TYPE_ERR(0,"string","system");
	return system(ARG(0).obj->str->c_str());
}
BT_FUNC(clock){
	if(argc!=0)ARGC_ERR(0,"clock");
	return clock();
}
BT_FUNC(exit){
	if(argc!=1)ARGC_ERR(1,"exit");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","exit");
	exit(ARG(0).num);
	return Value();
}
BT_FUNC(ascii){
	if(argc!=1)ARGC_ERR(1,"ascii");
	if(ARG(0).type!=TYPE_STR)ARG_TYPE_ERR(0,"string","ascii");
	string s=*ARG(0).obj->str;
	if(s.size()==1)return uint(s[0]); 
	
	Value value;
	value.type=TYPE_ARR;
	value.obj=NewArray(0);
	for(auto a:s)value.obj->arr->emplace_back(Value(uint(a)));
	
	return value;
}
BT_FUNC(char){
	if(argc==0)ARGC_ERR_2("at least 1 argument","char");
	string str="";
	for(uint i=0;i<argc;i++){
		if(ARG(i).type!=TYPE_NUM)ARG_TYPE_ERR(i,"number","char");
		str+=uchar(ARG(i).num);
	}
	return str;
}
BT_FUNC(toNumber){
	if(argc!=1)ARGC_ERR(1,"toNumber");
	return str2num(ARG(0).ToStr());
}

BT_FUNC(MapLength){
	if(argc!=0)ARGC_ERR(0,"map.Length");
	return thisObject->obj->dict->size();
}
BT_FUNC(MapHas){
	if(argc!=1)ARGC_ERR(1,"map.Has");
	map<Value,Value>*dict=thisObject->obj->dict;
	return dict->find(ARG(0))!=dict->end();
}
BT_FUNC(MapRemove){
	if(argc!=1)ARGC_ERR(1,"map.Remove");
	map<Value,Value>*dict=thisObject->obj->dict;
	auto g=dict->find(ARG(0));
	if(g!=dict->end())dict->erase(g);
	else return 0.0f;
	return 1.0f;
}
BT_FUNC(MapKeys){
	if(argc!=0)ARGC_ERR(0,"map.Keys");
	map<Value,Value>*dict=thisObject->obj->dict;
	Value arr;
	arr.type=TYPE_ARR;
	arr.obj=NewArray(0);
	for(auto a:*dict){
		arr.obj->arr->Add(a.first);
	} 
	return arr;
}
BT_FUNC(ArrLength){
	if(argc!=0)ARGC_ERR(0,"array.Length");
	return thisObject->obj->arr->size();
}
BT_FUNC(ArrHas){
	if(argc!=1)ARGC_ERR(0,"array.Has");
	vector<Value>*arr=thisObject->obj->arr;
	for(const auto&a:*arr){
		if(a==ARG(0))return 1.0f;
	}
	return 0.0f;
}
BT_FUNC(ArrRemove){
	if(argc!=1)ARGC_ERR(0,"array.Remove");
	vector<Value>*arr=thisObject->obj->arr;
	auto it=arr->begin();
	for(const auto&a:*arr){
		if(a==ARG(0)){
			arr->erase(it); 
			return 1.0f;
		}
		it++;
	}
	return 0.0f;
}
BT_FUNC(ArrRemoveAt){
	if(argc!=1)ARGC_ERR(0,"array.RemoveAt");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","array.RemoveAt");
	
	int index=ARG(0).num;
	vector<Value>*arr=thisObject->obj->arr;
	
	if(abs(index)>=arr->size())THROW("IndexError",FORMAT("index %d out of array bounds",index));
	if(index<0)index+=arr->size();
	
	auto it=arr->begin();
	arr->erase(it+index);
	return 1;
}
BT_FUNC(ArrAdd){
	for(uint i=0;i<argc;i++){
		thisObject->obj->arr->Add(ARG(i));
	}
	return thisObject->obj->arr->size();
}
BT_FUNC(ArrPop){
	if(argc!=1)ARGC_ERR(0,"array.Pop");
	thisObject->obj->arr->pop_back();
	return thisObject->obj->arr->size();
}
BT_FUNC(ArrReverse){
	vector<Value>*arr=thisObject->obj->arr;
	if(argc==0){
		reverse(arr->begin(),arr->end());
	}
	else if(argc==2){
		if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","array.Reverse");
		if(ARG(1).type!=TYPE_NUM)ARG_TYPE_ERR(1,"number","array.Reverse");
		
		int leftBound=ARG(0).num;
		int rightBound=ARG(1).num;
		if(abs(leftBound)>arr->size())THROW("IndexError",FORMAT("index %d out of array bounds",leftBound));
		if(abs(rightBound)>arr->size())THROW("IndexError",FORMAT("index %d out of array bounds",rightBound));
		if(leftBound<0)leftBound+=arr->size();
		if(rightBound<0)rightBound+=arr->size();
		
		if(leftBound>=rightBound)return -1;
		reverse(arr->begin()+leftBound,arr->begin()+rightBound);
	}
	return 1;
}
BT_FUNC(ArrSort){
	if(argc>1)ARGC_ERR_2("0 or 1","array.Sort");
	vector<Value>*arr=thisObject->obj->arr;
	if(argc==1){
		if(ARG(0).type!=TYPE_FUNC)ARG_TYPE_ERR(0,"function","array.Sort");
		Fn* fn=ARG(0).obj->fn;
		sort(arr->begin(),arr->end(),[&](const Value&a,const Value&b){
			return InnerCall(fn,a,b,&ENV).IsTrue();
		});
	}
	else{
		sort(arr->begin(),arr->end());
	}
	return 0.0f;
}
BT_FUNC(ArrResize){
	if(argc!=1)ARGC_ERR(1,"array.Resize");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","array.Resize");
	thisObject->obj->arr->resize(ARG(0).num);
	return Value();
}

BT_FUNC(StrSubstring){
	if(argc<0||argc>2)ARGC_ERR_2("1 or 2","string.Substring");
	if(argc==1)return substring(*thisObject->obj->str,ARG(0),Value());
	return substring(*thisObject->obj->str,ARG(0).num,ARG(1).num);
}
BT_FUNC(StrSubstr){
	if(argc!=2)ARGC_ERR(2,"string.Substr");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","string.Substr");
	if(ARG(1).type!=TYPE_NUM)ARG_TYPE_ERR(1,"number","string.Substr");
	return substr(*thisObject->obj->str,ARG(0).num,ARG(1).num);
}
BT_FUNC(StrToUpper){
	if(argc!=0)ARGC_ERR(0,"string.ToUpper");
	return changecase(*thisObject->obj->str,0);
}
BT_FUNC(StrToSmall){
	if(argc!=0)ARGC_ERR(0,"string.ToSmall");
	return changecase(*thisObject->obj->str,1);
}
BT_FUNC(StrStartsWith){
	if(argc!=1)ARGC_ERR(1,"string.StartsWith");
	return startswith(*thisObject->obj->str,*ARG(0).obj->str);
}
BT_FUNC(StrEndsWith){
	if(argc!=1)ARGC_ERR(1,"string.EndsWith");
	return endswith(*thisObject->obj->str,*ARG(0).obj->str);
}
BT_FUNC(StrTrim){
	if(argc!=0)ARGC_ERR(0,"string.Trim");
	return trim(*thisObject->obj->str);
}
BT_FUNC(StrFormat){
	vector<ValueRef> v;
	for(uint i=0;i<argc;i++)v.Add(args+i);
	return fmtprint(*thisObject->obj->str,v);
}
BT_FUNC(StrRepeat){
	if(argc!=1)ARGC_ERR(1,"string.Repeat");
	if(ARG(0).type!=TYPE_NUM)ARG_TYPE_ERR(0,"number","string.Repeat");
	return repeatstr(*thisObject->obj->str,ARG(0).num);
}
BT_FUNC(StrReplace){
	if(argc!=2)ARGC_ERR(2,"string.Replace");
	if(ARG(0).type!=TYPE_STR)ARG_TYPE_ERR(0,"string","string.Replace");
	if(ARG(1).type!=TYPE_STR)ARG_TYPE_ERR(1,"string","string.Replace");
	return replacestr(*thisObject->obj->str,*ARG(0).obj->str,*ARG(1).obj->str);
}
BT_FUNC(StrSplit){
	if(argc!=1)ARGC_ERR(1,"string.StrSplit");
	if(ARG(0).type!=TYPE_STR)ARG_TYPE_ERR(0,"string","string.StrSplit");
	return splitstr(*thisObject->obj->str,*ARG(0).obj->str);
}
BT_FUNC(StrIndexOf){
	if(argc<0||argc>2)ARGC_ERR_2("1 or 2","string.IndexOf");
	if(argc==1)return substring(*thisObject->obj->str,*ARG(0).obj->str,0.0f);
	return indexof(*thisObject->obj->str,*ARG(0).obj->str,ARG(1).num);
}
BT_FUNC(StrLength){
	if(argc!=0)ARGC_ERR(0,"string.Length");
	return thisObject->obj->str->size();
}

void SetupFuncs();

void Constants(){
	NUMBER_CONSTANT_VALUE=new double[NUMBER_CONSTANT.values.size()];
	for(auto a:NUMBER_CONSTANT.values){
		NUMBER_CONSTANT_VALUE[a.first]=a.second;
	}
	
	STRING_CONSTANT_VALUE=new string[STRING_CONSTANT.values.size()];
	for(auto a:STRING_CONSTANT.values){
		STRING_CONSTANT_VALUE[a.first]=a.second;
	}
}

void InitVM(uint glbvCount,uint builtinCount){
	GLB_CNT=glbvCount;
	BUILTIN_CNT=builtinCount;
	STACK=new Value[runstackSize];
	GLBV=new Value[glbvCount];
	BUILTIN_VAR=new Value[builtinCount];
	SetupFuncs();
	Constants();
}

vector<string> cmdArgs;

void SetupFuncs(){
	uint bdidx=0;
	#define BUILTIN(func)\
	BUILTIN_VAR[bdidx].type=TYPE_FUNC;\
	BUILTIN_VAR[bdidx].obj=NewBuiltinFunc(Builtin_##func);\
	bdidx++
	
	#define BUILTIN_VALUE(_value)\
	BUILTIN_VAR[bdidx]=_value;\
	bdidx++
	
	BUILTIN(print);
	BUILTIN(GC);
	BUILTIN(Loads);
	BUILTIN(Open);
	BUILTIN(Close);
	BUILTIN(EOF);
	BUILTIN(Read);
	BUILTIN(ReadLine);
	BUILTIN(ReadNumber);
	BUILTIN(ReadString);
	BUILTIN(ReadChar);
	BUILTIN(Write);
	
	BUILTIN(sin);
	BUILTIN(asin);
	BUILTIN(cos);
	BUILTIN(acos);
	BUILTIN(tan);
	BUILTIN(atan);
	BUILTIN(atan2);
	BUILTIN(abs);
	BUILTIN(ceil);
	BUILTIN(floor);
	BUILTIN(round);
	BUILTIN(exp);
	BUILTIN(log);
	BUILTIN(pow);
	BUILTIN(sqrt);
	BUILTIN(max);
	BUILTIN(min);
	BUILTIN(random);
	BUILTIN(isNan);
	BUILTIN(toDeg);
	BUILTIN(toRad);
	
	BUILTIN_VALUE(PI);
	BUILTIN(loadDll);
	BUILTIN(exit);
	BUILTIN(system);
	BUILTIN(clock);
	
	Value arr=Value();
	arr.type=TYPE_ARR;
	arr.obj=NewArray(cmdArgs.size());
	for(uint i=0;i<cmdArgs.size();i++){
		(*(arr.obj->arr))[i]=cmdArgs[i];
	}
	BUILTIN_VALUE(arr);
	
	BUILTIN(ascii);
	BUILTIN(char);
	BUILTIN(toNumber);
	
	MAP_ATTR_FN=new Value[MAP_ATTR_COUNT];
	#define MAP_FUNC(name,func)\
	bdidx=MAP_ATTR.Ensure(name);\
	MAP_ATTR_FN[bdidx].type=TYPE_FUNC;\
	MAP_ATTR_FN[bdidx].obj=NewBuiltinFunc(Builtin_Map##func)
	
	MAP_FUNC("Length",Length);
	MAP_FUNC("Has",Has);
	MAP_FUNC("Remove",Remove);
	MAP_FUNC("Keys",Keys);
	
	ARRAY_ATTR_FN=new Value[ARRAY_ATTR_COUNT];
	#define ARR_FUNC(name,func)\
	bdidx=ARRAY_ATTR.Ensure(name);\
	ARRAY_ATTR_FN[bdidx].type=TYPE_FUNC;\
	ARRAY_ATTR_FN[bdidx].obj=NewBuiltinFunc(Builtin_Arr##func)
	
	ARR_FUNC("Length",Length);
	ARR_FUNC("Has",Has);
	ARR_FUNC("Remove",Remove);
	ARR_FUNC("RemoveAt",RemoveAt);
	ARR_FUNC("Add",Add);
	ARR_FUNC("Pop",Pop);
	ARR_FUNC("Reverse",Reverse);
	ARR_FUNC("Sort",Sort);
	ARR_FUNC("Resize",Resize);
	
	
	STRING_ATTR_FN=new Value[STRING_ATTR_COUNT];
	
	#define STR_FUNC(name,func)\
	bdidx=STRING_ATTR.Ensure(name);\
	STRING_ATTR_FN[bdidx].type=TYPE_FUNC;\
	STRING_ATTR_FN[bdidx].obj=NewBuiltinFunc(Builtin_Str##func)
	
	STR_FUNC("Substring",Substring);
	STR_FUNC("Substr",Substr);
	STR_FUNC("ToUpper",ToUpper);
	STR_FUNC("ToSmall",ToSmall);
	STR_FUNC("StartsWith",StartsWith);
	STR_FUNC("EndsWith",EndsWith);
	STR_FUNC("Trim",Trim);
	STR_FUNC("Format",Format);
	STR_FUNC("Repeat",Repeat);
	STR_FUNC("Replace",Replace);
	STR_FUNC("Split",Split);
	STR_FUNC("IndexOf",IndexOf);
	STR_FUNC("Length",Length);
}

}
using namespace VirtualMachine;

namespace GarbageCollector{

void MarkObject(ObjectRef ref);
void MarkValue(ValueRef ref){
	switch(ref->type){
		case TYPE_NUM:case TYPE_UNDEF:case TYPE_NULL:return;
		default:{
			MarkObject(ref->obj);
			return;
		}
	}
}
uint tabs=0;
void MarkObject(ObjectRef ref){
	if(ref->isBlack)return;
	ref->isBlack=true;
	if(DEBUG_MODE){
		for(uint i=0;i<tabs;i++)cout<<"  ";
		cout<<"Marked "<<TYPE_NAME[ref->type]<<" @ "<<ref;
		if(ref->type==TYPE_FUNC){
			if((ref->fn->type)==FN_SCRIPT)cout<<"("<<ref->fn->info->fnName<<")";
			else cout<<"(primitive)";
		}
		cout<<endl;
	}
	tabs++;
	switch(ref->type){
		default:{
			break;
		} 
		case TYPE_STR:{
			break;
		}
		case TYPE_ARR:{
			for(auto a:*ref->arr){
				MarkValue(&a);
			}
			break;
		}
		case TYPE_MAP:{
			for(auto a:*ref->dict){
				MarkValue((ValueRef)&((a).first));
				MarkValue((ValueRef)&((a).second));
			}
			break;
		}
		case TYPE_FUNC:{
			if(ref->fn->type==FN_SCRIPT){
				for(uint i=0;i<ref->fn->info->upvalueInfo.size();i++){
					MarkValue(ref->fn->upvalues[i].value);
					MarkValue(&ref->fn->upvalues[i]._value);
				}
			}
			break;
		}
	}
	tabs--;
}
}

using namespace GarbageCollector;

void GC(){
	if(DEBUG_MODE)cout<<"GC Triggered"<<endl;
//	system("pause > nul");
	uint start=clock();
//	cout<<"STACK="<<STACK<<", ESP="<<globalEsp<<endl;
	for(uint i=0;i<GLB_CNT;i++){
		MarkValue(GLBV+i);
	}
	for(uint i=0;i<BUILTIN_CNT;i++){
		MarkValue(BUILTIN_VAR+i);
	}
	#define MARK_ATTR(klass)\
	for(uint i=0;i<klass##_ATTR_COUNT;i++){\
		MarkValue(klass##_ATTR_FN+i);\
	}
	MARK_ATTR(MAP)
	MARK_ATTR(STRING)
	MARK_ATTR(ARRAY)
	for(auto a=STACK;a<globalEsp;a++){
		MarkValue(a);
	}
	uint before=ALLOCATED;
	for(auto survived:OBJ_POOL){
		if(survived->isBlack){
			OBJ_POOL_2.Add(survived);
		}
	}
	for(auto ref:OBJ_POOL)if(!ref->isBlack){
		if(DEBUG_MODE){
			cout<<"FREE "<<TYPE_NAME[ref->type]<<" @ "<<ref;
			if(ref->type==TYPE_FUNC){
				if((ref->fn->type)==FN_SCRIPT)cout<<"("<<ref->fn->info->fnName<<")";
				else cout<<"(primitive)";
			}
			puts("");
		}
		switch(ref->type){
			case TYPE_FUNC:ALLOCATED-=sizeof(Fn);break;
			case TYPE_ARR:ALLOCATED-=sizeof(vector<Value>);break;
			case TYPE_MAP:ALLOCATED-=sizeof(map<Value,Value>);break;
			case TYPE_STR:ALLOCATED-=sizeof(string);break;
			default:break;
		}
		free(ref);
	}
	for(auto survived:OBJ_POOL_2)survived->isBlack=false;
	OBJ_POOL=OBJ_POOL_2;
	OBJ_POOL_2.clear();
	
	uint after=ALLOCATED;
	GC_TRIGGER*=GROW_FACTOR;
	if(DEBUG_MODE)printf("Garbage Collection done. Before: %u, after: %u, collected: %u, cost %.3gs\n",before,after,before-after,(clock()-start)/1000.0);
//	system("pause > nul");
}

struct rbq_env{
	Value (*NewNumber)(double x);
	Value (*NewString)(const char* buf);
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
	Value (*JSON)(const char*json);
	
	void (*Throw)(const char*type,const char*reason);
};

namespace Native{
	
Value Native_NewNumber(double num){
	return num;
}
Value Native_NewString(const char* buf){
	return string(buf);
}
Value Native_NewPointer(void* ptr){
	return Value(ptr);
}

Value Native_NewArray(){
	Value value;
	value.type=TYPE_ARR;
	value.obj=NewArray(0); 
	return value;
}
void Native_ResizeArray(Value value,int size){
	return value.obj->arr->resize(size);
}
Value Native_GetArrayIndex(Value value,int index){
	return (*value.obj->arr)[index];
}
void Native_AppendItem(Value value,Value v){
	value.obj->arr->emplace_back(v);
}
void Native_SetArrayIndex(Value value,int index,Value v){
	(*value.obj->arr)[index]=v;
}
int Native_GetArrayLength(Value value){
	return value.obj->arr->size();
}

Value Native_NewMap(){
	Value value;
	value.type=TYPE_MAP;
	value.obj=NewMap(); 
	return value;
}
Value Native_GetMapIndex(Value value,Value index){
	return (*value.obj->dict)[index];
}
int Native_HasKey(Value value,Value v){
	map<Value,Value>*dict=value.obj->dict;
	return dict->find(v)!=dict->end();
}
Value Native_Keys(Value value){
	return Builtin_MapKeys(&value,NULL,0);
}
void Native_SetMapIndex(Value value,Value index,Value v){
	(*value.obj->dict)[index]=v;
}
int Native_GetMapLength(Value value){
	return value.obj->dict->size();
}

Value Native_NewFunction(NativeFunction func){
    Value value;
    value.type=TYPE_FUNC;
    value.obj=NewNativeFunc(func);
    return value;
}
Value Native_GetAttribute(Value value,const char* attribute){
	if(HasBuiltinAttribute(value,attribute)){
		return GetBuiltinAttribute(value,attribute); 
	}
	else if(value.type==TYPE_MAP){
		return (*(value.obj->dict))[string(attribute)];
	}
	return Value();
}
Value Native_CallFunction(Value value,Value thisObject,Value* args,int argc){
	return InnerCall(value.obj->fn,args,argc,&thisObject);
}

const char* Native_ToStr(Value value){
	return value.ToStr().c_str();
}
Value Native_JSON(const char* json){
	return JSONParser::Loads(json);
}
void Native_Throw(const char* type,const char* reason){
	THROW(type,reason);
}

void InitNative(){
	env=new rbq_env();
	#define BIND_ENV(x)\
	env->x=Native_##x
	
	BIND_ENV(NewNumber);
	BIND_ENV(NewString);
	BIND_ENV(NewPointer);
	
	BIND_ENV(NewArray);
	BIND_ENV(ResizeArray);
	BIND_ENV(GetArrayIndex);
	BIND_ENV(AppendItem);
	BIND_ENV(SetArrayIndex);
	BIND_ENV(GetArrayLength);
	
	BIND_ENV(NewMap);
	BIND_ENV(GetMapIndex);
	BIND_ENV(Keys);
	BIND_ENV(HasKey);
	BIND_ENV(SetMapIndex);
	BIND_ENV(GetMapLength);
	
	BIND_ENV(NewFunction);
	BIND_ENV(GetAttribute);
	BIND_ENV(CallFunction);
	
	BIND_ENV(ToStr);
	BIND_ENV(JSON);
	
	BIND_ENV(Throw);
}

}

using namespace Native;

namespace Bytecodes{

uchar buffer[16];

void SaveByteCode(const string&file,uint glbvCount,uint builtinCount,CodeSet&mainBlock){
	ofstream out(file.c_str(),ios::binary);
	auto WriteByte=[&](uchar x){
		out.put(x);
	};
	auto WriteUint=[&](uint x){
		buffer[3]=x%256; x/=256;
		buffer[2]=x%256; x/=256;
		buffer[1]=x%256; x/=256;
		buffer[0]=x%256; x/=256;
		for(uint i=0;i<4;i++)out.put(buffer[i]);
	};
	auto WriteNumber=[&](double x){
		parser.x=x;
		for(uint i=0;i<8;i++)WriteByte(parser.bits[i]);
	};
	auto WriteString=[&](const string&x){
		WriteUint(x.size());
		for(uchar a:x)WriteByte(a);
	};
	
	WriteUint(MAGIC_NUMBER);
	WriteUint(VERSION_CODE);
	WriteUint(glbvCount);
	WriteUint(builtinCount);
	
	WriteUint(NUMBER_CONSTANT.values.size());
	for(auto num:NUMBER_CONSTANT.values){
		WriteUint(num.first);
		WriteNumber(num.second);
	}
	
	WriteUint(STRING_CONSTANT.values.size());
	for(const auto&str:STRING_CONSTANT.values){
		WriteUint(str.first);
		WriteString(str.second);
	}
	
	WriteUint(FUNC_CONSTANT.values.size());
	for(const auto&_fn:FUNC_CONSTANT.values){
		WriteUint(_fn.first);
		const FnInfo&fn=_fn.second;
		WriteString(fn.fnName);
		WriteUint(fn.upvalueInfo.size());
		for(const auto&upv:fn.upvalueInfo){
			WriteByte(upv.type);
			WriteUint(upv.args);
		}
		WriteUint(fn.localCnt);
		WriteUint(fn.argsCnt);
		WriteUint(fn.instr.size());
		for(const auto&ins:fn.instr){
			WriteByte(ins.type);
			WriteUint(ins.x);
		}
		WriteUint(fn.uniqueId);
	}
	
	WriteUint(mainBlock.size());
	for(const auto&ins:mainBlock){
		WriteByte(ins.type);
		WriteUint(ins.x);
	}
	
	out.close();
}

void ReadByteCode(const string&file,CodeSet&mainBlock){
	ifstream in(file.c_str(),ios::binary);
	auto ReadByte=[&](){
		return (uchar)in.get();
	};
	auto ReadUint=[&](){
		uint x=0;
		x+=(uchar)in.get(); x*=256;
		x+=(uchar)in.get(); x*=256;
		x+=(uchar)in.get(); x*=256;
		x+=(uchar)in.get();
		return x;
	};
	auto ReadNumber=[&](){
		for(uint i=0;i<8;i++)parser.bits[i]=(uchar)in.get();
		return parser.x;
	};
	auto ReadString=[&](){
		string s="";
		uint length=ReadUint();
		while(length--)s+=ReadByte();
		return s;
	};
	
	uint magicNumber=ReadUint();
	if(magicNumber!=MAGIC_NUMBER){
		THROW("SystemError",FORMAT("malformed or broken bytefile (magic number: %0X8)",magicNumber));
	} 
	
	uint versionCode=ReadUint();
	if(versionCode>VERSION_CODE){
		THROW("SystemError",FORMAT("not compatible version (bytecode: %0X8, vm: %0X8)",versionCode,VERSION_CODE));
	}
	
	uint glbvCount,builtinCount;
	glbvCount=ReadUint();
	builtinCount=ReadUint();
	
	GLB_CNT=glbvCount;
	BUILTIN_CNT=builtinCount;
	STACK=new Value[1024];
	GLBV=new Value[glbvCount];
	BUILTIN_VAR=new Value[builtinCount];
	
	uint size;
	
	NUMBER_CONSTANT=Table<double>();
	size=ReadUint();
	while(size--){
		uint index=ReadUint();
		double value=ReadNumber();
		NUMBER_CONSTANT.Bind(value,index);
	}
	NUMBER_CONSTANT_VALUE=new double[NUMBER_CONSTANT.values.size()];
	for(auto a:NUMBER_CONSTANT.values){
		NUMBER_CONSTANT_VALUE[a.first]=a.second;
	}
	
	STRING_CONSTANT=Table<string>();
	size=ReadUint();
	while(size--){
		uint index=ReadUint();
		string value=ReadString();
		STRING_CONSTANT.Bind(value,index);
	}
	STRING_CONSTANT_VALUE=new string[STRING_CONSTANT.values.size()];
	for(auto a:STRING_CONSTANT.values){
		STRING_CONSTANT_VALUE[a.first]=a.second;
	}
	
	FUNC_CONSTANT=Table<FnInfo>(); 
	size=ReadUint();
	while(size--){
		uint index=ReadUint();
		FnInfo fn;
		fn.fnName=ReadString();
		
		uint upvs=ReadUint();
		fn.upvalueInfo.resize(upvs);
		for(uint i=0;i<upvs;i++){
			fn.upvalueInfo[i].type=(UpvalType)ReadByte();
			fn.upvalueInfo[i].args=ReadUint();
		}
		
		fn.localCnt=ReadUint();
		fn.argsCnt=ReadUint();
		
		uint instrs=ReadUint();
		fn.instr.resize(instrs);
		for(uint i=0;i<instrs;i++){
			fn.instr[i].type=ReadByte();
			fn.instr[i].x=ReadUint();
		}
		fn.uniqueId=ReadUint();
		FUNC_CONSTANT.Bind(fn,index);
	}
	
	uint instrs=ReadUint();
	mainBlock.clear();
	mainBlock.resize(instrs);
	for(uint i=0;i<instrs;i++){
		mainBlock[i].type=ReadByte();
		mainBlock[i].x=ReadUint();
	}
	
	SetupFuncs();
}

}

int leftSyms[4]={
	0, // """
	0, // (
	0, // [
	0, // {
};

bool CheckComplete(const string&line){
	uint i=0;
	bool inQuote=false;
	char terminator=' ';
	while(i<line.size()){
		if(inQuote){
			if(line[i]==terminator){
				inQuote=false;
			}
			else if(line[i]=='\\'){
				i++;
			}
		}
		else if(leftSyms[0]){
			if(line[i]=='\"'||line[i]=='\''){
				if(i+2<line.size()&&line[i+1]==line[i]&&line[i+2]==line[i]){
					leftSyms[0]^=1;
				}
			}
		}
		else{
			if(line[i]=='\"'||line[i]=='\''){
				if(i+2<line.size()&&line[i+1]==line[i]&&line[i+2]==line[i]){
					leftSyms[0]^=1;
				}
				else{
					inQuote=true;
					terminator=line[i];
				}
			}
			else switch(line[i]){
				case '(':leftSyms[1]++;break;
				case ')':leftSyms[1]--;break;
				case '[':leftSyms[2]++;break;
				case ']':leftSyms[2]--;break;
				case '{':leftSyms[3]++;break;
				case '}':leftSyms[3]--;break;
				default:break;
			}
		}
		i++;
	}
	for(uint i=0;i<4;i++)if(leftSyms[i]>0)return false;
	return true;
}

void CliTokenize(const string&line){
	stringstream src("");
	vector<string>t=splitBySpace(line);
	if(t.size()&&t[0]=="#include"){
		for(uint i=1;i<t.size();i++){
			if(t[i][0]=='\''||t[i][0]=='"')t[i]=stringExpr(t[i].substr(1,t[i].size()-2));
			startTokenize(t[i]);
		}
		src<<endl;
	}
	else src<<line<<'\n';
	tokenize("<stdin>",src.str(),tokens,1);
}
void R_Cli(){
	NewScope();
	initDebug();
	InitCompiler();
	InitNative();
	uint builtinCount=builtinScope.usedLocal;
	
	GLB_CNT=1024;
	BUILTIN_CNT=builtinCount;
	STACK=new Value[runstackSize];
	GLBV=new Value[1024];
	BUILTIN_VAR=new Value[builtinCount];
	SetupFuncs();
	
	cliMode=true;
	
	while(1){
		try{
			tokens.clear();
			currentTokenIndex=0;
			memset(leftSyms,0,sizeof(leftSyms));
			string line,single;
			
			do{
				if(line=="")cout<<">>> ";
				else cout<<"... ";
				getline(cin,single);
				line+=single;
			}while(!CheckComplete(single));
			
			CliTokenize(line);
			CodeSet mainBlock=CliBlock(true);
			Constants();
			FnInfo fnInfo;
			fnInfo.instr=mainBlock;
			Fn fn=Fn(&fnInfo);
			RunCode(STACK,STACK,&fn,&ENV);
		}
		catch(Value&s){
			cout<<"RUNTIME ERROR: "<<s.ToStr()<<endl;
		}
		catch(string&s){
			cout<<"Line "<<tok.line<<", column "<<tok.column<<": "<<s<<endl;
			exit(0);
		}
	}
}

void R_Compile(const string&file,const string&out){
	uint glbvCount,builtinCount;
	CodeSet mainBlock;
	try{
		string s;
		startTokenize(file);
		
		NewScope();
		initDebug();
		InitCompiler();
		mainBlock=Block(true);
		glbvCount=CurScope().usedLocal;
		builtinCount=builtinScope.usedLocal;
		PopScope();
		
		Bytecodes::SaveByteCode(out,glbvCount,builtinCount,mainBlock);
	}
	catch(string&s){
		cout<<"File '"<<file<<"', line "<<tok.line<<", column "<<tok.column<<": "<<s<<endl;
		exit(0);
	}
}

void R_Run(const string&file){
	CodeSet mainBlock;
	try{
		Bytecodes::ReadByteCode(file,mainBlock);
		InitNative();
		FnInfo fnInfo;
		fnInfo.instr=mainBlock;
		unsigned b=clock(); 
		Fn fn=Fn(&fnInfo);
		RunCode(STACK,STACK,&fn,&ENV);
		printf("Elapsed: %dms",(int)(clock()-b));
	}
	catch(Value&s){
		cout<<"RUNTIME ERROR: "<<s.ToStr()<<endl;
	}
}

void HelpHint(string msg){
	puts(msg.c_str());
	puts(""); 
	puts("Usage:");
	puts("rbq6t <bytecode-file> [- {cmd-args}]      // run with arguments (optional)");
	puts("rbq6t ['-c' <source-file>]                // compile source file");
	puts("      ['-r' <bytecode-file>]              // execute bytecode file");
	puts("      ['-o' <saving-path>]                // specify the saving path of");
	puts("                                             compiled script");
	puts("rbq6t -cr <source-file>                   // compile and immediately run");
	exit(0);
}

void THROW(const string&type,const string&reason){
	Value value;
	value.type=TYPE_MAP; 
	value.obj=NewMap();
	map<Value,Value>*dict=value.obj->dict;
	(*dict)[(string)"__type__"]=type;
	(*dict)[(string)"message"]=reason;
	throw value;
}

int main(int argc,char** argv){
	bool needCompile=false;
	bool needRun=false;
	bool argsMode=false;
	bool needOutput=false;
	string sourceFile;
	string path;
	string byteFile;
	
	for(int i=1;i<argc;i++){
		if(argsMode){
			cmdArgs.Add(argv[i]);
		}
		else{
			if((string)argv[i]=="-"){
				argsMode=true;
			}
			else if((string)argv[i]=="-c"){
				if(needCompile)HelpHint("duplicated -c argument");
				needCompile=true;
				i++;
				if(i>=argc)HelpHint("expected source file name after -c argument");
				sourceFile=argv[i];
				path=ReplaceExt(sourceFile);
			}
			else if((string)argv[i]=="-r"){
				if(needRun)HelpHint("duplicated -r argument");
				needRun=true;
				i++;
				if(i>=argc)HelpHint("expected bytecode file name after -r argument");
				byteFile=argv[i];
			}
			else if((string)argv[i]=="-cr"){
				if(needCompile)HelpHint("duplicated -c argument");
				if(needRun)HelpHint("duplicated -r argument");
				needCompile=true;
				needRun=true;
				i++;
				if(i>=argc)HelpHint("expected source file name after -cr argument");
				sourceFile=argv[i];
				path=ReplaceExt(sourceFile);
				byteFile=ReplaceExt(sourceFile);
			}
			else if((string)argv[i]=="-o"){
				if(needOutput)HelpHint("duplicated -o argument");
				needOutput=true;
				i++;
				if(i>=argc)HelpHint("expected output file name after -o argument");
				path=argv[i];
			}
			else{
				if((i==1)&&(argc==2||((string)argv[2]=="-"))){
					byteFile=argv[i];
					needRun=true;
				}
				else HelpHint("unknown command args "+(string)argv[i]);
			}
		}
	}
	
	if(argc<=1){
		cout<<("RBQScript 6-turbo. Version: "+num2str(VERSION_CODE))<<endl;
		R_Cli();
		return 0;
	}
	if(needCompile){
		R_Compile(sourceFile,path);
	}
	if(needRun){
		R_Run(byteFile);
	}
	return 0;
}
