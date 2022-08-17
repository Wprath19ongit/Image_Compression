#include"quadtree.h"

// function to read PPM file type and image size
void ReadDescription(FILE *f,file_description *file){ 
	fread(file->type,2,1,f);
	file->type[2] = '\0';
	fscanf(f,"%d",&file->height);
	fscanf(f,"%d",&file->width);
	fscanf(f,"%d",&file->size_max);
	fseek(f,ftell(f)+1,SEEK_SET);
}

//-------------------------------------------------------------------------------------------------------------------------------- 

// function to read the pixel array
void SetMatrix(FILE *f, pixel **rgb, int n){ 
	int i;
	for (i=0;i<n;i++)
	{
		fread(rgb[i],sizeof(pixel),n,f);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------- 

// recursive function to build the tree
void MakeTree(TreeNode **tree, int lin, int col, pixel **rgb, int lenght, int *nr_nodes, int prag){
	long long red = 0;
	long long green = 0;
	long long blue = 0;
	long long mean = 0;
	int i,j;
	TreeNode *NewNode = (TreeNode*)malloc(sizeof(TreeNode));
	NewNode -> area = 0;
	NewNode -> top_left = NULL;
	NewNode -> top_right = NULL;
	NewNode -> bottom_left = NULL;
	NewNode -> bottom_right = NULL;
	(*tree) = NewNode;
	(*tree)->area = lenght * lenght;
	(*nr_nodes)++;
	for(i=lin;i<lenght+lin;i++)
	{
		for(j=col;j<lenght+col;j++)
		{
			red += rgb[i][j].red;
			green += rgb[i][j].green;
			blue += rgb[i][j].blue;
		}
	}
	red = red/(lenght*lenght);
	green = green/(lenght*lenght);
	blue = blue/(lenght*lenght);
	for(i=lin;i<lenght+lin;i++)
	{
		for(j=col;j<lenght+col;j++)
		{
			mean += (red - rgb[i][j].red)*(red - rgb[i][j].red)+(green - rgb[i][j].green)*(green - rgb[i][j].green)+(blue - rgb[i][j].blue)*(blue - rgb[i][j].blue);
		}
	}
	mean = mean/(3*lenght*lenght);
	if ( mean <= prag )
	{
		(*tree)->rgb.red = blue;
		(*tree)->rgb.green = green;
		(*tree)->rgb.blue = red;
	}
	else
	{
		(*tree)->rgb.red = 0;
		(*tree)->rgb.green = 0;
		(*tree)->rgb.blue = 0;
		MakeTree(&(*tree)->top_left, lin, col, rgb, lenght/2, nr_nodes, prag);
		MakeTree(&(*tree)->top_right, lin, col + lenght/2, rgb, lenght/2, nr_nodes, prag);
		MakeTree(&(*tree)->bottom_right, lin + lenght/2, col + lenght/2, rgb, lenght/2, nr_nodes, prag);
		MakeTree(&(*tree)->bottom_left, lin + lenght/2, col, rgb, lenght/2, nr_nodes, prag);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------- 

// recursive function to construct the compression vector
void MakeVector(TreeNode *tree, QuadtreeNode *v, int *position, int *nr_color){
	int position_vector;
	v[*position].rgb = tree->rgb;
	v[*position].area = tree->area;
	if (tree->top_left == NULL && tree->top_right == NULL && tree->bottom_right == NULL && tree->bottom_left == NULL)
	{
		v[*position].top_left = -1;
		v[*position].top_right = -1;
		v[*position].bottom_right = -1;
		v[*position].bottom_left = -1;
		(*nr_color)++;
	}
	else
	{
		position_vector = *position;
		(*position)++;
		v[position_vector].top_left = (*position); 
		MakeVector(tree->top_left, v, position, nr_color);
		(*position)++;
		v[position_vector].top_right = (*position);
		MakeVector(tree->top_right, v, position, nr_color);
		(*position)++;
		v[position_vector].bottom_right = (*position);
		MakeVector(tree->bottom_right, v, position, nr_color);
		(*position)++;
		v[position_vector].bottom_left = (*position);
		MakeVector(tree->bottom_left, v, position, nr_color);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------- 

// recursive function to construct the tree in the compression vector
void MakeTreeVector(TreeNode **tree, QuadtreeNode *v, int32_t position){
	TreeNode *NewNode = (TreeNode*)malloc(sizeof(TreeNode));
	NewNode->area = 0;
	NewNode->top_left = NULL;
	NewNode->bottom_left = NULL;
	NewNode->bottom_right = NULL;
	NewNode->top_right = NULL;
	*tree = NewNode;
	(*tree)->area = v[position].area;
	if (v[position].top_left == -1 && v[position].top_right == -1 && v[position].bottom_left == -1 && v[position].bottom_right == -1)
	{
		(*tree)->rgb.red = v[position].rgb.blue;
		(*tree)->rgb.green = v[position].rgb.green;
		(*tree)->rgb.blue = v[position].rgb.red;
	}	
	else
	{
		MakeTreeVector(&(*tree)->top_left, v, v[position].top_left);
		MakeTreeVector(&(*tree)->top_right, v, v[position].top_right);
		MakeTreeVector(&(*tree)->bottom_right, v, v[position].bottom_right);
		MakeTreeVector(&(*tree)->bottom_left, v, v[position].bottom_left);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------- 

// recursive function to construct the pixel array with the compression shaft
void MakeMatrixTree(TreeNode *tree, pixel **a, int lin, int col){
	int i, j;
	if (tree->top_left == NULL && tree->top_right == NULL && tree->bottom_right == NULL && tree->bottom_left == NULL)
	{
		for (i = lin; i < lin + (int)sqrt(tree->area); i++)
		{
			for (j = col; j < col + (int)sqrt(tree->area); j++)
			{
				a[i][j] = tree->rgb;
			}
		}
		return;
	}
	MakeMatrixTree(tree->top_left, a, lin, col);
	MakeMatrixTree(tree->top_right, a, lin, col + (int)sqrt(tree->area)/2);
	MakeMatrixTree(tree->bottom_right, a, lin + (int)sqrt(tree->area)/2, col + (int)sqrt(tree->area)/2);
	MakeMatrixTree(tree->bottom_left, a, lin + (int)sqrt(tree->area)/2, col);
}

//-------------------------------------------------------------------------------------------------------------------------------- 

// depending on the interchange of two nodes in the tree
void SwapNode( TreeNode **tree1 , TreeNode **tree2){
	TreeNode *aux = *tree1;
	*tree1 = *tree2;
	*tree2 = aux;
}

//-------------------------------------------------------------------------------------------------------------------------------- 

// depending on the change of all nodes in a tree in the case of horizontal mirroring
void SwapNodesOrizontal(TreeNode **tree){
	if(*tree == NULL || (*tree) -> top_left == NULL)
	{
		return;
	}
	SwapNode((&(*tree)->top_left),(&(*tree)->top_right));
	SwapNode((&(*tree)->bottom_left),(&(*tree)->bottom_right));
	SwapNodesOrizontal(&(*tree)->top_left);
	SwapNodesOrizontal(&(*tree)->top_right);
	SwapNodesOrizontal(&(*tree)->bottom_right);
	SwapNodesOrizontal(&(*tree)->bottom_left);
}

//-------------------------------------------------------------------------------------------------------------------------------- 

// depending on the change of all nodes in a tree in the case of vertical mirroring
void SwapNodesVertical(TreeNode **tree){
	if(*tree == NULL || (*tree) -> top_left == NULL)
	{
		return;
	}
	SwapNode((&(*tree)->top_left),(&(*tree)->bottom_left));
	SwapNode((&(*tree)->top_right),(&(*tree)->bottom_right));
	SwapNodesVertical(&(*tree)->top_left);
	SwapNodesVertical(&(*tree)->top_right);
	SwapNodesVertical(&(*tree)->bottom_right);
	SwapNodesVertical(&(*tree)->bottom_left);
}

//-------------------------------------------------------------------------------------------------------------------------------- 

// function for building the overlay image tree
void MakeTreeBonus( TreeNode **tree, TreeNode *tree1, TreeNode *tree2, int *nodes_bonus, int *r, int *g, int *b, int *choose){
	if( tree1 -> top_left != NULL && tree2 -> top_left != NULL)
	{
		TreeNode *NewNode = (TreeNode*)malloc(sizeof(TreeNode));
		NewNode->area = tree1->area;
		NewNode->top_left = NULL;
		NewNode->bottom_left = NULL;
		NewNode->bottom_right = NULL;
		NewNode->top_right = NULL;
		*tree = NewNode;
		*nodes_bonus = *nodes_bonus + 1;
		*choose = 0;
		MakeTreeBonus(&(*tree)->top_left,tree1->top_left,tree2->top_left,nodes_bonus,r,g,b,choose);
		MakeTreeBonus(&(*tree)->top_right,tree1->top_right,tree2->top_right,nodes_bonus,r,g,b,choose);
		MakeTreeBonus(&(*tree)->bottom_right,tree1->bottom_right,tree2->bottom_right,nodes_bonus,r,g,b,choose);
		MakeTreeBonus(&(*tree)->bottom_left,tree1->bottom_left,tree2->bottom_left,nodes_bonus,r,g,b,choose);
	}
	if( tree1 -> top_left != NULL && tree2 -> top_left == NULL)
	{
		TreeNode *NewNode = (TreeNode*)malloc(sizeof(TreeNode));
		NewNode->area = tree1->area;
		NewNode->top_left = NULL;
		NewNode->bottom_left = NULL;
		NewNode->bottom_right = NULL;
		NewNode->top_right = NULL;
		*tree = NewNode;
		*nodes_bonus = *nodes_bonus + 1;
		*choose = 1;
		*r = tree2 -> rgb.red;
		*g = tree2 -> rgb.green;
		*b = tree2 -> rgb.blue;
		MakeTreeBonus(&(*tree)->top_left,tree1->top_left,tree2,nodes_bonus,r,g,b,choose);
		MakeTreeBonus(&(*tree)->top_right,tree1->top_right,tree2,nodes_bonus,r,g,b,choose);
		MakeTreeBonus(&(*tree)->bottom_right,tree1->bottom_right,tree2,nodes_bonus,r,g,b,choose);
		MakeTreeBonus(&(*tree)->bottom_left,tree1->bottom_left,tree2,nodes_bonus,r,g,b,choose);

	}
	if( tree1 -> top_left == NULL && tree2 -> top_left != NULL)
	{
		TreeNode *NewNode = (TreeNode*)malloc(sizeof(TreeNode));
		NewNode->area = tree2->area;
		NewNode->top_left = NULL;
		NewNode->bottom_left = NULL;
		NewNode->bottom_right = NULL;
		NewNode->top_right = NULL;
		*tree = NewNode;
		*nodes_bonus = *nodes_bonus + 1;
		*choose = 2;
		*r = tree1 -> rgb.red;
		*g = tree1 -> rgb.green;
		*b = tree1 -> rgb.blue;
		MakeTreeBonus(&(*tree)->top_left,tree1,tree2->top_left,nodes_bonus,r,g,b,choose);
		MakeTreeBonus(&(*tree)->top_right,tree1,tree2->top_right,nodes_bonus,r,g,b,choose);
		MakeTreeBonus(&(*tree)->bottom_right,tree1,tree2->bottom_right,nodes_bonus,r,g,b,choose);
		MakeTreeBonus(&(*tree)->bottom_left,tree1,tree2->bottom_left,nodes_bonus,r,g,b,choose);
	}
	if( tree1 -> top_left == NULL && tree2 -> top_left == NULL)
	{
		TreeNode *NewNode = (TreeNode*)malloc(sizeof(TreeNode));
		if(*choose == 1)
			NewNode->area = tree1->area;
		else
			NewNode->area = tree2->area;
		NewNode->top_left = NULL;
		NewNode->bottom_left = NULL;
		NewNode->bottom_right = NULL;
		NewNode->top_right = NULL;
		*tree = NewNode;
		*nodes_bonus = *nodes_bonus + 1;
		if(*choose == 1)
		{
			(*tree) -> rgb.red = (int)(tree1 -> rgb.red + *r)/2;
			(*tree) -> rgb.green = (int)(tree1 -> rgb.green + *g)/2;
			(*tree) -> rgb.blue = (int)(tree1 -> rgb.blue + *b)/2;
		}
		else if(*choose == 2)
		{
			(*tree) -> rgb.red = (int)(tree2 -> rgb.red + *r)/2;
			(*tree) -> rgb.green = (int)(tree2 -> rgb.green + *g)/2;	
			(*tree) -> rgb.blue = (int)(tree2 -> rgb.blue + *b)/2;
		}
		else
		{
			(*tree) -> rgb.red = (int)(tree1 -> rgb.red + tree2 -> rgb.red)/2;
			(*tree) -> rgb.green = (int)(tree1 -> rgb.green + tree2 -> rgb.green)/2;
			(*tree) -> rgb.blue = (int)(tree1 -> rgb.blue + tree2 -> rgb.blue)/2;
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------------------- 

// function for releasing shaft memory
TreeNode* FreeTree(TreeNode *tree){
	if (tree->top_left == NULL && tree->top_right == NULL && tree->bottom_right == NULL && tree->bottom_left == NULL)
	{
		free(tree);
		return NULL;
	}
	tree->top_left = FreeTree(tree->top_left);
	tree->top_right = FreeTree(tree->top_right);
	tree->bottom_right = FreeTree(tree->bottom_right);
	tree->bottom_left = FreeTree(tree->bottom_left);
	free(tree);
	return NULL;
}

//-------------------------------------------------------------------------------------------------------------------------------- 

// function for building the pixel matrix in the case of mirrors
void MakeMatrixMirror(TreeNode *tree, pixel **a, int lin, int col){
	int i, j;
	if (tree->top_left == NULL || tree->top_right == NULL || tree->bottom_right == NULL || tree->bottom_left == NULL)
	{
		for (i = lin; i < lin + (int)sqrt(tree->area); i++)
		{
			for (j = col; j < col + (int)sqrt(tree->area); j++)
			{
				a[i][j].red = tree->rgb.blue;
				a[i][j].green = tree->rgb.green;
				a[i][j].blue = tree->rgb.red;
			}
		}
		return;
	}
	MakeMatrixMirror(tree->top_left, a, lin, col);
	MakeMatrixMirror(tree->top_right, a, lin, col + (int)sqrt(tree->area)/2);
	MakeMatrixMirror(tree->bottom_right, a, lin + (int)sqrt(tree->area)/2, col + (int)sqrt(tree->area)/2);
	MakeMatrixMirror(tree->bottom_left, a, lin + (int)sqrt(tree->area)/2, col);
}

//-------------------------------------------------------------------------------------------------------------------------------- 
