#include <stdio.h>
#include <cstring>
#include <stdlib.h>
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

bool connect(char *filename){
	bool res=false;
	printf("Database %s\n", filename);
	rc=sqlite3_open(filename, &db);
	if(rc){
		fprintf(stderr, "Error: %s\n", sqlite3_errmsg(db));
	}else{
		fprintf(stderr, "Ok\n");
		res=true;
	}
	return res;
}

void substr(char *src, int from, int to, char *dest){
	for(int i=from, j=0; i<=to; i++, j++){
		dest[j]=src[i];
	}
}

void command(char *str){
	const int begin=3;
	const int end=strlen(str);
	
	char *param=new char[end-begin];
	substr(str,begin,end,param);
	
	if(strncmp(":c",str,2)==0){
		connect(param);
	}
	
	free(param);
}

bool delim(char c, char *str){
	bool found=false;
	for(int i=0; i<=strlen(str); i++){
		found=c==str[i];
		if(found) break;
	}
	return found;
}

void prompt(){
	char sqlstr[1024]="";
	const int len=64;
	char str[len];
	bool endscript=true;
	while(strncmp("exit",str,4)!=0){
		printf(endscript?"sql> ":"   > ");
		fgets(str,len,stdin);
		if(strncmp(":",str,1)==0){
			str[strlen(str)-1]='\0';
			command(str);
		}else{
			strcat(sqlstr, str);
			endscript=delim(';',str);
			if(endscript){
				colexists=false;
				rc=sqlite3_exec(db,sqlstr,result,0,&zErrMsg);
				if(rc!=SQLITE_OK){
					fprintf(stderr,"SQL Error: %s",zErrMsg);
					sqlite3_free(zErrMsg);
				}
				printf("\n");
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
	char *fname;
	if(argv[1]){
		fname=argv[1];
	}else{
		fname=":memory:";
	}
	if(connect(fname)) prompt();
	close();
}