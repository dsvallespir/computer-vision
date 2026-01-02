# What is an image?

For practical purposes:
* A black and white image is an array of integers.
* Each number represents the intensity: 0 = black, 255 = white.
* An RGB image is 3 matrices (B, G, R).

## What's a kernel or filter?
A kernel is a small array that "passes" above the image.

#### Average blur kernel 3x3

```shell
1/9 * [
 1 1 1
 1 1 1
 1 1 1
]
```

#### Horizontal Sobel kernel

```shell
[-1 0 1
 -2 0 2
 -2 0 1]
```

#### Laplacian kernel
```shell
[ 0 -1  0
 -1  4 -1
  0 -1  0]
```

## What is a 2D convolution?

For each pixel, multiply yout neighborhood by the kernel and add everything.

