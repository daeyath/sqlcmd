#include <iostream>
#include <map>
#include <sqlite3.h>
#include "database.cxx"

using namespace std;

char term[8];

void help(){
	printf(":c file -> connect to database file\n");
	printf(":e sqlfile -> execute sqlfile\n");
	printf(":l -> show license\n");
	printf(":r command -> run shell command\n");
	printf(":s object -> show objects, ex: :s table\n");
	printf(":q -> exit and close database\n");
	printf(":t term -> set terminator for executing\n");
}

void show(const char *objname){
	char str[]="select name as \"%s\" from sqlite_schema where type='%s'";
	char sql[384];
	sprintf(sql, str, objname, objname);
	strcat(sql, " and name not like 'sqlite_%'");
	exec(sql);
}

void setparams(string &str){
	map<string,string> params;
	int start;
	while((start=str.find("${"))>-1){
		int end=str.find("}");
		if(end>-1){
			string name=str.substr(start+2,end-start-2);
			if(params[name]==""){
				cout<<name<<": ";
				char value[128];
				fgets(value,128,stdin);
				value[strlen(value)-1]='\0';
				params[name]=string(value);
			}
			str.erase(start,end-start+1);
			str.insert(start,params[name]);
		}else{
			break;
		}
	}
}

void setterm(const char *c){
	if(c!=0)
		strcpy(term,c);
	else
		printf("term %s\n",term);
}

void execfile(const char *fname){
	FILE *sqlfile=fopen(fname, "r");
	if(sqlfile!=NULL){
		const int maxl=128;
		char str[maxl];
		string sql;
		while(fgets(str, maxl, sqlfile)){
			sql+=string(str);
			if(strstr(str,term)!=NULL){
				sql.erase(sql.find(string(term)));
				setparams(sql);
				exec(sql.c_str());
				sql="";
			}
		}
		if(sql!=""){
			setparams(sql);
			exec(sql.c_str());
		}
		fclose(sqlfile);
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
	char *param[2]={0,0};
	char delim[]=" ";
	char *item=strtok(str, delim);
	while(item!=NULL){
		param[i]=item;
		i++;
		item=strtok(NULL, delim);
	}
	if(strcmp(param[0],":c")==0){
		if(closedb()) connectdb(param[1]);
	}else if(strcmp(param[0],":t")==0){
		setterm(param[1]);
	}else if(strcmp(param[0],":e")==0){
		execfile(param[1]);
	}else if(strcmp(param[0],":q")==0){
		value=1;
	}else if(strcmp(param[0],":s")==0){
		if(param[1]!=NULL) show(param[1]);
	}else if(strcmp(param[0],":l")==0){
		printf("%s %s\n",appname,copyright);
		printf("See %s at %s\n",license_version, copylinks);
	}else if(strcmp(param[0],":h")==0){
		help();
	}else{
		printf("'%s' command not found\n", str);
	}
	return value;
}

void prompt(){
	char *str=0;
	string sql;
	bool newline=true;
	int ric=0, len;
	size_t buflen=0;
	while(ric!=1){
		printf(newline?"SQL> ":"   | ");
		len=getline(&str,&buflen,stdin);
		if(strncmp(":",str,1)==0 && newline){
			str[len-1]='\0';
			ric=runinternalcmd(str);
			if(ric==1) if(!closedb()) ric=0;
		}else{
			sql+=string(str);
			newline=strstr(str,term)!=NULL;
			if(newline){
				if(strcmp(term,";")!=0)
					sql.replace(
						sql.find(string(term)),
						strlen(term),";");
				exec(sql.c_str());
				sql="";
			}
		}
	}
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
		setterm("//");
		prompt();
		state=0;
	}
	return state;
}