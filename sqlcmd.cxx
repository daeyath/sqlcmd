#include <iostream>
#include <map>
#include <fstream>
#include <sqlite3.h>
#include "database.cxx"

using namespace std;

typedef map<string,string> params;
const int maxt=6; char term[maxt];
string lastsql;
static int runinternalcmd(const char *str);

void help(){
	printf(":c dbfile -> connect to database file\n");
	printf(":e file -> execute sql file\n");
	printf(":l -> show license\n");
	printf(":print text -> show a text\n");
	printf(":r command -> run shell command\n");
	printf(":s object -> show objects, ex: :s table\n");
	printf(":w file -> write last typing to a file\n");
	printf(":q -> exit and close database\n");
	printf(":t term -> set terminator for exec sql\n");
}

void setparams(string &str, params &param){
	int start; const char empty[]="_";
	while((start=str.find("${"))>-1){
		int end=str.find("}");
		if(end>-1){
			string name=str.substr(start+2,end-start-2);
			if(param[name].empty()){
				cout<<name<<": ";
				string value;
				getline(cin, value);
				if(value.empty())value=empty;
				param[name]=value;
			}
			string value=param[name]==empty?"":param[name];
			str.replace(start, end-start+1, value);
		}else
			break;
	}
}

void show(const char *objname){
	char str[]="select name as \"%s\" from sqlite_schema where type='%s'";
	char tmp[]=" and name not like 'sqlite_%'";
	int len=strlen(str)-1+strlen(tmp)-1+(2*(strlen(objname)-1));
	char sql[len];
	sprintf(sql, str, objname, objname);
	strcat(sql, tmp);
	exec(sql);
}

void setterm(const char *f){
	if(f!=0){
		const int len=strlen(f);
		if(len<=maxt){
			strcpy(term,f);
		}else
			printf("Max chars %i\n",maxt);
	}else printf("terminating with \"%s\" for executing query\n",term);
}

void fixterm(string &s){
	if(strcmp(term,";")!=0) s.erase(
		s.find(term), strlen(term)
	);
}

int execfile(const char *fname){
	int state=0;
	ifstream file(fname); if(file){
		string sql, str; params p;
		while(getline(file, str)){
			setparams(str, p);
			if(str[0]==':'){
				state=runinternalcmd(str.c_str());
			}else{
				sql.append(str);
				sql.append("\n");
			}
			int termpos=str.find(term);
			if(termpos>-1){
				fixterm(sql);
				exec(sql.c_str());
				lastsql=sql;
				sql.clear();
			}
		}
		if(!sql.empty()){
			exec(sql.c_str());
			lastsql=sql;
		}
		file.close();
	}else{
		printf("File '%s' not found.\n", fname);
	}
	return state;
}

int runinternalcmd(const char *str){
	int value=0, j=0;
	while(str[j]!=32 && str[j]!=0)j++;
	const char *param=0, *var=str+j+1;
	if(var[0]!=0)param=var;
	if(strncmp(str,":c",j)==0){
		if(param!=NULL)
			if(closedb())
				connectdb(param);
	}else if(strncmp(str,":t",j)==0){
		setterm(param);
	}else if(strncmp(str,":e",j)==0){
		if(param!=NULL)
			value=execfile(param);
	}else if(strncmp(str,":print",j)==0){
		if(param!=NULL)
			printf("%s\n",param);
	}else if(strncmp(str,":q",j)==0){
		value=1;
	}else if(strncmp(str,":r",j)==0){
		system(param);
	}else if(strncmp(str,":s",j)==0){
		if(param!=NULL)show(param);
	}else if(strncmp(str,":w",j)==0){
		if(param!=NULL){
			FILE *f=fopen(param,"w");
			if(f!=NULL){
				fprintf(f,"%s",lastsql.c_str());
				fclose(f);
			}else{
				fprintf(stderr,"File can't wrote.\n");
			}
		}
	}else if(strncmp(str,":l",j)==0){
		printf("%s %s\n",appname,copyright);
		printf("See %s at %s\n",license_version, copylinks);
	}else if(strncmp(str,":h",j)==0){
		help();
	}else{
		printf("'%s' command not found\n", str);
	}
	return value;
}

void prompt(){
	string sql, str;
	int newline=1, cmd=0, termpos;
	while(cmd!=1){
		printf(newline?"sql> ":"   | ");
		getline(cin, str);
		if(str[0]==':' && newline){
			cmd=runinternalcmd(str.c_str());
			if(cmd==1)
				if(!closedb())
					cmd=0;
		}else{
			sql.append(str);
			termpos = str.find(term);
			newline = termpos > -1;
			if(newline){
				fixterm(sql);
				exec(sql.c_str());
				lastsql=sql;
				sql.clear();
			}
		}
	}
}

int main(int argc, char *argv[])
{
	printf("%s Version %s\n",appname,ver);
	char *fname;
	if(argv[1])
		fname=argv[1];
	else{
		char mem[]=":memory:";
		fname=mem;
	}
	int state=1;
	if(connectdb(fname)){
		printf("Type :h for help\n");
		setterm(";");
		prompt();
		printf("bye!\n");
		state=0;
	}
	return state;
}
