`ptm` (PokeTheMoos) this is a trivial commandline application which sends a single string to a moos community. Syntax is
```
./ptm var_name@app_name string_to_send
```

 or interactively reading input from stdin:
```
./ptm var_name@app_name
```

 Example : send the string "lala" uner the name of x and make it look like it
 came from an app called foo.
```
./ptm x@foo lala 
```
 Or, of course, using piping on the command line
```
echo lala | ./ptm x@foo 
```