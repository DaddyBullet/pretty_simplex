# pretty_simplex
Program to solve linear programming problem using simplex method

# Installing
```
git clone git@github.com:DaddyBullet/pretty_simplex.git
cd pretty_simplex/build
make
```
# Using
Run this program with two reqired parametrs:</br>
```./Pretty_simplex condition_file [delimiter] [output_file]```</br>
Where ```condtiton_file``` is a path to csv formated conditions.
It must have following stucture:</br>
max[min],c1,c2,c3,...,cn</br>
a11,a12,...,a1n,<=[=,>=],b1</br>
.</br>
.</br>
am1,am2,...,amn,<=[=,>=],bm</br>

```delimiter``` is delimiter of csv file (coma by defaut)</br>
```output_file``` path to result file (by default program print result to stdout)</br>
Result file will be csv formated with coma as a separator.
