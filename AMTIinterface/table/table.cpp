// File: table.cpp
// Description: Utilities to read/write data in a tabular format
 
/*
History:
	- original code by GBB
	- adapted from table.c, table.h
To do:
	- rewrite using C++ constructs: arrays, iostreams, 
	  new and delete operators, etc.
*/


#include <stdlib.h> // malloc, exit, atof
#include <stdio.h>
#include <string.h>

#include "table.h"

#define COLUMNS	30

#define DEBUG	0

// extern int errno
Table::Table(void)
{ 
	nbRow=0; 
	nbCol=0; 
	v=NULL;
};


Table::~Table(void)
{
	int r;
	if(v!=NULL) {
		for(r=0;r<nbRow;r++) {
			free(v[r]) ;
		}
		free(v);
		v=NULL;
	}
}

float Table::Get(int r, int col)
{
   return(v[r][col]);
}


void Table::Set(int r, int col, float val)
{
   v[r][col]=val;
   // printf("value inside: %8.4f\n",  val);
}


void Table::MakeTable(int nr, int nc)
{

	nbRow=nr;
	nbCol=nc;

	if(nbRow==0 || nbCol==0) {
		v=NULL;
		nbRow=0;
		nbCol=0;
		return;
	}

	int r, c;
	v=(float **)malloc(nbRow*sizeof(float*));
	for(r=0;r<nbRow;r++) {
		v[r]=(float *)malloc(nbCol*sizeof(float));
		for(c=0;c<nbCol;c++) v[r][c]=0.0;
	}
}


void Table::ReadTable(char *file)
/* read data file */
{
	FILE    *f;
	char line[1000];
	char seps[]=" \t";

	char *val;
	int  r, c;

	/* open text file */
	f = fopen (file, "rt");
	if (f == NULL)
	{
		fprintf (stderr, "Error opening file %s\n", file);
		exit(1);
	};
	strcpy(filename,file);

	/* reads number of rows and columns */
	fgets(line,1000,f);
	if(DEBUG)printf("%s",line);
	sscanf(line,"%d %d",&nbRow,&nbCol);

	/* allocate memory */
	v=(float **)malloc(nbRow*sizeof(float*));
	for(r=0;r<nbRow;r++) {
		v[r]=(float *)malloc(nbCol*sizeof(float));
		for(c=0;c<nbCol;c++) v[r][c]=0.0;
	}

	/* read table */
	for(r=0;r<nbRow;r++) {

		/* read next line */
		if(fgets(line,1000,f) == NULL) {
			printf( "ReadTable error: only %d rows read\n",r);
			return;
		}
	    if(DEBUG)printf("%s",line);

		/* get first token */
		val=strtok(line,seps);
		v[r][0]=(float)atof(val);
		if(DEBUG) printf("%g ",v[r][0]); 

		/* get other tokens */	
		for(c=1;c<nbCol;c++) {
			if((val=strtok(NULL,seps))!=NULL) 
				v[r][c]=(float)atof(val);
			else {
				printf( "ReadTable error: only %d columns (%d row)\n",c,r);
				return;
			}
			if(DEBUG) printf("%g ",v[r][c]); 
		}
		if(DEBUG) printf("(%d) \n",c);
	}

	fclose(f);
	if (DEBUG) printf("close file\n");

	if (DEBUG) SaveTable("table.log");

	return;
}


void Table::ReadTableUnformated(char *file)
/* read data file */
{
	FILE    *f;
	char line[1000];
	char *firstLine;

	char *val;
	float **tmp;
	int i, r, c;

	/* open text file */
	f = fopen (file, "rt");
	if (f == NULL)
	{
		fprintf (stderr, "Error opening file %s\n", file);
		exit(1);
	};
	strcpy(filename,file);

	/* reads first line */
	fgets(line,1000,f);
	if(DEBUG)printf("%s",line);

	/* counts number of columns in first line */
	firstLine=(char *)_strdup(line);
	val=strtok(firstLine," ");
	for(c=1;(val=strtok(NULL," "))!=NULL;c++); 
	nbCol=c;
	if( DEBUG) printf("nbCol=%d \n",nbCol); 
	free(firstLine);

	/* init table */
	for(r=0;!feof(f);r++) {

		/* allocate memory */
		/*v=(float **)realloc(v,(r+1)*sizeof(float*));*/
		tmp=(float **)malloc((r+1)*sizeof(float*));
		if(tmp==NULL) { printf("tmp NULL\n");}
		for(i=0;i<r;i++) tmp[i]=v[i];
		if(r>0) free(v); 
		v=tmp;
		v[r]=(float *)malloc((nbCol)*sizeof(float));
		if(v==NULL) { printf("v NULL\n");}

		/* set default values */
		for(c=1;c<nbCol;c++) v[r][c]=0.0;

		/* get first token */
		val=strtok(line," ");
		v[r][0]=(float)atof(val);
		if(DEBUG) printf("%g ",v[r][0]); 

		/* get other tokens */	
		for(c=1;(val=strtok(NULL," "))!=NULL;c++) {
		   if(c<nbCol) v[r][c]=(float)atof(val);
		   if(DEBUG) printf("%g ",v[r][c]); 
		}
		if(DEBUG) printf("(%d) \n",c);

		/* check column number */
		if(nbCol!=c) {
	  	   printf("read_table %s: row %d col %d (nbCol %d)\n",
				filename, r+1, c, nbCol);
		   nbRow=r;
                   fclose(f);
		   return;
		}

		/* read next line */
		fgets(line,1000,f);
	        if(DEBUG)printf("%s",line);
	}

	nbRow=r;
	if (DEBUG) printf("read_table: nbRow=%d\n",nbRow);

	fclose(f);
	if (DEBUG) printf("close file\n");

	if (DEBUG) SaveTable("table.log");

	return;
}

void Table::SaveTable(char *file) 
{
	FILE *f;
	int r,c;

        /* open text file */
        f = fopen (file, "wt");
        if (f == NULL) {
                fprintf (stderr, "Error opening file %s\n", file);
                exit(1);
        };

        if (DEBUG) printf("save_table: nbRow %d nbCol %d\n",
			  nbRow,nbCol);

    fprintf(f,"%d %d\n",nbRow,nbCol);
	for(r=0;r<nbRow;r++) 
	{
		for(c=0;c<nbCol;c++)
			fprintf (f, "%8.4f ", v[r][c]);
		fprintf(f,"\n");
	}

	fclose(f);
}

void Table::SaveTableRows(char *file, int nbRow) 
{
	FILE *f;
	int r,c;

    /* open text file */
    f = fopen (file, "wt");
    if (f == NULL) {
                fprintf (stderr, "Error opening file %s\n", file);
                exit(1);
    };

    if (DEBUG) printf("save_table: nbRow %d nbCol %d\n",
		  nbRow,nbCol);

    fprintf(f,"%d %d\n",nbRow,nbCol);
	for(r=0;r<nbRow;r++) 
	{
		for(c=0;c<nbCol;c++)
			fprintf (f, "%g ", v[r][c]);
		fprintf(f,"\n");
	}

	fclose(f);
}

void Table::PrintTable(void) 
{
	int r,c;

	for(r=0;r<nbRow;r++) 
	{
		for(c=0;c<nbCol;c++)
			printf ("%g ", v[r][c]);
		printf("\n");
	}
}

