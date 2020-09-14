#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

unsigned char red (unsigned char *p) {
  return *p;
}

unsigned char green (unsigned char *p) {
  return *(p+1);
}

unsigned char blue (unsigned char *p) {
  return *(p+2);
}

double distance (unsigned char *p1, unsigned char *p2) {
  double d2 = pow(red(p1) - red(p2), 2) + pow(green(p1) - green(p2), 2) + pow(blue(p1) - blue(p2), 2);
  return sqrt(d2);
}

double* compute_pixel_energies(unsigned char *data, int x, int y) {
  double *energies = malloc (x * y * sizeof(double));
  int i, j;
  for (i = 0; i < y; i++) {
    for (j = 0; j < x; j++) {
      unsigned char *p = data + (i*x + j)*3;
      unsigned char *left, *right;

      if ((j > 0) && (j < x-1)) {
        left = p - 3;
        right = p + 3;
      }
      else if (j == 0) {
        left = p;
        right = p + 6;
      }
      else {
        left = p - 6;
        right = p;
      }

      int r = red(left) - red(right);
      int g = green(left) - green(right);
      int b = blue(left) - blue(right);
      double x_energy = pow(r, 2) + pow(g, 2) + pow(b, 2);
      
      unsigned char *up, *down;

      if (i > 0 && i < y-1) {
        up = p - 3*x;
        down = p + 3*x;
      }

      else if (i == 0) {
        up = p;
        down = p + 6*x;
      }
     
      else {
        up = p - 6*x;
        down = p;
      }
     
      r = red(up) - red(down);
      g = green(up) - green(down);
      b = blue(up) - blue(down);
      double y_energy = pow(r, 2) + pow(g, 2) + pow(b, 2);
      
      double energy = sqrt(x_energy + y_energy);
      *(energies + i*x + j) = energy;
    }
  }
  return energies;
}

int* find_seam (unsigned char *data, int x, int y) {
  double* pixel_energies = compute_pixel_energies(data, x, y);
  double* lowest_energies = malloc (x * y * sizeof(double));
  
  int i, j;
  for (i = 0; i < x; i++) {
    lowest_energies[i] = pixel_energies[i];
  }

  for (i = 1; i < y; i++) {
    for (j = 0; j < x; j++) {
      double *e = lowest_energies + (i*x + j);
      double a, b, c; 
      if ((j > 0) && (j < x-1)) {
        a = *(e - x - 1);
        b = *(e - x);
        c = *(e - x + 1);
      }
      else if (j == 0) {
        a = b = *(e - x);
        c = *(e - x + 1);
      }
      else {
        a = *(e - x - 1);
        b = c = *(e - x);
      }
      
      double min = a < b ? a : b;
      min = min < c ? min : c;

      *e = *(pixel_energies + (i*x + j)) + min;
    }
  }
  free (pixel_energies);

  int *seam = malloc (y * sizeof(int));

  int min_index = 0;
  double min = DBL_MAX;
  for (i = 0; i < x; i++) {
    double cur = *(lowest_energies + i + x*(y-1));
    min_index = min < cur ? min_index : i;
    min = *(lowest_energies + x*(y-1) + min_index);
  }

  double *a, *b, *c; 
  seam[y-1] = min_index;
  for (i = y - 2; i >= 0; i--) {
    int prev_index = seam[i+1];
    double *prev_p = lowest_energies + prev_index + x*(i+1);

    if ((prev_index > 0) && (prev_index < x-1)) {
      a = prev_p - x - 1;
      b = prev_p - x;
      c = prev_p - x + 1;
    }
    else if (prev_index == 0) {
      a = b = prev_p - x;
      c = prev_p - x + 1;
    }
    else {
      a = prev_p - x - 1;
      b = c = prev_p - x;
    }
    
    double* min_p = *a < *b ? a : b;
    min_p = *min_p < *c ? min_p : c;
    min_index = min_p - lowest_energies - x * i;
    seam[i] = min_index;
  }
  
  free(lowest_energies);
  return seam;
}

unsigned char* remove_seam (unsigned char *data, int x, int y) {
  int *seam = find_seam(data, x, y);

  int i;
  for(i = 0; i < y; i++) {
    data[(i*x + seam[i])*3] = 255;
    data[(i*x + seam[i])*3 + 1] = 0;
    data[(i*x + seam[i])*3 + 2] = 0;
  }

  unsigned char *new_data = malloc((x-1)*y*3*sizeof(unsigned char));
  int j;
  for (i = 0; i < y; i++) {
    for (j = 0; j < x-1; j++) {
      unsigned char r, g, b;
      r = data[((i*x) + j + (j >= seam[i])) * 3]; 
      g = data[((i*x) + j + (j >= seam[i])) * 3 + 1]; 
      b = data[((i*x) + j + (j >= seam[i])) * 3 + 2]; 
      
      new_data[(i*(x-1) + j)*3] = r;
      new_data[(i*(x-1) + j)*3 + 1] = g;
      new_data[(i*(x-1) + j)*3 + 2] = b;
    }
  }

  free(seam);
  return new_data;
}

int main (int argc, char *argv[]) {
  if (argc > 4) {
    printf("Too many arguments. Please supply three arguments: the input and output image filenames and number of columns to remove\n");
    return 1;
  }

  if (argc < 4) {
    printf("Too few arguments. Please supply three arguments: the input and output image filenames and number of columns to remove\n");
    return 2;
  }

  int x,y,n;
  unsigned char *orig_data = stbi_load(argv[1], &x, &y, &n, 3);
  if (orig_data == NULL) {
    printf("Loading image data failed");
    return 3;
  }

  int cols_to_rem = atoi(argv[3]);
  if (cols_to_rem > (x - 2)) {
    printf ("Please specify a number of columns that is at least 2 fewer than the width of the image\n");
  }

  unsigned char *old_data = malloc(x * y * n * sizeof(unsigned char));
  int i;
  for (i = 0; i < x * y * n; i++)
    old_data[i] = orig_data[i];

  stbi_image_free(orig_data);

  for (i = 0; i < cols_to_rem; i++) {
    unsigned char *new_data = remove_seam(old_data, x-i, y);
    free(old_data);
    old_data = new_data;
  }

  stbi_write_jpg(argv[2], x - cols_to_rem, y, n, old_data, 100);

  free(old_data);
}
