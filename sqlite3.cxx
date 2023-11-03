#include <string>
#include <sqlite3.h>

using namespace std;

sqlite3 *db;
char *zErrMsg=0;
int rc;

bool connectdb(const char *filename){
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
	char **azResult;
	int nRow, nColumn;
	rc=sqlite3_get_table(db, str, &azResult, &nRow, &nColumn, &zErrMsg);
	if(rc!=SQLITE_OK){
		fprintf(stderr,"SQL Error: %s\n",zErrMsg);
		sqlite3_free(zErrMsg);
	}else if(nRow>0){
		int *lenCol=new int[nColumn];
		for(int i=0; i<nColumn; i++)lenCol[i]=0;
		int k=-1;
		for(int i=0; i<nRow+1; i++){
			for(int j=0; j<nColumn; j++){
				k++;
				int l=azResult[k]==NULL?0:strlen(azResult[k]);
				if(lenCol[j]<l)lenCol[j]=l;
			}
		}	
		k=-1;
		for(int i=0; i<nRow+1; i++){
			for(int j=0; j<nColumn; j++){
				k++;
				printf("%-*s", lenCol[j], azResult[k]);
				if(j<nColumn-1)printf(" ");
			}
			printf("\n");
			if(i==0){
				for(int j=0; j<nColumn; j++){
					for(int k=0; k<lenCol[j]; k++)printf("=");
					if(j<nColumn-1)printf(" ");
				}
				printf("\n");
			}
		}
		free(lenCol);
		printf("(%i rows)\n", nRow);
	}
	sqlite3_free_table(azResult);
}

void help(){
	printf(":c file - connect to database file\n");
	printf(":e sqlfile - execute sqlfile\n");
	printf(":s object - show objects, ex: :s table\n");
	printf(":q - exit and close database\n");
}

void show(const char *objname){
	char* str[5];
	char obj[strlen(objname)];
	strcpy(obj, objname);
	str[0]="select name as \"";
	str[1]=obj;
	str[2]="\" from sqlite_schema where type='";
	str[3]=obj;
	str[4]="' and name not like 'sqlite_%'";
	int len=0;
	for(int i=0; i<=4; i++)len+=strlen(str[i]);
	char sql[len];
	for(int i=0; i<=4; i++)strcat(sql, str[i]);
	exec(sql);
}

void execfile(const char* fname){
	FILE *afile=fopen(fname, "r");
	if(afile !=NULL){
		char sqlstr[1024]; sqlstr[0]=0;
		const int len=2;
		char str[len];
		while(fgets(str, len, afile)){
			strcat(sqlstr, str);
		}
		fclose(afile);
		exec(sqlstr);
	}else{
		printf("File '%s' not found.\n", fname);
	}
}

int command(const char *str){
	int ret=0;
	const int pos=3;
	const int len=strlen(str)-pos+1;
	char* param=new char[len];
	strncpy(param, str+pos, len);
	bool spc=param[0]==' '||param[strlen(param)-1]==' ';
	if(!spc){
		if(strncmp(str,":c ",3)==0){
			connectdb(param);
		}else if(strncmp(str, ":e ",3)==0){
			execfile(param);
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
			endscript=strstr(str,";")!=NULL;
			if(endscript){
				exec(sqlstr);
				sqlstr[0]='\0';
			}
		}
	}
}

void closedb(){
	sqlite3_close(db);
}

int main(int argc, char *argv[])
{
	printf("SQLCMD Version Alpha\nType :h for help\n");
	char* fname=new char[2];
	if(argv[1]){
		strcpy(fname,argv[1]);
	}else{
		strcpy(fname,":memory:");
	}
	if(connectdb(fname)) prompt();
	free(fname);
	closedb();
}