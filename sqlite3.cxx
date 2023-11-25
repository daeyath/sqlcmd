#include <iostream>
#include <sqlite3.h>

sqlite3 *db;
char *zErrMsg=0;
int rc;
const char nem[]="Not enough memory!\nTry dividing the SQL into several parts";
char *sqlstr=NULL;

bool connectdb(const char *filename){
	bool res=false;
	rc=sqlite3_open(filename, &db);
	if(rc){
		fprintf(stderr, "Error: '%s' %s\n", filename, sqlite3_errmsg(db));
	}else{
		fprintf(stderr, "Database %s connected\n", filename);
		res=true;
	}
	return res;
}

void closedb(){
	sqlite3_close(db);
}

void exec(const char *str){
	char **azResult;
	int nRow, nColumn;
	rc=sqlite3_get_table(db, str, &azResult, &nRow, &nColumn, &zErrMsg);
	if(rc!=SQLITE_OK){
		fprintf(stderr,"SQL Error: %s\n",zErrMsg);
		sqlite3_free(zErrMsg);
	}else if(nRow>0){
		int lenCol[nColumn];
		for(int i=0; i<nColumn; i++)lenCol[i]=0;
		int z=-1;
		for(int i=0; i<nRow+1; i++){
			for(int j=0; j<nColumn; j++){
				++z;
				int l=azResult[z]==NULL?0:strlen(azResult[z]);
				if(lenCol[j]<l)lenCol[j]=l;
			}
		}	
		z=-1;
		for(int i=0; i<nRow+1; i++){
			for(int j=0; j<nColumn; j++){
				++z;
				printf("%-*s", lenCol[j], azResult[z]);
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
		printf("(%i rows)\n", nRow);
	}else{
		int changes=sqlite3_changes(db);
		if(changes>0)
			printf("%i row changes\n", changes);
	}
	sqlite3_free_table(azResult);
}

void help(){
	printf(":c file - connect to database file\n");
	printf(":e sqlfile - execute sqlfile\n");
	printf(":r command - run shell command\n");
	printf(":s object - show objects, ex: :s table\n");
	printf(":q - exit and close database\n");
}

void show(const char *objname){
	char *str[5];
	char obj[strlen(objname)];
	strcpy(obj, objname);
	char str0[]="select name as \"";
	str[0]=str0;
	str[1]=obj;
	char str2[]="\" from sqlite_schema\nwhere type='";
	str[2]=str2;
	str[3]=obj;
	char str4[]="' and name not like 'sqlite_%'";
	str[4]=str4;
	int len=0;
	for(int i=0; i<5; i++)len+=strlen(str[i]);
	char sql[len]; sql[0]='\0';
	for(int i=0; i<5; i++)strcat(sql, str[i]);
	exec(sql);
}

void execfile(const char *fname){
	FILE *afile=fopen(fname, "r");
	if(afile !=NULL){
		const int maxl=128;
		char line[maxl];
		int dyn=0;
		while(fgets(line, maxl, afile)){
			dyn+=strlen(line);
			if(sqlstr==NULL){
				sqlstr=(char*)malloc(dyn);
				sqlstr[0]='\0';
			}else{
				sqlstr=(char*)realloc(sqlstr, dyn);
			}
			if(sqlstr==NULL){
				printf(nem);
				break;
			}else{
				strcat(sqlstr, line);
			}
		}
		fclose(afile);
		if(sqlstr!=NULL){
			exec(sqlstr);
			sqlstr[0]='\0';
		}
	}else{
		printf("File '%s' not found.\n", fname);
	}
}

int runinternalcmd(char *str){
	int value=0;
	// external
	if(strncmp(":r", str, 2)==0){
		char command[strlen(str)-2];
		strncpy(command, str+2, strlen(str));
		system(command);
		return value;
	}
	// internal
	int i=0;
	char *param[2];
	char delim[]=" ";
	char *item=strtok(str, delim);
	while(item!=NULL){
		param[i]=item;
		i++;
		item=strtok(NULL, delim);
	}
	if(strcmp(param[0],":c")==0){
		closedb();
		connectdb(param[1]);
	}else if(strcmp(param[0],":e")==0){
		execfile(param[1]);
	}else if(strcmp(param[0],":q")==0){
		value=1;
	}else if(strcmp(param[0],":s")==0){
		if(param[1]!=NULL) show(param[1]);
	}else if(strcmp(param[0],":h")==0){
		help();
	}else{
		printf("'%s' command not found\n", str);
	}
	return value;
}

void prompt(){
	char *str=NULL;
	bool endsql=true;
	int ric=0, dyn=0, len;
	size_t buflen=0;
	while(ric!=1){
		printf(endsql?"sql> ":"   | ");
		len=getline(&str,&buflen,stdin);
		if(strncmp(":",str,1)==0){
			str[len-1]='\0';
			ric=runinternalcmd(str);
		}else{
			dyn+=buflen;
			if(sqlstr==NULL){
				sqlstr=(char*)malloc(dyn);
				sqlstr[0]='\0';
			}else{
				sqlstr=(char*)realloc(sqlstr, dyn);
			}
			if(sqlstr==NULL){
				printf(nem);
				break;
			}else{
				strcat(sqlstr, str);
				endsql=strstr(str,";")!=NULL;
				if(endsql){
					exec(sqlstr);
					dyn=0;
					sqlstr[0]='\0';
				}
			}
		}
	}
	free(sqlstr);
	free(str);
}

int main(int argc, char *argv[])
{
	const char ver[]="0.0.5";
	printf("SQLCMD Version %s\nType :h for help\n", ver);
	char *fname;
	if(argv[1]){fname=argv[1];}
	else{
		char mem[]=":memory:";
		fname=mem;
	}
	if(connectdb(fname)){
		prompt();
		closedb();
	}
}