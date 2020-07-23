# TinyExprpp

TinyExprpp is minimalist shader-like scripting language, based on a C++ refactor of Tinyexpr, a very small recursive descent parser and evaluation engine for math expressions.

A simple program:

```
x: sqrt(y * y + z * z);
jump: is_negative ? x < 0;
return: x;
label: is_negative;
return: -1 * x;
```

The variables for this program are bound thusly:

```C
double x, y, z;
    te_variable vars[] = {{"x", &x}, {"y", &y}, {"z", &z}};
```

TinyExprpp can also call custom native functions. Here is a short example:

```C
double my_sum(double a, double b) {
    return a + b;
}

te_variable vars[] = {
    {"mysum", my_sum, TE_FUNCTION2}
};
```

# Useful legacy TinyExpr documentation

## How it works

`te_compile()` uses a simple recursive descent parser to compile your
expression into a syntax tree. For example, the expression `"sin x + 1/4"`
parses as:

![example syntax tree](doc/e1.png?raw=true)

`te_compile()` also automatically prunes constant branches. In this example,
the compiled expression returned by `te_compile()` would become:

![example syntax tree](doc/e2.png?raw=true)

`te_eval()` will automatically load in any variables by their pointer, and then evaluate
and return the result of the expression.

`te_free()` should always be called when you're done with the compiled expression.


## Speed


TinyExpr is pretty fast compared to C when the expression is short, when the
expression does hard calculations (e.g. exponentiation), and when some of the
work can be simplified by `te_compile()`. TinyExpr is slow compared to C when the
expression is long and involves only basic arithmetic.

Here is some example performance numbers taken from the included
**benchmark.c** program:

| Expression | te_eval time | native C time | slowdown  |
| :------------- |-------------:| -----:|----:|
| sqrt(a^1.5+a^2.5) | 15,641 ms | 14,478 ms | 8% slower |
| a+5 | 765 ms | 563 ms | 36% slower |
| a+(5*2) | 765 ms | 563 ms | 36% slower |
| (a+5)*2 | 1422 ms | 563 ms | 153% slower |
| (1/(a+1)+2/(a+2)+3/(a+3)) | 5,516 ms | 1,266 ms | 336% slower |



## Grammar

TinyExpr parses the following grammar:

    <list>      =    <expr> {"," <expr>}
    <expr>      =    <term> {("+" | "-") <term>}
    <term>      =    <factor> {("*" | "/" | "%") <factor>}
    <factor>    =    <power> {"^" <power>}
    <power>     =    {("-" | "+")} <base>
    <base>      =    <constant>
                   | <variable>
                   | <function-0> {"(" ")"}
                   | <function-1> <power>
                   | <function-X> "(" <expr> {"," <expr>} ")"
                   | "(" <list> ")"

In addition, whitespace between tokens is ignored.

Valid variable names consist of a lower case letter followed by any combination
of: lower case letters *a* through *z*, the digits *0* through *9*, and
underscore. Constants can be integers, decimal numbers, or in scientific
notation (e.g.  *1e3* for *1000*). A leading zero is not required (e.g. *.5*
for *0.5*)


## Functions supported

TinyExpr supports addition (+), subtraction/negation (-), multiplication (\*),
division (/), exponentiation (^) and modulus (%) with the normal operator
precedence (the one exception being that exponentiation is evaluated
left-to-right, but this can be changed - see below).

The following C math functions are also supported:

- abs (calls to *fabs*), acos, asin, atan, atan2, ceil, cos, cosh, exp, floor, ln (calls to *log*), log (calls to *log10* by default, see below), log10, pow, sin, sinh, sqrt, tan, tanh

The following functions are also built-in and provided by TinyExpr:

- fac (factorials e.g. `fac 5` == 120)
- ncr (combinations e.g. `ncr(6,2)` == 15)
- npr (permutations e.g. `npr(6,2)` == 30)

Also, the following constants are available:

- `pi`, `e`


## Compile-time options


By default, TinyExpr does exponentiation from left to right. For example:

`a^b^c == (a^b)^c` and `-a^b == (-a)^b`

This is by design. It's the way that spreadsheets do it (e.g. Excel, Google Sheets).


If you would rather have exponentiation work from right to left, you need to
define `TE_POW_FROM_RIGHT` when compiling `tinyexpr.c`. There is a
commented-out define near the top of that file. With this option enabled, the
behaviour is:

`a^b^c == a^(b^c)` and `-a^b == -(a^b)`

That will match how many scripting languages do it (e.g. Python, Ruby).

Also, if you'd like `log` to default to the natural log instead of `log10`,
then you can define `TE_NAT_LOG`.

## Hints

- All functions/types start with the letters *te*.

- To allow constant optimization, surround constant expressions in parentheses.
  For example "x+(1+5)" will evaluate the "(1+5)" expression at compile time and
  compile the entire expression as "x+6", saving a runtime calculation. The
  parentheses are important, because TinyExpr will not change the order of
  evaluation. If you instead compiled "x+1+5" TinyExpr will insist that "1" is
  added to "x" first, and "5" is added the result second.

