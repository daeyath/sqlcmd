#include <iostream>
#include <sqlite3.h>

sqlite3 *db;
char *zErrMsg=0;
int rc;
const char nem[]="Not enough memory.\n";
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
		char str[maxl];
		int dynlen=0;
		while(fgets(str, maxl, afile)){
			dynlen+=strlen(str);
			if(sqlstr==NULL){
				sqlstr=(char*)malloc(dynlen);
				sqlstr[0]='\0';
			}else{
				sqlstr=(char*)realloc(sqlstr, dynlen);
			}
			if(sqlstr==NULL){
				printf(nem);
				break;
			}else{
				strcat(sqlstr, str);
			}
		}
		fclose(afile);
		if(sqlstr!=NULL){
			exec(sqlstr);
			strcpy(sqlstr, "");
		}
	}else{
		printf("File '%s' not found.\n", fname);
	}
}

int runinternalcmd(const char *str){
	int ret=0;
	const int pos=3;
	const int len=strlen(str)-pos+1;
	char param[len];
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
	return ret;
}

void prompt(){
	char *str=NULL;
	bool endsql=true;
	int ric, dyn=0, len;
	size_t buflen=0;
	while(true){
		printf(endsql?"sql> ":"   | ");
		len=getline(&str,&buflen,stdin);
		if(strncmp(":",str,1)==0){
			str[len-1]='\0';
			ric=runinternalcmd(str);
			if(ric==1) break;
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
					strcpy(sqlstr,"");
				}
			}
		}
	}
	free(sqlstr);
	free(str);
}

void closedb(){
	sqlite3_close(db);
}

int main(int argc, char *argv[])
{
	printf("SQLCMD Version Alpha\nType :h for help\n");
	char *fname=new char[2];
	if(argv[1]){
		strcpy(fname,argv[1]);
	}else{
		strcpy(fname,":memory:");
	}
	if(connectdb(fname)) prompt();
	free(fname);
	closedb();
}