# Symbolic executor

Example incorrect program:
```C
func max(x, y) {

  dy = x-y+1
  if(dy == 5) {
    r = 2*dy + 5
  } else {
    r = x - dy +1
  }
  assert(r == y)
}
```

The tool execute two query to SMT solver:

```dy = x-y+1 /\  dy==5  /\  r = 2*dy + 5 /\  not(r = y)```

```dy = x-y+1 /\  not(dy==5)  /\  r = x + dy-1 /\  not(r = y)```

The first query returns SAT and model: {x=6; y=1}, it means that the assertion failed. 

Grammar of input language is described in ```calc.y```.

## Requiments

Yod should install:

* flex
* bison
* [cvc4](https://cvc4.github.io/)


