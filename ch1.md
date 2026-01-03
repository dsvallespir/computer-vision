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

$$ newPixel = \sum { im[i + a][j + b] * k[a][b]} , a,b \in [-1, 0, 1] $$

## Manual implementation in C++ 

See exercise ex04.

## Gaussian Convolution
A Gaussian kernle gives more weight to the cneter and less to the edges.

Example of 5x5 Gaussian (normalized)

```bash
1  4  6  4  1
4 16 24 16  4
6 24 36 24  6
4 16 24 16  4
1  4  6  4  1

All divided by 256
```

#### Properties:
* Soften without losing too much sharpness.
* Eliminates Gaussian noise.
* It is the bassis of Canny, SIFT, SURF, etc.

In OpenCv

```cpp
cv::GaussianBlur(frame, dst, cv::Size(5,5), 1.0);
```

## Gradients: the base for detecting edges
An edge is a **sharp change in intensity**.

The gradient measures how much the intensity changes in X and Y:

```bash
Gx = horizontal change
Gy = vertical change
|G| = sqrt(Gx² + Gy²)
```

## Sobel (the most classic edge detector in the world)

### Sobel x:
