# sqlcmd

After my job is finished at the end of 2022 I will replace my smartphone as a means to practice my programming skills including SQL. And then I haven't found sqlite with command prompt in Google play store. So I made it so it can be used on Android smartphones for my hobby. sqlcmd is not similar to Microsoft SQL Server's sqlcmd, it's just a personal or hobby project.

Run `clang -o sqlcmd sqlite3.cxx` to compile source code and then run `./sqlcmd` in your favorite shell.

Usually, to test data entry, you can use a shell script.  As support, I provide a small program, namely execsql, which can be used together with a shell script. To using it, compile source code with `clang -o execsql execsql.cxx`, and you may use it in your shell script as:

```
export sqlconnection
sqlconnection="test.file"
./execsql "select * from atable"
```

But execsql only supports transactions within its own quotes. Does not apply after next execsql.
