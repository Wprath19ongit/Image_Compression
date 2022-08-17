#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct pixel
{
	unsigned char red;		
	unsigned char green;
	unsigned char blue;
}__attribute__((packed)) pixel;

typedef struct QuadtreeNode {
  	pixel rgb;
  	int area;
  	int top_left , top_right;
  	int bottom_left , bottom_right;
}__attribute__((packed)) QuadtreeNode;

typedef struct file_description
{
	char type[3];
	int width;
	int height;
	int size_max;
}__attribute__((packed)) file_description;

typedef struct TreeNode
{
	pixel rgb;
	int area;
	struct TreeNode *top_left, *bottom_left, *bottom_right, *top_right;
}__attribute__((packed)) TreeNode;

void ReadDescripton (FILE * f, file_description * file); // function to read PPM file type and image size
void SetMatrix (FILE * f, pixel ** a, int n); // function to read the pixel array
void MakeTree (TreeNode ** tree, int lin, int col, pixel ** rgb, int lenght, int * nr_nodes, int threshold); // recursive function to build the tree
void MakeVector (TreeNode * tree, QuadtreeNode * v, int * position, int * nr_color); // recursive function to construct the compression vector
void MakeTreeVector (TreeNode ** tree, QuadtreeNode * v, int position); // recursive function to construct the tree in the compression vector
void MakeMatrixTree (TreeNode * tree, pixel ** a, int lin, int col); // recursive function to construct the pixel matrix with the compression tree
void SwapNode (TreeNode ** tree1, TreeNode ** tree2); // function of interchange of two nodes in the tree
void SwapNodesOrizontal (TreeNode ** tree); // function of changing all nodes in a tree in the case of horizontal mirroring
void SwapNodesVertical (TreeNode ** tree); // function of changing all nodes in a tree in the case of vertical mirroring
void MakeTreeBonus (TreeNode ** tree, TreeNode * tree1, TreeNode * tree2, int * nodes_bonus, int * r, int * g, int * b, int * choose); // function for building the overlay image tree
TreeNode * FreeTree (TreeNode * tree); // function for releasing shaft memory
void MakeMatrixMirror (TreeNode * tree, pixel ** a, int lin, int col); // function for building the pixel matrix in the case of mirrors