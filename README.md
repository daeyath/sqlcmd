# sqlcmd

After my job is finished at the end of 2022 I will replace my smartphone as a means to practice my programming skills including SQL. And then I haven't found sqlite with command prompt in Google play store. So I made it so it can be used on Android smartphones for my hobby. sqlcmd is not similar to Microsoft SQL Server's sqlcmd, it's just a personal or hobby project, and last used for data base in my company. This product may use under GPL 3.

Commonly to use binary please use your favorit C++ compiler to compile source code. And then run compiled program with `./sqlcmd` in Unix shell. I'm using LLVM compiler for using.

Sqlcmd has an input prompt for entering data as parameters. Parameters are inserted into SQL scripts. For example, the '${Test}' parameter will display Test prompt when the script is run, and the user can enter the value. And parameter changed in SQL script with value is given.

```SELECT '${Name}'``` will prompt
```Name: ``` and filling the prompt with `Hidayat` it will return `Hidayat`. The parameters only run when executing the sql script.
