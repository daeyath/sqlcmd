#include <string>
#include <sqlite3.h>

using namespace std;

sqlite3 *db;
char *zErrMsg=0;
int rc;
bool colexists;

static int result(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	if(!colexists){
		for(i=0; i<argc; i++){
			printf("%s\t", azColName[i]);
		}
		colexists=true;
	}
	printf("\n");
	for(i=0; i<argc; i++){
		printf("%s\t", argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

bool connect(const char *filename){
	bool res=false;
	rc=sqlite3_open(filename, &db);
	if(rc){
		fprintf(stderr, "Error: '%s' %s\n", filename, sqlite3_errmsg(db));
	}else{
		fprintf(stderr, "Database '%s' connected\n", filename);
		res=true;
	}
	return res;
}

void exec(const char *str){
	colexists=false;
	rc=sqlite3_exec(db,str,result,0,&zErrMsg);
	if(rc!=SQLITE_OK){
		fprintf(stderr,"SQL Error: %s",zErrMsg);
		sqlite3_free(zErrMsg);
	}
	printf("\n");
}

void help(){
	printf(":c file - connect to database file\n");
	printf(":s object - show objects, ex: :s table\n");
	printf(":q - exit and close database\n");
}

void show(const char *objname){
	string obj(objname);
	string str="select name from sqlite_schema where type='' and name not like 'sqlite_%'";
	int i=str.find("'")+1;
	str.insert(i, obj);
	exec(str.c_str());
}

int command(const char *str){
	int ret=0;
	const int paramPos=3;
	const int len=strlen(str)-paramPos+1;
	char *param=new char[len];
	strncpy(param, str+paramPos, len);
	bool spc=param[0]==' '||param[strlen(param)-1]==' ';
	if(!spc){
		if(strncmp(str,":c ",3)==0){
			connect(param);
		}else if(strcmp(str,":q")==0){
			ret=1;
		}else if(strncmp(str,":s ",3)==0){
			show(param);
		}else if(strcmp(str,":h")==0){
			help();
		}else{
			printf("'%s' command not found\n", str);
		}
	}else{
		printf("Don't use space after or first parameter '%s'\n",param);
	}
	free(param);
	return ret;
}

bool delim(char s, char *str){
	string data(str);
	int loc = data.find(s);
	return (loc > -1);
}

void prompt(){
	char sqlstr[1024]="";
	const int len=64;
	char str[len];
	bool endscript=true;
	int res;
	while(true){
		printf(endscript?"sql> ":"   > ");
		fgets(str,len,stdin);
		if(strncmp(":",str,1)==0){
			str[strlen(str)-1]='\0';
			res=command(str);
			if(res==1) break;
		}else{
			strcat(sqlstr, str);
			endscript=delim(';',str);
			if(endscript){
				exec(sqlstr);
				sqlstr[0]='\0';
			}
		}
	}
}

void close(){
	sqlite3_close(db);
}

int main(int argc, char *argv[])
{
	printf("SQLCMD Version Alpha\nType :h for help\n");
	char *fname;
	if(argv[1]){
		fname=argv[1];
	}else{
		fname=":memory:";
	}
	if(connect(fname)) prompt();
	close();
}