struct ImgProp
{
	int Hpixels;
	int Vpixels;
	unsigned char HeaderInfo[54];
	unsigned long int Hbytes;
};

struct Pixel
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

struct SwapSpace
{
    unsigned long a;
    unsigned long b;
    unsigned long c;
};

unsigned char** CreateBlankBMP(unsigned char FILL);
unsigned char** ReadBMP(char* );
void WriteBMP(unsigned char** , char*);

extern struct ImgProp 	ip;
