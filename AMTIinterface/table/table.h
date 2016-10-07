/* table.h */

class Table {
	float **v ;
	char filename[50];
	int nbRow;
	int nbCol;

public:

	Table(void);
	~Table(void);

	void Set(int r, int col, float val);
	float Get(int r, int col);
	int GetRow(void) {return nbRow;};
	int GetCol(void) {return nbCol;};

	void MakeTable(int nbRow, int nbCol);
	void ReadTable(char *filename);
	void ReadTableUnformated(char *filename);
	void PrintTable(void); 
	void SaveTable(char *filename);
	void SaveTableRows(char *filename, int nbRow); 
};
