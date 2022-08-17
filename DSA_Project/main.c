#include "quadtree.h"

int main(int argc, char const *argv[]){
	FILE *f;
	file_description file;  				// structure to retain information from the beginning of the PPM file
	pixel **pixels; 						// pixel array
	pixel **pixels2; 						// pixel array (in case of bonus)
	int i;
	int lin = 0,col = 0;
	int r = 0,g = 0,b = 0,choose = 0; 		// r, g, b variables to remember the colors for a bigger block (in case of the bonus) and choose to remember which block is bigger
	QuadtreeNode *v = NULL; 				// compression vector
	int prag; 								// threshold for compression
	int nr_nodes = 0; 						// no nodes
	int nr_nodes1 = 0;	
	int nr_nodes2 = 0;
	int nr_color = 0; 						// no colors / leaves
	int position = 0; 						// variable to put elements in vector
	TreeNode *tree = NULL;
	TreeNode *tree1 = NULL;
	TreeNode *tree2 = NULL;
	
	
	// COMPRESSION
	if(!strcmp(argv[1], "-c")){
		prag = atoi(argv[2]);
		f = fopen(argv[3],"rb");
		ReadDescription(f,&file); 								// We read the PPM file header
		pixels = (pixel**)malloc(file.height*sizeof(pixel*)); 	// We allocate memory for the pixel array
		
		for (i=0;i<file.height;i++){
			pixels[i] = (pixel*)malloc(file.height*sizeof(pixel));
		}	

		SetMatrix(f, pixels, file.height); 						// We read from the PPM file and assign in the pixel matrix
		fclose(f);
		MakeTree(&tree, lin, col, pixels, file.height, &nr_nodes, prag); // We build the compression shaft
		v = (QuadtreeNode*)malloc(nr_nodes*sizeof(QuadtreeNode)); // We allocate memory for the compression vector
		MakeVector (tree, v, & position, & nr_color); 			// We construct the compression vector
		f = fopen(argv[4], "wb");
		fwrite(&nr_color, sizeof(int), 1, f); 				// Writing to file
		fwrite(&nr_nodes, sizeof(int), 1, f);
		fwrite(v, sizeof(QuadtreeNode), nr_nodes, f);
		fclose(f);
		for (i = 0; i < file.height; i++)
			free(pixels[i]);
		free(pixels);
		free(v);
		tree = FreeTree(tree);
	}

	// DECOMPRESSION
	if (!strcmp(argv[1], "-d"))
	{
		f = fopen(argv[2], "rb"); 									// File reading and memory allocation for vector
		fread(&nr_color, sizeof(int), 1, f); 
		fread(&nr_nodes, sizeof(int), 1, f);
		v = (QuadtreeNode*)malloc(nr_nodes*sizeof(QuadtreeNode));	
		fread(v, sizeof(QuadtreeNode), nr_nodes, f);
		fclose(f);
		pixels = (pixel**)malloc((int)sqrt(v[0].area)*sizeof(pixel*)); // We allocate memory for the pixel array
		for (i=0;i<(int)sqrt(v[0].area);i++)
		{
			pixels[i] = (pixel*)malloc((int)sqrt(v[0].area)*sizeof(pixel));
		}
		MakeTreeVector(&tree, v, position); 							// We build the tree using the compression vector
		MakeMatrixTree (tree, pixels, lin, col); 						// We build the pixel matrix with the help of the tree
		f = fopen (argv [3], "wb"); 									// Write the header in the PPM file
		fprintf(f,"P6\n");
		fprintf(f,"%d",(int)sqrt(v[0].area));
		fprintf(f," ");
		fprintf(f,"%d",(int)sqrt(v[0].area));
		fprintf(f,"\n");
		fprintf(f,"255\n");
		for (i = 0; i < (int)sqrt(v[0].area); i++) // Writing the pixel matrix
			fwrite(pixels[i], sizeof(pixel), (int)sqrt(v[0].area), f);
		fclose(f);
		for (i = 0; i < (int)sqrt(v[0].area); i++)
			free(pixels[i]);
		free(pixels);
		free(v);
		tree = FreeTree(tree);
	}

	// MIRRORING
	if (!strcmp(argv[1], "-m"))
	{
		if(!strcmp(argv[2], "h")) // HORIZONTAL
		{
			prag = atoi(argv[3]);
			f = fopen(argv[4],"rb");
			ReadDescription(f,&file); // We read the PPM file header
			pixels = (pixel**)malloc(file.height*sizeof(pixel*)); // We allocate memory for the pixel array
			for (i=0;i<file.height;i++)
			{
				pixels[i] = (pixel*)malloc(file.height*sizeof(pixel));
			}	
			SetMatrix(f, pixels, file.height); // We read from the PPM file and assign in the pixel matrix
			fclose(f);

			MakeTree(&tree, lin, col, pixels, file.height, &nr_nodes, prag); // We build the compression tree 
			SwapNodesOrizontal (& tree); // Change the 
			MakeMatrixMirror(tree, pixels, 0, 0); // We build the pixel matrix
			f = fopen(argv[5], "wb");
			fprintf(f,"P6\n");
			fprintf(f,"%d",file.height);
			fprintf(f," ");
			fprintf(f,"%d",file.height);
			fprintf(f,"\n");
			fprintf(f,"255\n");
			for (i = 0; i < file.height; i++)
				fwrite(pixels[i], sizeof(pixel), file.height, f);
			fclose(f);
			for (i = 0; i < file.height; i++)
				free(pixels[i]);
			free(pixels);
			tree = FreeTree(tree);
		}
		else // vertically
		{
			prag = atoi(argv[3]);
			f = fopen(argv[4],"rb");
			ReadDescription(f,&file); 									// We read the PPM file header
			pixels = (pixel**)malloc(file.height*sizeof(pixel*)); 		// We allocate memory for the pixel array
			for (i=0;i<file.height;i++)
			{
				pixels[i] = (pixel*)malloc(file.height*sizeof(pixel));
			}	
			SetMatrix(f, pixels, file.height);								// We read from the PPM file and assign in the pixel matrix
			fclose(f);
			MakeTree(&tree, lin, col, pixels, file.height, &nr_nodes, prag); // We build the compression shaft
			SwapNodesVertical(&tree); 										 // We change the knots
			MakeMatrixMirror(tree, pixels, 0, 0); 							 // We build the pixel matrix
			f = fopen(argv[5], "wb");
			fprintf(f,"P6\n");
			fprintf(f,"%d",file.height);
			fprintf(f," ");
			fprintf(f,"%d",file.height);
			fprintf(f,"\n");
			fprintf(f,"255\n");
			for (i = 0; i < file.height; i++)
				fwrite(pixels[i], sizeof(pixel), file.height, f);
			fclose(f);
			for (i = 0; i < file.height; i++)
				free(pixels[i]);
			free(pixels);
			tree = FreeTree(tree);
		}
	}

	// OVERLAP
	if(!strcmp(argv[1], "-o"))
	{
		prag = atoi(argv[2]);

		f = fopen(argv[3],"rb");	
		ReadDescription(f,&file);					 						// We read the PPM file header
		pixels = (pixel**)malloc(file.height*sizeof(pixel*));
		for (i=0;i<file.height;i++)
		{
			pixels[i] = (pixel*)malloc(file.height*sizeof(pixel));
		}	
		SetMatrix(f, pixels, file.height);								 	// We read from the PPM file and assign in the pixel matrix
		fclose(f);
		MakeTree(&tree1, lin, col, pixels, file.height, &nr_nodes1, prag);	// We're building the tree
		
		f = fopen(argv[4],"rb");
		ReadDescription(f,&file); 											 // We read the PPM file header
		pixels2 = (pixel**)malloc(file.height*sizeof(pixel*));

		for (i=0;i<file.height;i++){
			pixels2[i] = (pixel*)malloc(file.height*sizeof(pixel));
		}	

		SetMatrix(f, pixels2, file.height); 		 						// We read from the PPM file and assign in the pixel matrix
		fclose(f);
		MakeTree(&tree2, lin, col, pixels2, file.height, &nr_nodes2, prag); // We're building the tree
		
		MakeTreeBonus(&tree,tree1,tree2,&nr_nodes,&r,&g,&b,&choose); 		// We build the final image tree
		MakeMatrixMirror(tree, pixels, lin, col); 	 						// We build the matrix
		f = fopen(argv[5], "wb");
		fprintf(f,"P6\n");
		fprintf(f,"%d",file.height);
		fprintf(f," ");
		fprintf(f,"%d",file.height);
		fprintf(f,"\n");
		fprintf(f,"255\n");
		for (i = 0; i < file.height; i++)
			fwrite(pixels[i], sizeof(pixel), file.height, f);
		fclose(f);

		for (i = 0; i < file.height; i++)
			free(pixels[i]);
		free(pixels);
		for (i = 0; i < file.height; i++)
			free(pixels2[i]);
		free(pixels2);

		FreeTree(tree);
		FreeTree(tree2);
		FreeTree(tree1);
	}

	return 0;
}