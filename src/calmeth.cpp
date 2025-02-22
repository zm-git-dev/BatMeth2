#include <iostream>
#include <string.h>
#include <cstdio>
#include <assert.h>
#include <cstdlib>
#include <string>
#include <math.h>
#include "limits.h"
#include <map>
#include<vector>
#include <pthread.h>
#include <algorithm>
#include <sstream>
#include <ctype.h>
#define MAX_HITS_ALLOWED 1500
#define CHROMSIZE 100
#define BATBUF 2000
#define MAXTAG 500

#define QLIMIT_FLOAT 30.0f
//#define max(a,b) ( ((a)>(b)) ? (a):(b) )
//#define min(a,b) ( ((a)>(b)) ? (b):(a) )

const float QLIMIT=QLIMIT_FLOAT;

struct Mismatch
{
	char Base;
	int Pos;
};
struct Gene_Hash
{
	char* Genome;
	int Index;
};
struct Methy_Hash
{
	int *plusMethylated,*plusUnMethylated;//,*plusCover
	int *plusG,*plusA;
	int *NegMethylated,*NegUnMethylated;//,*NegCover
	int *NegG,*NegA;
	int *MethContext;
	int Index;
};
struct Offset_Record
{
	char Genome[40];
	unsigned Offset;
} Temp_OR; 
typedef struct {
   Gene_Hash* Genome_List;
   Offset_Record* Genome_Offsets;
   char* Org_Genome;
   char* Marked_Genome;
   char* Marked_GenomeE;
   Methy_Hash* Methy_List;
   FILE* OUTFILE;
   FILE* INFILE;
   int ThreadID;
   off64_t File_Size;
} ARGS;
struct Threading
{
	pthread_t Thread;
	unsigned r;
	void *ret;
	ARGS Arg;
};

using namespace std;
bool Collision=false;
map <string,int> String_Hash;
float ENTROPY_CUTOFF=0;
///g++ ./src/split.cpp -o ./src/split -lpthread
//{-----------------------------  FUNCTION PRTOTYPES  -------------------------------------------------
int strSearch(char *str1,char *str2);
int Get_ED(std::string & S);
char* replace(char*src, char*sub, char*dst);
void MinandSec(unsigned a[MAX_HITS_ALLOWED][10], int left, int right, int&min, int&second,int & loci);
//void MaxandSec(int a[], int left, int right, int & max, int & second);
FILE* File_Open(const char* File_Name,const char* Mode);
void Show_Progress(float Percentage);
void fetch(char *str, char c1, char c2, char *buf);
string&   replace_all(string&   str,const   string&   old_value,const   string&   new_value,int Read_Len);
std::string m_replace(std::string str,std::string pattern,std::string dstPattern,int count=-1);
float Pr(float Q);
void Build_Pow10();
void initMem(char* temp,int size,char* temp2);
void *Process_read(void *arg);
#define Init_Progress()	printf("======================]\r[");//progress bar....
#define Done_Progress()	printf("\r[++++++++ 100%% ++++++++]\n");//progress bar....
#define random(x) (rand()%x)
inline unsigned char Hash(char* S);
int Get_String_Length(FILE* INFILE);
string remove_soft_split(string & cigar,int Read_Len,int pos);
void Get_longest_macth(const char* cig,int & edit,int & longestM,int & longestM_location);
void ReverseC_Context(char* Dest,const char* seq,int & stringlength);
//---------------------------------------------------------------------------------------------------------------
unsigned Total_Reads_all;
//----mapping count
pthread_mutex_t mapper_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned Tot_Unique_Org=0;//total unique hits obtained
unsigned ALL_MAP_Org=0;
unsigned Tot_Unique_Remdup=0;//total unique hits obtained after removing dups...
unsigned ALL_Map_Remdup=0;

const int MN=2,Q_Gap=10,Match_score=1;
bool REMOVE_DUP=false; //true to removeDup, false will not remove PCR-dup
unsigned Mismatch_Qual[255][255][255]; //[readLength][255][255]
int QualCut=0;
const int POWLIMIT=300;
float POW10[POWLIMIT];
int QUALITYCONVERSIONFACTOR=33;
//----------
bool Paired=false;
int UPPER_MAX_MISMATCH=2;
bool UNIQUE=true;
bool Methratio=false;
//-------meth count
pthread_mutex_t meth_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t meth_counter_mutex24 = PTHREAD_MUTEX_INITIALIZER;
unsigned long non_met_CG=0;
unsigned long met_CG=0;
unsigned long non_met_CHG=0;
unsigned long met_CHG=0;
unsigned long non_met_CHH=0;
unsigned long met_CHH=0;

int PrintMethState=1;//1 print 0 no
int MethAllMap=1;
int Sam=1;//true
unsigned Number_of_Tags;
//}-----------------------------   GLOBAL VARIABLES  -------------------------------------------------
char Char2Comp[255];
const int INITIAL_PROGRESS_READS =1000;//progress bar initial count..
float calc_Entropy(string readString, int L);
void Print_Mismatch_Quality(FILE* OUTFILE_MM, int L);
bool NP=false;//not paired
bool SamSeqBeforeBS=false;
int RegionBins=1000;
bool SAM_Header=true;
int NTHREAD=6;
int main(int argc, char* argv[])
{
	time_t Start_Time,End_Time;
	printf("\nBatMeth2::Split v2.0\n");
	const char* Help_String="Command Format :  calmeth [options]  -g GENOME -n <number of mismatches> -i mapfile -m <methratio outfile> -p 6\n"
		"\nUsage:\n"
		"\t-o|--out              output modified sam file\n"
		"\t-g|--genome           Genome\n"
		"\t-n|--Nmismatch        Number of mismatches\n"
		"\t-i|--input            map format file\n"
		"\t-p|--threads          the number of threads.\n"
		"\t-m|--methratio        [MethFileName]  Methratio output file\n"
		"\t-Q [int]              caculate the methratio while read QulityScore >= Q. default:10\n"
		"\t-M Get methratio      Only use perfectMapping reads\n"
		"\t-C|--coverage         >= <INT> coverage. default:3\n"
		"\t-nC		         >= <INT> nCs per region. default:3\n"
		"\t-R |--Regions         Bins for DMR caculate , default 1kb .\n"
		"\t-PF| --out_Prefix     Cytosines/Bins methylation output file prefix, DMC/DMR inputfile. default: Methyl\n"
		"\t-b|--binsfile         DNA methylation level distributions in chrosome, default output file: methBins.txt\n"
		"\t-S|--chromStep         Chrosome using an overlapping sliding window of 100000bp at a step of 50000bp. default step: 50000(bp)\n"
		"\t-r|--remove_dup       REMOVE_DUP\n"
//		"\t-S|--single-end       Single-end-reads <default>\n"
//		"\t-P|--paired-end       Paired-end-reads\n"
		"\t-f|--sam [1|0]        f for sam format outfile.default: 1,sam format.\n"
		"\t--sam-nohead          Supppress header lines for SAM output.\n"
		"\t--sam-seq-beforeBS    Converting BS read to the genome sequences.\n"
		"\t-u|--unique           u for unique mapping <default>\n"
		"\t-a|--all              a for unique and multipe mapping\n"
		"\t-ms|--methState [1|0]        add sequence methylation mapping state in the output file. default: 1, print\n"
		//"\t-c                    c for color-read\n"
		"\t-h|--help";

	Char2Comp['A']=Char2Comp['a']='T';
	Char2Comp['C']=Char2Comp['c']='G';
	Char2Comp['G']=Char2Comp['g']='C';
	Char2Comp['T']=Char2Comp['t']='A';
	Char2Comp['N']=Char2Comp['n']='N';
	int Genome_CountX=0;
	char* Output_Name;
	char* methOutfileName;
	string Geno;
//	int Current_Option=0;
	int InFileStart=0,InFileEnd=0;
	char *InFile;
	string binsOutfileName="methBins.txt";
	string RegionOutFiles="Methyl";
	int coverage=3;
	int binspan=50000;
	int nCs=3;
	//
	unsigned int M=0,Mh=0,H=0,hU=0,U=0;
	unsigned int M_CG=0,Mh_CG=0,H_CG=0,hU_CG=0,U_CG=0;
	unsigned long mCGdensity[100],mCHGdensity[100],mCHHdensity[100];
	unsigned long plus_mCGcount=0,plus_mCHGcount=0,plus_mCHHcount=0;
	unsigned long plusCGcount=0,plusCHGcount=0,plusCHHcount=0;
	unsigned long Neg_mCGcount=0,Neg_mCHGcount=0,Neg_mCHHcount=0;
	unsigned long NegCGcount=0,NegCHGcount=0,NegCHHcount=0;
//	int Par_Count=0;
	std::string CMD;

	for(int i=1;i<argc;i++)
	{
		if(!strcmp(argv[i], "-o") ||!strcmp(argv[i], "--out")  )
		{
			Output_Name=argv[++i];
		}
		else if(!strcmp(argv[i], "-g") || !strcmp(argv[i], "--genome"))
		{
			Geno=argv[++i];
		}else if(!strcmp(argv[i], "-Q"))
		{
			QualCut=atoi(argv[++i]);
		}
		else if(!strcmp(argv[i], "-a") || !strcmp(argv[i], "--all"))
		{
			UNIQUE=false;
		}else if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--threads"))
		{
			NTHREAD=atoi(argv[++i]);
		}
		else if(!strcmp(argv[i], "--sam-seq-beforeBS"))
		{
			SamSeqBeforeBS=true;
		}
		else if(!strcmp(argv[i], "-M") )
		{
			MethAllMap=0;
		}
		else if(!strcmp(argv[i], "-u") || !strcmp(argv[i], "--unique"))
		{
			UNIQUE=true;
		}
		else if(!strcmp(argv[i], "-r") || !strcmp(argv[i], "--remove_dup"))
		{
			REMOVE_DUP=true;
		}else if(!strcmp(argv[i], "--sam-nohead"))
		{
			SAM_Header=false;
		}else if(!strcmp(argv[i], "-R") || !strcmp(argv[i], "--Regions"))
		{
			RegionBins=atoi(argv[++i]);
		}
		else if(!strcmp(argv[i], "-ms") || !strcmp(argv[i], "--methState"))
		{
			PrintMethState=atoi(argv[++i]);
		}
		else if(!strcmp(argv[i], "-b") || !strcmp(argv[i], "--binsfile"))
		{
			binsOutfileName=argv[++i];
		}
		else if(!strcmp(argv[i], "-RF") || !strcmp(argv[i], "--Region_Files"))
		{
			RegionOutFiles=argv[++i];
		}
		else if(!strcmp(argv[i], "-S") || !strcmp(argv[i], "--chromStep"))
		{
			binspan=atoi(argv[++i]);
		}
		else if(!strcmp(argv[i],"-m") || !strcmp(argv[i],"--methratio"))
		{
			Methratio=true;
			methOutfileName=argv[++i];
		}
		else if(!strcmp(argv[i],"-C") || !strcmp(argv[i],"--coverage"))
		{
			coverage=atoi(argv[++i]);
		}else if(!strcmp(argv[i],"-nC"))
		{
			nCs=atoi(argv[++i]);
		}
		else if(!strcmp(argv[i], "-P") || !strcmp(argv[i], "--paired-end") )
		{
			Paired=true;
		}
		else if(!strcmp(argv[i], "-S") || !strcmp(argv[i], "--single-end") )
		{
			Paired=false;
		}
		else if(!strcmp(argv[i], "-f") || !strcmp(argv[i], "--sam") )
		{
			Sam=atoi(argv[++i]);
		}
		else if(!strcmp(argv[i], "-n") || !strcmp(argv[i], "--Nmismatch"))
		{
			UPPER_MAX_MISMATCH=atoi(argv[++i]);
		}
		else if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--input"))
		{
			InFileStart=++i;
			while(i!=(argc-1) && argv[i][0]!='-')
			{
				i++;
				continue;
			}
			if(argv[i][0]=='-') {InFileEnd=--i;}else {InFileEnd=i ;}
		}
		else
		{
			printf("%s \n",Help_String);
			exit(0);
		}
		
	}
	for(int i = 0; i < argc; i++) {CMD.append(argv[i]); CMD.append(" ");}

	if(argc<=1) printf("%s \n",Help_String);
	if (argc >1  && InFileStart) 
	{
		string log=Output_Name;log+=".log.txt";
		FILE* OUTLOG=File_Open(log.c_str(),"w");
		string mCdensity=Output_Name;mCdensity+=".mCdensity.txt";
                string mCcatero=Output_Name;mCcatero+=".mCcatero.txt";
		
		try
		{
			time(&Start_Time);
			Build_Pow10();
			
			string G=Geno;G+=".bin";
			string L=Geno;L+=".ann.location";

			FILE* BINFILE=File_Open(G.c_str(),"r");
			FILE* Location_File=File_Open(L.c_str(),"r");
			ARGS args;
			fseek(BINFILE, 0L, SEEK_END);off64_t Genome_Size=ftello64(BINFILE);rewind(BINFILE);//load original genome..
			args.Org_Genome=new char[Genome_Size];if(!args.Org_Genome) throw("Insufficient memory to load genome..\n"); 
			if(!fread(args.Org_Genome,Genome_Size,1,BINFILE)) throw ("Error reading file..\n");
			args.Marked_Genome=new char[Genome_Size+1];if(!args.Marked_Genome) throw("Insufficient memory to Mark genome..\n"); 
			args.Marked_GenomeE=new char[Genome_Size+1];if(!args.Marked_GenomeE) throw("Insufficient memory to Mark genome..\n"); 
			
			if(Paired) printf("Paired-end\nMax Mismatch: %d\nGENOME: %s\nBINFILE: %s\nLocationFile: %s\n",UPPER_MAX_MISMATCH,Geno.c_str(),G.c_str(),L.c_str() );
			else printf("Single-end\nMax Mismacth: %d\nGENOME: %s\nBINFILE: %s\nLocationFile: %s\n",UPPER_MAX_MISMATCH,Geno.c_str(),G.c_str(),L.c_str() );
			printf("Splitting Genome..\n");

			while (fgets(Temp_OR.Genome,39,Location_File)!=0)//count genomes..
			{
				fgets(Temp_OR.Genome,39,Location_File);
				Genome_CountX++;	
			}
			rewind(Location_File);
		
			args.Genome_Offsets = new Offset_Record[Genome_CountX+1];

			int Genome_Count=0;
			while (fgets(args.Genome_Offsets[Genome_Count].Genome,39,Location_File)!=0 && Genome_Count<=Genome_CountX)
			{
				args.Genome_Offsets[Genome_Count].Offset=atoi(args.Genome_Offsets[Genome_Count].Genome);
				fgets(args.Genome_Offsets[Genome_Count].Genome,39,Location_File);
				for(int i=0;i<40;i++) 
				{
					if (args.Genome_Offsets[Genome_Count].Genome[i] == '\n' ||args.Genome_Offsets[Genome_Count].Genome[i] == '\r')
					{ 
						args.Genome_Offsets[Genome_Count].Genome[i]=0;
						break;
					} 
				}
				Genome_Count++;	
			}
			Genome_Count--;
			args.OUTFILE=File_Open(Output_Name,"w");
			if(Sam && UNIQUE && SAM_Header) fprintf(args.OUTFILE,"@HD\tVN:2.0\tSO:unsorted\n");//Unique
			if(Sam && !UNIQUE && SAM_Header) fprintf(args.OUTFILE,"@HD\tVN:2.0\tSO:unsorted\n");//AllM
			
			char* Split_Point=args.Org_Genome;//split and write...

			
			args.Genome_List = new Gene_Hash[Genome_Count];
			args.Methy_List = new Methy_Hash[Genome_Count];
			for ( int i=0;i<Genome_Count;i++)//Stores the location in value corresponding to has..
			{
				printf("%s, ",args.Genome_Offsets[i].Genome);
				if(Sam && SAM_Header) fprintf(args.OUTFILE,"@SQ\tSN:%s\tLN:%d\n",args.Genome_Offsets[i].Genome,args.Genome_Offsets[i+1].Offset);
				String_Hash[args.Genome_Offsets[i].Genome]=i;
				//unsigned char H=Hash(Genome_Offsets[i].Genome);
				args.Genome_List[i].Genome=(Split_Point+=args.Genome_Offsets[i].Offset);
				args.Genome_List[i].Index=i;
				//meth ini
				if(Methratio)
				{
					args.Methy_List[i].plusG = new int[args.Genome_Offsets[i+1].Offset];
					args.Methy_List[i].plusA = new int[args.Genome_Offsets[i+1].Offset];
					args.Methy_List[i].NegG = new int[args.Genome_Offsets[i+1].Offset];
					args.Methy_List[i].NegA = new int[args.Genome_Offsets[i+1].Offset];
					args.Methy_List[i].MethContext =new int[args.Genome_Offsets[i+1].Offset];
					args.Methy_List[i].plusMethylated = new int[args.Genome_Offsets[i+1].Offset];
					args.Methy_List[i].plusUnMethylated = new int[args.Genome_Offsets[i+1].Offset];
					//Methy_List[i].plusCover = new int[Genome_Offsets[i+1].Offset];
					args.Methy_List[i].NegMethylated = new int[args.Genome_Offsets[i+1].Offset];
					args.Methy_List[i].NegUnMethylated = new int[args.Genome_Offsets[i+1].Offset];
					//Methy_List[i].NegCover = new int[Genome_Offsets[i+1].Offset];
					
					//=========Genome_List[i].Genome;
					args.Methy_List[i].Index=i;
				}
			}
			printf("Loaded\n");
			
			
			fclose(BINFILE);
			fclose(Location_File);

			if(Sam && SAM_Header) 
				fprintf(args.OUTFILE,"@PG\tID:BatMeth2\tVN:v2.0\tCL:%s\n",CMD.c_str());
			
			////read file
			if(NTHREAD>0) printf("\n[%d Threads runnning]\n",NTHREAD);
			
			for(int f=InFileStart;f<=InFileEnd;f++)
			{
				printf("\nProcessing %d out of %d. File: %s\n\n", f-InFileStart+1,InFileEnd-InFileStart+1, argv[f]);
				args.INFILE=File_Open(argv[f],"r");
				fseek(args.INFILE, 0L, SEEK_END);args.File_Size=ftello64(args.INFILE);rewind(args.INFILE);
				
				char s2t[BATBUF];
				fgets(s2t,BATBUF,args.INFILE);// read first header marker..
				//threads
				if(NTHREAD)
				{
					Threading* Thread_Info=(Threading*) malloc(sizeof(Threading)*NTHREAD);
					pthread_attr_t Attrib;
					pthread_attr_init(&Attrib);
					pthread_attr_setdetachstate(&Attrib, PTHREAD_CREATE_JOINABLE);
				        for (int i=0;i<NTHREAD;i++)
				        {
				                args.ThreadID=i;
				                Thread_Info[i].Arg=args;

				                Thread_Info[i].r=pthread_create(&Thread_Info[i].Thread,&Attrib,Process_read,(void*) &Thread_Info[i].Arg);
				                if(Thread_Info[i].r) {printf("Launch_Threads():Cannot create thread..\n");exit(-1);} 
				        }
				        
				        pthread_attr_destroy(&Attrib);

				        for (int i=0;i<NTHREAD;i++)
				        {
				                pthread_join(Thread_Info[i].Thread,NULL);
				        }
				}else
					Process_read(&args);
				Done_Progress();
				fclose(args.INFILE);
			 }

			//

			if(Methratio)
			{
				printf("\nPrinting MethRatio...\n\n");
				FILE* METHOUTFILE=File_Open(methOutfileName,"w");
				FILE* BINsOUTFILE=File_Open(binsOutfileName.c_str(),"w");
				//---DMR
				string RegionOutFiles_CG=RegionOutFiles;RegionOutFiles_CG+="_Region.CG.txt";
				string RegionOutFiles_CHG=RegionOutFiles;RegionOutFiles_CHG+="_Region.CHG.txt";
				string RegionOutFiles_CHH=RegionOutFiles;RegionOutFiles_CHH+="_Region.CHH.txt";
				FILE* REGION_OUT_CG=File_Open(RegionOutFiles_CG.c_str(),"w");//RegionBins RegionOutFiles
				FILE* REGION_OUT_CHG=File_Open(RegionOutFiles_CHG.c_str(),"w");
				FILE* REGION_OUT_CHH=File_Open(RegionOutFiles_CHH.c_str(),"w");
				//--------DMR---------------//
				//----------DMC
				string LociOutFiles_CG=RegionOutFiles;LociOutFiles_CG+="_loci.CG.txt";
				string LociOutFiles_CHG=RegionOutFiles;LociOutFiles_CHG+="_loci.CHG.txt";
				string LociOutFiles_CHH=RegionOutFiles;LociOutFiles_CHH+="_loci.CHH.txt";
				FILE* LOC_OUT_CG=File_Open(LociOutFiles_CG.c_str(),"w");//RegionBins RegionOutFiles
				FILE* LOC_OUT_CHG=File_Open(LociOutFiles_CHG.c_str(),"w");
				FILE* LOC_OUT_CHH=File_Open(LociOutFiles_CHH.c_str(),"w");
				//--------DMC---------------//
				//
				int plus_mCG=0,plus_mCHG=0,plus_mCHH=0;
				int plusCG=0,plusCHG=0,plusCHH=0;
				int count_plus_CG=0,count_plus_CHG=0,count_plus_CHH=0;
				int Neg_mCG=0,Neg_mCHG=0,Neg_mCHH=0;
				int NegCG=0,NegCHG=0,NegCHH=0;
				int count_neg_CG=0,count_neg_CHG=0,count_neg_CHH=0;
				//------F
				fprintf(METHOUTFILE,"#chromsome\tloci\tstrand\tcontext\tC_count\tCT_count\tmethRatio\teff_CT_count\trev_G_count\trev_GA_count\tMethContext\t5context\n");
				int whichBins=0;int wBins=0;//which bins
				//printf("Processed:\n");
				for ( int i=0;i<Genome_Count;i++)
				{
					string Genome_Seq;Genome_Seq=args.Genome_List[i].Genome;
					//printf("%s, ",Genome_Offsets[i].Genome);//Genome_Offsets[Hash_Index+1].Offset
					// weight methylation level
				       int pluscountperCG=0,pluscountperCHG=0,pluscountperCHH=0,NegcountperCG=0,NegcountperCHG=0,NegcountperCHH=0,pluscountCG=0,NegcountCG=0,pluscountCHG=0,NegcountCHG=0,
				               pluscountCHH=0,NegcountCHH=0;
				       int pluscountperCG_1=0,pluscountperCHG_1=0,pluscountperCHH_1=0,NegcountperCG_1=0,NegcountperCHG_1=0,NegcountperCHH_1=0,pluscountCG_1=0,NegcountCG_1=0,pluscountCHG_1=0,
				            NegcountCHG_1=0,pluscountCHH_1=0,NegcountCHH_1=0;  
					int nb=0,nbins = ceil(args.Genome_Offsets[i+1].Offset/binspan);
					int nRegionBins = ceil(args.Genome_Offsets[i+1].Offset/RegionBins);
					char * Genome=args.Genome_Offsets[i].Genome;
					printf("%s; ",Genome);
					for(int l=0;l<args.Genome_Offsets[i+1].Offset;l++)//loci
					{
				            if( (nb!=nbins && l==(nb+1)*binspan-1) || i==args.Genome_Offsets[i+1].Offset-1){
				                if(nb<=nbins && nb>0){
				                	
				                	if(pluscountCG+pluscountCG_1>coverage) fprintf(BINsOUTFILE,"%s\t%d\t%f\tCG\n",args.Genome_Offsets[i].Genome,nb,((double)(pluscountperCG+pluscountperCG_1)/(pluscountCG+pluscountCG_1)) );
				                	if(pluscountCHG+pluscountCHG_1>coverage) fprintf(BINsOUTFILE,"%s\t%d\t%f\tCHG\n",args.Genome_Offsets[i].Genome,nb,((double)(pluscountperCHG+pluscountperCHG_1)/(pluscountCHG+pluscountCHG_1)) );
				                	if(pluscountCHH+pluscountCHH_1>coverage) fprintf(BINsOUTFILE,"%s\t%d\t%f\tCHH\n",args.Genome_Offsets[i].Genome,nb,((double)(pluscountperCHH+pluscountperCHH_1)/(pluscountCHH+pluscountCHH_1)) );

				                	if(NegcountCG+NegcountCG_1>coverage) fprintf(BINsOUTFILE,"%s\t%d\t-%f\tCG\n",args.Genome_Offsets[i].Genome,nb,((double)(NegcountperCG+NegcountperCG_1)/(NegcountCG+NegcountCG_1)));
				                	if(NegcountCHG+NegcountCHG_1>coverage) fprintf(BINsOUTFILE,"%s\t%d\t-%f\tCHG\n",args.Genome_Offsets[i].Genome,nb,((double)(NegcountperCHG+NegcountperCHG_1)/(NegcountCHG+NegcountCHG_1)));
				                	if(NegcountCHH+NegcountCHH_1>coverage) fprintf(BINsOUTFILE,"%s\t%d\t-%f\tCHH\n",args.Genome_Offsets[i].Genome,nb,((double)(NegcountperCHH+NegcountperCHH_1)/(NegcountCHH+NegcountCHH_1)) );
				                	/*
				                	if(pluscountCG+pluscountCG_1+NegcountCG+NegcountCG_1>coverage) fprintf(BINsOUTFILE,"%s\t%d\t%f\tCG\n",Genome_Offsets[i].Genome,nb,((double)(pluscountperCG+pluscountperCG_1+NegcountperCG+NegcountperCG_1)/(pluscountCG+pluscountCG_1+NegcountCG+NegcountCG_1)) );
				                	if(pluscountCHG+pluscountCHG_1+NegcountCHG+NegcountCHG_1>coverage) fprintf(BINsOUTFILE,"%s\t%d\t%f\tCHG\n",Genome_Offsets[i].Genome,nb,((double)(pluscountperCHG+pluscountperCHG_1+NegcountperCHG+NegcountperCHG_1)/(pluscountCHG+pluscountCHG_1+NegcountCHG+NegcountCHG_1)) );
				                	if(pluscountCHH+pluscountCHH_1+NegcountCHH+NegcountCHH_1>coverage) fprintf(BINsOUTFILE,"%s\t%d\t%f\tCHH\n",Genome_Offsets[i].Genome,nb,((double)(pluscountperCHH+pluscountperCHH_1+NegcountperCHH+NegcountperCHH_1)/(pluscountCHH+pluscountCHH_1+NegcountCHH+NegcountCHH_1)) );
							*/
				                } 
				                pluscountperCG_1=pluscountperCG;pluscountperCHG_1=pluscountperCHG;pluscountperCHH_1=pluscountperCHH;NegcountperCG_1=NegcountperCG;NegcountperCHG_1=NegcountperCHG;NegcountperCHH_1=NegcountperCHH;pluscountCG_1=pluscountCG;NegcountCG_1=NegcountCG;pluscountCHG_1=pluscountCHG;
				                NegcountCHG_1=NegcountCHG;pluscountCHH_1=pluscountCHH;NegcountCHH_1=NegcountCHH;
				                nb++;
				                pluscountperCG=0;pluscountperCHG=0;pluscountperCHH=0;NegcountperCG=0;NegcountperCHG=0;NegcountperCHH=0;pluscountCG=0;NegcountCG=0;pluscountCHG=0;NegcountCHG=0;pluscountCHH=0;NegcountCHH=0;
				            }
				            //-----------DMR region--//
						wBins=whichBins;
						whichBins = floor((double)l/RegionBins);
						if(wBins+1==whichBins || l==args.Genome_Offsets[i+1].Offset-1 )
						{//chr10   60441   -       CHH     0       4
							if(/*plusCG>=coverage && */count_plus_CG>=nCs ) fprintf(REGION_OUT_CG,"%s\t%d\t+\tCG\t%d\t%d\n",Genome,wBins*RegionBins+1,plus_mCG,plusCG);
							if(count_neg_CG>=nCs) fprintf(REGION_OUT_CG,"%s\t%d\t-\tCG\t%d\t%d\n",Genome,wBins*RegionBins+1,Neg_mCG,NegCG);
							if(count_plus_CHG>=nCs ) fprintf(REGION_OUT_CHG,"%s\t%d\t+\tCHG\t%d\t%d\n",Genome,wBins*RegionBins+1,plus_mCHG,plusCHG);
							if(count_neg_CHG>=nCs) fprintf(REGION_OUT_CHG,"%s\t%d\t-\tCHG\t%d\t%d\n",Genome,wBins*RegionBins+1,Neg_mCHG,NegCHG);
							if(count_plus_CHH>=nCs ) fprintf(REGION_OUT_CHH,"%s\t%d\t+\tCHH\t%d\t%d\n",Genome,wBins*RegionBins+1,plus_mCHH,plusCHH);
							if(count_neg_CHH>=nCs) fprintf(REGION_OUT_CHH,"%s\t%d\t-\tCHH\t%d\t%d\n",Genome,wBins*RegionBins+1,Neg_mCHH,NegCHH);
							
							plus_mCG=0;plus_mCHG=0;plus_mCHH=0;plusCG=0;plusCHG=0;plusCHH=0;
							Neg_mCG=0;Neg_mCHG=0;Neg_mCHH=0;NegCG=0;NegCHG=0;NegCHH=0;
							count_plus_CG=0;count_plus_CHG=0;count_plus_CHH=0;
						}
						//-----F
						std::string context;
						if(args.Methy_List[i].plusMethylated[l]+args.Methy_List[i].plusUnMethylated[l]>=coverage)
						{//chromsome loci strand context methratio eff_CT_count C_count T_count CT_count rev_G_count rev_GA_count
							int C_count=args.Methy_List[i].plusMethylated[l];
							int T_count=args.Methy_List[i].plusUnMethylated[l];
							int rev_G=args.Methy_List[i].NegG[l];
							int rev_A=args.Methy_List[i].NegA[l];
							float revGA=0;
							if( (rev_G+rev_A)> 20 ) revGA=float(rev_G)/float(rev_G+rev_A); 
							//context
							
							string Fivecontext;
							//char genome_Char = toupper(Genome_Seq[l]);
							if(l+2 < args.Genome_Offsets[i+1].Offset)
							{
								if(l>=2 ) Fivecontext= Genome_Seq.substr(l-2,5);
								else if(l==1) Fivecontext = "N" + Genome_Seq.substr(l-1,4);
								else if(l==0) Fivecontext = "NN" + Genome_Seq.substr(l,3);
							}else if(l+1 < args.Genome_Offsets[i+1].Offset)
							{
								if(l>=2 ) Fivecontext= Genome_Seq.substr(l-2,4)+"N";
								else if(l==1) Fivecontext = "N" + Genome_Seq.substr(l-1,3)+"N";
								else if(l==0) Fivecontext = "NN" + Genome_Seq.substr(l,2)+"N";
							}else
							{
								if(l>=2 ) Fivecontext= Genome_Seq.substr(l-2,3)+"NN";
								else if(l==1) Fivecontext = "N" + Genome_Seq.substr(l-1,2)+"NN";
								else if(l==0) Fivecontext = "NN" + Genome_Seq.substr(l,1)+"NN";
							}
							int stringlength=Fivecontext.length();
													
							transform(Fivecontext.begin(), Fivecontext.end(), Fivecontext.begin(), ::toupper);

							char charFor1='N',charFor2='N';//printf("\n%s\n",Fivecontext.c_str());
							if(l+1< args.Genome_Offsets[i+1].Offset) charFor1=toupper(Genome_Seq[l+1]);
							if(l+2< args.Genome_Offsets[i+1].Offset) charFor2=toupper(Genome_Seq[l+2]);
							
							
							if(charFor1=='G') //(args.Methy_List[i].MethContext[l]==1)
							{
								context="CG";
								plus_mCGcount+=C_count;
								plusCGcount+=(C_count+T_count);
								//--DMR
								plus_mCG+=C_count;
								plusCG+=(C_count+T_count);
								count_plus_CG++;
								//chromsome bins
				                        pluscountperCG+=C_count;
				                        pluscountCG+=(C_count+T_count);
							}
							else if(charFor1!='G' && charFor2=='G') //(args.Methy_List[i].MethContext[l]==2)
							{
								context="CHG";
								plus_mCHGcount+=C_count;
								plusCHGcount+=(C_count+T_count);
								//--DMR
								plus_mCHG+=C_count;
								plusCHG+=(C_count+T_count);
								count_plus_CHG++;
								//bins
					                   pluscountperCHG+=C_count;
					                   pluscountCHG+=(C_count+T_count);								
							}
							else if(charFor1!='G' && charFor1!='G') //(args.Methy_List[i].MethContext[l]==3)
							{
								context="CHH";
								plus_mCHHcount+=C_count;
								plusCHHcount+=(C_count+T_count);
								//DMR
								plus_mCHH+=C_count;
								plusCHH+=(C_count+T_count);
								count_plus_CHH++;
								//bins
				                        pluscountperCHH+=C_count;
				                        pluscountCHH+=(C_count+T_count);							
							}
							else context="NA";
							
							//methratio	
							float PlusMethratio;
							if(revGA>0) 
								PlusMethratio=std::min(float(C_count)/(float(C_count+T_count)*revGA),(float)1.0);
							else 
								PlusMethratio=float(C_count)/float(C_count+T_count);
							
							unsigned int mCdensityloci=std::min( (int)floor((double)(PlusMethratio*100) )  ,99);
							if( !strcmp(context.c_str(),"CG")) mCGdensity[mCdensityloci]++;
							else if(!strcmp(context.c_str(),"CHG")) mCHGdensity[mCdensityloci]++;
							else if(!strcmp(context.c_str(),"CHH")) mCHHdensity[mCdensityloci]++;							
							
							string category = "NA";
					              if(PlusMethratio>=0.8){
					                  M++;
					                  category="M";
					                  if(!strcmp(context.c_str(),"CG")) M_CG++;
					              }else if(PlusMethratio >=0.6){
					                  Mh++;
					                  category="Mh";
					                  if(!strcmp(context.c_str(),"CG")) Mh_CG++;
					              }else if(PlusMethratio >=0.4){
					                  H++;
					                  category="H";
					                  if(!strcmp(context.c_str(),"CG")) H_CG++;
					              }else if(PlusMethratio >=0.2){
					                  hU++;
					                  category="hU";
					                  if(!strcmp(context.c_str(),"CG")) hU_CG++;
					              }else{
					                  U++;
					                  category="U";
					                  if(!strcmp(context.c_str(),"CG")) U_CG++;
					              }
					              
							if(revGA>0) fprintf(METHOUTFILE,"%s\t%d\t+\t%s\t%d\t%d\t%f\t%0.001f\t%d\t%d\t%s\t%s\n",args.Genome_Offsets[i].Genome,l+1,context.c_str(),C_count,(C_count+T_count),PlusMethratio,float(C_count+T_count)*revGA,rev_G,(rev_A+rev_G),category.c_str(),Fivecontext.c_str());
							else fprintf(METHOUTFILE,"%s\t%d\t+\t%s\t%d\t%d\t%f\tnull\t%d\t%d\t%s\t%s\n",args.Genome_Offsets[i].Genome,l+1,context.c_str(),C_count,(C_count+T_count),PlusMethratio,rev_G,(rev_A+rev_G),category.c_str(),Fivecontext.c_str());
							
							if(!strcmp(context.c_str(),"CG")) fprintf(LOC_OUT_CG,"%s\t%d\t+\tCG\t%d\t%d\n",Genome,l+1,C_count,(C_count+T_count));
							else if(!strcmp(context.c_str(),"CHG")) fprintf(LOC_OUT_CHG,"%s\t%d\t+\tCHG\t%d\t%d\n",Genome,l+1,C_count,(C_count+T_count));
							else if(!strcmp(context.c_str(),"CHH")) fprintf(LOC_OUT_CHH,"%s\t%d\t+\tCHH\t%d\t%d\n",Genome,l+1,C_count,(C_count+T_count));
							
						}
						if(args.Methy_List[i].NegMethylated[l]+args.Methy_List[i].NegUnMethylated[l]>=coverage)
						{
							int C_count=args.Methy_List[i].NegMethylated[l];
							int T_count=args.Methy_List[i].NegUnMethylated[l];
							int rev_G=args.Methy_List[i].plusG[l];
							int rev_A=args.Methy_List[i].plusA[l];
							float revGA=0;
							if( (rev_G+rev_A)>20 ) revGA=float(rev_G)/float(rev_G+rev_A); 
							//context
							string Fivecontext;
							//char genome_Char = toupper(Genome_Seq[l]);
							if(l+2 < args.Genome_Offsets[i+1].Offset)
							{
								if(l>=2 ) Fivecontext= Genome_Seq.substr(l-2,5);
								else if(l==1) Fivecontext = "N" + Genome_Seq.substr(l-1,4);
								else if(l==0) Fivecontext = "NN" + Genome_Seq.substr(l,3);
							}else if(l+1 < args.Genome_Offsets[i+1].Offset)
							{
								if(l>=2 ) Fivecontext= Genome_Seq.substr(l-2,4)+"N";
								else if(l==1) Fivecontext = "N" + Genome_Seq.substr(l-1,3)+"N";
								else if(l==0) Fivecontext = "NN" + Genome_Seq.substr(l,2)+"N";
							}else
							{
								if(l>=2 ) Fivecontext= Genome_Seq.substr(l-2,3)+"NN";
								else if(l==1) Fivecontext = "N" + Genome_Seq.substr(l-1,2)+"NN";
								else if(l==0) Fivecontext = "NN" + Genome_Seq.substr(l,1)+"NN";
							}
							
							int stringlength=Fivecontext.length();
							
							char Fcontext[6];
							memcpy(Fcontext,Fivecontext.c_str(),stringlength+1);
							ReverseC_Context(Fcontext,Fivecontext.c_str(),stringlength);
							
							char charBac1='N',charBac2='N';
							if(l>=1) charBac1=toupper(Genome_Seq[l-1]);
							if(l>=2) charBac2=toupper(Genome_Seq[l-2]);
							if(charBac1=='C') //(args.Methy_List[i].MethContext[l]==1)
							{
								context="CG";
								Neg_mCGcount+=C_count;
								NegCGcount+=(C_count+T_count);
								//--DMR
								Neg_mCG+=C_count;
								NegCG+=(C_count+T_count);
								count_neg_CG++;
								//bins
				                          NegcountperCG+=C_count;
				                          NegcountCG+=(C_count+T_count);
							}
							else if(charBac1!='C' && charBac2=='C') //(args.Methy_List[i].MethContext[l]==2)
							{
								context="CHG";
								Neg_mCHGcount+=C_count;
								NegCHGcount+=(C_count+T_count);
								//---DMR
								Neg_mCHG+=C_count;
								NegCHG+=(C_count+T_count);
								count_neg_CHG++;
								//bins
				                        NegcountperCHG+=C_count;
				                        NegcountCHG+=(C_count+T_count);
							}
							else if(charBac1!='C' && charBac2!='C') //(args.Methy_List[i].MethContext[l]==3)
							{
								context="CHH";
								Neg_mCHHcount+=C_count;
								NegCHHcount+=(C_count+T_count);
								//---DMR
								Neg_mCHH+=C_count;
								NegCHH+=(C_count+T_count);
								count_neg_CHH++;
								//bins
				                        NegcountperCHH+=C_count;
				                        NegcountCHH+=(C_count+T_count);
							}
							else context="NA";
							//methratio	
							float NegMethratio;
							if(revGA>0)
								NegMethratio=std::min(float(C_count)/(float(C_count+T_count)*revGA),(float)1.0);
							else 
								NegMethratio=float(C_count)/(float(C_count+T_count));
							
							unsigned int mCdensityloci= std::min( (int)floor((double)(NegMethratio*100) ) ,99);
							if(!strcmp(context.c_str(),"CG")) mCGdensity[mCdensityloci]++;
							else if(!strcmp(context.c_str(),"CHG")) mCHGdensity[mCdensityloci]++;
							else if(!strcmp(context.c_str(),"CHH")) mCHHdensity[mCdensityloci]++;
							
							string category = "NA";
					              if(NegMethratio>=0.8){
					                  M++;
					                  category="M";
					                  if(!strcmp(context.c_str(),"CG")) M_CG++;
					              }else if(NegMethratio >=0.6){
					                  Mh++;
					                  category="Mh";
					                  if(!strcmp(context.c_str(),"CG")) Mh_CG++;
					              }else if(NegMethratio >=0.4){
					                  H++;
					                  category="H";
					                  if(!strcmp(context.c_str(),"CG")) H_CG++;
					              }else if(NegMethratio >=0.2){
					                  hU++;
					                  category="hU";
					                  if(!strcmp(context.c_str(),"CG")) hU_CG++;
					              }else{
					                  U++;
					                  category="U";
					                  if(!strcmp(context.c_str(),"CG")) U_CG++;
					              }
							if(revGA>0) fprintf(METHOUTFILE,"%s\t%d\t-\t%s\t%d\t%d\t%f\t%0.001f\t%d\t%d\t%s\t%s\n",args.Genome_Offsets[i].Genome,l+1,context.c_str(),C_count,(C_count+T_count),NegMethratio,float(C_count+T_count)*revGA,rev_G,(rev_G+rev_A),category.c_str(),Fcontext);
							else fprintf(METHOUTFILE,"%s\t%d\t-\t%s\t%d\t%d\t%f\tnull\t%d\t%d\t%s\t%s\n",args.Genome_Offsets[i].Genome,l+1,context.c_str(),C_count,(C_count+T_count),NegMethratio,rev_G,(rev_G+rev_A),category.c_str(),Fcontext);

							if(!strcmp(context.c_str(),"CG")) fprintf(LOC_OUT_CG,"%s\t%d\t-\tCG\t%d\t%d\n",Genome,l+1,C_count,(C_count+T_count));
							else if(!strcmp(context.c_str(),"CHG")) fprintf(LOC_OUT_CHG,"%s\t%d\t-\tCHG\t%d\t%d\n",Genome,l+1,C_count,(C_count+T_count));
							else if(!strcmp(context.c_str(),"CHH")) fprintf(LOC_OUT_CHH,"%s\t%d\t-\tCHH\t%d\t%d\n",Genome,l+1,C_count,(C_count+T_count));

						}
					}
				}
				printf("\n");
				fclose(METHOUTFILE);
				fclose(BINsOUTFILE);
				fclose(REGION_OUT_CG);
				fclose(REGION_OUT_CHG);
				fclose(REGION_OUT_CHH);
				fclose(LOC_OUT_CG);
				fclose(LOC_OUT_CHG);
				fclose(LOC_OUT_CHH);
			}//end methratio
			//Print_Mismatch_Quality(OUTFILE_MM_QUALITY, Read_Len);
	//}
			fclose(args.OUTFILE);
			
			for ( int i=0;i<Genome_Count;i++)
			{
				if(Methratio)
				{
					delete[] args.Methy_List[i].plusG;
					delete[] args.Methy_List[i].plusA ;
					delete[] args.Methy_List[i].NegG;
					delete[] args.Methy_List[i].NegA;
					//delete[] args.Methy_List[i].MethContext ;
					delete[] args.Methy_List[i].plusMethylated ;
					delete[] args.Methy_List[i].plusUnMethylated;
					delete[] args.Methy_List[i].NegMethylated ;
					delete[] args.Methy_List[i].NegUnMethylated ;
				}
			}
			delete [] args.Genome_List;
			delete [] args.Methy_List;
			delete [] args.Genome_Offsets;
			delete [] args.Org_Genome;
			
			printf("Raw count of Met_C in CG: %lu\n",met_CG);
			printf("Raw count of Non_Met_C in CG: %lu\n",non_met_CG);
			printf("Raw count of Met_C in CHG: %lu\n",met_CHG);
			printf("Raw count of Non_Met_C in CHG: %lu\n",non_met_CHG);
			printf("Raw count of Met_C in CHH: %lu\n",met_CHH);
			printf("Raw count of Non_Met_C in CHH: %lu\n",non_met_CHH);
			fprintf(OUTLOG,"Raw count of Met_C in CG: %lu\n",met_CG);
			fprintf(OUTLOG,"Raw count of Non_Met_C in CG: %lu\n",non_met_CG);
			fprintf(OUTLOG,"Raw count of Met_C in CHG: %lu\n",met_CHG);
			fprintf(OUTLOG,"Raw count of Non_Met_C in CHG: %lu\n",non_met_CHG);
			fprintf(OUTLOG,"Raw count of Met_C in CHH: %lu\n",met_CHH);
			fprintf(OUTLOG,"Raw count of Non_Met_C in CHH: %lu\n",non_met_CHH);
			if(Methratio)
			{
				FILE* mC_DENSITY=File_Open(mCdensity.c_str(),"w");
				fprintf(mC_DENSITY,"mCG");
				for(int i=0;i<100;i++)
				{
					fprintf(mC_DENSITY,"\t%lu",mCGdensity[i]);
				}
				fprintf(mC_DENSITY,"\nmCHG");
				for(int i=0;i<100;i++)
				{
					fprintf(mC_DENSITY,"\t%lu",mCHGdensity[i]);
				}
				fprintf(mC_DENSITY,"\nmCHH");
				for(int i=0;i<100;i++)
				{
					fprintf(mC_DENSITY,"\t%lu",mCHHdensity[i]);
				}
                                fclose(mC_DENSITY);

                                FILE* mC_CATERO=File_Open(mCcatero.c_str(),"w");
				fprintf(mC_CATERO,"\nM\t%u\nMh\t%u\nH\t%u\nhU\t%u\nU\t%u",M,Mh,H,hU,U);
				fprintf(mC_CATERO,"\nCpG_M\t%u\nCpG_Mh\t%u\nCpG_H\t%u\nCpG_hU\t%u\nCpG_U\t%u",M_CG,Mh_CG,H_CG,hU_CG,U_CG);
                                fclose(mC_CATERO);				

				printf("\n[CpG]  M: %u Mh: %u H: %u hU: %u U: %u\n",M_CG,Mh_CG,H_CG,hU_CG,U_CG);
				printf("\n[mC]   M: %u Mh: %u H: %u hU: %u U: %u\n",M,Mh,H,hU,U);
				fprintf(OUTLOG,"\n[CpG]  M: %u Mh: %u H: %u hU: %u U: %u\n",M_CG,Mh_CG,H_CG,hU_CG,U_CG);
				fprintf(OUTLOG,"\n[mC]   M: %u Mh: %u H: %u hU: %u U: %u\n",M,Mh,H,hU,U);
				//+
				printf("\nStrand+ :\nmC/(C+T) {%ld / %ld} = %f% \n",(plus_mCGcount+plus_mCHGcount+plus_mCHHcount),(plusCGcount+plusCHGcount+plusCHHcount),double (100*(plus_mCGcount+plus_mCHGcount+plus_mCHHcount))/(plusCGcount+plusCHGcount+plusCHHcount) );
				printf("mCG/(CG+TG) {%ld / %ld} = %f% \n",plus_mCGcount,plusCGcount,double (100*(plus_mCGcount))/(plusCGcount) );
				printf("mCHG/(CHG+THG) {%ld / %ld} = %f% \n",plus_mCHGcount,plusCHGcount,double (100*(plus_mCHGcount))/(plusCHGcount) );
				printf("mCHH/(CHH+THH) {%ld / %ld} = %f% \n",plus_mCHHcount,plusCHHcount,double (100*(plus_mCHHcount))/(plusCHHcount) );
				//-
				printf("\nStrand- :\nmC/(C+T) {%ld / %ld} = %f% \n",(Neg_mCGcount+Neg_mCHGcount+Neg_mCHHcount),(NegCGcount+NegCHGcount+NegCHHcount),double (100*(Neg_mCGcount+Neg_mCHGcount+Neg_mCHHcount))/double (NegCGcount+NegCHGcount+NegCHHcount) );
				printf("mCG/(CG+TG) {%ld / %ld} = %f% \n",Neg_mCGcount,NegCGcount,double (100*(Neg_mCGcount))/double (NegCGcount) );
				printf("mCHG/(CHG+THG) {%ld / %ld} = %f% \n",Neg_mCHGcount,NegCHGcount,double (100*(Neg_mCHGcount))/double (NegCHGcount) );
				printf("mCHH/(CHH+THH) {%ld / %ld} = %f% \n",Neg_mCHHcount,NegCHHcount,double (100*(Neg_mCHHcount))/double (NegCHHcount) );
				//+ -
				printf("\nStrand+- :\nmC/(C+T) {%ld / %ld} = %f% \n",(plus_mCGcount+plus_mCHGcount+plus_mCHHcount+Neg_mCGcount+Neg_mCHGcount+Neg_mCHHcount),(plusCGcount+plusCHGcount+plusCHHcount+NegCGcount+NegCHGcount+NegCHHcount),double (100*(plus_mCGcount+plus_mCHGcount+plus_mCHHcount+Neg_mCGcount+Neg_mCHGcount+Neg_mCHHcount))/double (plusCGcount+plusCHGcount+plusCHHcount+NegCGcount+NegCHGcount+NegCHHcount) );
				printf("mCG/(CG+TG) {%ld / %ld} = %f% \n",plus_mCGcount+Neg_mCGcount,plusCGcount+NegCGcount,double (100*(plus_mCGcount+Neg_mCGcount))/double (plusCGcount+NegCGcount) );
				printf("mCHG/(CHG+THG) {%ld / %ld} = %f% \n",plus_mCHGcount+Neg_mCHGcount,plusCHGcount+NegCHGcount,double (100*(plus_mCHGcount+Neg_mCHGcount))/double (plusCHGcount+NegCHGcount) );
				printf("mCHH/(CHH+THH) {%ld / %ld} = %f% \n",plus_mCHHcount+Neg_mCHHcount,plusCHHcount+NegCHHcount,double (100*(plus_mCHHcount+Neg_mCHHcount))/double (plusCHHcount+NegCHHcount) );
				//
				fprintf(OUTLOG,"\nStrand+- :\nmC/(C+T) {%ld / %ld} = %f% \n",(plus_mCGcount+plus_mCHGcount+plus_mCHHcount+Neg_mCGcount+Neg_mCHGcount+Neg_mCHHcount),(plusCGcount+plusCHGcount+plusCHHcount+NegCGcount+NegCHGcount+NegCHHcount),double (100*(plus_mCGcount+plus_mCHGcount+plus_mCHHcount+Neg_mCGcount+Neg_mCHGcount+Neg_mCHHcount))/double (plusCGcount+plusCHGcount+plusCHHcount+NegCGcount+NegCHGcount+NegCHHcount) );
				fprintf(OUTLOG,"mCG/(CG+TG) {%ld / %ld} = %f% \n",plus_mCGcount+Neg_mCGcount,plusCGcount+NegCGcount,double (100*(plus_mCGcount+Neg_mCGcount))/double (plusCGcount+NegCGcount));
				fprintf(OUTLOG,"mCHG/(CHG+THG) {%ld / %ld} = %f% \n",plus_mCHGcount+Neg_mCHGcount,plusCHGcount+NegCHGcount,double (100*(plus_mCHGcount+Neg_mCHGcount))/double (plusCHGcount+NegCHGcount));
				fprintf(OUTLOG,"mCHH/(CHH+THH) {%ld / %ld} = %f% \n",plus_mCHHcount+Neg_mCHHcount,plusCHHcount+NegCHHcount,double (100*(plus_mCHHcount+Neg_mCHHcount))/double (plusCHHcount+NegCHHcount) );
			}
		}
		catch(char* Err)
		{
			printf(Err);
			exit(-1);
		}
		//printf("@>\nMet_loc / Non-Met_loc : %u\t%u\n ", met_cnt,non_met_cnt);
		//printf("Methylation Rate : %.01f \n",(float(non_met_cnt)*100)/float((non_met_cnt+met_cnt)));
		if(Paired)
		{
			Total_Reads_all=(int) Total_Reads_all/2;
			ALL_MAP_Org=(int) ALL_MAP_Org/2;
			Tot_Unique_Org=(int) Tot_Unique_Org/2;
			ALL_Map_Remdup=(int) ALL_Map_Remdup/2;
			Tot_Unique_Remdup=(int) Tot_Unique_Remdup/2;
		}
		if(!UNIQUE && REMOVE_DUP) 
		{
			printf("\nTotal Reads/Total mapped/Total after PCR-dup-removal/OutputFile : %u\t%u (%f%)\t%u (%f%)\t%s ",Total_Reads_all,ALL_MAP_Org,(float(ALL_MAP_Org)*100)/float(Total_Reads_all),ALL_Map_Remdup,(float(ALL_Map_Remdup)*100)/float(Total_Reads_all),Output_Name);
			fprintf(OUTLOG,"\nTotal Reads/Total mapped/Total after PCR-dup-removal/OutputFile : %u\t%u (%f%)\t%u (%f%)\t%s ",Total_Reads_all,ALL_MAP_Org,(float(ALL_MAP_Org)*100)/float(Total_Reads_all),ALL_Map_Remdup,(float(ALL_Map_Remdup)*100)/float(Total_Reads_all),Output_Name);
		}else {
			printf("\nTotal Reads/Total mapped : %u\t%u (%f%) ",Total_Reads_all,ALL_MAP_Org,(float(ALL_MAP_Org)*100)/float(Total_Reads_all) );
			fprintf(OUTLOG,"\nTotal Reads/Total mapped : %u\t%u (%f%) ",Total_Reads_all,ALL_MAP_Org,(float(ALL_MAP_Org)*100)/float(Total_Reads_all));
		}
		if(REMOVE_DUP) 
		{
			printf("\nTotal Reads/Total unique mapped/Total after PCR-dup-removal/OutputFile : %u\t%u(%f%)\t%u(%f%)\t%s \n",Total_Reads_all,Tot_Unique_Org,(float(Tot_Unique_Org)*100)/float(Total_Reads_all),Tot_Unique_Remdup,(float(Tot_Unique_Remdup)*100)/float(Total_Reads_all),Output_Name);
			fprintf(OUTLOG,"\nTotal Reads/Total unique mapped/Total after PCR-dup-removal/OutputFile : %u\t%u(%f%)\t%u(%f%)\t%s \n",Total_Reads_all,Tot_Unique_Org,(float(Tot_Unique_Org)*100)/float(Total_Reads_all),Tot_Unique_Remdup,(float(Tot_Unique_Remdup)*100)/float(Total_Reads_all),Output_Name);
		}else 
		{
			printf("\nTotal Reads/Total unique mapped/OutputFile : %u\t%u(%f%)\t%s \n",Total_Reads_all,Tot_Unique_Org,(float(Tot_Unique_Org)*100)/float(Total_Reads_all),Output_Name);
			fprintf(OUTLOG,"\nTotal Reads/Total unique mapped/OutputFile : %u\t%u(%f%)\t%s \n",Total_Reads_all,Tot_Unique_Org,(float(Tot_Unique_Org)*100)/float(Total_Reads_all),Output_Name);
		}
		time(&End_Time);printf("Time Taken  - %.0lf Seconds ..\n ",difftime(End_Time,Start_Time));
	
	fclose(OUTLOG);
	}
	
}

int Read_Tag(FILE *INFILE,char s2t[],string hits[],int& cntHit)
{
	flockfile(INFILE);
	char Buf[BATBUF];
	if (!feof(INFILE))
	{
		Total_Reads_all++;
		fgets(s2t,BATBUF,INFILE);//read description..
		cntHit=0;
		fgets(Buf,BATBUF,INFILE);
		while(!feof(INFILE) && Buf[0]!='@')//while in the record.. 
		{
			hits[cntHit++]=Buf; //read hit info..
			fgets(Buf,BATBUF,INFILE);
			if(cntHit>=MAX_HITS_ALLOWED) 
			{
				cntHit=0;
		            while(!feof(INFILE) && Buf[0]!='@')
		            {
		                    fgets(Buf,BATBUF,INFILE);
		            }
				break;
			}
			assert(MAX_HITS_ALLOWED > cntHit);
		}
		funlockfile(INFILE);
		return 1;
	}
	else 
	{
		funlockfile(INFILE);
		return 0;
	}
}

char* process_cigar(const char* cig,int Read_Len)
{
	char temp[8];
	char* cigar_rm = new char[1000];*cigar_rm=0;unsigned n=0;
	char* buffer_cigar=new char[100];*buffer_cigar=0;
//	string cigs=cig;
	while(*cig!='\0')
	{
		if(*cig>='0' && *cig<='9')
		{
			temp[n]=*cig;
			cig++;n++;
		}else if(*cig=='S')
		{
			int i;temp[n]='\0';int length=atoi(temp);
			if(length>0) 
			{
				sprintf(buffer_cigar,"%dS",length);
				strcat(cigar_rm, buffer_cigar);
			}
            cig++;n=0;
		}else if(*cig=='M')
		{
			temp[n]='\0';int length=atoi(temp);
			if(length>0) 
			{
				sprintf(buffer_cigar,"%dM",length);
				strcat(cigar_rm, buffer_cigar);
			}
			cig++;n=0;
			char buf[1024]="\0";
			sprintf( buf , "%dM",Read_Len);
			char buf_tmp[1024]="\0";
			sprintf( buf_tmp, "%dM",length);
			if( !strcmp(buf,  buf_tmp ) )
			{
				if(!strcmp(buf, cig))
					break;
				else
				{
					strcpy(cigar_rm, buffer_cigar);
				}
				
			}
		}else if(*cig=='I')
		{
			int i;temp[n]='\0';int length=atoi(temp);
			if(length>0) 
			{
				if(length==1) sprintf(buffer_cigar,"1I");
				else if(length==2) sprintf(buffer_cigar,"2I");
				else sprintf(buffer_cigar,"%dI",length);
				strcat(cigar_rm, buffer_cigar);
			}
			cig++;n=0;
		}else if(*cig=='D')
		{
			int i;temp[n]='\0';int length=atoi(temp);
			if(length>0) 
			{
				if(length==1) sprintf(buffer_cigar,"1D");
				else if(length==2) sprintf(buffer_cigar,"2D");
				else sprintf(buffer_cigar,"%dD",length);
				strcat(cigar_rm, buffer_cigar);
			}
			cig++;n=0;
		}else
		{
		//printf(" --%d%s-- ",atoi(temp),cigs.c_str());
			//printf(" --%d%s-- ",atoi(temp),cig);
			exit(0);
    		continue;//break;
		}
	}
	return cigar_rm;
}
void Get_longest_macth(const char* cig,int & edit,int & longestM,int & longestM_location)
{
	char temp[8];
	unsigned n=0;longestM=0;edit=0;int location=0;
	string cigs=cig;
	while(*cig!='\0')
	{
		if(*cig>='0' && *cig<='9')
		{
			temp[n]=*cig;
			cig++;n++;
		}else if(*cig=='S')
		{
			int i;temp[n]='\0';int length=atoi(temp);
			location++;
            cig++;n=0;
		}else if(*cig=='M')
		{
			temp[n]='\0';int length=atoi(temp);
			location++;
			if(length>longestM) 
			{
				longestM_location=location;
				longestM = length;
			}
			cig++;n=0;
		}else if(*cig=='I')
		{
			int i;temp[n]='\0';int length=atoi(temp);
			location++;
			cig++;n=0;edit++;
		}else if(*cig=='D')
		{
			int i;temp[n]='\0';int length=atoi(temp);
			location++;
			cig++;n=0;edit++;
		}else
		{
			location++;
			break;
			longestM=-1;
			//printf(" --here %d%s-- ",atoi(temp),cigs.c_str());
			//exit(0);
    			continue;//break;
		}
	}

}
void *Process_read(void *arg)
{
	unsigned Total_Reads=0;
	int Progress=0;Number_of_Tags=INITIAL_PROGRESS_READS;
	Init_Progress();
	int true_matches=0;int true_matchesPE=0;unsigned pos;int Top_Penalty=0;int mapQuality=0;int Flag=0;
	string readString="";
	int hitType=0;
	int n_mismatch[MAX_HITS_ALLOWED];
	int hitsToken[MAX_HITS_ALLOWED][13];//7--(allM=1 else 0) //8 Genome_move_size //9 headClip //10 move size(S/D/I) //match_BQScore //11-- 0 is normal but 1 is remove soft split
	
	string hits[MAX_HITS_ALLOWED];
	char Comp_String[MAXTAG];for (int i=1;i<MAXTAG;Comp_String[i++]=0);
	//start to read batman hit file
	char s2t[BATBUF],Dummy[BATBUF],forReadString[BATBUF],rcReadString[BATBUF],Chrom[CHROMSIZE],Strand;
	char Chrom_P[CHROMSIZE];int pos_P=0;int Insert_Size=0;int Qsingle=0; //Paired-end reads
	char CIGr[BATBUF];
	string CIG[MAX_HITS_ALLOWED];
	char forQuality[BATBUF],rcQuality[BATBUF],Quality[BATBUF];
	bool HasAM=false;//have all match 
	bool HasAmS=false;//have all match after delete clip
	int cntHit=0;
	while (Read_Tag(( (ARGS *)arg)->INFILE,s2t,hits,cntHit) ) 
	{
		Total_Reads++;//Total_Reads_all++;
		Progress++;NP=false;bool all_Indels=true;
		HasAM=false;HasAmS=false;int PMs1=0,PMs2=0,PMs3=0,PMs4=0;
		if (Progress>Number_of_Tags && ( ((ARGS *)arg)->ThreadID==1 || !NTHREAD ) ) 
		{
			off64_t Current_Pos=ftello64(((ARGS *)arg)->INFILE);
			off64_t Average_Length=Current_Pos/Total_Reads+1;//+1 avoids divide by zero..
			Number_of_Tags=( ((ARGS *)arg)->File_Size/Average_Length )/20;
			Progress=0;
			Show_Progress(Current_Pos*100/((ARGS *)arg)->File_Size);
		}
		//space-trouble in read description
		for(int i=0;s2t[i];i++) if(s2t[i]==' ') s2t[i]='_';
		if(s2t[0]=='@')
		{
			int i=0;
			for(;s2t[i+1];i++) 
			{
				s2t[i]=s2t[i+1];
			}
			s2t[i]='\0';
		}
		sscanf(s2t,"%s%s%s",Dummy,forReadString,forQuality);
		int Read_Len=0;
		for(;forReadString[Read_Len]!=0 && forReadString[Read_Len]!='\n' && forReadString[Read_Len]!='\r';Read_Len++);
		for (int i=0;i<Read_Len;i++) {rcReadString[Read_Len-i-1]=Char2Comp[forReadString[i]];}
		rcReadString[Read_Len]='\0';forReadString[Read_Len]='\0';
		for (int i=0;i<Read_Len;i++) 
		{
			rcQuality[i]=forQuality[Read_Len-i-1];
		}
		rcQuality[Read_Len]='\0';
		forQuality[Read_Len]='\0';
		char* Genome=NULL;
		bool partialQC=false;

		bool exactmatch=true;

		for (int j=0; j<cntHit; j++) 
		{
			//We init the string which will contain the methylation status of a read wrt a putative genomic location
			//we check for the mismatches information here and decide if we should carry them over for future computation
			//assert(!hits[j].empty());
			if(hits[j].empty()) {cntHit=0;break;}
			if(!Paired) sscanf(hits[j].c_str(),"%d%s%s%u%d%d%s%d",&hitType,Chrom,&Strand,&pos,&true_matches,&Read_Len,CIGr,&Top_Penalty);
			else 
			{
				//sscanf(hits[j].c_str(),"%d%s%d%u%d%d%s%d%s%s%s",&hitType,Chrom,&Flag,&pos,&true_matchesPE,&Read_Len,CIGr,&mapQuality,&Chrom_P);
				//if(strcmp(Chrom_P, "=")) {NP=true;break;}//*
				
				//if( (hits[j].find("="))!=string::npos )
					sscanf(hits[j].c_str(),"%d%s%d%u%d%d%s%d%s%d%d%d",&hitType,Chrom,&Flag,&pos,&true_matchesPE,&Read_Len,CIGr,&mapQuality,&Chrom_P,&pos_P,&Insert_Size,&Qsingle);
					strcpy(CIGr,process_cigar(CIGr,Read_Len));
					if(strcmp(Chrom_P,"*") == 0) {
						pos_P = 0;
						Insert_Size = 0;
					}
					//else
				//	{NP=true;break;}
			}

			if(Top_Penalty) exactmatch=false;
			//printf("\n%d %c\n",hitType,hits[j].c_str()[0]);
			if(hitType==0) hitType=int(hits[j].c_str()[0]-'0');
			CIG[j]=CIGr;
			//int hitLen=hits[j].length();
			//char* tmp=new char[hitLen];
			//strcpy(tmp,hits[j].c_str());
			//if(Strand=='-') fetch(tmp, '\t','-', Chrom);
			//else if(Strand=='+') fetch(tmp, '\t','+', Chrom);
			true_matches=0;
			char const *C=hits[j].c_str();char Mark,Mis_Letter;
			int Num,Skip,k=0,l=0;
			string tempString="",Qual="";//printf("\n%s-r\n",forReadString);
			if (hitType==3 || hitType==4) 
			{
				tempString = rcReadString;
				Qual=rcQuality;
			}
			else 
			{
				tempString=forReadString;
				Qual=forQuality;
			}
			readString=tempString;
			//Read_Len=readString.size();
			int H=String_Hash[Chrom];
			Genome=((ARGS *)arg)->Genome_List[H].Genome;//load current genome..
			int Hash_Index=((ARGS *)arg)->Genome_List[H].Index;//load current genome..
			hitsToken[j][0]=hitType;hitsToken[j][1]=H;hitsToken[j][3]=pos;hitsToken[j][4]=0;
			if(!Paired) hitsToken[j][2]=Strand;
			else hitsToken[j][2]=Flag;
			if(Paired)
			{
				hitsToken[j][4] = mapQuality;
				hitsToken[j][5] = true_matchesPE;
			}
		
		if(!Paired)
		{
			int /*Score=0,*/BQScore=0,Clip_mismatch=0,Score_Clip=0,Clip_Count=0,ClipScore=0,ClipBQScore=0,ClipLength=0;int Quality_Score=0;
			int Score=0;const int QUAL_START=60;  int match_BQScore=0;int match_Score=0;
			const int MX=4/*6*/,BOPEN=6,BEXT=1/*3*/,MATCH_BONUS=0;
			//int gap_open=40,gap_extension=6;
			bool LessClip=false;int lociClip1=0,lociClip2=0,gClip1=0,gClip2=0;
			char temp[8];unsigned lens=0;int Glens=0;int RLens=0;
			unsigned n=0;
			CIG[j]=replace_all(CIG[j],"X","M",Read_Len);
			const char* cigr=CIG[j].c_str();bool ob =false;
			int edit=0,longestM=0,longestM_location=0;
			//if(!strcmp(Dummy,"HWI-D00523:75:C4PY7ANXX:7:1211:15606:28288")) 
			Get_longest_macth(cigr,edit,longestM,longestM_location);
			if(longestM==-1) continue;
			//int ed = Get_ED(CIG[j]);
//			if(UNIQUE &&  Get_ED(CIG[j]) >= 2*((double)Read_Len/100+0.5) ) continue;
			//if(Read_Len*0.02+1 < ed)  {cntHit--;continue;}
			int genome_move_size=0; int headClip=0;int moveSize=0;
			char cigar_rm[1000];*cigar_rm=0;
			char buffer_cigar[100];*buffer_cigar=0; bool cigar_remove=false;
			const char* cig=CIG[j].c_str();
			int BQScore_up1=0,BQScore_up2=0,BQScore_longestM=0,BQScore_down1=0,BQScore_down2=0;
			int Score_up1=0,Score_up2=0,Score_longestM=0,Score_down1=0,Score_down2=0;
			int location=0;int mismatch_longestM=0,mismatch_Temp=0;
			int real_location=0;

			while(*cig!='\0')//RLens--READs Length \\ lens--raw reads length \\ GLens--genome Lens
			{
				if(*cig>='0' && *cig<='9')
				{
					temp[n]=*cig;
					cig++;n++;
				}else if(*cig=='S')
				{
					exactmatch=false;
					int i;temp[n]='\0';int length=atoi(temp);
					location++;
					if(length==0) cigar_remove=true;
					if(length>0) 
					{
						real_location++;
						sprintf(buffer_cigar,"%dS",length);
						strcat(cigar_rm, buffer_cigar);
					}
					if(ClipLength==0 && lens==0 && Glens==0 && RLens==0) {genome_move_size-=length;headClip=length;}
					
					for(int k=lens,r=RLens,g=Glens;k<length+lens;r++,k++,g++)
					{
						if (pos-length+g-1<0 || pos-length+g-1 >= ((ARGS *)arg)->Genome_Offsets[Hash_Index+1].Offset) break;
						char genome_Char = toupper(Genome[pos-length+g-1]);//
						if (hitType==1 || hitType==3) {
							if(readString[k]==genome_Char) // (readString[k]=='C' && genome_Char=='C')
							{
								float Q_Value=Qual[k]-QUALITYCONVERSIONFACTOR;//make quality to integer..
								if(Q_Value<0) Q_Value=0;
								ClipBQScore+=MATCH_BONUS;
								ClipScore += Match_score;
							}
							else if(readString[k]=='T' && genome_Char=='C')
							{
								float Q_Value=Qual[k]-QUALITYCONVERSIONFACTOR;//make quality to integer..
								if(Q_Value<0) Q_Value=0;
								ClipBQScore+=MATCH_BONUS;
								ClipScore+= Match_score;
							}
							else if (readString[k] != genome_Char) 
							{
								Clip_mismatch++;
								float Q_Value=Qual[k]-QUALITYCONVERSIONFACTOR;
								if(Q_Value<0) Q_Value=0;
								ClipBQScore-= MN + floor( (MX-MN)*(std::min(Q_Value, 40.0f)/40.0f) );
								float Penalty = MN + floor( (MX-MN)*(std::min(Q_Value, 40.0f)/40.0f) );
								ClipScore -= Penalty;//Convert to probability of base being wrong =1-10^(Q_Value/10)
							}
						}
						else if (hitType==2 || hitType==4) {
							if(readString[k]==genome_Char) //(readString[k]=='G' && genome_Char=='G')
							{
								float Q_Value=Qual[k]-QUALITYCONVERSIONFACTOR;//make quality to integer..
								if(Q_Value<0) Q_Value=0;
								ClipBQScore+=MATCH_BONUS;
								ClipScore += Match_score;
							}
							else if (readString[k]=='A' && genome_Char=='G')
							{
								float Q_Value=Qual[k]-QUALITYCONVERSIONFACTOR;//make quality to integer..
								if(Q_Value<0) Q_Value=0;
								ClipBQScore+=MATCH_BONUS;
								ClipScore+= Match_score;
							}
							else if (readString[k] != genome_Char) {
								Clip_mismatch++;
								float Q_Value=Qual[k]-QUALITYCONVERSIONFACTOR;
								if(Q_Value<0) Q_Value=0;
								ClipBQScore-= MN + floor( (MX-MN)*(std::min(Q_Value, 40.0f)/40.0f) );
								float Penalty = MN + floor( (MX-MN)*(std::min(Q_Value, 40.0f)/40.0f) );
								ClipScore -= Penalty;//Convert to probability of base being wrong =1-10^(Q_Value/10)
							}
						}
					}
					/**/
					ClipLength+=length;
					lens+=length;Clip_Count+=length;
                    RLens+=length;
                    cig++;n=0;
				}else if(*cig=='M')
				{
					temp[n]='\0';int length=atoi(temp);
					location++;
					if(length==0) cigar_remove=true;
					if(length>0) 
					{
						real_location++;
						sprintf(buffer_cigar,"%dM",length);
						strcat(cigar_rm, buffer_cigar);
					}
					/*if( !strcmp(Read_Len+"M", length+"M"  ) )
					{
						char tempChar[30];sprintf(tempChar,"%dM",length);CIG[j]=tempChar;
						hitsToken[j][3]=pos+Glens;BQScore=0;hitsToken[j][6]=1;HasAM=true;genome_move_size=0;moveSize=0;
						if(hitType==1) PMs1++; else if(hitType==2) PMs2++; else if(hitType==3) PMs3++; else if(hitType==4) PMs4++;
					} else hitsToken[j][6]=0;
					*/
					match_BQScore=0;mismatch_Temp=0;match_Score=0;
					for(int k=lens,r=RLens,g=Glens;k<length+lens;r++,k++,g++)
					{
						if (pos+g-1 >= ((ARGS *)arg)->Genome_Offsets[Hash_Index+1].Offset) { ob=true; break;}
						char genome_Char = toupper(Genome[pos+g-1]);
						if (hitType==1 || hitType==3) {
							if(readString[k]==genome_Char || (readString[k]=='C' && genome_Char=='C') )
							{
								float Q_Value=Qual[k]-QUALITYCONVERSIONFACTOR;//make quality to integer..
								if(Q_Value<0) Q_Value=0;
								BQScore+=MATCH_BONUS;match_BQScore+=MATCH_BONUS;
								//float Penalty= -10*log10(Pr(Q_Value));
								//Score+= Penalty;
								//match_Score+= Penalty;
								Score += Match_score;
								match_Score += Match_score;
							}
							else if(readString[k]=='T' && genome_Char=='C')
							{
								float Q_Value=Qual[k]-QUALITYCONVERSIONFACTOR;//make quality to integer..
								if(Q_Value<0) Q_Value=0;
								BQScore+=MATCH_BONUS;match_BQScore+=MATCH_BONUS;
								//float Penalty= -10*log10(Pr(Q_Value));printf(";; %f ",Penalty);
								//Score+= Penalty;
								//match_Score+= Penalty;
								Score += Match_score;
								match_Score += Match_score;
							}
							else if (readString[k] != genome_Char) 
							{
								true_matches++;mismatch_Temp++;
								if(true_matches> std::max(int(Read_Len*0.6),UPPER_MAX_MISMATCH) && cntHit >4) 
								{
									hitsToken[j][4] = -10000;
									break;
								}
								float Q_Value=Qual[k]-QUALITYCONVERSIONFACTOR;
								if(Q_Value<0) Q_Value=0;
								BQScore-= MN + floor( (MX-MN)*(std::min(Q_Value, 40.0f)/40.0f) );
								match_BQScore-= MN + floor( (MX-MN)*(std::min(Q_Value, 40.0f)/40.0f) );
								float Penalty= MN + floor( (MX-MN)*(std::min(Q_Value, 40.0f)/40.0f) );
								Score -= Penalty;//Convert to probability of base being wrong =1-10^(Q_Value/10)
								match_Score -= Penalty;
							}
						}
						else if (hitType==2 || hitType==4) {
							if(readString[k]==genome_Char || (readString[k]=='G' && genome_Char=='G') )
							{
								float Q_Value=Qual[k]-QUALITYCONVERSIONFACTOR;//make quality to integer..
								if(Q_Value<0) Q_Value=0;
								BQScore+=MATCH_BONUS;match_BQScore+=MATCH_BONUS;
								//float Penalty= -10*log10(Pr(Q_Value));
								Score += Match_score;
								match_Score += Match_score;
							}
							else if (readString[k]=='A' && genome_Char=='G')
							{
								float Q_Value=Qual[k]-QUALITYCONVERSIONFACTOR;//make quality to integer..
								if(Q_Value<0) Q_Value=0;
								BQScore+=MATCH_BONUS;match_BQScore+=MATCH_BONUS;
								
								Score += Match_score;
								match_Score += Match_score;
							}
							else if (readString[k] != genome_Char) 
							{//printf("\n%c %c %d\n",readString[k],genome_Char,k);
								true_matches++;mismatch_Temp++;
								if(true_matches > std::max(int(Read_Len*0.6),UPPER_MAX_MISMATCH) && cntHit >4) 
								{
									hitsToken[j][4] = -10000;
									break;
								}
								float Q_Value=Qual[k]-QUALITYCONVERSIONFACTOR;
								if(Q_Value<0) Q_Value=0;
								BQScore-= MN + floor( (MX-MN)*(std::min(Q_Value, 40.0f)/40.0f) );
								match_BQScore-= MN + floor( (MX-MN)*(std::min(Q_Value, 40.0f)/40.0f) );
								float Penalty= MN + floor( (MX-MN)*(std::min(Q_Value, 40.0f)/40.0f) );
								Score -= Penalty;//Convert to probability of base being wrong =1-10^(Q_Value/10)
								match_Score -= Penalty;
							}
						}
					}
					if(location==longestM_location) 
					{
						BQScore_longestM=match_BQScore;
						mismatch_longestM=mismatch_Temp;
						Score_longestM=match_Score;
					}else if(location==longestM_location-1) 
					{
						BQScore_up1=match_BQScore;
						Score_up1=match_Score;
					}else if(location==longestM_location-2) 
					{
						Score_up2=match_Score;
						BQScore_up2=match_BQScore;
					}else if(location==longestM_location+1)
					{
						Score_down1=match_Score;
						BQScore_down1=match_BQScore;
					}else if(location==longestM_location+2) 
					{
						Score_down2=match_Score;
						BQScore_down2=match_BQScore;
					}
					cig++;n=0;lens+=length;Glens+=length;RLens+=length;
					char buf[1024]="\0";
					sprintf( buf , "%dM",Read_Len);
					char buf_tmp[1024]="\0";
					sprintf( buf_tmp, "%dM",length);
					if( !strcmp(buf,  buf_tmp ) ) 
					{ 
						if(!strcmp(buf, cig))
							break;
						else
						{
							cigar_remove=true;
							strcpy(cigar_rm, buffer_cigar);
						}
						
					}
				}else if(*cig=='I')
				{
					exactmatch=false;
					//Clip_mismatch=UPPER_MAX_MISMATCH+1;//perfect match no insert
					int i;temp[n]='\0';int length=atoi(temp);
					location++;
					if(length==0) cigar_remove=true;
					if(length>0) 
					{
						real_location++;
						if(real_location==1)
							sprintf(buffer_cigar,"%dS",length);
						else
							if(length==1) sprintf(buffer_cigar,"1I");
							else if(length==2) sprintf(buffer_cigar,"2I");
							else sprintf(buffer_cigar,"%dI",length);
						strcat(cigar_rm, buffer_cigar);
					}
					if(real_location!=1)
					{
						match_BQScore=0;match_Score=0;
						match_BQScore-=BOPEN+length*BEXT;
						match_Score -=BOPEN+length*BEXT;
						if(location==longestM_location-1) 
						{
							Score_up1=match_Score;
							BQScore_up1=match_BQScore;
						}else if(location==longestM_location-2) 
						{
							Score_up2=match_Score;
							BQScore_up2=match_BQScore;
						}else if(location==longestM_location+1){
							Score_down1=match_Score;
							BQScore_down1=match_BQScore;
						}else if(location==longestM_location+2) 
						{
							Score_down2=match_Score;
							BQScore_down2=match_BQScore;
						}
	
						BQScore-= BOPEN + length*BEXT;
						Score -=BOPEN+length*BEXT;
					}

					lens+=length;RLens+=length;
					cig++;n=0;genome_move_size-=length;moveSize++;
				}else if(*cig=='D')
				{
					exactmatch=false;
					//Clip_mismatch=UPPER_MAX_MISMATCH+1;//perfect match no delete
					int i;temp[n]='\0';int length=atoi(temp);
					location++;
					if(length==0) cigar_remove=true;
					if(length>0) 
					{
						real_location++;
						if(real_location==1)
							sprintf(buffer_cigar,"%dS",length);
						else
							if(length==1) sprintf(buffer_cigar,"1D");
							else if(length==2) sprintf(buffer_cigar,"2D");
							else sprintf(buffer_cigar,"%dD",length);
						strcat(cigar_rm, buffer_cigar);
					}
					if(real_location!=1)
					{
						match_BQScore=0;match_Score=0;
						match_BQScore-=BOPEN+length*BEXT;
						match_Score -=BOPEN+length*BEXT;
						if(location==longestM_location-1) 
						{
							Score_up1=match_Score;
							BQScore_up1=match_BQScore;
						}else if(location==longestM_location-2) 
						{
							Score_up2=match_Score;
							BQScore_up2=match_BQScore;
						}else if(location==longestM_location+1) 
						{
							Score_down1=match_Score;
							BQScore_down1=match_BQScore;
						}else if(location==longestM_location+2) 
						{
							Score_down2=match_Score;
							BQScore_down2=match_BQScore;
						}
						BQScore-= BOPEN + length*BEXT;
						Score -=BOPEN+length*BEXT;
					}
					Glens+=length;RLens+=length;
					if(real_location==1) hitsToken[j][3]+=length;
					cig++;n=0;genome_move_size+=length;moveSize++;
				}else
				{
					exactmatch=false;
					//*cig='M';cig--;
					//printf("\nq--%d%c-- ",atoi(temp),*cig);
					location++;
					//continue;
					//exit(0);
            			ob=true; break;
				}
			}
			if(hitsToken[j][4] <= -10000) {continue;}//printf("\n%s-r\n",forReadString);
		       if(cigar_remove) CIG[j]=cigar_rm;
		      hitsToken[j][7]=genome_move_size;
		      hitsToken[j][8]=headClip;
		      hitsToken[j][9]=moveSize;
		      if(moveSize<1) all_Indels=false;

			int Sub_Opt_Score=INT_MAX;
			//int MapQ=BQScore;
			int Offset=0;hitsToken[j][11]=0;
			hitsToken[j][5]=true_matches;
		    if(Clip_mismatch+true_matches <= 2 && ClipLength!=0  ) //cc
		    {
				hitsToken[j][11]=1;
				hitsToken[j][5] = true_matches+Clip_mismatch;
		    
		        if(Clip_Count)
			{      
				int Score_Add=std::min((Clip_Count)*MN+1,BOPEN+BEXT*(Clip_Count-1));
				if(Clip_Count<=5 && cntHit>1) 
				{	
					BQScore +=ClipBQScore;
					Score += ClipScore;
				}
				else
				{	 // remove
					Score -= BOPEN+(Clip_Count)*BEXT; 
					BQScore-=Score_Add;
				}
			}
		    }

			//Quality_Score=std::max(1,QUAL_START-Offset+MapQ-std::min(Top_Penalty,QUAL_START/3));
			hitsToken[j][4] = Score; 
			hitsToken[j][12]=0;
			if( (edit<3 && cntHit>20) || Read_Len<100 )
				hitsToken[j][6]=BQScore;
			else
			{//local
				hitsToken[j][5]=mismatch_longestM;
				hitsToken[j][6]=std::max(BQScore_longestM+BQScore_up1+BQScore_up2,BQScore_longestM);
				hitsToken[j][6]=std::max(BQScore_longestM+BQScore_down1+BQScore_down2,hitsToken[j][6]);

				if(hitsToken[j][6]==BQScore_longestM) hitsToken[j][4]=Score_longestM;
				else if(hitsToken[j][6]==BQScore_longestM+BQScore_up1+BQScore_up2) hitsToken[j][4]=Score_longestM+Score_up1+Score_up2;
				else if(hitsToken[j][6]==BQScore_longestM+BQScore_down1+BQScore_down2) hitsToken[j][4]=Score_longestM+Score_down1+Score_down2;
				hitsToken[j][12]=1;
			}	
//			printf("%d %d %d %d %d %s %d %d %d %d\n",Clip_mismatch,true_matches,mismatch_longestM,Score,BQScore,hits[j].c_str(),longestM, Score_longestM, BQScore_longestM, hitsToken[j][6]);
			hitsToken[j][10] = longestM;
		    if(ob) {hitsToken[j][4] = INT_MAX;continue;}
		}
	}// !paired
		for (int i=0; i<cntHit;i++) 
		{
			if(hitsToken[i][4]==INT_MAX) hits[i].clear();
			if (hits[i].empty()) continue;
			for (int j=i+1; j<cntHit;j++) 
			{
				if (hits[j].empty()) continue;
				bool same = true;
				for (int z=1;z<=1;z++) //1 H chrom //3 position
				{
					if (hitsToken[i][z]!=hitsToken[j][z] || abs((int)hitsToken[i][3]-(int)hitsToken[j][3])>Read_Len )
					{
						same = false;
						break;
					}
				}
				if (same) 
				{
					if(hitsToken[i][4] >= hitsToken[j][4] ) //if(hitsToken[i][5]<=hitsToken[j][5])
						hits[j].clear();
					else 
					{
						hits[i]=hits[j];
						for (int z=0;z<=11;z++) 
						{
							hitsToken[i][z]=hitsToken[j][z];
						}
						//strcpy(read_Methyl_Info[i],read_Methyl_Info[j]);
						CIG[i]=CIG[j];
						hits[j].clear();
					}
				}	
			}
		}
				
		//making sure the hit is unique
		int first=0;	
		//int Min_Score=hitsToken[0][4]-3;//UPPER_MAX_MISMATCH+1;
		int ind = -1;short secondind =-1;
		short gdHit = 0;
		int maxScore=-1,secondScore=-1;int nSecond=0;
		const int QUAL_START=60,Q_GAP=10;int Offset=0;
		int Quality_Score=0;
		for (int i=0; i<cntHit; i++ ) 
		{
			if (!hits[i].empty() && (hitsToken[i][5] <= 10 /*UPPER_MAX_MISMATCH*/ || Paired) ) //&& (hitsToken[i][5] <= UPPER_MAX_MISMATCH /*std::max(int(Read_Len*0.1),UPPER_MAX_MISMATCH)*/ || cntHit==1 || (Read_Len >=150 && \
				std::max(int(Read_Len*0.1),UPPER_MAX_MISMATCH) ) )  ) 
			{
				//if(!first) minScore=hitsToken[i][4]-3; 
				//first=1;
				int tempNum = 0;
				if(all_Indels)
				{
					tempNum = hitsToken[i][4];// 9 is edit distance
					
				}
				else
					tempNum = hitsToken[i][4];
				if ( tempNum > maxScore) // cha diff 2 score tempNum> (Max_QScore+2 )
				{
					secondScore=maxScore;
					secondind=ind;
					nSecond=1;
					
					maxScore=tempNum;
					ind=i;
					gdHit=1;
				}
				else if ( tempNum>secondScore ) //abs(tempNum-Max_QScore)<=2
				{
					secondScore=tempNum;
					secondind=i;
					nSecond=1;
				}else if(tempNum==secondScore)
				{
					nSecond++;
				}
			}
		}
		nSecond=0;
		for(int i=0; i<cntHit; i++)
		{
			if(ind==-1) break;
			else if(hitsToken[i][4]==secondScore)
				nSecond++;
		}
		if(ind!=-1 && hitsToken[ind][12]==1) partialQC=true;
		//if(ind!=-1 && all_Indels) hitsToken[ind][6]=hitsToken[ind][10];
		if(ind!=-1 && hitsToken[ind][11]==1)
		{
			CIG[ind] = remove_soft_split(CIG[ind],Read_Len,hitsToken[ind][3]);
		}
		//MinandSec(hitsToken,0,cntHit-1, minScore,secondScore,ind);
		if(ind!=-1 && maxScore!=-1 && !Paired)
		{
			if(secondScore==INT_MAX || (exactmatch && maxScore > secondScore + 2*MN ) )//unique
			{
				Quality_Score=std::min(60,std::max(1/*0*/,QUAL_START-Offset+(int)hitsToken[ind][6]-std::min(Top_Penalty,QUAL_START/3) ));//
				gdHit=1;
				if(!exactmatch && Quality_Score>50 && Quality_Score <=56 ) Quality_Score=50;
				if(exactmatch && Quality_Score<=50 && Quality_Score >40 ) Quality_Score=51;
				if(hitsToken[ind][5]>10 && Read_Len<100) Quality_Score=0;
			}
			else
			{
				Quality_Score=QUAL_START-Offset+(int)hitsToken[ind][6]-std::min(Top_Penalty,QUAL_START/3);
	//			printf("\nQS %d %d %d %d %d\n",hitsToken[ind][6],Quality_Score,secondScore, maxScore, MN);
				if( ( maxScore >= 120 && maxScore > secondScore + 4*MN) || (maxScore < 120 && maxScore > secondScore + 30)  )
				{
					if(hitsToken[ind][5] > UPPER_MAX_MISMATCH && Read_Len<100 && maxScore <= secondScore + 3*Q_Gap) Quality_Score=0;
					else if(maxScore > secondScore + 3*Q_Gap && !partialQC )
					{
						Quality_Score=std::min(60,std::max(1/*0*/,QUAL_START-Offset+(int)hitsToken[ind][6]-std::min(Top_Penalty,QUAL_START/3) ));
					}else if(!partialQC || (maxScore > secondScore + 2*Q_Gap && maxScore >= 0.6*Read_Len*Match_score ) )
					{
						double identity;
						int l = hitsToken[ind][10];
						identity = 1. - (double)(l * Match_score - maxScore) / (Match_score + MN) / l;
						double tmp,mapQ_coef_len=50.0;
						int mapQ_coef_fac=3;
						tmp = l < mapQ_coef_len? 1. : mapQ_coef_fac / log(l);
						Quality_Score = (int)(6.02 * (maxScore - secondScore) / Match_score * tmp * tmp + .499);//printf("\n--- %d %f",Quality_Score,tmp);
						Quality_Score -= (int)(4.343 * log(nSecond/3+1) + .499);//printf("\n--- %d %d %d\n",Quality_Score,nSecond,secondScore);
						if(Quality_Score>60) Quality_Score=60;
						if(Quality_Score<0) Quality_Score=0;
						Quality_Score = (int)(Quality_Score * 1.0 + .499);
					}else
						Quality_Score=0;
					
					if(hitsToken[ind][5]> 10 /*0.1 * Read_Len*/) Quality_Score=0;
					//if( hitsToken[ind][5] > UPPER_MAX_MISMATCH ) Quality_Score=std::min(0,Quality_Score-8);
					/*
					if(minScore < secondScore-Q_GAP*3 )
						Quality_Score=std::min(60,std::max(0,QUAL_START-Offset+(int)hitsToken[ind][6]-std::min(Top_Penalty,QUAL_START/3) ));
					if(minScore < secondScore-Q_GAP*2 )
						Quality_Score=std::min(60,std::max(0,QUAL_START*(rand()%(3)+3)/5-(secondScore-minScore)/3-Offset+(int)hitsToken[ind][6]-std::min(Top_Penalty,QUAL_START/3) ));
					else if(minScore < secondScore-Q_GAP )
						Quality_Score=std::max(1,QUAL_START*(rand()%2+2)/5+(int)hitsToken[ind][6]*(rand()%2+2)/2);
					else
						Quality_Score=std::max(0,QUAL_START*(rand()%2+1)/5+(int)hitsToken[ind][6]*(rand()%2+2)/2);
					*/
					/*
					
					int Second=hitsToken[secondind][6];int First=hitsToken[ind][6];
					Quality_Score=std::max(0,(int)(std::min(QUAL_START*1.0,QUAL_START*std::max(0.1, (double)(Second -First)/Second) )+
						First) );
					gdHit=1;
				*///qw
				/*	if(First <= Second+Q_GAP && Quality_Score>0) //too closet
					{
						Quality_Score=std::max(0,(int)(QUAL_START*std::min(0.8, (double)(Second -First)/Second+0.05) +
							(int)First + (int)(nSecond*(int)(( (First+2)*rand()%(First-Second+1)+Second) )* (1*( (float)(Second -First)/Second) )  ) ));
						//rand()%(n-m+1)+m //[m,n]
					}else*/ 
					/*//qw
					if(hitsToken[ind][6] <= hitsToken[secondind][6]+Q_GAP*1/5 && Quality_Score>0)
					{
						Quality_Score=0;//std::max(0,(int)(std::min(QUAL_START*1.0,QUAL_START*std::min(0.1, 0.8*(double)(Second -First)/Second+0.05) )+
						//(rand()%2+2)*First+(int)((int)( (First+1)*rand()%(First-Second+1)+Second)* (3*( (float)(Second - First)/Second) )) ));
					}else if(hitsToken[ind][6] <= hitsToken[secondind][6]+Q_GAP*2/5 && Quality_Score>0)
					{
						Quality_Score=std::max(0,(int)(std::min(QUAL_START*1.0,QUAL_START*std::min(0.3, 0.3*(double)(Second -First)/Second+0.05) )+
						2*First));//+(int)((int)( (First+1)*rand()%(First-Second+1)+Second)* (3*( (float)(Second -First)/Second) )) ))
					}
					if(hitsToken[ind][6] <= hitsToken[secondind][6]+Q_GAP/2 && Quality_Score>0)//rand()%(n-m+1)+m //[m,n]
					{//fprintf(stdout,"%s\t%d\t%d\t%d\t%d\t%d\n",Dummy,cntHit,First,Second,hitsToken[ind][0],hitsToken[secondind][0]);
						Quality_Score=std::max(0,(int)(std::min(QUAL_START*1.0,QUAL_START*std::min(0.6, 0.5*(double)(Second -First)/Second+0.1) )+
						1.5*First));
					}else if(hitsToken[ind][6] <= hitsToken[secondind][6]+Q_GAP*3/5 && Quality_Score>0)
					{
						Quality_Score=std::max(0,(int)(std::min(QUAL_START*1.0,QUAL_START*std::min(0.8, 0.8*(double)(Second -First)/Second+0.15) )+
						First));
					}
					*/
					//if(cntHit>400) Quality_Score=std::min(1,Quality_Score-cntHit/800);
				}
				else
				{
						Quality_Score=0; //0
						gdHit=0;
				}
				//if(Quality_Score==0 && tmpScore>=30) Quality_Score=1;
			}
			hitsToken[ind][4]=Quality_Score ;// <(int)((cntHit/100)*3)?0/*1*/:Quality_Score-(int)((cntHit/100)*3);
		}
        			int maxHits=40;
				if(UPPER_MAX_MISMATCH > 0) maxHits=300;
				//if(UPPER_MAX_MISMATCH > 0 && maxHits > 40 ) ENTROPY_CUTOFF=0.25;
				if( NP || PMs1> maxHits  ||PMs2 > maxHits  || PMs3> maxHits || PMs4> maxHits  ) ind=-1;
				bool OUTP=false;
				if(UNIQUE && gdHit==1) OUTP=true;
				if(!UNIQUE) OUTP=true;
				if(ind!=-1 && calc_Entropy(readString, Read_Len)>= ENTROPY_CUTOFF )
				{
					pthread_mutex_lock(&mapper_counter_mutex);
					ALL_MAP_Org++;
					pthread_mutex_unlock(&mapper_counter_mutex);
				}
				if (OUTP && ind!=-1 && calc_Entropy(readString, Read_Len)>= ENTROPY_CUTOFF ) //
				{
					if(gdHit==1) 
					{
						pthread_mutex_lock(&mapper_counter_mutex);
						Tot_Unique_Org++;
						pthread_mutex_unlock(&mapper_counter_mutex);
					}
					string s3t = hits[ind];
					int *HL = hitsToken[ind];char Flag=0;
					unsigned int H=HL[1];
					Genome=((ARGS *)arg)->Genome_List[H].Genome;//load current genome..
					unsigned G_Skip=Genome-((ARGS *)arg)->Org_Genome;
					if(HL[0] == '1' || HL[0] == '3' ) Flag=2; else Flag=4;
					char Mark=((ARGS *)arg)->Marked_Genome[HL[3]+G_Skip];
					char MarkE='1';//((ARGS *)arg)->Marked_GenomeE[HL[3]+G_Skip+readString.size()];
					if ( (!Mark || !(Mark & Flag)) || (!MarkE || !(MarkE & Flag))  || !REMOVE_DUP)//Remove duplicate hits..
					{
						//Genome=Genome_List[H].Genome;//load current genome..
						int Hash_Index=((ARGS *)arg)->Genome_List[H].Index;//load current genome..
						unsigned pos=HL[3];
						
						int hitType=HL[0];
						if (hitType==3 || hitType==4) readString = rcReadString;
						else readString=forReadString;
						char read_Methyl_Info[Read_Len*5];char rawReadBeforeBS[Read_Len*5];strcpy(rawReadBeforeBS,readString.c_str());
						char temp[5];unsigned lens=0;int Glens=0;int RLens=0;
						unsigned n=0;
						const char* cigr=CIG[ind].c_str();
						while(*cigr!='\0')//RLens--READs Length \\ lens--raw reads length \\ GLens--genome Lens
						{
							if(*cigr>='0' && *cigr<='9')
							{
								temp[n]=*cigr;
								cigr++;n++;
							}else if(*cigr=='S')
							{
								int i;temp[n]='\0';int length=atoi(temp);
								for(i=RLens;i<RLens+length;i++)
								{
									read_Methyl_Info[i] = 'S';
								}
								lens+=length;
	                                             RLens+=length;
	                                             cigr++;n=0;
							}else if(*cigr=='M')
							{
								pthread_mutex_lock(&meth_counter_mutex);
								temp[n]='\0';int length=atoi(temp);
								for(int k=lens,r=RLens,g=Glens;k<length+lens;r++,k++,g++)
								{
									read_Methyl_Info[r] = '=';
									if (pos+g-1 >= ((ARGS *)arg)->Genome_Offsets[Hash_Index+1].Offset) break;
				
									char genome_Char = toupper(Genome[pos+g-1]);//
									char genome_CharFor1 = toupper(Genome[pos+g+1-1]);
									char genome_CharFor2 = toupper(Genome[pos+g+2-1]);
									char genome_CharBac1,genome_CharBac2;
									if(pos+g-1 > 2)
									{
										genome_CharBac1 = toupper(Genome[pos+g-1-1]);
										genome_CharBac2 = toupper(Genome[pos+g-2-1]);
									}
									if (hitType==1 || hitType==3) {
										//pthread_mutex_lock(&meth_counter_mutex13);
										if (readString[k]=='C' && genome_Char=='C')
										{
											read_Methyl_Info[r] = 'M';
											if(gdHit==1 && Methratio && hitsToken[ind][4] > QualCut) ((ARGS *)arg)->Methy_List[H].plusMethylated[pos+g-1]++;
										//	if(hitType==1) {
												if(genome_CharFor1=='G') 
												{
													read_Methyl_Info[r] = 'Z';met_CG++;
													//if(Methratio && hitsToken[ind][4] > QualCut ) ((ARGS *)arg)->Methy_List[H].MethContext[pos+g-1]=1;
												}//Z methylated C in CpG context
												else if(genome_CharFor1!='G' && genome_CharFor2!='G')
												{
													read_Methyl_Info[r] = 'H';met_CHH++;
													//if(Methratio && hitsToken[ind][4] > QualCut) ((ARGS *)arg)->Methy_List[H].MethContext[pos+g-1]=3;
												}//H methylated C in CHH context
												else if(genome_CharFor1!='G' && genome_CharFor2=='G')
												{
													read_Methyl_Info[r] = 'X';met_CHG++;
													//if(Methratio && hitsToken[ind][4] > QualCut  )  ((ARGS *)arg)->Methy_List[H].MethContext[pos+g-1]=2;
												}//X methylated C in CHG context
										}
										else if (readString[k]=='T' && genome_Char=='C')
										{
											read_Methyl_Info[r] = 'U';
											rawReadBeforeBS[k] = 'C';
											if(gdHit==1 && Methratio && hitsToken[ind][4] > QualCut ) ((ARGS *)arg)->Methy_List[H].plusUnMethylated[pos+g-1]++;
										//	if(hitType==1) {
												if(genome_CharFor1=='G')
												{
													read_Methyl_Info[r] = 'z';non_met_CG++;
													//if(Methratio && hitsToken[ind][4] > QualCut ) ((ARGS *)arg)->Methy_List[H].MethContext[pos+g-1]=1;
												}//z unmethylated C in CpG context
												else if(genome_CharFor1!='G' && genome_CharFor2!='G') 
												{
													read_Methyl_Info[r] = 'h';non_met_CHH++;
													//if(Methratio && hitsToken[ind][4] > QualCut ) ((ARGS *)arg)->Methy_List[H].MethContext[pos+g-1]=3;
												}//h unmethylated C in CHH context
												else if(genome_CharFor1!='G' && genome_CharFor2=='G') 
												{
													read_Methyl_Info[r] = 'x';non_met_CHG++;
													//if(Methratio  && hitsToken[ind][4] > QualCut ) ((ARGS *)arg)->Methy_List[H].MethContext[pos+g-1]=2;
												}//x unmethylated C in CHG context
											
										}
										else if (readString[k] != genome_Char) 
										{
											read_Methyl_Info[r] = readString[k]; // genome_Char; for hypol
										}
										if(gdHit==1 && Methratio && hitsToken[ind][4] > QualCut )
										{
											if(readString[k]=='G') ((ARGS *)arg)->Methy_List[H].plusG[pos+g-1]++;
											else if(readString[k]=='A') ((ARGS *)arg)->Methy_List[H].plusA[pos+g-1]++;
											//Methy_List[H].plusCover[pos+g-1]++;
										}
										//pthread_mutex_unlock(&meth_counter_mutex13);
									}
									else if (hitType==2 || hitType==4) {
										//pthread_mutex_lock(&meth_counter_mutex24);
										if (readString[k]=='G' && genome_Char=='G')
										{
											read_Methyl_Info[r] = 'M';
											if(gdHit==1 && Methratio && hitsToken[ind][4] > QualCut ) ((ARGS *)arg)->Methy_List[H].NegMethylated[pos+g-1]++;
										//	if(hitType==2) 
										//	{
												if(genome_CharBac1=='C') 
												{
													read_Methyl_Info[r] = 'Z';met_CG++;
													//if(Methratio  && hitsToken[ind][4] > QualCut) ((ARGS *)arg)->Methy_List[H].MethContext[pos+g-1]=1;
												}
												else if(genome_CharBac1!='C' && genome_CharBac2!='C') 
												{
													read_Methyl_Info[r] = 'H';met_CHH++;
													//if(Methratio  && hitsToken[ind][4] > QualCut) ((ARGS *)arg)->Methy_List[H].MethContext[pos+g-1]=3;
												}
												else if(genome_CharBac1!='C' && genome_CharBac2=='C') 
												{
													read_Methyl_Info[r] = 'X';met_CHG++;
													//if(Methratio && hitsToken[ind][4] > QualCut) ((ARGS *)arg)->Methy_List[H].MethContext[pos+g-1]=2;
												}
										}
										else if (readString[k]=='A' && genome_Char=='G')
										{
											read_Methyl_Info[r] = 'U';
											rawReadBeforeBS[k] = 'G';
											if(gdHit==1 && Methratio  && hitsToken[ind][4] > QualCut) ((ARGS *)arg)->Methy_List[H].NegUnMethylated[pos+g-1]++;
										//	if(hitType==2) 
										//	{
												if(genome_CharBac1=='C') 
												{
													read_Methyl_Info[r] = 'z';non_met_CG++;
													//if(Methratio && hitsToken[ind][4] > QualCut) ((ARGS *)arg)->Methy_List[H].MethContext[pos+g-1]=1;
												}
												else if(genome_CharBac1!='C' && genome_CharBac2!='C') 
												{
													read_Methyl_Info[r] = 'h';non_met_CHH++;
													//if(Methratio && hitsToken[ind][4] > QualCut ) ((ARGS *)arg)->Methy_List[H].MethContext[pos+g-1]=3;
												}
												else if(genome_CharBac1!='C' && genome_CharBac2=='C') 
												{
													read_Methyl_Info[r] = 'x';non_met_CHG++;
													//if(Methratio  && hitsToken[ind][4] > QualCut) ((ARGS *)arg)->Methy_List[H].MethContext[pos+g-1]=2;
												}
										}
										else if (readString[k] != genome_Char) {
											
											read_Methyl_Info[r] = readString[k]; //genome_Char;
										}
										if(gdHit==1 && Methratio  && hitsToken[ind][4] > QualCut)
										{
											if(readString[k]=='C') ((ARGS *)arg)->Methy_List[H].NegG[pos+g-1]++;
											else if(readString[k]=='T') ((ARGS *)arg)->Methy_List[H].NegA[pos+g-1]++;
											//Methy_List[H].NegCover[pos+g-1]++;
										}
										//pthread_mutex_unlock(&meth_counter_mutex24);
									}
								}
								pthread_mutex_unlock(&meth_counter_mutex);
								cigr++;n=0;lens+=length;Glens+=length;RLens+=length;
							}else if(*cigr=='I')
							{
								int i;temp[n]='\0';int length=atoi(temp);
								for(i=0;i<length;i++)
								{
									read_Methyl_Info[i+RLens] = 'I';
								}
								lens+=length;
								RLens+=length;
								cigr++;n=0;
							}else if(*cigr=='D')
							{
								int i;temp[n]='\0';unsigned int length=atoi(temp);
								for(i=0;i<length;i++)
								{
									read_Methyl_Info[i+RLens] = 'D';
								}
								Glens+=length;RLens+=length;
								cigr++;n=0;
							}else
							{
								printf(" --%d%c%s-- ",atoi(temp),*cigr,Dummy);
								//continue;
								exit(0);
							}
						}
						read_Methyl_Info[RLens]='\0';rawReadBeforeBS[lens]='\0';
						forQuality[lens]='\0';rcQuality[lens]='\0';
						forReadString[lens]='\0';rcReadString[lens]='\0';
						string perfectMactch = "PM";
						if(hitsToken[ind][6]== 1 ) perfectMactch="PM"; //|| hitsToken[ind][6]==2
						else perfectMactch = "SM";
						string mappingStrand="N";
						if(HL[0]==1) mappingStrand="BSW";
						else if(HL[0]==4) mappingStrand="BSC";
						else if(HL[0]==3) mappingStrand="BSWRC";
						else if(HL[0]==2) mappingStrand="BSCRC";
						flockfile(((ARGS *)arg)->OUTFILE);
						if(UNIQUE) //unique mapping
						{
							if(Sam)//
							{
								if(Paired)
								{
									if(SamSeqBeforeBS) 
									{
										if(PrintMethState)//&Chrom_P,&pos_P,&Insert_Size
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t%s\t%d\t%d\t%s\t%s\tXT:A:U\tNM:i:%d\tMD:Z:%s\tXB:Z:%s\tRA:Z:%s\n",Dummy,HL[2]/*(HL[0]==1 || HL[0]==3)?0:16*/,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),Chrom_P,pos_P,Insert_Size,rawReadBeforeBS,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,HL[5],read_Methyl_Info,mappingStrand.c_str(),(HL[0]==1 || HL[0]==2)?forReadString:rcReadString);
										else
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t%s\t%d\t%d\t%s\t%s\tXT:A:U\tNM:i:%d\tXB:Z:%s\tRA:Z:%s\n",Dummy,HL[2]/*(HL[0]==1 || HL[0]==3)?0:16*/,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),Chrom_P,pos_P,Insert_Size,rawReadBeforeBS,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,HL[5],mappingStrand.c_str(), (HL[0]==1 || HL[0]==2)?forReadString:rcReadString);
									}else 
									{
										if(PrintMethState)
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t%s\t%d\t%d\t%s\t%s\tXT:A:U\tNM:i:%d\tMD:Z:%s\tXB:Z:%s\n",Dummy,HL[2]/*(HL[0]==1 || HL[0]==3)?0:16*/,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),Chrom_P,pos_P,Insert_Size,(HL[0]==1 || HL[0]==2)?forReadString:rcReadString,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,HL[5],read_Methyl_Info,mappingStrand.c_str());
										else
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t%s\t%d\t%d\t%s\t%s\tXT:A:U\tNM:i:%d\tXB:Z:%s\n",Dummy,HL[2]/*(HL[0]==1 || HL[0]==3)?0:16*/,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),Chrom_P,pos_P,Insert_Size,(HL[0]==1 || HL[0]==2)?forReadString:rcReadString,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,HL[5],mappingStrand.c_str());
									}
								}
								else 
								{
									if(SamSeqBeforeBS) 
									{
										if(PrintMethState)
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t*\t0\t0\t%s\t%s\tXT:A:U\tNM:i:%d\tMD:Z:%s\tXB:Z:%s\tRA:Z:%s\n",Dummy,(HL[0]==1 || HL[0]==3)?0:16,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),rawReadBeforeBS,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,HL[5],read_Methyl_Info,mappingStrand.c_str(), (HL[0]==1 || HL[0]==2)?forReadString:rcReadString);
										else
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t*\t0\t0\t%s\t%s\tXT:A:U\tNM:i:%d\tXB:Z:%s\tRA:Z:%s\n",Dummy,(HL[0]==1 || HL[0]==3)?0:16,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),rawReadBeforeBS,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,HL[5],mappingStrand.c_str(), (HL[0]==1 || HL[0]==2)?forReadString:rcReadString);
									}else 
									{
										if(PrintMethState)
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t*\t0\t0\t%s\t%s\tXT:A:U\tNM:i:%d\tMD:Z:%s\tXB:Z:%s\n",Dummy,(HL[0]==1 || HL[0]==3)?0:16,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),(HL[0]==1 || HL[0]==2)?forReadString:rcReadString,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,HL[5],read_Methyl_Info,mappingStrand.c_str());
										else
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t*\t0\t0\t%s\t%s\tXT:A:U\tNM:i:%d\tXB:Z:%s\n",Dummy,(HL[0]==1 || HL[0]==3)?0:16,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),(HL[0]==1 || HL[0]==2)?forReadString:rcReadString,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,HL[5],mappingStrand.c_str());
									}
								}
							}
							else
							{
								if(Paired) 
								{
									fprintf(((ARGS *)arg)->OUTFILE,"@\n%s%u\t%s\t%d\t%u\t%u\t%d\t%s\t%s\t%d\t%d\n",s2t,HL[0],((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[2],HL[3],HL[5]/*min_Mis*/,Read_Len,read_Methyl_Info,CIG[ind].c_str(),HL[4],Insert_Size );//,perfectMactch.c_str()
								}else
								{
									fprintf(((ARGS *)arg)->OUTFILE,"@\n%s%u\t%s\t%c\t%u\t%u\t%d\t%s\t%s\n",s2t,HL[0],((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[2],HL[3],HL[5]/*min_Mis*/,Read_Len,read_Methyl_Info,CIG[ind].c_str());
								}
							}
						}
						else //multipe mapping
						{
						 	string mapping="M";
							if(gdHit==1) mapping="U";
							if(Sam)
							{
								if(Paired)
								{
									if(SamSeqBeforeBS) 
									{
										if(PrintMethState)
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t%s\t%d\t%d\t%s\t%s\tXT:A:%s\tNM:i:%d\tMD:Z:%s\tXB:Z:%s\tRA:Z:%s\n",Dummy,(HL[0]==1 || HL[0]==3)?0:16,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),Chrom_P,pos_P,Insert_Size,rawReadBeforeBS,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,mapping.c_str(),HL[5],read_Methyl_Info,mappingStrand.c_str(),(HL[0]==1 || HL[0]==2)?forReadString:rcReadString);
										else
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t%s\t%d\t%d\t%s\t%s\tXT:A:%s\tNM:i:%d\tXB:Z:%s\tRA:Z:%s\n",Dummy,(HL[0]==1 || HL[0]==3)?0:16,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),Chrom_P,pos_P,Insert_Size,rawReadBeforeBS,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,mapping.c_str(),HL[5],mappingStrand.c_str(),(HL[0]==1 || HL[0]==2)?forReadString:rcReadString);
									}else
									{ 
										if(PrintMethState)
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t%s\t%d\t%d\t%s\t%s\tXT:A:%s\tNM:i:%d\tMD:Z:%s\tXB:Z:%s\n",Dummy,(HL[0]==1 || HL[0]==3)?0:16,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),Chrom_P,pos_P,Insert_Size,(HL[0]==1 || HL[0]==2)?forReadString:rcReadString,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,mapping.c_str(),HL[5],read_Methyl_Info,mappingStrand.c_str());
										else
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t%s\t%d\t%d\t%s\t%s\tXT:A:%s\tNM:i:%d\tXB:Z:%s\n",Dummy,(HL[0]==1 || HL[0]==3)?0:16,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),Chrom_P,pos_P,Insert_Size,(HL[0]==1 || HL[0]==2)?forReadString:rcReadString,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,mapping.c_str(),HL[5],mappingStrand.c_str());
									}
								}
								else 
								{	
									if(SamSeqBeforeBS) 
									{
										if(PrintMethState)
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t*\t0\t0\t%s\t%s\tXT:A:%s\tNM:i:%d\tMD:Z:%s\tXB:Z:%s\tRA:Z:%s\n",Dummy,(HL[0]==1 || HL[0]==3)?0:16,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),rawReadBeforeBS,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,mapping.c_str(),HL[5],read_Methyl_Info,mappingStrand.c_str(),(HL[0]==1 || HL[0]==2)?forReadString:rcReadString);
										else
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t*\t0\t0\t%s\t%s\tXT:A:%s\tNM:i:%d\tXB:Z:%s\tRA:Z:%s\n",Dummy,(HL[0]==1 || HL[0]==3)?0:16,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),rawReadBeforeBS,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,mapping.c_str(),HL[5],mappingStrand.c_str(),(HL[0]==1 || HL[0]==2)?forReadString:rcReadString);
									}else
									{ 
										if(PrintMethState)
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t*\t0\t0\t%s\t%s\tXT:A:%s\tNM:i:%d\tMD:Z:%s\tXB:Z:%s\n",Dummy,(HL[0]==1 || HL[0]==3)?0:16,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),(HL[0]==1 || HL[0]==2)?forReadString:rcReadString,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,mapping.c_str(),HL[5],read_Methyl_Info,mappingStrand.c_str());
										else
											fprintf(((ARGS *)arg)->OUTFILE,"%s\t%u\t%s\t%u\t%d\t%s\t*\t0\t0\t%s\t%s\tXT:A:%s\tNM:i:%d\tXB:Z:%s\n",Dummy,(HL[0]==1 || HL[0]==3)?0:16,((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[3],HL[4],CIG[ind].c_str(),(HL[0]==1 || HL[0]==2)?forReadString:rcReadString,(HL[0]==1 || HL[0]==2)?forQuality:rcQuality,mapping.c_str(),HL[5],mappingStrand.c_str());
									}
								}
							}
							else//MAP format
							{
								if(Paired) fprintf(((ARGS *)arg)->OUTFILE,"@\n%s%u\t%s\t%d\t%u\t%u\t%d\t%s\t%s\t%d\t%d\t%s\n",s2t,HL[0],((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[2],HL[3],HL[5]/*min_Mis*/,Read_Len,read_Methyl_Info,CIG[ind].c_str(),HL[4],Insert_Size,mapping.c_str());//,perfectMactch.c_str()
								else 
								{
								         fprintf(((ARGS *)arg)->OUTFILE,"@\n%s%u\t%s\t%c\t%u\t%u\t%d\t%s\t%s\t%s\t%s\n",s2t,HL[0],((ARGS *)arg)->Genome_Offsets[((ARGS *)arg)->Genome_List[HL[1]].Index].Genome,HL[2],HL[3],HL[5]/*min_Mis*/,Read_Len,read_Methyl_Info,CIG[ind].c_str(),perfectMactch.c_str(),mapping.c_str());
								}
							}
						}
						funlockfile(((ARGS *)arg)->OUTFILE);
						pthread_mutex_lock(&mapper_counter_mutex);
						ALL_Map_Remdup++;
						if(gdHit==1)
							Tot_Unique_Remdup++;
						pthread_mutex_unlock(&mapper_counter_mutex);
					}
					((ARGS *)arg)->Marked_Genome[HL[3]+G_Skip] |= Flag;
					((ARGS *)arg)->Marked_GenomeE[HL[3]+G_Skip+readString.size()] |= Flag;
				}
			}
			
}

void Print_Mismatch_Quality(FILE* OUTFILE_MM, int L) {
	char bases[] = "ACGT";

	for (int i=0; i<L; i++) {
		for (int j=0; j<4; j++) {
			for (int k=0; k<4; k++) {
				fprintf(OUTFILE_MM,"%u\t", Mismatch_Qual[i][bases[j]][bases[k]]);
			}
		} fprintf(OUTFILE_MM,"\n");
	}

}

inline unsigned char Hash(char* S)
{
	assert(false);
	unsigned char C=0;
	for (int i=2;S[i];C+=i*S[i++]);
	return C;
}

inline float calc_Entropy (string readString, int L)
{ 
	short entropy_arr[255]={0};
	//int length=strlen(readString);
	for(int i=0; i<L; i++) entropy_arr[readString[i]]++;
	
	float entropy=0.0;
	for(int i=0; i<4; i++) {
		double p = 1.0*entropy_arr["ACGT"[i]]/L;
		if(p>0) entropy-=p*log(p);
	}
  //if( entropy_arr["ACGT"[1]] + entropy_arr["ACGT"[3]] ==L ||  entropy_arr["ACGT"[0]] + entropy_arr["ACGT"[2]] == L ) entropy = 0;
	return entropy;
}

int Get_String_Length(FILE* INFILE)
{
	char Buf[BATBUF],Dummy[BATBUF],Tag[BATBUF];int L;
	fgets(Buf,BATBUF,INFILE);
	fgets(Buf,BATBUF,INFILE);
	//space-trouble in read description
	for(int i=0;Buf[i];i++) if(Buf[i]==' ') Buf[i]='_';
	sscanf(Buf,"%s%s",Dummy,Tag);	
	for (L=0;Tag[L];L++);
	rewind (INFILE);
	return L;
}
//{----------------------------------- FILE HANDLING ---------------------------------------------------------

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  File_Open
 *  Description:  Open a file:
 *  Mode - "w" create/Write text from top    - "wb" Write/Binary  -"w+" open text read/write            -"w+b" same as prev/Binary
 *         "r" Read text from top            - "rb" Read/Binary   -"r+" open text read/write/nocreate   -"r+b" same as prev/binary
 *       - "a" text append/write                                  -"a+" text open file read/append      -"a+b" open binary read/append
 *
 * =====================================================================================
 */
FILE* File_Open(const char* File_Name,const char* Mode)
{
	FILE* Handle;
	Handle=fopen64(File_Name,Mode);
	if (Handle==NULL)
	{
		printf("File %s Cannot be opened ....",File_Name);
		exit(1);
	}
	else return Handle;
}

//}----------------------------------- FILE HANDLING ---------------------------------------------------------


void Show_Progress(float Percentage)
{
	if (Percentage >98) return;
	printf("+%.0f%\b\b\b",Percentage);
	fflush(stdout);
}
/*
string Get_RealRead(char* cig,string & read)
{
	std::string ReadR="";
	char temp[5];unsigned lens=0;
	unsigned n=0;
	while(*cig!='\0')
	{
		if(*cig>='0' && *cig<='9')
		{
			temp[n]=*cig;
			cig++;n++;
		}else if(*cig=='S')
		{
			int i;temp[n]='\0';
			for(i=0;i<atoi(temp);i++)
			{
				ReadR.append("S");
			}
			lens+=atoi(temp);
			cig++;n=0;
		}else if(*cig=='M')
		{
			temp[n]='\0';std::cout<<read<<endl;
			ReadR.append(read.substr(lens,atoi(temp)));
			cig++;n=0;lens+=atoi(temp);
		}else if(*cig=='I')
		{
			int i;temp[n]='\0';
			for(i=0;i<atoi(temp);i++)
			{
				ReadR.append("I");
			}
			lens+=atoi(temp);
			cig++;n=0;
		}else if(*cig=='D')
		{
			int i;temp[n]='\0';
			for(i=0;i<atoi(temp);i++)
			{
				ReadR.append("D");
			}
			//lens+=atoi(temp);
			cig++;n=0;
		}else
		{
			printf(" --%c-- ",*cig);
			exit(0);
		}
	}
	return ReadR;
}
*/
float Pr(float Q)
{
        assert((int)Q>=0 && (int)Q<POWLIMIT-1);
        return(1-POW10[(int)Q]);
        //printf("table: %f\tlib: %f\n",POW10[(int)Q],1-pow(10,-Q/10));
        //return (1-pow(10,-Q/10));
}
void Build_Pow10()
{
	for(int Q=0;Q<POWLIMIT;Q++)
	{
		POW10[Q]=(pow(10,-float(Q)/10));
	}
}
void fetch(char *str, char c1, char c2, char *buf){
    char *pl = strchr(str, c1)+1,
        *ph = strchr(str, c2);
    while( ph-pl<=0 && *(ph++)!='\t')
	ph++;
    strncpy(buf, pl, ph-pl-1);
    buf[ph-pl] = '\0';
}
void initMem(char* temp,int size,char* temp2)
{
	temp= ( char* )malloc(size);
	strcpy(temp,temp2);
}
/*
X   for methylated C in CHG context
x   for not methylated C CHG
H   for methylated C in CHH context
h   for not methylated C in CHH context
Z   for methylated C in CpG context
z   for not methylated C in CpG context
M   for methylated C in Unknown context (CN or CHN )
U   for not methylated C in Unknown context (CN or CHN)
=    for match bases
A/T/C/G   for mismatch bases
*/
string Int_to_String(int n)
{
	ostringstream stream;
	stream<<n;
	return stream.str();
}
string&   replace_all(string&   str,const   string&   old_value,const   string&   new_value,int Read_Len)   
{   
    while(true)   {   
        string::size_type   pos(0);   
        if(   (pos=str.find(old_value))!=string::npos   )   
            str.replace(pos,old_value.length(),new_value);   
        else   break;   
    }   
    if(( str.find(new_value))==string::npos)
    {
    	str=Int_to_String(Read_Len);
	str+="M";
    }
    return   str;   
}  

std::string m_replace(std::string str,std::string pattern,std::string dstPattern,int count)
{
    std::string retStr="";
    string::size_type pos;

    int szStr=str.length();
    int szPattern=pattern.size();
    int i=0;
    int l_count=0;
    if(-1 == count) // replace all
        count = szStr;

    for(i=0; i<szStr; i++)
    {
        pos=str.find(pattern,i);

        if(std::string::npos == pos)
            break;
        if(pos < szStr)
        {
            std::string s=str.substr(i,pos-i);
            retStr += s;
            retStr += dstPattern;
            i=pos+pattern.length()-1;
            if(++l_count >= count)
            {
                i++;
                break;
            }
        }
    }
    retStr += str.substr(i);
    return retStr;
}
int strSearch(char *str1,char *str2)
{
       int at,flag=1;
       if (strlen(str2) > strlen(str1))
       {
           at = -1;
       }
       else if (!strcmp(str1,str2))
       {
           at = 0;
       }
       else
       {
            unsigned i=0,j=0;
            for (i=0;i < strlen(str1)&&flag;)
           {
                  for (j=0;j < strlen(str2)&&flag;)
                 {
                       if (str1[i]!=str2[j])
                       {
                              i++;
                              j=0;
                       }
                       else if (str1[i]==str2[j])
                       {
                              i++;
                              j++;
                       }
                       if (j==strlen(str2))
                       {
                             flag = 0;
                       }
                       if(i==strlen(str1)) break;
                 }
            }
            at = i-j;
            if(flag==1) at=-1;
       }
       return at;
}

char* replace(char*src, char*sub, char*dst)
{
    int pos =0;
    int offset =0;
    int srcLen, subLen, dstLen;
    char*pRet = NULL;


    srcLen = strlen(src);
    subLen = strlen(sub);
    dstLen = strlen(dst);
    pRet = (char*)malloc(srcLen + dstLen - subLen +1);//(memory)if (NULL != pRet)
    {
        pos = strstr(src, sub) - src;
        memcpy(pRet, src, pos);
        offset += pos;
        memcpy(pRet + offset, dst, dstLen);
        offset += dstLen;
        memcpy(pRet + offset, src + pos + subLen, srcLen - pos - subLen);
        offset += srcLen - pos - subLen;
        *(pRet + offset) ='\0';
    }
    return pRet;
}
int Get_ED(std::string & S)//edit distance
{
	int ed=0;
	int i;
	for(i=0;i<S.size();i++)//momomo
	{
		if(S[i]=='I' || S[i]=='D') ed++;
		else if(S[i]=='M' || S[i]=='S' || isdigit(S[i]) ) continue;
		else return 1000;
	}
	return ed;
}
// 
void MinandSec(unsigned a[MAX_HITS_ALLOWED][10], int left, int right, int & min, int & second,int & loci)
{
	if(left == right)
	{
		min = a[left][4] ;
		second =  INT_MIN;
		loci=left;
	}
	else if(left +1== right)
	{
		min = a[left][4] < a[right][4] ? a[left][4] : a[right][4] ;
		second = a[left][4] > a[right][4] ? a[left][4] : a[right][4] ;
		loci=a[left][4] < a[right][4] ? left : right ;
	}
	else
	{
		int mid =(right + left) /2 ;

		int leftmin ;
		int leftsecond ;
		int leftloci;
		MinandSec(a, left, mid, leftmin, leftsecond,leftloci) ;

		int rightmin ;
		int rightsecond ;
		int rightloci;
		MinandSec(a, mid +1, right, rightmin, rightsecond,rightloci) ;

		if (leftmin < rightmin)
		{
			min = leftmin ;
			second = leftsecond < rightmin ? leftsecond : rightmin ;
			loci=leftloci;
		}
		else
		{
			min = rightmin ;
			second = leftmin > rightsecond ? rightsecond : leftmin ;
			loci=rightloci;
		}
	}
	return;
}
/*
void MaxandSec(int a[], int left, int right, int&max, int&second)
{
	if(left == right)
	{
		max = a[left] ;
		second =  INT_MIN;
	}
	else if(left +1== right)
	{
		max = a[left] > a[right] ? a[left] : a[right] ;
		second = a[left] < a[right] ? a[left] : a[right] ;
	}
	else
	{
		int mid =(right + left) /2 ;

		int leftmax ;
		int leftsecond ;
		MaxandSec(a, left, mid, leftmax, leftsecond) ;

		int rightmax ;
		int rightsecond ;
		MaxandSec(a, mid +1, right, rightmax, rightsecond) ;

		if (leftmax > rightmax)
		{
			max = leftmax ;
			second = leftsecond > rightmax ? leftsecond : rightmax ;
		}
		else
		{
			max = rightmax ;
			second = leftmax < rightsecond ? rightsecond : leftmax ;
		}
	}
}
*/

string remove_soft_split(string & cigar,int Read_Len,int pos)
{

	const char* cig=cigar.c_str();
	char temp[20];
	int n=0,lens=0,length=0,RLens=0,Glens=0;
	int genome_move_size=0; int headClip=0;int moveSize=0;
	char cigar_rm[1000];*cigar_rm=0;
	char buffer_cigar[100];*buffer_cigar=0; bool cigar_remove=false;
	char upper='N';char upper_cigar[1024]="\0";int upper_length=0;
	while(*cig!='\0')//RLens--READs Length \\ lens--raw reads length \\ GLens--genome Lens
	{
		if(*cig>='0' && *cig<='9')
		{
			temp[n]=*cig;
			cig++;n++;
		}else if(*cig=='S')
		{
			int i;temp[n]='\0';int length=atoi(temp);
			if(length==0) cigar_remove=true;
			if(length>0) 
			{
				if(upper=='M')
					sprintf(upper_cigar,"%dM",length+upper_length);
				else
					sprintf(upper_cigar,"%dM",length);
				
				if(upper=='N')
					pos-=length;
			}

			lens+=length;
            RLens+=length;
            cig++;n=0;
            upper='M';upper_length=length;
		}else if(*cig=='M')
		{
			temp[n]='\0';int length=atoi(temp);
			if(length==0) cigar_remove=true;
			if(length>0) 
			{
				if(upper=='M')
					sprintf(upper_cigar,"%dM",length+upper_length);
				else
					sprintf(upper_cigar,"%dM",length);
			}

			cig++;n=0;lens+=length;Glens+=length;RLens+=length;
			char buf[1024]="\0";
			sprintf( buf , "%dM",Read_Len);
			char buf_tmp[1024]="\0";
			sprintf( buf_tmp, "%dM",length);
			if( !strcmp(buf,  buf_tmp ) ) 
			{ 
				if(!strcmp(buf, cig))
					break;
				else
				{
					cigar_remove=true;
					strcpy(cigar_rm, buffer_cigar);
				}
				
			}
			upper='M';upper_length=length;
		}else if(*cig=='I')
		{
			if(upper=='M')
				strcat(cigar_rm, upper_cigar);
			int i;temp[n]='\0';int length=atoi(temp);

			if(length>0) 
			{
				if(length==1) sprintf(buffer_cigar,"1I");
				else if(length==2) sprintf(buffer_cigar,"2I");
				else sprintf(buffer_cigar,"%dI",length);
				strcat(cigar_rm, buffer_cigar);
			}

			lens+=length;RLens+=length;
			cig++;n=0;genome_move_size-=length;moveSize++;
			upper='I';
		}else if(*cig=='D')
		{
			if(upper=='M')
				strcat(cigar_rm, upper_cigar);
			int i;temp[n]='\0';int length=atoi(temp);

			if(length>0) 
			{
				if(length==1) sprintf(buffer_cigar,"1D");
				else if(length==2) sprintf(buffer_cigar,"2D");
				else sprintf(buffer_cigar,"%dD",length);
				strcat(cigar_rm, buffer_cigar);
			}

			Glens+=length;RLens+=length;
			cig++;n=0;genome_move_size+=length;moveSize++;
			upper='D';
		}else
		{
			printf(" --%d%s-- ",atoi(temp),cig);
			exit(0);
			break;
		}
	}
	if(upper=='M')
		strcat(cigar_rm, upper_cigar);
	string s(cigar_rm);
	return s;
}

void ReverseC_Context(char* Dest,const char* seq,int & stringlength)
{
	if(stringlength!=5 || strlen(seq)!=5) exit(0);
	
        for (int i=stringlength-1;i>=0;i--)
        {
                *Dest=Char2Comp[seq[i]];
                Dest++;
        }
        *Dest=0;
        //return Dest;
}
