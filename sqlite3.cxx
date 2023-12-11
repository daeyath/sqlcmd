#include <iostream>
#include <sqlite3.h>
#include "database.cxx"

const char nem[]="Not enough memory!\nTry dividing the SQL into several parts";
char *str=NULL;
char *sqlstr=NULL;

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
		str=(char*)realloc(str,maxl);
		int dyn=0;
		while(fgets(str, maxl, afile)){
			dyn+=strlen(str);
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
		if(closedb()) connectdb(param[1]);
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
	bool endsql=true;
	int ric=0, dyn=0, len;
	size_t buflen=0;
	while(ric!=1){
		printf(endsql?"sql> ":"   | ");
		len=getline(&str,&buflen,stdin);
		if(strncmp(":",str,1)==0){
			str[len-1]='\0';
			ric=runinternalcmd(str);
			if(ric==1) if(!closedb()) ric=0;
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
	printf("%s Version %s\nType :h for help\n", appname, ver);
	char *fname;
	if(argv[1])
		fname=argv[1];
	else{
		char mem[]=":memory:";
		fname=mem;
	}
	int state=1;
	if(connectdb(fname)){
		prompt();
		state=0;
	}
	return state;
}