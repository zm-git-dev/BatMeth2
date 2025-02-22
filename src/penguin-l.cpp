//#define NDEBUG
//Routines for pairing...  
#define __MAIN_CODE__

//{-----------------------------  INCLUDE FILES  -------------------------------------------------/ 
#include <cstdio> 
#include "math.h"
#include <stdarg.h> 
#include <string> 
#include <getopt.h> 
#include <limits.h> 
#include <string.h> 
#include <stdlib.h> 
#include <xmmintrin.h> 
#include <emmintrin.h> 
#include <ctype.h> 
#include "bfix.h" 
#include "assert.h"
#include "ssw-l.h"
#include "common-l.h" 
#include "rqindex-l.h"
#include "batlib-l.h"
#include <map>
#include <set>
#include <queue>
#include "global-l.h"
#include "unistd.h"
#include "swroutines-l.h"
#include "print-l.h"
#include "filters-l.h"
#include <pthread.h>
#include "sched.h"
#include "fastsw-l.h"
extern "C" 
{
	#include "iniparser-l.h"
	#include <time.h>
	#include "MemManager-l.h"
	#include "MiscUtilities-l.h"
	#include "TextConverter-l.h"
	#include "BWT-l.h"
}

//}-----------------------------  INCLUDE FILES  -------------------------------------------------/
long File_size;
const int MAX_REC_SIZE = 2000;//maximum size a record of batman will occupy..
const int INITIAL_PROGRESS_READS =1000;//progress bar initial count..
int MAXHITS = 1200;
const int FILEBUFSIZE=10000000;//Space for batman to write records.. 600000
int MAXHITS_batmeth = 195;//195;
int maxhits=150; //150
int SecondNcutoff=400;
extern bool FindInDels;
unsigned MAX_Hits=1000;
MEMLOOK MLookGA,MLookCT;
int TOP_TEN=0;
int BOOST=0;
int Dummy_Int=0;
int CUT_MAX_SWALIGN=1000;
int MODE=INT_MAX;
int SEEDSIZE=INT_MAX;
int INSERT=5;
int DELETE=5;
const int ScaleQ[]={0,1.5,1.75,2,3,4};
extern const Alignment Default_Alignment={0};
const int ORGSTRINGLENGTH=2200; 
//extern const int QUALITYCONVERSIONFACTOR=64;
//extern const int QUALITYSCALEFACTOR=33;
extern bool Only_non_directional;
bool FASTDECODE=false;
bool FASTSW=true;
bool DEBUG_SEGS=false;
int QUALITYCONVERSIONFACTOR=33;
int QUALITYSCALEFACTOR=1;
BATPARAMETERS BP;
int THREAD = 0;
int Top_Penalty;
int JUMP=8;
int INDELGAP=21;//15 good for //17 for 8, 19=g00d for 9
FMFILES FMFiles;
FMFILES CT,GA;//FMFiles,
char Char_To_R[255];

int indelMis=3;
//{-----------------------------  FUNCTION PRTOTYPES  -------------------------------------------------/*
inline void Copy_H_to_A(Alignment & A,Hit_Info & H);
void FileInfo(char *PATTERNFILE,char *HITSFILE,char MAX_MISMATCHES,int Patternfile_Count,char* PATTERNFILE1,char FILETYPE,LEN & L,char FORCESOLID);
int Find_Cigar(char* Current_Tag_raw,READ & RawR,unsigned char* Original_Text,char* Cigar,Hit_Info & H,char* Current_Tag,int StringLength,READ & R,int & Clip_H,int & Clip_T);
void Build_Names(const char* Genome_Name,FMFILES & F,BATPARAMETERS & BP);
inline void ReplaceCtoT(READ & R);
inline void ReplaceGtoA(READ & R);
int Get_ED(std::string & S);
int Get_ED(const char* cig);
int Get_Match(const char* cig);
void MEM_STRUC(MEMX & MF, MEMX & MC,MEMX & MCLH,MEMX & MFLH, MEMX & MFLT,MEMX & MCLT,MEMX & MFH,MEMX & MCH,MEMX & MFT,MEMX & MCT,MEMLOOK & MLook,LEN & L);
void Mapping(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,READ & RawR,bool batmeth1,RQINDEX & RQHALF,RQINDEX & RQ,unsigned char* Original_Text,unsigned Entries,char source,int Read_Length,BWT *fwfmi,BWT *revfmi,READ & R,BATREAD & B,unsigned & Conversion_Factor,MEMX & MF,MEMX & MC,MEMX & MFLH,MEMX & MCLH,MEMX & MFLT,MEMX & MCLT,MEMX & MFH,MEMX & MCH,MEMX & MFT,MEMX & MCT,LEN & L,LEN & L_Main,LEN & L_Half,LEN & L_Third,unsigned & Actual_Tag,FILE* Single_File,FILE* Mishit_File,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,Pair* & Pairs,bool PRINT,int SEG_SIZE,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments_Mid,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments_Mid,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments_End,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments_End);
bool Do_Mismatch_Scan(READ& RawR,bool batmeth1,unsigned char* Original_Text,MEMX & MF,MEMX & MC,LEN & L,BWT* fwfmi,BWT* revfmi,int Start_Mis,int End_Mis,int & Last_Mis,int & Head_Top_Count,Hit_Info & H,int & Quality_Score,READ & R,BATREAD & B,FILE* Mishit_File,FILE* Single_File,unsigned Conversion_Factor,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments);
//void Extend_Left(int Plus_Hits,int Minus_Hits,BWT* revfmi,MEMX & MFL,MEMX & MCL,char* Temp_Current_Tag,int StringLength,int & Err, READ & R,int Mis_In_Anchor,FILE* Single_File,int Current_Score,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments );
//void Extend_Right(int Plus_Hits,int Minus_Hits,BWT* revfmi,MEMX & MFL,MEMX & MCL,char* Temp_Current_Tag,int StringLength,int & Err, READ & R,int Mis_In_Anchor,FILE* Single_File,int Current_Score,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments );
void Extend_Right(char* Temp_Current_Tag_raw,READ& RawR,RQINDEX & RQHALF,unsigned char* Original_Text,int Plus_Hits,int Minus_Hits,BWT* revfmi,MEMX & MFL,MEMX & MCL,char* Temp_Current_Tag,int StringLength,int & Err, READ & R,int Mis_In_Anchor,FILE* Single_File,int Current_Score,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,int & Tot_SW_Scans,int & Filter );
void Extend_Left(char* Temp_Current_Tag_raw,READ& RawR,RQINDEX & RQHALF,unsigned char* Original_Text,int Plus_Hits,int Minus_Hits,BWT* revfmi,MEMX & MFL,MEMX & MCL,char* Temp_Current_Tag,int StringLength,int & Err, READ & R,int Mis_In_Anchor,FILE* Single_File,int Current_Score,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,int & Tot_SW_Scans,int & Filter );
int Do_Indel(READ& RawR,bool batmeth1,RQINDEX & RQHALF,RQINDEX & RQ,unsigned char* Original_Text,unsigned Entries,BWT *fwfmi,BWT *revfmi,MEMX & MFLH,MEMX & MCLH,MEMX & MFLT,MEMX & MCLT,MEMX & MFH,MEMX & MCH,MEMX & MFT,MEMX & MCT,int StringLength,Pair* & Pairs,FILE* & Single_File,READ & R,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,const Hit_Info & H);
void Show_Progress(unsigned Percentage);
void Read_INI(char* Config_File,unsigned & MAXCOUNT,FMFILES & F,BATPARAMETERS & BP);
void Get_Bases (unsigned char* Original_Text,unsigned Location,int StringLength,char* Org_String);
void Pair_Reads(BWT* fwfmi,BWT* revfmi,RQINDEX & R,FILE* Data_File, In_File IN,unsigned MAXCOUNT,BATPARAMETERS BP,Pair* Pairs,unsigned Entries);
void Get_Best_Alignment_Pair(READ & RawR,unsigned char* Original_Text,FILE *Single_File,char source,Alignment & A,Alignment & B,READ & R,const int StringLength,BATREAD & Read,Hit_Info & H,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,bool Force_Indel,int & Clip_H,int & Clip_T,char* CIG,bool PRINT);
void Parse_Command_line(int argc, char* argv[],unsigned & MAXCOUNT,char* & GENOME,FMFILES & FMFiles,BATPARAMETERS & BP);
int Head_Tail(BWT* revfmi,SARange* Head_Hits,SARange* Tail_Hits,int Insert,int MAXCOUNT,char & In_Large,RQINDEX & R,unsigned Entries,Pair* Pairs,int & Pairs_Index,int & HITS,int & Err,unsigned Conversion_Factor);
void *Map_And_Pair_Solexa(void *T);
void Verbose(char *BWTFILE,char *OCCFILE,char *REVBWTINDEX,char *REVOCCFILE,char *PATTERNFILE,char *HITSFILE,char* LOCATIONFILE,char MAX_MISMATCHES,int Patternfile_Count,char* PATTERNFILE1,char FILETYPE,LEN & L,char FORCESOLID);
void Init(int & Pcount,BWT* revfmi,In_File & IN,FMFILES F,RQINDEX R,BATPARAMETERS & BP,char Solid,char Npolicy,LEN & L);
void  Paired_Extension(char* Fwd_Read_raw,char *Revcomp_Read_raw,READ& RawR,unsigned char* Original_Text,unsigned Entries,BWT *revfmi,int Last_MisT,int Last_MisH,char* Fwd_Read,char *Revcomp_Read, RQINDEX & RQ,Pair* & Pairs,SARange* & MFH_Hit_Array,SARange* & MFT_Hit_Array,SARange* & MCH_Hit_Array,SARange* & MCT_Hit_Array,int StringLength,FILE* Single_File,READ & R,int & Err,unsigned Conversion_Factor,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,int Current_Score,int & Tot_SW_Scans);
void Launch_Threads(int NTHREAD, void* (*Map_t)(void*),Thread_Arg T);
bool Report_SW_Hits(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,READ & RawR,unsigned char* Original_Text,char source,const int Err,READ & R,FILE* Single_File,const int StringLength,BATREAD & Read,Hit_Info & Mismatch_Hit,int Quality_Score,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,bool Force_Indel,bool PRINT);
bool Get_Info(BWT *revfmi,MEMX & MF,MEMX & MC,int STRINGLENGTH,Hit_Info & H,unsigned Conversion_Factor);
bool Unique_Hits(int Plus_Hits,int Minus_Hits,SARange & P,SARange & M);
bool Check_Subopt(int & Plus_Hits,int & Minus_Hits,int Top_Mis,int Subopt_Mis,READ & R, int StringLength,MEMX & MF,MEMX & MC,float & Top_Score,float & Top_BQScore,float & Sub_Score,float & Sub_BQScore, int & Quality_Score);
Alignment Realign(READ& RawR,unsigned char* Original_Text,Hit_Info &  H,BATREAD & Read,int StringLength,const READ & R,bool Dont_Push_To_Q,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments);
Alignment Realign(READ& RawR,unsigned char* Original_Text,Hit_Info &  H,BATREAD & Read,int StringLength,const READ & R,bool Dont_Push_To_Q,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments);
Alignment RealignX(READ& RawR,unsigned char* Original_Text,Hit_Info &  H,BATREAD & Read,int StringLength, READ & R,bool Dont_Push_To_Q,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,char* Cigar,int & Clip_H,int & Clip_T,int & Filter=Dummy_Int,bool Do_Filter=false);
bool Remove_Duplicates_And_Filter_Top_Hits(Hit_Info *Hit_List,int & Hit_Count,int StringLength);
float Calc_Top_Score(MEMX & MF,MEMX & MC,float & Top_BQ,int Top_Mis,int StringLength,int Plus_Hits,int Minus_Hits,READ & R);
bool Recover_With_SW(READ& RawR,unsigned char* Original_Text,BWT *revfmi,int Plus_Hits,int Minus_Hits,READ & R,BATREAD & BR, int StringLength,MEMX & MF,MEMX & MC,int & Quality_Score,unsigned Conversion_Factor,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,Hit_Info & H);
bool Extend_With_SW(READ& RawR,unsigned char* Original_Text,BWT *revfmi,int Plus_Hits,int Minus_Hits,READ & R,BATREAD & BR, int StringLength,MEMX & MF,MEMX & MC,int & Quality_Score,unsigned Conversion_Factor,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,Hit_Info & H);
void Map_One_Seg(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,READ & RawR,bool batmeth1,RQINDEX & RQHALF,RQINDEX & RQ,unsigned char* Original_Text,unsigned Entries,char source,BWT* fwfmi,BWT* revfmi,READ & R,BATREAD & B,unsigned & Conversion_Factor,MEMX & MF,MEMX & MC,MEMX & MFLH,MEMX & MCLH,MEMX & MFLT,MEMX & MCLT,MEMX & MFH,MEMX & MCH,MEMX & MFT,MEMX & MCT,LEN & L,LEN & L_Main,LEN & L_Half,LEN & L_Third,unsigned & Actual_Tag,FILE* Single_File,FILE* Mishit_File,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,Pair* & Pairs,bool PRINT,Hit_Info & H,int & Quality_Score,int Segment_Length,int SEG_SIZE);
unsigned SA2Loc(BWT *revfmi,SARange S,int Pos,unsigned Conversion_Factor);
void Mode_Parameters(BATPARAMETERS BP);
void Set_Force_Indel(bool & Force_Indel,int Last_Mis,Hit_Info & H,int Avg_Q);
int Calculate_Average_Quality(READ & R);
void Set_Affinity();
Alignment RealignFast(READ & RawR,unsigned char* Original_Text,Hit_Info &  H,BATREAD & Read,int StringLength, READ & R,int OFF,int Filter,bool Do_Filter);
Alignment RealignFastMinus(READ & RawR,unsigned char* Original_Text,Hit_Info &  H,BATREAD & Read,int StringLength, READ & R,int OFF,int Filter,bool Do_Filter);
void FreeQ(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment>  & t );
void FreeQ_Final(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment>  & t );
bool Report_Single(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,READ & RawR,unsigned char* Original_Text,char source,READ & R,FILE* Single_File,const int StringLength,BATREAD & Read,bool & Print_Status,int Clip_H,int Clip_T,Alignment & A);
void Get_Basic_MapQ(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,Alignment & C1, Alignment & C2,int & MapQ);
void Adjust_Alignments(READ & RawR,unsigned char* Original_Text,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,int Offset,READ & RTemp, BATREAD & BTemp);
void Pop_And_Realign(READ & RawR,unsigned char* Original_Text,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & A,int Offset,READ & RTemp, BATREAD & BTemp);
bool SW_List(READ& RawR,unsigned char* Original_Text,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,int Offset,READ & RTemp, BATREAD & BTemp,bool & List_Exceeded,int & List_Size);
//}-----------------------------  FUNCTION PRTOTYPES  -------------------------------------------------/*

#undef DEBUG
//bool TESTMODE=false;
const bool EXACT=false;
const int QUAL_UNMAPPED=20000;
int SW_STRING_BUFFER=1400;
const char* Code_To_CharCAPS="ACGT";
const int DISK_BUFFER_SIZE=5000;
const int MAXALN=30000;
const int MAXCOUNT=200;
const int MIN_SCORE_DIFF=10;
bool PRINT_ALL_HITS=false;
bool PRINT_HEADER=false;
int INDEX_RESOLUTION=30000;
int Inter_MM = 5;
int Max_MM_GAP = 2;
int Max_MM_GAP_Adjust;
int Max_MM = 5;
int MAX_SW_HITS = 100;
int MAX_SW_sav;
bool USE_MULTI_OUT=true;//true;//false;
bool MAIN_HEADER_ONLY=false;
bool STACK_LOWQ=false;
const int DEFAULTFASTAQUAL=35;//40;
LEN L_Main;
In_File IN;
INFILE Head_File;
time_t Start_Time,End_Time;
FILE* Main_Out;
int gap_openP,gap_extensionP;
int MAX_PER_LIST=60;
int SCOREGAP=20;
extern int MAX_MISMATCHES;//moxian
extern bool non_directional;

int Genome_CountX=0;
Offset_Record *Genome_Offsets;
std::map <std::string,int> String_Hash;
Gene_Hash Genome_List[100000];
int Genome_CountCT;
int Genome_CountGA;
char *GENOME;
//{---------------------------- GLOBAL VARIABLES -------------------------------------------------


int main(int argc, char* argv[])
{
	FILE* Data_File;
	FILE* Mishit_File;
	MMPool *mmPool_CT;
	MMPool *mmPool_GA;
	char *CT_GENOME,*GA_GENOME;//,*GENOME;//moxian
	unsigned MAXCOUNT=1;
	BP.PAIRING_MODE=NORMALFILEMODE;BP.NTHREADS=0;BP.FORCELENGTH=0;BP.ROLLOVER=FALSE;BP.SCANBOTH=FALSE;BP.ONEFMINDEX =FALSE; BP.MAXHITS=1;BP.CMD_Buffer=new char [5000];

	gap_openP=40,gap_extensionP=6;
	//Read_INI(NULL,MAXCOUNT,FMFiles,BP);
	Read_INI(NULL,MAXCOUNT,CT,BP);
	Read_INI(NULL,MAXCOUNT,GA,BP);
	match=2;mismatch=2; gap_open=3; gap_extension=1;
	Parse_Command_line(argc,argv,MAXCOUNT,GENOME,GA,BP);//FMFiles
	//std::string tmp=GENOME;
	//tmp+="-GtoA";
	//GENOME=(char*)tmp.c_str();
	//-----------------------------CT GA genome--------------
	std::string CTS=GENOME,GAS=GENOME;
	if (GENOME)
	{
		CTS+="-CtoT"; //-CtoT
		GAS+="-GtoA";// -GtoA 
		CT_GENOME=(char*)CTS.c_str();GA_GENOME=(char*)GAS.c_str();
	}
	//std::string tmp=GENOME;
	//tmp+="-CtoT";
	//GENOME=(char*)tmp.c_str();
	Build_Names(GENOME,FMFiles,BP);
	Build_Names(GA_GENOME,GA,BP);// FMFiles--GA
	Build_Names(CT_GENOME,CT,BP);
	//---------------------------------------------------------------
	//-----------------------------batmeth1
		std::string L=GENOME;L+=".ann.location";
		
		FILE* Location_File=File_Open(L.c_str(),"r");
		
		char Genome[40];
		while (fgets(Genome,39,Location_File)!=0)//count genomes..
		{
			fgets(Genome,39,Location_File);
			Genome_CountX++;	
		}
		rewind(Location_File);
		Genome_Offsets = new Offset_Record[Genome_CountX+1];

		int Genome_Count=0;
		while (fgets(Genome_Offsets[Genome_Count].Genome,39,Location_File)!=0 && Genome_Count<=Genome_CountX)
		{
			Genome_Offsets[Genome_Count].Offset=atoi(Genome_Offsets[Genome_Count].Genome);
			fgets(Genome_Offsets[Genome_Count].Genome,39,Location_File);
			for(int i=0;i<40;i++) 
			{
				if (Genome_Offsets[Genome_Count].Genome[i] == '\n' ||Genome_Offsets[Genome_Count].Genome[i] == '\r')
				{ 
					Genome_Offsets[Genome_Count].Genome[i]=0;
					break;
				} 
			}
			String_Hash[Genome_Offsets[Genome_Count].Genome]=Genome_Count;
			Genome_Offsets[Genome_Count].index=Genome_Count;
			Genome_Count++;	
		}
		Genome_Count--;
		
		/*
		std::string G=GENOME;G+=".bin";
		FILE* BINFILE=File_Open(G.c_str(),"r");
		
		fseek(BINFILE, 0L, SEEK_END);off64_t Genome_Size=ftello64(BINFILE);rewind(BINFILE);//load original genome..
		char* Org_Genome=new char[Genome_Size];if(!Org_Genome) throw("Insufficient memory to load genome..\n"); 
		if(!fread(Org_Genome,Genome_Size,1,BINFILE)) throw ("Error reading file..\n");
		char* Marked_Genome=new char[Genome_Size+1];if(!Marked_Genome) throw("Insufficient memory to Mark genome..\n"); 
		
		char* Split_Point=Org_Genome;//split and write...
		
		//Gene_Hash Genome_List[Genome_Count];
		for ( int i=0;i<Genome_Count;i++)//Stores the location in value corresponding to has..
		{
			String_Hash[Genome_Offsets[i].Genome]=i;
			Genome_List[i].Genome=(Split_Point+=Genome_Offsets[i].Offset);
			Genome_List[i].Index=i;
		}
		//-----------------final--------------------------
		*/
	
	//---------------------------------------------------------------
	//init_SSW();
	Build_Pow10();
	if(CONFIG_FILE) {Read_INI(CONFIG_FILE,MAXCOUNT,GA,BP);Read_INI(CONFIG_FILE,MAXCOUNT,CT,BP);}
	if (MISC_VERB) fprintf(stderr,"\n[BatMeth2 v2.0]\n");
	if (BP.MAX_MISMATCHES != INT_MAX)
	{
		if (BP.MAX_MISMATCHES <=5) Inter_MM=BP.MAX_MISMATCHES;
		else fprintf(stderr,"Error tolerence too high ...\n");
	}
	
	Match_FA_Score= -10*log10(Pr(DEFAULTFASTAQUAL));
	Mis_FA_Score= -10*log10((1-Pr(DEFAULTFASTAQUAL))/3);
	Head_File.Input_File=File_Open(BP.PATTERNFILE,"r");
	Analyze_File(Head_File,L_Main);
	IN.HEAD_LENGTH=IN.TAIL_LENGTH=IN.STRINGLENGTH=L_Main.STRINGLENGTH;
	Load_Range_Index(RQ_GA,L_Main.STRINGLENGTH/3,GA,Entries_GA);
	Load_Range_Index(RQ_CT,L_Main.STRINGLENGTH/3,CT,Entries_CT);
	if(FASTDECODE) {Load_Range_Index(RQHALF_GA,L_Main.STRINGLENGTH/2,GA,Entries_Half_GA);Load_Range_Index(RQHALF_CT,L_Main.STRINGLENGTH/2,CT,Entries_Half_CT);}
	
	int Pcount=0;
	Init(Pcount,revfmiGA,IN,GA,RQ_GA,BP,Head_File.SOLID,0,L_Main);
	Init(Pcount,revfmiCT,IN,CT,RQ_CT,BP,Head_File.SOLID,0,L_Main); 
	FILE* Original_File_GA=File_Open(GA.BINFILE,"rb");
	Original_Text_GA=(unsigned char*) malloc(Get_File_Size(Original_File_GA));
	if(!fread(Original_Text_GA,Get_File_Size(Original_File_GA),1,Original_File_GA))fprintf (stderr,"Init(): Error loading genome...\n");
	File_size=sizeof(unsigned)*Get_File_Size(Original_File_GA);
	
	FILE* Original_File_CT=File_Open(CT.BINFILE,"rb");
	Original_Text_CT=(unsigned char*) malloc(Get_File_Size(Original_File_CT));
	if(!fread(Original_Text_CT,Get_File_Size(Original_File_CT),1,Original_File_CT))fprintf (stderr,"Init(): Error loading genome...\n");
	
	FILE* Original_File=File_Open(FMFiles.BINFILE,"rb");
	Original_Text_Ori=(unsigned char*) malloc(Get_File_Size(Original_File));
	if(!fread(Original_Text_Ori,Get_File_Size(Original_File),1,Original_File))fprintf (stderr,"Init(): Error loading genome...\n");

	//Original_Text_Ori=Original_Text_GA;

	//-----------------------------------------------------------------------------------------------------------------------------------
	Mode_Parameters(BP);

	unsigned Location_Array[80];
	
	fprintf(stderr,"==============================================================\n");
	printf("Genome : %s\n",GENOME);
	printf("Loading C->T Index..");Load_FM_and_Sample(fwfmiGA,revfmiGA,mmPool_GA,GA);printf(" [Done]\n");
	printf("Loading G->A Index..");Load_FM_and_Sample(fwfmiCT,revfmiCT,mmPool_CT,CT);printf(" [Done]\n");
	
	Load_Location(GA.LOCATIONFILE,Annotations,S,E,Location_Array);
	if (NFILE) Load_LocationN(GA.NLOCATIONFILE,Annotations,S,E,Location_Array);
	
	// Initialise indexes 
	//
	MLookCT.Lookupsize=MLookGA.Lookupsize=3;//Get_Lookup_Size(BP.MAX_MISMATCHES,L.STRINGLENGTH);
	Build_Tables(fwfmiGA,revfmiGA,MLookGA);
	Build_Tables(fwfmiCT,revfmiCT,MLookCT);
	
	if(!USE_MULTI_OUT)
	{
		Main_Out=File_Open(GA.OUTPUTFILE,"w");
		if(PRINT_HEADER)
		{
		std::map <unsigned, Ann_Info> ::iterator S,E;
		S=Annotations.begin();E=Annotations.end();
		unsigned CSize=0;
		while (S!=E)
		{
			Ann_Info T=S->second;
			if(T.Size > CSize) CSize=T.Size;
			fprintf(Main_Out,"@SQ\tSN:%s\tLN:%d\n",T.Name,T.Size);
			S++;
		}
		if(NFILE && CSize) fprintf(Main_Out,"@SQ\tSN:%s\tLN:%d\n","NOISE",CSize);
		char Current_Dir[1000];
		if (!getcwd(Current_Dir,990))
		{
			sprintf (Current_Dir,"%d",rand());
		}
		fprintf(Main_Out,"@RG\tID:%s\tSM:%s\n",Current_Dir,BP.PATTERNFILE);
		fprintf(Main_Out,"@PG\tID:PEnGuin\tCL:%s",BP.CMD_Buffer);
		}
	}
	// reverse
	Char_To_R['A']='T';Char_To_R['a']='t';
	Char_To_R['C']='G';Char_To_R['c']='g';
	Char_To_R['G']='C';Char_To_R['g']='c';
	Char_To_R['T']='A';Char_To_R['t']='a';
	Char_To_R['N']='N';Char_To_R['n']='n';
//********************************************************************************************************
	Thread_Arg T;
	if (THREAD)
	{
		Launch_Threads(THREAD, Map_And_Pair_Solexa,T);
	}
	else
	{
		Set_Affinity();
		Map_And_Pair_Solexa(NULL);
	}

//********************************************************************************************************

	if (PROGRESSBAR) fprintf(stderr,"\r[++++++++100%%+++++++++]\n");//progress bar....
	if (MISC_VERB)
	{
		fprintf(stderr,"%d / %d Reads / Pairs ...\n",Total_Reads,Missed_Hits);
		//printf("%d Large reads....\n",Large);
		time(&End_Time);fprintf(stderr,"\n Time Taken  - %.0lf Seconds ..\n ",difftime(End_Time,Start_Time));
	}
	if (LOG_SUCCESS_FILE) fprintf(Log_SFile,"DONE\n");
}

//------------------------------- Print /Verify the reads ----------------------------------------------------
void *Map_And_Pair_Solexa(void *T)
{
	Thread_Arg *TA;
	TA=(Thread_Arg*) T;
	int Thread_ID;
	if(T)
	{
		Thread_ID=TA->ThreadID;
	}

	std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> Alignments;
	std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> Alignments_Mid;
	std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> Alignments_End;
	std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> Good_Alignments;
	std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> Good_Alignments_Mid;
	std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> Good_Alignments_End;
	std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> Final_Alignments; //Comp_Alignment

	Pair* Pairs;
	FILE* Output_File;
	FILE* Discordant_File;
	FILE* Single_File;
	FILE* Unmapped_File;
	FILE* Multi_File;
	FILE* Mishit_File;
	MMPool *mmPool_GA;
	LEN L=L_Main,L_Half,L_Third;L.IGNOREHEAD=0;
	LEN L_batmeth=L;
	OUTPUT O;
	HEADER Header;
	//time_t Start_Time,End_Time;
	//time(&Start_Time);
	int MF1_Top_End,MC1_Top_End;
	char* Disc_Hit_Buffer;
#ifndef NDEBUG
	//DISC_HIT_BUFFER_ORG=Disc_Hit_Buffer_Org;
#endif
//------------------batmeth
	OUTPUT O_CT,O_GA,O_RevCT,O_RevGA;
	std::string tmpList1[1000];
	std::string tmpList2[1000];
	std::string tmpList3[1000];
	std::string tmpList4[1000];

	char* Buffer_CT=(char*) malloc(FILEBUFSIZE);
	char* Buffer_RevCT=(char*) malloc(FILEBUFSIZE);
	char* Buffer_GA=(char*) malloc(FILEBUFSIZE);
	char* Buffer_RevGA=(char*) malloc(FILEBUFSIZE);

	O_CT.SAM=0;
	O_CT.PLUSSTRAND=0;
	O_CT.MaxHits=MAXHITS;
	O_GA.MaxHits=MAXHITS;
	O_CT.Offset=0;
	O_CT.Genome_Count=Genome_CountCT;
	O_CT.FILETYPE=FQ;
	O_CT.Length_Array[0]=O_CT.Length_Array[1]=O_CT.Length_Array[2]=L_batmeth.STRINGLENGTH;
	O_RevCT=O_CT;
	O_GA=O_CT;O_RevGA=O_CT;
	O_CT.Buffer=Buffer_CT;//Buffers to write batman output ...
	O_GA.Buffer=Buffer_GA;
	O_RevCT.Buffer=Buffer_RevCT;
	O_RevGA.Buffer=Buffer_RevGA;
	
	//----------------------------------

//{--------------------------- INIT STUF ---------------------------------------

	L_Half=L_Third=L;
	if (BP.FORCELENGTH) 
	{
		Split_Read(BP.FORCELENGTH,L);SEEDSIZE=BP.FORCELENGTH;
	}
	else 
	{
		Split_Read(L.STRINGLENGTH_ORG,L);SEEDSIZE=L.STRINGLENGTH_ORG;
	}
	if (!(Pairs=(Pair*)malloc(sizeof(Pair)*30000))) {if(LOG_SUCCESS_FILE) fprintf(Log_SFile,"Init():malloc error...\n");fprintf(stderr,"Init():malloc error...\n");exit(-1);}
	Split_Read(IN.STRINGLENGTH/2,L_Half);
	Split_Read(IN.STRINGLENGTH/3,L_Third);

// Misc stuff 
	if(THREAD==0 || THREAD==1) {Thread_ID=1;}
	if(Thread_ID==1) {
		//Verbose(CT.BWTFILE,CT.OCCFILE,CT.REVBWTINDEX,CT.REVOCCFILE,BP.PATTERNFILE,CT.OUTPUTFILE,CT.LOCATIONFILE,Inter_MM,BP.Patternfile_Count,BP.PATTERNFILE1,Head_File.FILETYPE,L,BP.FORCESOLID);
		//Verbose(GA.BWTFILE,GA.OCCFILE,GA.REVBWTINDEX,GA.REVOCCFILE,BP.PATTERNFILE,GA.OUTPUTFILE,GA.LOCATIONFILE,Inter_MM,BP.Patternfile_Count,BP.PATTERNFILE1,Head_File.FILETYPE,L,BP.FORCESOLID);
		FileInfo(BP.PATTERNFILE,GA.OUTPUTFILE,Inter_MM,BP.Patternfile_Count,BP.PATTERNFILE1,Head_File.FILETYPE,L,BP.FORCESOLID);
	}
	if(!USE_MULTI_OUT)
	{
		Output_File=Main_Out;
	}
	else
	{
	if(THREAD==0 || THREAD==1) {Output_File=File_Open(GA.OUTPUTFILE,"w");Thread_ID=1;}
		else
		{
			std::string Temp=GA.OUTPUTFILE;
			char Temp_Char[20];sprintf(Temp_Char,".%d",Thread_ID);
			Temp+=Temp_Char;
			Output_File=File_Open(Temp.c_str(),"w");
		}
	}

	if(PRINT_MISHIT) Mishit_File=File_Open(BP.MISFILE1,"w");
	if (PRINT_DICTIONARY && USE_MULTI_OUT && PRINT_HEADER)
	{
		std::map <unsigned, Ann_Info> ::iterator S,E;
		S=Annotations.begin();E=Annotations.end();
		unsigned CSize=0;
		while (S!=E)
		{
			Ann_Info T=S->second;
			if(T.Size > CSize) CSize=T.Size;
			fprintf(Output_File,"@SQ\tSN:%s\tLN:%d\n",T.Name,T.Size);
			S++;
		}
		if(NFILE && CSize) fprintf(Output_File,"@SQ\tSN:%s\tLN:%d\n","NOISE",CSize);
		char Current_Dir[1000];
		if (!getcwd(Current_Dir,990))
		{
			sprintf (Current_Dir,"%d",rand());
		}
		fprintf(Output_File,"@RG\tID:%s\tSM:%s\n",Current_Dir,BP.PATTERNFILE);
		fprintf(Output_File,"@PG\tID:PEnGuin\tCL:%s",BP.CMD_Buffer);
	}
	if (WRITE_DISCORDANT) Discordant_File=fopen(GA.DISCORDANTFILE,"w");else Discordant_File=Output_File;
	//if (WRITE_SINGLE) Single_File=fopen(FMFiles.SINGLEFILE,"w");else 
	Single_File=Output_File;
	if (WRITE_UNMAPPED) Unmapped_File=fopen(BP.UNMAPPED_FILE,"w");else Unmapped_File=Output_File;
	if (WRITE_MULTI) Multi_File=fopen(BP.MULTI_FILE,"w");else Multi_File=Output_File;
	WRITE_DISCORDANT=WRITE_MULTI=WRITE_UNMAPPED=WRITE_SINGLE=TRUE;


	//FILE* Log_File=File_Open(LOGFILE,"w");GLog_File=Log_File;
	unsigned Mapped=0,Actual_Tag=0,Large=0,Total_Reads=0;
	int LOOKUPSIZE=BP.MAX_MISMATCHES;
	MAX_MISMATCHES=BP.MAX_MISMATCHES;
	char ONEFMINDEX=BP.ONEFMINDEX;
	READ R;BATREAD B;
	READ R_CT,R_GA;//+ strand reads..
	READ R_Rev,R_Rev_CT,R_Rev_GA;
	GUESS G;GUESS G1;
	READ RawRead,revRawR,revR;
	//--batmeth1
	BATREAD B_batmeth;
	READ R_CT_bat,R_GA_bat,R_Rev_CT_bat,R_Rev_GA_bat;
	//--final
	//------GA--------
	MEMX MF_GA,MC_GA,MFLH_GA,MCLH_GA,MFLT_GA,MCLT_GA;//MEMX MF1,MC1;
	MEMX MFH_GA,MCH_GA,MFT_GA,MCT_GA;//One third pref/suf..
	//----CT----
	MEMX MF_CT,MC_CT,MFLH_CT,MCLH_CT,MFLT_CT,MCLT_CT;//moxian
	MEMX MFH_CT,MCH_CT,MFT_CT,MCT_CT;
// calculate read portions 
	LOOKUPSIZE=3;//Get_Lookup_Size(MAX_MISMATCHES,L.STRINGLENGTH);
	B.IGNOREHEAD=L.IGNOREHEAD;B.StringLength=L.STRINGLENGTH;
	//--------------------------------------GA----------------------------------
	MF_GA.Lookupsize=LOOKUPSIZE;MC_GA.Lookupsize=LOOKUPSIZE;MFLH_GA.Lookupsize=LOOKUPSIZE;MCLH_GA.Lookupsize=LOOKUPSIZE;
	MFLT_GA.Lookupsize=LOOKUPSIZE;MCLT_GA.Lookupsize=LOOKUPSIZE;
	MFH_GA.Lookupsize=LOOKUPSIZE;MCH_GA.Lookupsize=LOOKUPSIZE;MFT_GA.Lookupsize=LOOKUPSIZE;MCT_GA.Lookupsize=LOOKUPSIZE;
	MF_GA.L=MC_GA.L=MFLH_GA.L=MCLH_GA.L=MFLT_GA.L=MCLT_GA.L=MFH_GA.L=MCH_GA.L=MFT_GA.L=MCT_GA.L=L;
	//--------------------------------------CT-------------------------------------------
	MF_CT.Lookupsize=LOOKUPSIZE;MC_CT.Lookupsize=LOOKUPSIZE;MFLH_CT.Lookupsize=LOOKUPSIZE;MCLH_CT.Lookupsize=LOOKUPSIZE;
	MFLT_CT.Lookupsize=LOOKUPSIZE;MCLT_CT.Lookupsize=LOOKUPSIZE;
	MFH_CT.Lookupsize=LOOKUPSIZE;MCH_CT.Lookupsize=LOOKUPSIZE;MFT_CT.Lookupsize=LOOKUPSIZE;MCT_CT.Lookupsize=LOOKUPSIZE;
	MF_CT.L=MC_CT.L=MFLH_CT.L=MCLH_CT.L=MFLT_CT.L=MCLT_CT.L=MFH_CT.L=MCH_CT.L=MFT_CT.L=MCT_CT.L=L;

// initialise memory structures 
      //-----------------------------GA--------------------------
	Copy_MEM(MLookGA,MF_GA,MC_GA,MAX_MISMATCHES);
	Copy_MEM(MLookGA,MFLH_GA,MCLH_GA,MAX_MISMATCHES);
	Copy_MEM(MLookGA,MFLT_GA,MCLT_GA,MAX_MISMATCHES);
	Copy_MEM(MLookGA,MFH_GA,MCH_GA,MAX_MISMATCHES);
	Copy_MEM(MLookGA,MFT_GA,MCT_GA,MAX_MISMATCHES);
	//-------------------------CT----------------------------
	Copy_MEM(MLookCT,MF_CT,MC_CT,MAX_MISMATCHES);
	Copy_MEM(MLookCT,MFLH_CT,MCLH_CT,MAX_MISMATCHES);
	Copy_MEM(MLookCT,MFLT_CT,MCLT_CT,MAX_MISMATCHES);
	Copy_MEM(MLookCT,MFH_CT,MCH_CT,MAX_MISMATCHES);
	Copy_MEM(MLookCT,MFT_CT,MCT_CT,MAX_MISMATCHES);
	
//------------------batmeth1
	B_batmeth.IGNOREHEAD=L_batmeth.IGNOREHEAD;B_batmeth.StringLength=L_batmeth.STRINGLENGTH;
	Split_Read(L_batmeth.STRINGLENGTH_ORG,L_batmeth);
	MEMX MF_CT_bat,MC_CT_bat;
	MEMX MF_GA_bat,MC_GA_bat;
	MF_CT_bat.L=MC_CT_bat.L=MF_GA_bat.L=MC_GA_bat.L=L_batmeth;
	MF_CT_bat.Lookupsize=LOOKUPSIZE;MC_CT_bat.Lookupsize=LOOKUPSIZE;
	MF_GA_bat.Lookupsize=LOOKUPSIZE;MC_GA_bat.Lookupsize=LOOKUPSIZE;
	
	Copy_MEM(MLookCT,MF_CT_bat,MC_CT_bat,MAX_MISMATCHES);
	Copy_MEM(MLookGA,MF_GA_bat,MC_GA_bat,MAX_MISMATCHES);
//-------------------------
	if(Thread_ID==1) time(&Start_Time);
	int Progress=0;unsigned Number_of_Tags=1000;
	if (PROGRESSBAR) Init_Progress();
	int NCount_For_SW=L.STRINGLENGTH/4;
	unsigned Conversion_Factor;

	bwase_initialize(); 
	INPUT_FILE_TYPE=Head_File.FILETYPE;
	MAX_SW_sav=MAX_SW;
	int Read_Length=Head_File.TAG_COPY_LEN;
//}--------------------------- INIT STUF ---------------------------------------
	int preReadLen=0;
	while (Read_Tag(Head_File.Input_File,Head_File.FILETYPE,R))
	{
		R.Real_Len=0;
		for(;R.Tag_Copy[R.Real_Len]!=0 && R.Tag_Copy[R.Real_Len]!='\n';R.Real_Len++);
		if(R.Tag_Copy[R.Real_Len] == '\n') R.Tag_Copy[R.Real_Len]='\0';
		if(R.Quality[R.Real_Len] == '\n') R.Quality[R.Real_Len]='\0';
		//-------------batmeth1------------------------------------------
		if(R.Real_Len!=L_batmeth.STRINGLENGTH)
		{
			GET_LEN(L_batmeth,R);
			Split_Read(L_batmeth.STRINGLENGTH_ORG,L_batmeth);//calculate read portions for Batman algo..
			O_CT.Length_Array[0]=O_CT.Length_Array[1]=O_CT.Length_Array[2]=L_batmeth.STRINGLENGTH;
        		O_RevCT=O_CT;
        		O_GA=O_CT;O_RevGA=O_CT;
			B_batmeth.IGNOREHEAD=L_batmeth.IGNOREHEAD;B_batmeth.StringLength=L_batmeth.STRINGLENGTH;
		}
		strcpy(R.Raw_Tag_Copy,R.Tag_Copy);
//Read Head start ..
		//-----------------------output seq header --------------------------------moxian
		//---------------------------//
		int SEG_SIZE=75;
		int Hits_Printed=0;
		Progress++;Total_Reads++;
		Actual_Tag++;
		if (READS_TO_PROCESS && READS_TO_PROCESS <= Total_Reads) break;
		if (Thread_ID==1 && Progress>Number_of_Tags && PROGRESSBAR) 
		{
			off64_t Current_Pos=ftello64(Head_File.Input_File);
			off64_t Average_Length=Current_Pos/Actual_Tag+1;
			Number_of_Tags=(Head_File.File_Size/Average_Length)/20;
			Progress=0;
			Show_Progress(Current_Pos*100/Head_File.File_Size);
		}
		
		if(!Final_Alignments.empty()) FreeQ(Final_Alignments);
		RawRead=revRawR=R_CT=R_Rev_CT=R_Rev_GA=R_GA=R_CT_bat=R_Rev_CT_bat=R_Rev_GA_bat=R_GA_bat=R;
		
		int i;int Lens=R.Real_Len;
		for (i=0;i<=Lens-1;i++){revRawR.Raw_Tag_Copy[Lens-1-i] = revRawR.Tag_Copy[Lens-1-i]=R_Rev_GA.Tag_Copy[Lens-1-i]=R_Rev_CT.Tag_Copy[Lens-1-i]=R_Rev_GA_bat.Tag_Copy[Lens-1-i]=R_Rev_CT_bat.Tag_Copy[Lens-1-i]=Char_To_CharC[R.Tag_Copy[i]];}; //Reverse complement reads 
		Reverse_Quality(R_Rev_GA.Quality,R,R.Real_Len);Reverse_Quality(R_Rev_CT.Quality,R,R.Real_Len);
		Reverse_Quality(R_Rev_GA_bat.Quality,R,R.Real_Len);Reverse_Quality(R_Rev_CT_bat.Quality,R,R.Real_Len);
		Reverse_Quality(revRawR.Quality,R,R.Real_Len);
		
		//-----------
		
		char Banner[2000];bool Header_Printed=false;
		sprintf(Banner,"%s\t%s\t%s\t%d:%d:%d:%d:%d:%d:%d\n",R.Description,R.Tag_Copy,R.Quality,MF_GA.Stats[0],MF_GA.Stats[1],MF_GA.Stats[2],MF_GA.Stats[3],MF_GA.Stats[4],MF_GA.Stats[5],MF_GA.Stats[6]);//moxian
		if (!Header_Printed) 
		{
			fprintf(Output_File,"@\n%s",Banner);
			Header_Printed = true;
		}
		
	//-------------batmeth1------------------------------------------
		
		int Hits;
		bool Multipe=false;
		
		int minTrueMis=100;
		int maxTrueMis=0;
		int SecondMisN=0;
		
		bool batmeth1=true;int Last_Mis=-1;
//------------- C->T scan Start ----------------------------------------------
		std::string readString;
		if(!Only_non_directional)
		{
			Hits=0;
			ReplaceCtoT(R_CT_bat);
			Process_Read_bat(R_CT_bat,B_batmeth,MF_CT_bat,MC_CT_bat);
			if(Hits=Map_Strand(Last_Mis,batmeth1,MAX_MISMATCHES,maxhits,L_batmeth,fwfmiCT,revfmiCT,MF_CT_bat)){Mapped++;}
			readString=R.Tag_Copy;
			Print_Hits(RawRead,Last_Mis,batmeth1,Final_Alignments,SecondMisN,Multipe,minTrueMis,maxTrueMis,'1',Genome_Offsets,readString,MF_CT_bat,L_batmeth,Output_File,FALSE,revfmiCT,fwfmiCT,O_CT);
			
			//------------- G->A reversed scan Start ----------------------------------------------
			Hits=0;
			ReplaceGtoA(R_Rev_GA_bat);
			Process_Read_bat(R_Rev_GA_bat,B_batmeth,MF_GA_bat,MC_GA_bat);
			Last_Mis=-1;
			if(Hits=Map_Strand(Last_Mis,batmeth1,MAX_MISMATCHES,maxhits,L_batmeth,fwfmiGA,revfmiGA,MF_GA_bat)){Mapped++;}
			readString=revRawR.Tag_Copy;
			Print_Hits(revRawR,Last_Mis,batmeth1,Final_Alignments,SecondMisN,Multipe,minTrueMis,maxTrueMis,'4',Genome_Offsets,readString,MF_GA_bat,L_batmeth,Output_File,FALSE,revfmiGA,fwfmiGA,O_RevGA);
		}
//------------- G->A scan Start ----------------------------------------------
		if(non_directional) 
		{
			Hits=0;
			ReplaceGtoA(R_GA_bat);
			Process_Read_bat(R_GA_bat,B_batmeth,MF_GA_bat,MC_GA);
			Last_Mis=-1;
			if(Hits=Map_Strand(Last_Mis,batmeth1,MAX_MISMATCHES,maxhits,L_batmeth,fwfmiGA,revfmiGA,MF_GA_bat)){Mapped++;}
			readString=R.Tag_Copy;
			Print_Hits(RawRead,Last_Mis,batmeth1,Final_Alignments,SecondMisN,Multipe,minTrueMis,maxTrueMis,'2',Genome_Offsets,readString,MF_GA_bat,L_batmeth,Output_File,FALSE,revfmiGA,fwfmiGA,O_GA);
		
//------------- C->T reversed scan Start ----------------------------------------------
			Hits=0;
			ReplaceCtoT(R_Rev_CT_bat);
			Process_Read_bat(R_Rev_CT_bat,B_batmeth,MF_CT_bat,MC_CT);
			Last_Mis=-1;
			if(Hits=Map_Strand(Last_Mis,batmeth1,MAX_MISMATCHES,maxhits,L_batmeth,fwfmiCT,revfmiCT,MF_CT_bat)){Mapped++;}
			readString=R_Rev.Tag_Copy;
			Print_Hits(revRawR,Last_Mis,batmeth1,Final_Alignments,SecondMisN,Multipe,minTrueMis,maxTrueMis,'3',Genome_Offsets,readString,MF_CT_bat,L_batmeth,Output_File,FALSE,revfmiCT,fwfmiCT,O_RevCT);

		}
		
	//------------- G->A reversed scan Finish ----------------------------------------------
//------------------------batmeth2---------------------------------------
		batmeth1=false;
		int minMis=100;
		int maxQuality=0;bool batmeth2=false;
		if( (MAX_MISMATCHES>=indelMis && Multipe && minTrueMis >= indelMis || minTrueMis >= indelMis  || Final_Alignments.empty()) && FindInDels && R.Real_Len >=150 )
		{
			batmeth2=true;
			if(!Only_non_directional)
			{
				//----------------------------CtoT--------------------------------------
				RawRead.state='1';//c2t
				ReplaceCtoT(R_CT);//CT--CTgenome
				//READ R_CT2=R_CT;
				Mapping(Final_Alignments,RawRead,batmeth1,RQHALF_CT,RQ_CT,Original_Text_CT,Entries_CT,'1',Read_Length,fwfmiCT,revfmiCT,R_CT,B,Conversion_Factor,MF_CT,MC_CT,MFLH_CT,MCLH_CT,MFLT_CT,MCLT_CT,MFH_CT,MCH_CT,MFT_CT,MCT_CT,L,L_Main,L_Half,L_Third,Actual_Tag,Single_File,Mishit_File,Alignments,Good_Alignments,Pairs,false,SEG_SIZE,Alignments_Mid,Good_Alignments_Mid,Alignments_End,Good_Alignments_End);
				//----------------------G->A reversed----------- --------------------
				revRawR.state='2';//g2a
				ReplaceGtoA(R_Rev_GA);//CT--GAgenome
				//READ R_Rev_GA2=R_Rev_GA;
				Mapping(Final_Alignments,revRawR,batmeth1,RQHALF_GA,RQ_GA,Original_Text_GA,Entries_GA,'4',Read_Length,fwfmiGA,revfmiGA,R_Rev_GA,B,Conversion_Factor,MF_GA,MC_GA,MFLH_GA,MCLH_GA,MFLT_GA,MCLT_GA,MFH_GA,MCH_GA,MFT_GA,MCT_GA,L,L_Main,L_Half,L_Third,Actual_Tag,Single_File,Mishit_File,Alignments,Good_Alignments,Pairs,false,SEG_SIZE,Alignments_Mid,Good_Alignments_Mid,Alignments_End,Good_Alignments_End);
				
				/**/
				//if(0 && ( Final_Alignments.empty() || Final_Alignments.size() <10 ))
		                {    
					/*revRawR.state='2';//1
					Mapping(Final_Alignments,revRawR,batmeth1,RQHALF_CT,RQ_CT,Original_Text_CT,Entries_CT,'1',Read_Length,fwfmiCT,revfmiCT,R_Rev_GA2,B,Conversion_Factor,MF_CT,MC_CT,MFLH_CT,MCLH_CT,MFLT_CT,MCLT_CT,MFH_CT,MCH_CT,MFT_CT,MCT_CT,L,L_Main,L_Half,L_Third,Actual_Tag,Single_File,Mishit_File,Alignments,Good_Alignments,Pairs,false,SEG_SIZE,Alignments_Mid,Good_Alignments_Mid,Alignments_End,Good_Alignments_End);
					*///if( Final_Alignments.empty() || (Final_Alignments.size() <2 && maxQuality <= 10) )
					{    //printf("\n======= ------- ========\n");
						//RawRead.state='1';//2
						//Mapping(Final_Alignments,RawRead,batmeth1,RQHALF_GA,RQ_GA,Original_Text_GA,Entries_GA,'4',Read_Length,fwfmiGA,revfmiGA,R_CT2,B,Conversion_Factor,MF_GA,MC_GA,MFLH_GA,MCLH_GA,MFLT_GA,MCLT_GA,MFH_GA,MCH_GA,MFT_GA,MCT_GA,L,L_Main,L_Half,L_Third,Actual_Tag,Single_File,Mishit_File,Alignments,Good_Alignments,Pairs,false,SEG_SIZE,Alignments_Mid,Good_Alignments_Mid,Alignments_End,Good_Alignments_End);
					}    
			        }
			
			}
			bool allstrand=false;//true;
			if( (allstrand && Final_Alignments.size()==0 ) || non_directional)
			{
				//----------------------C->T reversed---------------------------------
				revRawR.state='1';//c2t
				ReplaceCtoT(R_Rev_CT);//GA--CTgenome
				Mapping(Final_Alignments,revRawR,batmeth1,RQHALF_CT,RQ_CT,Original_Text_CT,Entries_CT,'3',Read_Length,fwfmiCT,revfmiCT,R_Rev_CT,B,Conversion_Factor,MF_CT,MC_CT,MFLH_CT,MCLH_CT,MFLT_CT,MCLT_CT,MFH_CT,MCH_CT,MFT_CT,MCT_CT,L,L_Main,L_Half,L_Third,Actual_Tag,Single_File,Mishit_File,Alignments,Good_Alignments,Pairs,false,SEG_SIZE,Alignments_Mid,Good_Alignments_Mid,Alignments_End,Good_Alignments_End);
				//----------------------------GtoA--------------------------------------
				RawRead.state='2';//g2a
				ReplaceGtoA(R_GA);//GA--GAgenome
				Mapping(Final_Alignments,RawRead,batmeth1,RQHALF_GA,RQ_GA,Original_Text_GA,Entries_GA,'2',Read_Length,fwfmiGA,revfmiGA,R_GA,B,Conversion_Factor,MF_GA,MC_GA,MFLH_GA,MCLH_GA,MFLT_GA,MCLT_GA,MFH_GA,MCH_GA,MFT_GA,MCT_GA,L,L_Main,L_Half,L_Third,Actual_Tag,Single_File,Mishit_File,Alignments,Good_Alignments,Pairs,false,SEG_SIZE,Alignments_Mid,Good_Alignments_Mid,Alignments_End,Good_Alignments_End);
			}
		}
		//---------------------finished print result------------------------------
		if(MAX_MISMATCHES<3) SecondNcutoff=350;
		if( (SecondMisN < SecondNcutoff /*|| MAX_MISMATCHES >= indelMis*/ ) )//&& List_Filter
		{
			Alignment A;
			while(!Final_Alignments.empty())
			{
				A=Final_Alignments.top();Final_Alignments.pop();	
				//if(A.Mismatch <= minMis+1 || A.QScore >= maxQuality )
				if(A.Mismatch <= minMis + Lens*0.1 )
				//if( (batmeth2 || (A.Mismatch <= (minMis+2)) ) && Get_ED(A.Cigar) < 2*((double)Lens/100+0.5) && (double)Get_Match(A.Cigar)/Lens > 0.2 )
					fprintf(Output_File,"%c\t%s\t%c\t%u\t%d\t%d\t%s\t%d\n",A.source,A.Chr,A.Sign,A.Loc,A.Mismatch,R.Real_Len,A.Cigar,A.Top_Penalty);
				if(A.Mismatch < minMis)
					minMis = A.Mismatch;
			}
		}
		
	}
}
void Mapping(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,READ & RawR,bool batmeth1,RQINDEX & RQHALF,RQINDEX & RQ,unsigned char* Original_Text,unsigned Entries,char source,int Read_Length,BWT *fwfmi,BWT *revfmi,READ & R,BATREAD & B,unsigned & Conversion_Factor,MEMX & MF,MEMX & MC,MEMX & MFLH,MEMX & MCLH,MEMX & MFLT,MEMX & MCLT,MEMX & MFH,MEMX & MCH,MEMX & MFT,MEMX & MCT,LEN & L,LEN & L_Main,LEN & L_Half,LEN & L_Third,unsigned & Actual_Tag,FILE* Single_File,FILE* Mishit_File,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,Pair* & Pairs,bool PRINT,int SEG_SIZE,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments_Mid,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments_Mid,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments_End,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments_End)
{
		Alignment A1,A2,B1,B2,C1,C2;
		int MapQ1=1,MapQ2=1,MapQ3=1;

		Hit_Info H1,H2,H3;int Quality_Score1,Quality_Score2,Quality_Score3;
		READ RTemp=R;BATREAD BTemp=B;READ RawRTemp=RawR;
		Map_One_Seg(Final_Alignments,RawRTemp,batmeth1,RQHALF,RQ,Original_Text,Entries,source,fwfmi,revfmi,R,B,Conversion_Factor,MF,MC,MFLH,MCLH,MFLT,MCLT,MFH,MCH,MFT,MCT,L,L_Main,L_Half,L_Third,Actual_Tag,Single_File,Mishit_File,Alignments,Good_Alignments,Pairs,false,H1,Quality_Score1,0,SEG_SIZE);
		Get_Basic_MapQ(Good_Alignments,A1,A2,MapQ1);
		for(int i=0;i<=SEG_SIZE;i++)
		{
			R.Tag_Copy[i]=R.Tag_Copy[i+SEG_SIZE];R.Quality[i]=R.Quality[i+SEG_SIZE];
			RawRTemp.Tag_Copy[i]=RawRTemp.Tag_Copy[i+SEG_SIZE];RawRTemp.Quality[i]=RawRTemp.Quality[i+SEG_SIZE];
		}
		
		Map_One_Seg(Final_Alignments,RawRTemp,batmeth1,RQHALF,RQ,Original_Text,Entries,source,fwfmi,revfmi,R,B,Conversion_Factor,MF,MC,MFLH,MCLH,MFLT,MCLT,MFH,MCH,MFT,MCT,L,L_Main,L_Half,L_Third,Actual_Tag,Single_File,Mishit_File,Alignments_Mid,Good_Alignments_Mid,Pairs,false,H2,Quality_Score2,0,SEG_SIZE);
		Get_Basic_MapQ(Good_Alignments_Mid,B1,B2,MapQ2);

		BTemp.StringLength=Read_Length;
		RTemp.Real_Len=Read_Length;
		Process_Read(RawR,RTemp,BTemp,MF,MC);
		if(MapQ1>0 && MapQ2>0)
		{
			if((A1.Sign=='+')? (A1.Loc <= B1.Loc) :(B1.Loc<=A1.Loc))
			{
				if((A1.Sign=='+')?(B1.Loc-A1.Loc<90):(A1.Loc-B1.Loc<90))
				{
					H1.Status=UNMAPPED;
					Adjust_Alignments(RawR,Original_Text,Alignments,0,RTemp,BTemp);
					Report_SW_Hits(Final_Alignments,RawR,Original_Text,source,0,RTemp,Single_File,Read_Length,BTemp,H1,Quality_Score1,Alignments,Good_Alignments,0,true);//Force_Indel
					return;
				}
			}
		}
		for(int i=0;i<=SEG_SIZE;i++)
		{
			R.Tag_Copy[i]=R.Tag_Copy[i+2*SEG_SIZE];R.Quality[i]=R.Quality[i+2*SEG_SIZE];
		}
		for(int i=0;i<=SEG_SIZE;i++)
		{
			RawRTemp.Tag_Copy[i]=RawRTemp.Tag_Copy[i+2*SEG_SIZE];RawRTemp.Quality[i]=RawRTemp.Quality[i+2*SEG_SIZE];
		}
		
		if(Read_Length>3*SEG_SIZE)//3rd segment..
		{

			Map_One_Seg(Final_Alignments,RawRTemp,batmeth1,RQHALF,RQ,Original_Text,Entries,source,fwfmi,revfmi,R,B,Conversion_Factor,MF,MC,MFLH,MCLH,MFLT,MCLT,MFH,MCH,MFT,MCT,L,L_Main,L_Half,L_Third,Actual_Tag,Single_File,Mishit_File,Alignments_End,Good_Alignments_End,Pairs,false,H3,Quality_Score3,0,SEG_SIZE);
			Get_Basic_MapQ(Good_Alignments_End,C1,C2,MapQ3);

			if(MapQ3>0)
			{
				if(MapQ1>0)
				{
					if((A1.Sign=='+')? (A1.Loc<= C1.Loc) :(C1.Loc<=A1.Loc))
					{
						if((A1.Sign=='+')?(C1.Loc-A1.Loc<160):(A1.Loc-C1.Loc<160))
						{
							H1.Status=UNMAPPED;
							Adjust_Alignments(RawR,Original_Text,Alignments,0,RTemp,BTemp);
							Report_SW_Hits(Final_Alignments,RawR,Original_Text,source,0,RTemp,Single_File,Read_Length,BTemp,H1,Quality_Score1,Alignments,Good_Alignments,0,true);
							return;
						}
					}
				}
				if (MapQ2>0)
				{
					if((B1.Sign=='+')? (B1.Loc<= C1.Loc) :(C1.Loc<=B1.Loc))
					{
						if((B1.Sign=='+')?(C1.Loc-B1.Loc<90):(B1.Loc-C1.Loc<90))
						{
							H1.Status=UNMAPPED;
							Adjust_Alignments(RawR,Original_Text,Alignments_Mid,75,RTemp,BTemp);
							Report_SW_Hits(Final_Alignments,RawR,Original_Text,source,0,RTemp,Single_File,Read_Length,BTemp,H1,Quality_Score1,Alignments_Mid,Good_Alignments_Mid,0,true);
							return;
						}
					}
				}

			}
		}
		
		RECOVER_N=0;
		Hit_Info H;
		Alignment Aln;
		int Filter=0,ClipT,ClipH;
		char CIG2[MAX_SIGLEN];


		if(!DEBUG_SEGS)
		{
			FreeQ(Alignments);

			bool List_Exceeded=false;int Remaining_List_Size=0;
			SW_List(RawR,Original_Text,Good_Alignments,Alignments,0,RTemp,BTemp,List_Exceeded,Remaining_List_Size);
			SW_List(RawR,Original_Text,Good_Alignments_Mid,Alignments,75,RTemp,BTemp,List_Exceeded,Remaining_List_Size);
			if(Read_Length>3*SEG_SIZE)//3rd segment..
				SW_List(RawR,Original_Text,Good_Alignments_End,Alignments,150,RTemp,BTemp,List_Exceeded,Remaining_List_Size);
	
			FreeQ(Good_Alignments);
			H1.Status=UNMAPPED;
			assert(Remaining_List_Size>=0);
			//if(Remaining_List_Size<1000)
			Report_SW_Hits(Final_Alignments,RawR,Original_Text,source,0,RTemp,Single_File,Read_Length,B,H1,Quality_Score1,Alignments,Good_Alignments,0,true);
			return;
		}
		else
		{
			H1.Status=UNMAPPED;
			FreeQ(Alignments);
			bool List_Exceeded=false;int Remaining_List_Size=0;
			SW_List(RawR,Original_Text,Good_Alignments,Alignments,0,RTemp,BTemp,List_Exceeded,Remaining_List_Size);
			FreeQ(Good_Alignments);
			Report_SW_Hits(Final_Alignments,RawR,Original_Text,source,0,RTemp,Single_File,Read_Length,B,H1,Quality_Score1,Alignments,Good_Alignments,0,true);

			FreeQ(Alignments);
			SW_List(RawR,Original_Text,Good_Alignments_Mid,Alignments,75,RTemp,BTemp,List_Exceeded,Remaining_List_Size);
			H1.Status=UNMAPPED;
			Report_SW_Hits(Final_Alignments,RawR,Original_Text,source,0,RTemp,Single_File,Read_Length,B,H1,Quality_Score1,Alignments,Good_Alignments,0,true);
			FreeQ(Good_Alignments);
			if(Read_Length>3*SEG_SIZE)//3rd segment..
			{
				FreeQ(Alignments);
				SW_List(RawR,Original_Text,Good_Alignments_End,Alignments,150,RTemp,BTemp,List_Exceeded,Remaining_List_Size);
				H1.Status=UNMAPPED;
				Report_SW_Hits(Final_Alignments,RawR,Original_Text,source,0,RTemp,Single_File,Read_Length,B,H1,Quality_Score1,Alignments,Good_Alignments,0,true);
				FreeQ(Good_Alignments);
			}
		}
}
int Do_Indel(READ& RawR,bool batmeth1,RQINDEX & RQHALF,RQINDEX & RQ,unsigned char* Original_Text,unsigned Entries,BWT *fwfmi,BWT *revfmi,MEMX & MFLH,MEMX & MCLH,MEMX & MFLT,MEMX & MCLT,MEMX & MFH,MEMX & MCH,MEMX & MFT,MEMX & MCT,int StringLength,Pair* & Pairs,FILE* & Single_File,READ & R,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,const Hit_Info & H)
{
	Ann_Info A;
	int Err=0,Tot_SW_Scans=Alignments.size();
	char Temp_Current_TagX[MAXDES];
	char Temp_Current_TagY[MAXDES];
	char Temp_Current_TagX_raw[MAXDES];
	char Temp_Current_TagY_raw[MAXDES];
	char *Forward_Read=MFLH.Current_Tag,*Revcomp_Read=MCLH.Current_Tag;
	char *Forward_Read_raw=MFLH.Current_Tag_raw,*Revcomp_Read_raw=MCLH.Current_Tag_raw;
	
	MFLH.Least_Mis=MCLH.Least_Mis=INT_MAX;
	MFLH.Extend=false;MCLH.Extend=false;
	MFH.Extend=false;MCH.Extend=false;
	MFLT.Extend=false;MCLT.Extend=false;
	MFT.Extend=false;MCT.Extend=false;
	MEMX Temp_MFL=MFLT,Temp_MCL=MCLT;
	//------------------------Extend Routines Start ----------------------------
	memcpy(Temp_Current_TagX,(MFLH.Current_Tag+StringLength-MFLH.L.STRINGLENGTH),MFLH.L.STRINGLENGTH);//T_C_T=[+RH:+LH]
	memcpy(Temp_Current_TagX+MFLH.L.STRINGLENGTH,MFLH.Current_Tag,StringLength-MFLH.L.STRINGLENGTH);
	MFLH.Current_Tag=Temp_Current_TagX;

	memcpy(Temp_Current_TagY,(Temp_MCL.Current_Tag+StringLength-Temp_MCL.L.STRINGLENGTH),Temp_MCL.L.STRINGLENGTH);//T_C_T=[-RH:-LH]
	memcpy(Temp_Current_TagY+Temp_MCL.L.STRINGLENGTH,Temp_MCL.Current_Tag,StringLength-Temp_MCL.L.STRINGLENGTH);
	Temp_MCL.Current_Tag=Temp_Current_TagY;
	//--------------raw
	memcpy(Temp_Current_TagX_raw,(MFLH.Current_Tag_raw+StringLength-MFLH.L.STRINGLENGTH),MFLH.L.STRINGLENGTH);//T_C_T=[+RH:+LH]
	memcpy(Temp_Current_TagX_raw+MFLH.L.STRINGLENGTH,MFLH.Current_Tag_raw,StringLength-MFLH.L.STRINGLENGTH);
	MFLH.Current_Tag_raw=Temp_Current_TagX_raw;

	memcpy(Temp_Current_TagY_raw,(Temp_MCL.Current_Tag_raw+StringLength-Temp_MCL.L.STRINGLENGTH),Temp_MCL.L.STRINGLENGTH);//T_C_T=[-RH:-LH]
	memcpy(Temp_Current_TagY_raw+Temp_MCL.L.STRINGLENGTH,Temp_MCL.Current_Tag_raw,StringLength-Temp_MCL.L.STRINGLENGTH);
	Temp_MCL.Current_Tag_raw=Temp_Current_TagY_raw;
	
	int Plus_HitsX=0,Minus_HitsX=0;
	int Head_Top_Count,Tail_Top_Count;//# of top hits in H/T
	int Mis_StartX=0,Mis_StartY=0;
	bool Exit_Loop=false;
	int Filter=0;
	int Mis_Penalty=(MODE>=FAST) ? Mis_FA_Score:-mismatch;
	int Match_Bonus=(MODE>=FAST) ? 0:match;
	while(!Exit_Loop)
	{
		if(Mis_StartX!= -1) {Mis_StartX=Scan(batmeth1,MFLH,MCLH,Max_MM_GAP_Adjust,MFLH.L,fwfmi,revfmi,Mis_StartX,Head_Top_Count,INT_MAX);}
		if(Mis_StartX>=0)
		{
			Plus_HitsX=MFLH.Hit_Array_Ptr-1,Minus_HitsX=MCLH.Hit_Array_Ptr-1;
			Extend_Left(Temp_Current_TagX_raw,RawR,RQHALF,Original_Text,Plus_HitsX,Minus_HitsX,revfmi,MFLH,MCLH,Temp_Current_TagX,StringLength,Err,R,Mis_StartX,Single_File,Match_Bonus*(StringLength/2-Mis_StartX)+Mis_StartX*Mis_Penalty,Alignments,Tot_SW_Scans,Filter );
			if(++Mis_StartX >Max_MM_GAP_Adjust) Mis_StartX= -1;
		}
		if(Err) return Err;

		int Plus_HitsY=0,Minus_HitsY=0;
		if(Mis_StartY!= -1){Mis_StartY=Scan(batmeth1,Temp_MFL,Temp_MCL,Max_MM_GAP_Adjust,Temp_MCL.L,fwfmi,revfmi,Mis_StartY,Head_Top_Count,INT_MAX);}
		if(Mis_StartY>=0)
		{
			Plus_HitsY=Temp_MFL.Hit_Array_Ptr-1,Minus_HitsY=Temp_MCL.Hit_Array_Ptr-1;
			Extend_Right(Temp_Current_TagY_raw,RawR,RQHALF,Original_Text,Plus_HitsY,Minus_HitsY,revfmi,Temp_MFL,Temp_MCL,Temp_Current_TagY,StringLength,Err,R,Mis_StartY,Single_File,Match_Bonus*(StringLength/2-Mis_StartY)+Mis_StartY*Mis_Penalty,Alignments,Tot_SW_Scans,Filter);
			if(++Mis_StartY >Max_MM_GAP_Adjust) Mis_StartY= -1;
		}
		if(Err) return Err;

		if(!Alignments.empty()) Exit_Loop=true;
		if(Mis_StartY==-1 && Mis_StartX==-1) Exit_Loop=true; 
		MFLH.Hit_Array_Ptr=MCLH.Hit_Array_Ptr=Temp_MCL.Hit_Array_Ptr=Temp_MFL.Hit_Array_Ptr=0;
	}
	//------------------------Extend Routines End ----------------------------
	SARange *MFH_Top_Start=MFH.Hit_Array,*MCH_Top_Start=MCH.Hit_Array;
	SARange *MFT_Top_Start=MFT.Hit_Array,*MCT_Top_Start=MCT.Hit_Array;
	SARange *MFH_Subopt_Start,*MCH_Subopt_Start;
	SARange *MFT_Subopt_Start,*MCT_Subopt_Start;
	SARange *MFH_Least_Start,*MCH_Least_Start;
	SARange *MFT_Least_Start,*MCT_Least_Start;
	int Plus_Hits=0,Minus_Hits=0;
	unsigned Conversion_Factor=revfmi->textLength-MFH.L.STRINGLENGTH;
	int Max_MM_GAP_Peng=Max_MM_GAP_Adjust;
	if(HEURISTIC) return Err;
	if(EXACT) Max_MM_GAP_Peng=Max_MM_GAP;
	//-----------------Pair Top hits ---------------------------------------
	int Top_MisH=Scan(batmeth1,MFH,MCH,Max_MM_GAP_Peng,MCH.L,fwfmi,revfmi,0,Head_Top_Count,INT_MAX);
	if(Top_MisH>=0)
	{
		MFH_Subopt_Start= &MFH.Hit_Array[MFH.Hit_Array_Ptr];MCH_Subopt_Start= &MCH.Hit_Array[MCH.Hit_Array_Ptr];
	}
	int Top_MisT=Scan(batmeth1,MFT,MCT,Max_MM_GAP_Peng,MCT.L,fwfmi,revfmi,0,Head_Top_Count,INT_MAX);
	if(Top_MisT>=0)
	{
		MFT_Subopt_Start= &MFT.Hit_Array[MFT.Hit_Array_Ptr];MCT_Subopt_Start= &MCT.Hit_Array[MCH.Hit_Array_Ptr];
	}
	int Current_Penalty=2*(MFT.L.STRINGLENGTH)*Match_Bonus-(Top_MisT+Top_MisH)*Mis_Penalty;
	Paired_Extension(Forward_Read_raw,Revcomp_Read_raw,RawR,Original_Text,Entries,revfmi,Top_MisT,Top_MisH,Forward_Read,Revcomp_Read,RQ,Pairs,MFH_Top_Start,MFT_Top_Start,MCH_Top_Start,MCT_Top_Start,StringLength,Single_File,R,Err,Conversion_Factor,Alignments,Current_Penalty,Tot_SW_Scans);
	//-----------------Pair Top hits ---------------------------------------
	if(Alignments.empty() && \
			((Top_MisH==0 && Top_MisT<=Max_MM_GAP_Peng-1)||(Top_MisT==0 && Top_MisH<=Max_MM_GAP_Peng-1)) && \
			((Top_MisT+1<=Max_MM_GAP_Peng) && (Top_MisH+1<=Max_MM_GAP_Peng)) && \
			!SKIPHARD)
	{
		int Sub_MisH=(Top_MisH==-1) ? -1 : Scan(batmeth1,MFH,MCH,Max_MM_GAP_Peng,MCH.L,fwfmi,revfmi,Top_MisH+1,Head_Top_Count,INT_MAX);
		if(Sub_MisH>=0)
		{
			MFH_Least_Start= &MFH.Hit_Array[MFH.Hit_Array_Ptr];MCH_Least_Start= &MCH.Hit_Array[MCH.Hit_Array_Ptr];
		}
		int Sub_MisT=(Top_MisT==-1) ? -1 : Scan(batmeth1,MFT,MCT,Max_MM_GAP_Peng,MCT.L,fwfmi,revfmi,Top_MisT+1,Head_Top_Count,INT_MAX);
		if(Sub_MisT>=0)
		{
			MFT_Least_Start= &MFT.Hit_Array[MFT.Hit_Array_Ptr];MCT_Least_Start= &MCT.Hit_Array[MCH.Hit_Array_Ptr];
		}
	//-----------------Pair Top-Subopt Start ---------------------------------------
		if(Sub_MisT!= -1)
		{
			Current_Penalty=2*(MFT.L.STRINGLENGTH)*Match_Bonus-(Sub_MisT+Top_MisH)*Mis_Penalty;
			Paired_Extension(Forward_Read_raw,Revcomp_Read_raw,RawR,Original_Text,Entries,revfmi,Sub_MisT,Top_MisH,Forward_Read,Revcomp_Read,RQ,Pairs,MFH_Top_Start,MFT_Subopt_Start,MCH_Top_Start,MCT_Subopt_Start,StringLength,Single_File,R,Err,Conversion_Factor,Alignments,Current_Penalty,Tot_SW_Scans);
		}
		if(Sub_MisH!= -1)
		{
			Current_Penalty=2*(MFT.L.STRINGLENGTH)*Match_Bonus-(Top_MisT+Sub_MisH)*Mis_Penalty;
			Paired_Extension(Forward_Read_raw,Revcomp_Read_raw,RawR,Original_Text,Entries,revfmi,Top_MisT,Sub_MisH,Forward_Read,Revcomp_Read,RQ,Pairs,MFH_Subopt_Start,MFT_Top_Start,MCH_Subopt_Start,MCT_Top_Start,StringLength,Single_File,R,Err,Conversion_Factor,Alignments,Current_Penalty,Tot_SW_Scans);
		}
	//-----------------Pair Top-Subopt end ---------------------------------------
	//-----------------Pair Subopt-Subopt Start ---------------------------------------
		if((Sub_MisH!= -1 && Sub_MisT!= -1) && (Sub_MisT+Sub_MisH<=Max_MM_GAP_Peng))
		{
			Current_Penalty=2*(MFT.L.STRINGLENGTH)*Match_Bonus-(Sub_MisT+Sub_MisH)*Mis_Penalty;
			Paired_Extension(Forward_Read_raw,Revcomp_Read_raw,RawR,Original_Text,Entries,revfmi,Sub_MisT,Sub_MisH,Forward_Read,Revcomp_Read,RQ,Pairs,MFH_Subopt_Start,MFT_Subopt_Start,MCH_Subopt_Start,MCT_Subopt_Start,StringLength,Single_File,R,Err,Conversion_Factor,Alignments,Current_Penalty,Tot_SW_Scans);

		}
	//-----------------Pair Subopt-Subopt end ---------------------------------------
		if((Sub_MisH==Max_MM_GAP_Peng-1 && Top_MisT==0)||(Sub_MisT==Max_MM_GAP_Peng-1 && Top_MisH==0))
		{
			int Least_MisH=(Sub_MisH==-1) ? -1 : Scan(batmeth1,MFH,MCH,Max_MM_GAP_Peng,MCH.L,fwfmi,revfmi,Sub_MisH+1,Head_Top_Count,INT_MAX);
			int Least_MisT=(Sub_MisT==-1) ? -1 : Scan(batmeth1,MFT,MCT,Max_MM_GAP_Peng,MCT.L,fwfmi,revfmi,Sub_MisT+1,Head_Top_Count,INT_MAX);
			//-----------------Pair Top-Least Start ---------------------------------------
			if(Least_MisH==Max_MM_GAP_Peng && Top_MisT==0)
			{
				Paired_Extension(Forward_Read_raw,Revcomp_Read_raw,RawR,Original_Text,Entries,revfmi,Top_MisT,Least_MisH,Forward_Read,Revcomp_Read,RQ,Pairs,MFH_Least_Start,MFT_Top_Start,MCH_Least_Start,MCT_Top_Start,StringLength,Single_File,R,Err,Conversion_Factor,Alignments,Current_Penalty,Tot_SW_Scans);

			}
			if(Least_MisT==Max_MM_GAP_Peng && Top_MisH==0)
			{
				Paired_Extension(Forward_Read_raw,Revcomp_Read_raw,RawR,Original_Text,Entries,revfmi,Top_MisH,Least_MisT,Forward_Read,Revcomp_Read,RQ,Pairs,MFH_Top_Start,MFT_Least_Start,MCH_Top_Start,MCT_Least_Start,StringLength,Single_File,R,Err,Conversion_Factor,Alignments,Current_Penalty,Tot_SW_Scans);
			}
			//-----------------Pair Top-Least End ---------------------------------------
		}
	}
	return Err;
}

#define dist(x,y) (((x)>(y)) ? (x)-(y): (y)-(x))
bool Report_Mismatch_Hits(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,READ & RawR,unsigned char* Original_Text,char source,READ & R,FILE* Single_File,const int StringLength,Hit_Info & Mismatch_Hit,int Quality_Score)
{
	assert (Mismatch_Hit.Loc || Mismatch_Hit.Status==UNMAPPED);
	if(Mismatch_Hit.Status==UNIQUEHIT || Mismatch_Hit.Status==SHARP_UNIQUEHIT)//Hit found and uniquely..
	{
		assert(Quality_Score<=40);
		Print_Sam(RawR,Final_Alignments,Original_Text,source,Single_File,R,Mismatch_Hit,StringLength,30,Default_Alignment,Mismatch_Hit.Clip_H,Mismatch_Hit.Clip_T,Mismatch_Hit.Cigar);return true;
	}
	else if(Mismatch_Hit.Status==MULTI_HIT)
	{
		Mismatch_Hit.Loc=Mismatch_Hit.Sub_Opt_Hit;
		Print_Sam(RawR,Final_Alignments,Original_Text,source,Single_File,R,Mismatch_Hit,StringLength,30,Default_Alignment,Mismatch_Hit.Clip_H,Mismatch_Hit.Clip_T,Mismatch_Hit.Cigar);return true;
	}
	else if(Mismatch_Hit.Status==UNRESOLVED_HIT)
	{
		Mismatch_Hit.Status=MULTI_HIT;
		Print_Sam(RawR,Final_Alignments,Original_Text,source,Single_File,R,Mismatch_Hit,StringLength,0,Default_Alignment,0,0,Mismatch_Hit.Cigar);return true;
	}
	else
	{
		assert(Mismatch_Hit.Loc!=UINT_MAX || Mismatch_Hit.Status==UNMAPPED);//||Mismatch_Hit.Status==UNRESOLVED_HIT);
		return false;
	}
}

bool Report_Single_SW(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,READ & RawR,unsigned char* Original_Text,char source,const int Err,READ & R,FILE* Single_File,const int StringLength,BATREAD & Read,Hit_Info & Mismatch_Hit,bool & Print_Status,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,int Clip_H,int Clip_T,char *CIG)
{
	Alignment A;
	Ann_Info Ann;
	Hit_Info H;
	if(Mismatch_Hit.Status==SW_RECOVERED)
	{
		A=Alignments.top();A.Score= -A.Score;
		if(!CIG)//If cigar not present, multi hit recovery in mismatch state did not actually produce many hits..
		{
			assert(BOOST);//due to clipping of some realignings being stored..
			CIG=A.Cigar;Clip_T=A.Clip_T;Clip_H=A.Clip_H;
		}
		assert(A.Realigned==1);
		H.Org_Loc=A.Loc;H.Sign=A.Sign;H.QScore=A.QScore;H.Status=SW_RECOVERED;
		H.Loc = A.Loc;Location_To_Genome(H.Loc,Ann);H.Chr=Ann.Name;
		if (H.Loc+StringLength <= Ann.Size ) // && Err<=1
		{
			Cigar_Check_And_Print(RawR,Final_Alignments,Original_Text,source,H,Read,StringLength,Single_File,R,true,30,A,Clip_H,Clip_T,CIG);Print_Status=true;
		}
	}
	else
	{
		int SW_Quality_Score=INT_MAX;
		assert(Alignments.size());
		A=Alignments.top();
		assert(A.Realigned==NO || A.Realigned==1);

		FreeQ(Good_Alignments);
		H.Org_Loc=A.Loc;H.Loc = A.Loc;
		Location_To_Genome(H.Loc,Ann);H.Chr=Ann.Name; 
		H.Sign=A.Sign;H.Mismatch=A.Mismatch;H.Indel=A.Indel;H.Score=A.Score;H.QScore=A.QScore;
		if(A.Realigned==NO)/* hit only from indel stage,or 75bp hit..*/
		{
			//assert(Mismatch_Hit.QScore== -1);
			bool Do_Smith_Waterman=true;
			if(H.Sign=='+')
			{
				Hit_Info H2=H;
				Alignment A=RealignFast(RawR,Original_Text,H2,Read,StringLength,R,0,0,true);
				if(A.Score!=INT_MAX)
				{
					Do_Smith_Waterman=false;
					assert(A.Score<=0);
					A.Realigned=1;A.Clip_T=A.Clip_H=0;
					Good_Alignments.push(A);
				}
			}

			if(Do_Smith_Waterman)
			{
				RealignX(RawR,Original_Text,H,Read,StringLength,R,false,Good_Alignments,NULL,Clip_H,Clip_T);
			}
			//RealignX(H,Read,StringLength,R,false,Alignments,Good_Alignments,NULL,Clip_H,Clip_T);

			if(!Good_Alignments.empty())
			{
				A=Good_Alignments.top();
				A.Loc--;
			}
			else
					return false;
		}
		A.Score= -A.Score;

		if(Mismatch_Hit.Status !=UNMAPPED && (A.Score>=Mismatch_Hit.Score)) //moxian changed  //
		{//printf("\nelse %d %d %d\n",A.Loc,A.Score,Mismatch_Hit.Score);
			Print_Status=Report_Mismatch_Hits(Final_Alignments,RawR,Original_Text,source,R,Single_File,StringLength,Mismatch_Hit,30);
		}
		else//hit in indel stage..
		{
			assert(A.QScore!=INT_MAX);
			//A.Loc--;//moxian
			H.Org_Loc=A.Loc;H.Loc = A.Loc;H.Sign=A.Sign;H.QScore=A.QScore;H.Status=UNIQUEHIT;
			//A.Score= -A.Score;
			Location_To_Genome(H.Loc,Ann);H.Chr=Ann.Name;
			assert(A.Realigned);
			if (H.Loc+StringLength <= Ann.Size) // && Err<=1
			{
				Cigar_Check_And_Print(RawR,Final_Alignments,Original_Text,source,H,Read,StringLength,Single_File,R,true,30,A,A.Clip_H,A.Clip_T,A.Cigar);Print_Status=true;
			}
		}
	}
	return true;
}

void Get_Best_Alignment_Pair(READ & RawR,unsigned char* Original_Text,FILE *Single_File,char source,Alignment & A,Alignment & B,READ & R,const int StringLength,BATREAD & Read,Hit_Info & H,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,bool Force_Indel,int & Clip_H,int & Clip_T,char* CIG,bool PRINT)
{
	//std::map <std::string,int> temps; 
	//std::map <std::string,int> ::iterator iterl;
	int C=0;
	FreeQ(Good_Alignments);
	int Best=INT_MAX;
	int Inc_Count=0;
	char CIG2[MAX_SIGLEN];
	int TClip_T,TClip_H;
	
	while(!Alignments.empty())
	{
		A=Alignments.top();Alignments.pop();	
		assert(A.Realigned==NO || A.Realigned==1);
		//if(Force_Indel && !A.Indel && A.QScore!= -1) continue;//no indels or clippings..
		if(MODE!=VERYSENSITIVE && Force_Indel && -A.Score>Best && A.QScore != -1) continue;
		if(BOOST && A.Loc==INT_MAX) continue;

		C++;
		if(C>CUT_MAX_SWALIGN) break;//100000) break;//60) break;

		H.Org_Loc=A.Loc;H.Sign=A.Sign;
		if(A.Realigned==NO)//only realign if necessary..
		{
			bool Do_Smith_Waterman=true;
			if(H.Sign=='+')
			{
				Hit_Info H2=H;
				A=RealignFast(RawR,Original_Text,H2,Read,R.Real_Len,R,0,0,true);
				if(A.Score!=INT_MAX)
				{
					Do_Smith_Waterman=false;
					assert(A.Score<=0);
					A.Realigned=1;A.Clip_T=A.Clip_H=0;
					Good_Alignments.push(A);
				}
			}
			if(Do_Smith_Waterman)
			{
				A=RealignX(RawR,Original_Text,H,Read,StringLength,R,true,Good_Alignments,CIG2,TClip_H,TClip_T);//dont push to Q..
			}
		}
		else
		{
			assert(A.Cigar);
			CIG2[0]=0;
		}

		if(A.Sign == '-')
		{
			A.Loc+=(R.Real_Len-StringLength);
		}

		Good_Alignments.push(A);
		if(A.Loc!=INT_MAX && -A.Score<Best) 
		{
			C=0;
			Best= -A.Score;
			//strcpy(CIG,CIG2);Clip_T=TClip_T;Clip_H=TClip_H;
			strcpy(CIG,A.Cigar);Clip_T=A.Clip_T;Clip_H=A.Clip_H;
		}
	}
	
	if(!PRINT) return;

	if(!Good_Alignments.empty())//Pick the top two..
	{
		A=Good_Alignments.top();Good_Alignments.pop();A.Score= -A.Score;
		if(!Good_Alignments.empty())
		{
			B=Good_Alignments.top();Good_Alignments.pop();B.Score= -B.Score;
			while(dist(A.Loc,B.Loc)<10 && !TOP_TEN)//std::max(10,(R.Real_Len-StringLength))) 
			{
				if(Good_Alignments.empty())
				{
					B.Score=INT_MAX; 
					break;
				}
				else
				{
					B=Good_Alignments.top();Good_Alignments.pop();B.Score= -B.Score;
				}
			}
			if(dist(A.Loc,B.Loc)<10)//std::max(10,(R.Real_Len-StringLength))) 
				B.Score=INT_MAX; 
			//if(B.Mismatch >MAX_MISMATCHES  || (B.Cigar[0]!=0 && (Get_ED(B.Cigar) >= (int) (3*((double)R.Real_Len/100)) || (double)Get_Match(B.Cigar)/R.Real_Len <= 0.2) )  )
			//	B.Score=INT_MAX; 
		}
		else 
			B.Score=INT_MAX; 
	}
	else A.Score=B.Score=INT_MAX;

	if(CIG[0]!=0)//will not pass thru second alignment..
	{
		A.Loc--;
		if(A.Sign=='-')
			A.Loc-=(R.Real_Len-StringLength);
	}
	
}

int Calc_SW_Quality(READ& RawR,unsigned char* Original_Text,Alignment A,READ & R,const int StringLength,BATREAD & Read,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments)
{
	return 30;
	Alignment B;
	assert(A.QScore!=INT_MAX);
	//float Top_Prob=pow(10,-A.QScore/10);
	float Top_Prob=Pow10(A.QScore);
	float Prob_Sum=0;
	Hit_Info H;

	if(A.Score>B.Score-10)//use only best hits..
	{
		//Prob_Sum=Top_Prob+pow(10,-B.QScore/10);
		Prob_Sum=Top_Prob+Pow10(B.QScore);
		while(!Good_Alignments.empty())
		{
			B=Good_Alignments.top();Good_Alignments.pop();B.Score= -B.Score;
			if(A.Score<B.Score-10)
			{
				if(B.QScore==INT_MAX)
				{
					H.Org_Loc=B.Loc;H.Sign=B.Sign;
					B=Realign(RawR,Original_Text,H,Read,StringLength,R,true,Good_Alignments);
					B.Score= -B.Score;
				}
				assert(B.QScore!=INT_MAX);
				//Prob_Sum+=pow(10,-B.QScore/10);
				Prob_Sum+=Pow10(B.QScore);
			}
			else
				break;
		}
		float Mapping_Quality,Posterior=Top_Prob/(Prob_Sum);
		Mapping_Quality=(int(Posterior)==1) ? 30 : -10*log10(1-Posterior);
		assert(int(Mapping_Quality)>=0);
		return std::max(1,int(Mapping_Quality));
	}
	else
	{
		return 30;
	}

}
bool Report_SW_Hits(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,READ & RawR,unsigned char* Original_Text,char source,const int Err,READ & R,FILE* Single_File,const int StringLength,BATREAD & Read,Hit_Info & Mismatch_Hit,int Quality_Score,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,bool Force_Indel,bool PRINT)
{
	int Alignment_Count=Alignments.size();
	bool Print_Status=false;
	if(!Alignment_Count)//Smith waterman recovery failed or not performed..(Mismatch stage)
	{
		if(!PRINT) 
		{
			return true;
		}
		Print_Status=Report_Mismatch_Hits(Final_Alignments,RawR,Original_Text,source,R,Single_File,StringLength,Mismatch_Hit,Quality_Score);
	}
	else
	{
		Alignment A,B;
		Ann_Info Ann;
		Hit_Info H;H.Sub_Opt_Score=INT_MAX;

		int SW_Quality_Score=INT_MAX;
		int Flag;
		int Clip_H,Clip_T;
		char CIG[MAX_SIGLEN];

		if(Alignment_Count==1)//Smith waterman got a unique hit..
		{
			if(!PRINT)
			{
				Get_Best_Alignment_Pair(RawR,Original_Text,Single_File,source,A,B,R,StringLength,Read,H,Alignments,Good_Alignments,Force_Indel,Clip_H,Clip_T,CIG,PRINT);
				return true;
			}
			Alignment A=Alignments.top();Alignments.pop();
			A.Loc--;
			Alignments.push(A);//moxian
			if(!Report_Single_SW(Final_Alignments,RawR,Original_Text,source,Err,R,Single_File,StringLength,Read,Mismatch_Hit,Print_Status,Alignments,Good_Alignments,0,0,NULL)) return false;
		}
		else if (Alignment_Count)//Multiple SW hits..
		{	
			Get_Best_Alignment_Pair(RawR,Original_Text,Single_File,source,A,B,R,StringLength,Read,H,Alignments,Good_Alignments,Force_Indel,Clip_H,Clip_T,CIG,PRINT);
			if(!PRINT) return true;
			if(A.Score==INT_MAX) return false;
			if(Mismatch_Hit.Status!=SW_RECOVERED && Mismatch_Hit.Status!=UNMAPPED && A.Score>=Mismatch_Hit.Score)
			{
				assert(false);
				Print_Status=Report_Mismatch_Hits(Final_Alignments,RawR,Original_Text,source,R,Single_File,StringLength,Mismatch_Hit,30);
			}
			else if(B.Score==INT_MAX)
			{
				A.Score= -A.Score;A.Sub_Opt_Score=INT_MAX; Alignments.push(A);
				assert(A.Score!=INT_MAX);
				if(!Report_Single_SW(Final_Alignments,RawR,Original_Text,source,Err,R,Single_File,StringLength,Read,Mismatch_Hit,Print_Status,Alignments,Good_Alignments,Clip_H,Clip_T,CIG)) return false;
			}
			else if(A.Score<B.Score)//Top hit with multiple hits..
			{
				if(A.QScore==INT_MAX)
				{
					H.Org_Loc=A.Loc;H.Sign=A.Sign;
					A=Realign(RawR,Original_Text,H,Read,StringLength,R,true,Good_Alignments);
					A.Score= -A.Score;
				}
				A.Sub_Opt_Score=B.Score;
				SW_Quality_Score=Calc_SW_Quality(RawR,Original_Text,A,R,StringLength,Read,Alignments,Good_Alignments);
				H.Org_Loc=A.Loc;H.Loc = A.Loc;Location_To_Genome(H.Loc,Ann);H.Chr=Ann.Name;H.Sign=A.Sign;H.Mismatch=A.Mismatch;H.Indel=A.Indel;
				H.Score=A.Score;H.Sub_Opt_Score= B.Score;H.QScore=A.QScore;H.SW_Sub_Opt_Score=B.SW_Score;
				if(Err>1) Flag=4; else if (H.Sign=='+') Flag=0; else Flag=16; 
				if(TOP_TEN) if (H.Sign=='+') Flag=0; else Flag=16; 

			strcpy(CIG,A.Cigar);Clip_T=A.Clip_T;Clip_H=A.Clip_H;
				if(Flag!=4||TOP_TEN) 
				{
					H.Status=Mismatch_Hit.Status=MULTI_HIT;
					Cigar_Check_And_Print(RawR,Final_Alignments,Original_Text,source,H,Read,StringLength,Single_File,R,true,SW_Quality_Score,A,Clip_H,Clip_T,CIG);Print_Status=true;
				}
				if(TOP_TEN)
				{
					Report_Single(Final_Alignments,RawR,Original_Text,source,R,Single_File,StringLength,Read,Print_Status,Clip_H,Clip_T,B);
					int c=0;std::set <unsigned> S;
					S.insert(A.Loc);S.insert(B.Loc);
					while(TOP_TEN >2 && !Good_Alignments.empty() && c!=(TOP_TEN-2))
					{
						A=Good_Alignments.top();Good_Alignments.pop();A.Score= -A.Score;
						if(S.find(A.Loc)!=S.end())
							continue;
						S.insert(A.Loc);
						Report_Single(Final_Alignments,RawR,Original_Text,source,R,Single_File,StringLength,Read,Print_Status,Clip_H,Clip_T,A);
						c++;
					}
				}
			}
			else //Multiple hits..
			{
strcpy(CIG,A.Cigar);Clip_T=A.Clip_T;Clip_H=A.Clip_H;
				H.Org_Loc=A.Loc;H.Loc = A.Loc;Location_To_Genome(H.Loc,Ann);H.Chr=Ann.Name;H.Sign=A.Sign;H.Mismatch=A.Mismatch;H.Indel=A.Indel;H.Score=A.Score;H.QScore=A.QScore;
				if (H.Sign=='+') Flag=0; else Flag=16; 
				Cigar_Check_And_Print(RawR,Final_Alignments,Original_Text,source,H,Read,StringLength,Single_File,R,true,0,A,Clip_H,Clip_T,CIG);Print_Status=true;
				if(TOP_TEN)
				{
					Report_Single(Final_Alignments,RawR,Original_Text,source,R,Single_File,StringLength,Read,Print_Status,Clip_H,Clip_T,B);
					int c=0;std::set <unsigned> S;
					S.insert(A.Loc);S.insert(B.Loc);
					while(TOP_TEN >2 && !Good_Alignments.empty() && c!=(TOP_TEN-2))
					{
						A=Good_Alignments.top();Good_Alignments.pop();A.Score= -A.Score;
						if(S.find(A.Loc)!=S.end())
							continue;
						S.insert(A.Loc);
						Report_Single(Final_Alignments,RawR,Original_Text,source,R,Single_File,StringLength,Read,Print_Status,Clip_H,Clip_T,A);
						c++;
					}
				}
			}

		}
		else fprintf(stderr,"%d\n",Alignment_Count);
	}
	return Print_Status;
}

void  Paired_Extension(char* Fwd_Read_raw,char *Revcomp_Read_raw,READ & RawR,unsigned char* Original_Text,unsigned Entries,BWT *revfmi,int Last_MisT,int Last_MisH,char* Fwd_Read,char *Revcomp_Read, RQINDEX & RQ,Pair* & Pairs,SARange* & MFH_Hit_Array,SARange* & MFT_Hit_Array,SARange* & MCH_Hit_Array,SARange* & MCT_Hit_Array,int StringLength,FILE* Single_File,READ & R,int & Err,unsigned Conversion_Factor,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,int Current_Penalty,int & Tot_SW_Scans)
{
	if(Last_MisT>=0 && Last_MisH>=0)
	{
		char In_Large;
		Ann_Info A;
		int Pairs_Index=0,Pairings=0;
		Head_Tail(revfmi,MFH_Hit_Array,MFT_Hit_Array,2*StringLength/2+INDELGAP,MAXCOUNT,In_Large,RQ,Entries,Pairs,Pairs_Index,Pairings,Err,Conversion_Factor);
		int SW_Hits=0;
		SW_Hits+=Do_SW_Pair(Fwd_Read_raw+StringLength/3,RawR,Original_Text,Pairs,Fwd_Read+StringLength/3,StringLength/3+INDELGAP,StringLength, Err,StringLength/3+1,'+',Alignments,Current_Penalty,Tot_SW_Scans);
		Pairs_Index=0,Pairings=0;
		Head_Tail(revfmi,MCT_Hit_Array,MCH_Hit_Array,2*StringLength/2+INDELGAP,MAXCOUNT,In_Large,RQ,Entries,Pairs,Pairs_Index,Pairings,Err,Conversion_Factor);
		SW_Hits=0;
		SW_Hits+=Do_SW_Pair(Revcomp_Read_raw+StringLength/3,RawR,Original_Text,Pairs,Revcomp_Read+StringLength/3,StringLength/3+INDELGAP,StringLength, Err,StringLength/3+1,'-',Alignments,Current_Penalty,Tot_SW_Scans);
	}
}

void Extend_Right(char* Temp_Current_Tag_raw,READ& RawR,RQINDEX & RQHALF,unsigned char* Original_Text,int Plus_Hits,int Minus_Hits,BWT* revfmi,MEMX & MFL,MEMX & MCL,char* Temp_Current_Tag,int StringLength,int & Err, READ & R,int Mis_In_Anchor,FILE* Single_File,int Current_Score,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,int & Tot_SW_Scans,int & Filter )
{
	if(Plus_Hits+Minus_Hits)
	{
		Hit_Info H,T;
		bool Head_Disc=false,Tail_Disc=false;

		if(Plus_Hits)
		{
			int Aln_Index=0;
			int SW_Hits=0;
			SW_Hits+=Do_SW(MFL.Current_Tag_raw+MFL.L.STRINGLENGTH,RawR,RQHALF,Original_Text,revfmi,MFL.Hit_Array,MFL.Current_Tag+MFL.L.STRINGLENGTH,MFL.L.STRINGLENGTH+INDELGAP,StringLength, Err,MFL.L.STRINGLENGTH+1,'+',Current_Score,Alignments,Tot_SW_Scans,Filter);

		}
		if(Minus_Hits)
		{
			int Aln_Index=0;
			int SW_Hits=0;
			SW_Hits+=Do_SW(Temp_Current_Tag_raw+MCL.L.STRINGLENGTH,RawR,RQHALF,Original_Text,revfmi,MCL.Hit_Array,Temp_Current_Tag+MCL.L.STRINGLENGTH,MCL.L.STRINGLENGTH+INDELGAP,StringLength, Err,-MCL.L.STRINGLENGTH-INDELGAP,'-',Current_Score,Alignments,Tot_SW_Scans,Filter);
		}

	}
}

void Extend_Left(char* Temp_Current_Tag_raw,READ& RawR,RQINDEX & RQHALF,unsigned char* Original_Text,int Plus_Hits,int Minus_Hits,BWT* revfmi,MEMX & MFL,MEMX & MCL,char* Temp_Current_Tag,int StringLength,int & Err, READ & R,int Mis_In_Anchor,FILE* Single_File,int Current_Score,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,int & Tot_SW_Scans,int & Filter)
{
	if(Plus_Hits+Minus_Hits)
	{
		Hit_Info H,T;

		if(Plus_Hits)
		{
			int Aln_Index=0;
			int SW_Hits=0;
			SW_Hits+=Do_SW(Temp_Current_Tag_raw+MFL.L.STRINGLENGTH,RawR,RQHALF,Original_Text,revfmi,MFL.Hit_Array,Temp_Current_Tag+MFL.L.STRINGLENGTH,MFL.L.STRINGLENGTH+INDELGAP,StringLength,Err,-MFL.L.STRINGLENGTH-INDELGAP,'+',Current_Score,Alignments,Tot_SW_Scans,Filter);
		}
		if(Minus_Hits)
		{
			int Aln_Index=0;
			int SW_Hits=0;
			SW_Hits+=Do_SW(MCL.Current_Tag_raw+MCL.L.STRINGLENGTH,RawR,RQHALF,Original_Text,revfmi,MCL.Hit_Array,MCL.Current_Tag+MCL.L.STRINGLENGTH,MCL.L.STRINGLENGTH+INDELGAP,StringLength, Err,MCL.L.STRINGLENGTH+1,'-',Current_Score,Alignments,Tot_SW_Scans,Filter);
		}
	}
}


//map reads..
int Head_Tail(BWT* revfmi,SARange* Head_Hits,SARange* Tail_Hits,int Insert,int MAXCOUNT,char & In_Large,RQINDEX & R,unsigned Entries,Pair* Pairs,int & Pairs_Index,int & HITS,int & Err,unsigned Conversion_Factor)
{
	assert (MAXCOUNT >0);assert (revfmi);assert(Head_Hits);assert(Tail_Hits);assert(Entries>0);assert(Pairs);
	HITS=0;
	SARange Head,Tail;
	int TGap,HGap;
	if (Pairs_Index >=MAXCOUNT){if(!Err) Err=1;return HITS;}
	for(int i=0;Head_Hits[i].Start && HITS <MAXCOUNT;i++)//Iterate Head Hits
	{
		for(int j=0;Tail_Hits[j].Start && HITS <MAXCOUNT;j++)//With Tail Hits
		{
			Head.Start=Head_Hits[i].Start;
			Head.End=Head_Hits[i].End;
			Head.Mismatches=Head_Hits[i].Mismatches;
			Tail.Start=Tail_Hits[j].Start;
			Tail.End=Tail_Hits[j].End;
			Tail.Mismatches=Tail_Hits[j].Mismatches;
			TGap=Tail.End-Tail.Start;
			HGap=Head.End-Head.Start;
			if(HGap>=INDEX_RESOLUTION || TGap>=INDEX_RESOLUTION){if(!Err) Err=1;}

			if (TGap> MAXGAP || HGap > MAXGAP)//sa ranges not cached..
			{
				In_Large=TRUE;
				continue;
			}
			if (TGap<= SAGAP_CUTOFF || HGap <= SAGAP_CUTOFF)//Small sa ranges
			{
				if(TGap && HGap)
				{
					unsigned Start;
					if(HGap<TGap)
					{
						Start=Head.Start;
						for (int i=0;i<=HGap;i++)
						{
							Head.Start=Start;
							Head.End=Head.Start=Conversion_Factor-BWTSaValue(revfmi,Head.Start);
							Search_Small_Gap(revfmi,R,Head,Tail,Insert,Pairs,Pairs_Index,MAXCOUNT,HITS,Entries,Conversion_Factor);
							if (HITS >= MAXCOUNT) break;
							Start++;
						}
					}
					else
					{
						Start=Tail.Start;
						for (int i=0;i<=TGap;i++)
						{
							Tail.Start=Start;
							Tail.End=Tail.Start=Conversion_Factor-BWTSaValue(revfmi,Tail.Start);
							Search_Small_Gap(revfmi,R,Head,Tail,Insert,Pairs,Pairs_Index,MAXCOUNT,HITS,Entries,Conversion_Factor);
							if (HITS >= MAXCOUNT) break;
							Start++;
						}
					}
				}
				else
				{
					Search_Small_Gap(revfmi,R,Head,Tail,Insert,Pairs,Pairs_Index,MAXCOUNT,HITS,Entries,Conversion_Factor);
				}
			}
			else//Two big SA gaps...
			{
				Get_Head_Tail(revfmi,R,Head, Tail,Insert,Pairs,Pairs_Index,MAXCOUNT,HITS,Entries,Conversion_Factor);
			}
			if (Pairs_Index >=MAXCOUNT){if(!Err) Err=1;return HITS;}

		}
	}
	Pairs[Pairs_Index].Head=0;
	assert (HITS >=0);
	return HITS;
}



void Verbose(char *BWTFILE,char *OCCFILE,char *REVBWTINDEX,char *REVOCCFILE,char *PATTERNFILE,char *HITSFILE,char* LOCATIONFILE,char MAX_MISMATCHES,int Patternfile_Count,char* PATTERNFILE1,char FILETYPE,LEN & L,char FORCESOLID)
{
	if (!MISC_VERB) return;
	fprintf(stderr,"BatMeth ALIGN - Long ..\n");
	fprintf(stderr,"+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
	fprintf(stderr,"Using the genome files\n %s\t %s\n %s\t %s\n", BWTFILE,OCCFILE,REVBWTINDEX,REVOCCFILE); 
	fprintf(stderr,"Location file: %s\n", LOCATIONFILE); 
/*	fprintf(stderr,"Query File : %s \t\t Output file: %s\n",PATTERNFILE,HITSFILE);
	if(Patternfile_Count) fprintf(stderr,"Mate File : %s \n ",PATTERNFILE1);
	fprintf(stderr,"Length of Tags: %d\t", L.STRINGLENGTH);
	fprintf(stderr,"Mismatches allowed : %d\n",MAX_MISMATCHES);
	if (SOLID) 
	{
		if (FORCESOLID) fprintf (stderr,"DIBASE-SOLiD reads...\n");
		else fprintf (stderr,"SOLiD reads...\n");
	}
	if (FILETYPE == FQ) fprintf(stderr,"FASTQ file..\n"); else fprintf(stderr,"FASTA file..\n");
	if (UNIQ_HIT) fprintf(stderr,"Unique hit mode..\n");
	if (PRIORITYMODE) fprintf(stderr,"Lowest mismatch mode..\n");
	fprintf(stderr,"+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n");
*/
}
void FileInfo(char *PATTERNFILE,char *HITSFILE,char MAX_MISMATCHES,int Patternfile_Count,char* PATTERNFILE1,char FILETYPE,LEN & L,char FORCESOLID)
{
	fprintf(stderr,"==============================================================\n");
	fprintf(stderr,"Query File : %s \t\t Output file: %s\n",PATTERNFILE,HITSFILE);
	if(Patternfile_Count) fprintf(stderr,"Mate File : %s \n ",PATTERNFILE1);
	fprintf(stderr,"Length of Tags: %d\t", L.STRINGLENGTH);
	fprintf(stderr,"Mismatches allowed : %d\n",MAX_MISMATCHES);
	if (SOLID) 
	{
		if (FORCESOLID) fprintf (stderr,"DIBASE-SOLiD reads...\n");
		else fprintf (stderr,"SOLiD reads...\n");
	}
	if (FILETYPE == FQ) fprintf(stderr,"FASTQ file..\n"); else fprintf(stderr,"FASTA file..\n");
	if (UNIQ_HIT) fprintf(stderr,"Unique hit mode..\n");
	if (PRIORITYMODE) fprintf(stderr,"Lowest mismatch mode..\n");
	fprintf(stderr,"---------------------------------------------------------------------------------------------------------------\n");
}

void Init(int & Pcount,BWT* revfmi,In_File & IN,FMFILES F,RQINDEX R,BATPARAMETERS & BP,char Solid,char Npolicy,LEN & L)
{
	JUMP=BP.INDELSIZE;
	INSERT=DELETE=BP.INDELSIZE;
	INDELGAP=2*JUMP+1;

	SOLID=Solid; 
	srand(0);
	if (SOLID) 
	{
		L.IGNOREHEAD=2;
		PAIRING_SCHEME = 5353;
	}
	//NPOLICY=Npolicy;
	Char_To_Code['A']=0;Char_To_Code['C']=1;Char_To_Code['G']=2;Char_To_Code['T']=3;Char_To_Code['a']=0;Char_To_Code['c']=1;Char_To_Code['g']=2;Char_To_Code['t']=3;
	Char_To_Code['0']=0;Char_To_Code['1']=1;Char_To_Code['2']=2;Char_To_Code['3']=3;Char_To_Code['4']=0;
	Char_To_CodeC['A']=3;Char_To_CodeC['C']=2;Char_To_CodeC['G']=1;Char_To_CodeC['T']=0;Char_To_CodeC['a']=3;Char_To_CodeC['c']=2;Char_To_CodeC['g']=1;Char_To_CodeC['t']=0;
		Char_To_CodeC['n']=0;Char_To_CodeC['N']=0;//0 moxian//we are using character count to store the fmicode for acgt
	Char_To_CodeC['N']=3;Char_To_CodeC['n']=3;Char_To_CodeC['.']=3;//3; moxian
		Char_To_CodeC[0]=3;Char_To_CodeC[1]=2;Char_To_CodeC[2]=1;Char_To_CodeC[3]=0;
			Char_To_CodeC['a']=3;Char_To_CodeC['c']=2;Char_To_CodeC['g']=1;Char_To_CodeC['t']=0;Char_To_CodeC['-']='-';Char_To_CodeC['+']='+';//we are using character count to store the fmicode for acgt
	//Char_To_CodeC['0']=3;Char_To_CodeC['1']=2;Char_To_CodeC['2']=1;Char_To_CodeC['3']=0;

	Char_To_CharC['A']='T';Char_To_CharC['C']='G';Char_To_CharC['G']='C';Char_To_CharC['T']='A';Char_To_CharC['a']='t';Char_To_CharC['c']='g';Char_To_CharC['g']='c';Char_To_CharC['t']='a';
		Char_To_CharC['n']='n';Char_To_CharC['N']='N';//we are using character count to store the fmicode for acgt
	//Char_To_CodeCS['3']=3;Char_To_CodeCS['2']=2;Char_To_CodeCS['1']=1;Char_To_CodeCS['0']=0;Char_To_CodeCS['.']=0;
	Code_To_CodeC[0]=3;Code_To_CodeC[1]=2;Code_To_CodeC[2]=1;Code_To_CodeC[3]=0;
	//if (SOLID) sprintf(Default_Cigar,"%dM",IN.STRINGLENGTH-1);else sprintf(Default_Cigar,"%dM",IN.STRINGLENGTH);
	if (BP.STD)//set a margin of avg+3*std
	{
		BP.FLANKSIZE= 3*BP.STD;//for normal pairing
		BP.SW_FLANKSIZE= 2*IN.STRINGLENGTH+6*BP.STD;
	}
	else if (!BP.SW_FLANKSIZE)
	{
		BP.SW_FLANKSIZE= IN.STRINGLENGTH/2+50;//check 50 bp either side if not specified...
		if (BP.SW_FLANKSIZE > BP.INSERTSIZE) BP.SW_FLANKSIZE=BP.INSERTSIZE;
	}
	if (BP.SW_FLANKSIZE>=SW_STRING_BUFFER) {fprintf(stderr,"SW Flank too large.. Defaulting\n");BP.SW_FLANKSIZE= IN.STRINGLENGTH/2+50;}
	if (BP.PLUSSTRAND) BP.PLUSSTRAND=IN.STRINGLENGTH;
	IN.Positive_Head=IN.Tag_Copy;

/*
	FILE* Original_File=File_Open(F.BINFILE,"rb");
	Original_Text=(unsigned char*) malloc(Get_File_Size(Original_File));
	if(!fread(Original_Text,Get_File_Size(Original_File),1,Original_File))fprintf (stderr,"Init(): Error loading genome...\n");
*/
	if (IN.STRINGLENGTH < SW_MIN_MATCH_LEN) SW_MIN_MATCH_LEN=IN.STRINGLENGTH-1;
	if (BP.MISHITS) 
	{
		Mishit_File1=File_Open(BP.MISFILE1,"w"); 
		Mishit_File2=File_Open(BP.MISFILE2,"w"); 
	}

	if(MISC_VERB)
	{
		if(Pcount==0)
		{
			//fprintf (stderr,"------------------------------------------------------------------------------------------------------------------\n");
			if (R.COMPRESS) fprintf (stderr,"Using compressed index...\n");
			//fprintf(stderr,"Read length %d\n",R.Real_Len);
			if (BP.SMITH_WATERMAN)
			{
				fprintf(stderr,"Mate rescue mode : Flank size - %d\n",BP.SW_FLANKSIZE);
			}
			fprintf(stderr,"Max Gap : %d\n",BP.INSERTSIZE+BP.FLANKSIZE);
			//fprintf(stderr,"Using Pairing index %s\t %s\n", F.INDFILE,F.BLKFILE); 
		}
		Pcount++;
	}

}

void FreeQ(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment>  & t ) 
{
	std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> tmp; 
	std::swap(t,tmp );
}
void FreeQ_Final(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment>  & t ) 
{
	std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> tmp; 
	std::swap(t,tmp );
}
const int FHSIZE=10;//00;
const int FHSKIP=500;

void Print_SA(BWT *revfmi,SARange* SAList,int Count,int & Hits,char Sign,int STRINGLENGTH,Hit_*  Hits_,int & First_Hit_Ptr,unsigned Conversion_Factor)
{
	static int Last_Resize=FHSIZE;
	unsigned Loc;
	Ann_Info A;
	int Rand_Hit=1;
	for(int i=0;Hits<HITS_IN_SAM && i<Count-1 && First_Hit_Ptr==0 && First_Hit_Ptr<= FHSIZE;i++)
	{
		SARange S= SAList[i];
		int j=0;
		if (S.Start==S.End) 
		{
			Loc = S.Start;
			Hits_[First_Hit_Ptr].Loc=Loc;
			Hits_[First_Hit_Ptr++].Sign=Sign;
			if ( First_Hit_Ptr == Last_Resize)//Too many hits..
			{
				assert(false);//shouldnt happen..
				Hit_ *THit=new Hit_[Last_Resize+FHSKIP];
				memcpy(THit,Hits_,sizeof(Hit_)*Last_Resize);
				for (int i=0;i<Last_Resize;i++) assert(THit[i].Loc==Hits_[i].Loc && THit[i].Chr==Hits_[i].Chr && THit[i].Sign==Hits_[i].Sign);
				Last_Resize+=FHSKIP;
				delete [] Hits_;
				Hits_=THit;
			}
			j++;Hits++;
		}
		else
		{
			assert (S.Start>=0);
			while (Hits<HITS_IN_SAM && j<=(S.End-S.Start) && First_Hit_Ptr==0 && First_Hit_Ptr<= FHSIZE)
			{
				Loc = Conversion_Factor-BWTSaValue(revfmi,S.Start+j);
				Hits_[First_Hit_Ptr].Loc=Loc;//+1;
				Hits_[First_Hit_Ptr++].Sign=Sign;
				if ( First_Hit_Ptr == Last_Resize)//Too many hits..
				{
					assert(false);
					Hit_ *THit=new Hit_[Last_Resize+FHSKIP];
					memcpy(THit,Hits_,sizeof(Hit_)*Last_Resize);
					for (int i=0;i<Last_Resize;i++) assert(THit[i].Loc==Hits_[i].Loc && THit[i].Chr==Hits_[i].Chr && THit[i].Sign==Hits_[i].Sign);
					Last_Resize+=FHSKIP;
					delete [] Hits_;
					Hits_=THit;
					fprintf(stderr,"Print_SA():Array rezized %d\n",Last_Resize); 
				}
				j++;Hits++;
			}
		}
	}
}

bool Get_Info(BWT *revfmi,MEMX & MF,MEMX & MC,int STRINGLENGTH,Hit_Info & H,unsigned Conversion_Factor)
{
	int MFC=MF.Hit_Array_Ptr;
	int MCC=MC.Hit_Array_Ptr;

	Hit_ First_Hits[FHSIZE];
	First_Hits[0].Loc=FHSIZE;


	int First_Hit_Ptr=0;
	assert(MFC >=0 && MCC >=0);
	H.Chr=NULL;H.Loc=0;
	char* Org_MH=H.MH;
	int Hits=0;
	if(MFC)
	{
		Print_SA(revfmi,MF.Hit_Array,MFC,Hits,'+',STRINGLENGTH,First_Hits,First_Hit_Ptr,Conversion_Factor);
	}
	assert (Hits<=HITS_IN_SAM);
	if(MCC)
	{
		Print_SA(revfmi,MC.Hit_Array,MCC,Hits,'-',STRINGLENGTH,First_Hits,First_Hit_Ptr,Conversion_Factor);
	}

	if (First_Hit_Ptr)
	{
		int Seed=rand()%(First_Hit_Ptr); if (Seed) Seed--;
		H.Chr=First_Hits[Seed].Chr;H.Loc=First_Hits[Seed].Loc;H.Sign=First_Hits[Seed].Sign;
		if (First_Hit_Ptr >1) H.MH+=sprintf(Org_MH,"\tXX:Z:");
		for (int i=0;i<First_Hit_Ptr;i++)
		{
			if (i!=Seed) 
				if (H.MH-Org_MH<MULTIHIT_COUNT-150) H.MH+=sprintf(H.MH,"%s:%c:%u;",First_Hits[i].Chr,First_Hits[i].Sign,First_Hits[i].Loc);
				else break;
		}
		*H.MH=0;
	}
	H.Hits=Hits;
	return (H.Loc);
}

bool Unique_Hits(int Plus_Hits,int Minus_Hits,SARange & P,SARange & M)
{
	if(Plus_Hits)
	{
		if (P.Start!=P.End) 
		{
			return false; 
		}
		else 
		{
			return true;
		}
	}
	else
	{
		if (M.Start!=M.End) 
		{
			return false;
		}	
		else 
		{
			return true;
		}
	}
}

bool Check_Subopt(int & Plus_Hits,int & Minus_Hits,int Top_Mis,int Subopt_Mis,READ & R, int StringLength,MEMX & MF,MEMX & MC,float & Top_Score,float & Top_BQScore,float & Sub_Score,float & Sub_BQScore, int & Quality_Score)
{
	if (SUPOPT_EXIST_FILTER) return false;
	else if (PHRED_FILTER) return Phred_Check(Plus_Hits,Minus_Hits,Top_Mis,Subopt_Mis,R,StringLength,MF,MC,Top_Score,Top_BQScore,Sub_Score,Sub_BQScore,Quality_Score);
	else SNP_Check(Plus_Hits,Minus_Hits,Top_Mis,Subopt_Mis,R,StringLength,MF,MC);
}

Alignment Realign(READ& RawR,unsigned char* Original_Text,Hit_Info &  H,BATREAD & Read,int StringLength,const READ & R,bool Dont_Push_To_Q,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments)
{
	s_align* Aln;
	Alignment A;
	char Org_String[500],Cigar[MAX_SIGLEN];
	char Org_String_Ori[500];
	Cigar_Info Cig_Info;

	char* Current_Tag=(H.Sign=='+')? Read.Forward : Read.Complement;
	char* Current_Tag_raw=(H.Sign=='+')? Read.Forward_raw : Read.Complement_raw;
	
	A.Realigned=NO;
	int Jump=0;if(H.Sign=='-') Jump= 0+JUMP;
	int8_t* mata_tmp; mata_tmp = (int8_t*)calloc(25, sizeof(int8_t));
	char state=RawR.state;
	if(H.Sign=='-') state=RawR.state=='1'?'2':'1';
	if(state=='1')
		init_SSW_BS_c2t(mata_tmp);
	else if(state=='2')
		init_SSW_BS_g2a(mata_tmp);
	else init_SSW(mata_tmp);
	
	s_profile* p = ssw_init((int8_t*)Current_Tag/*_raw*/, StringLength, mata_tmp, n, 1);
	Get_Bases(Original_Text,H.Org_Loc+Jump,StringLength+INDELGAP,Org_String);
	//Get_Bases(Original_Text_Ori,H.Org_Loc+Jump,StringLength+INDELGAP,Org_String_Ori);//momo
	Aln=mengyao_ssw_core(Org_String/*_Ori*//*+Jump*/,StringLength, Current_Tag/*_raw*/,StringLength+INDELGAP,0,0/*DP*/, p);
	if(Aln->score1 >= ACC_SCORE)
	{
		A.SW_Score=Aln->score1;
		ssw_cigar_processQ(H.Sign,RawR,Org_String_Ori,Aln,Cig_Info,Org_String,Aln->ref_begin1,Current_Tag,Aln->read_begin1,StringLength,R.Quality,NULL,0,0);
		A.Loc=H.Org_Loc+Aln->ref_begin1;
		A.Score= -Cig_Info.Score;
		A.QScore=Cig_Info.QScore;
		A.BQScore=Cig_Info.BQScore;
		A.Mismatch=Cig_Info.Mis;
		A.Indel=Cig_Info.Indel_Count;
		A.Sign=H.Sign;
		if(A.Indel)
		{
			if(!Dont_Push_To_Q)
			{
				Good_Alignments.push(A);
			}
		}
		else
		{
			//if(A.Mismatch> Inter_MM && !Dont_Push_To_Q)
			{
				Good_Alignments.push(A);
			}
		}
	}
	align_destroy(Aln);
	init_destroy(p); 
	return A;
	
}


//TODO
float Calc_Top_Score(MEMX & MF,MEMX & MC,float & Top_BQ,int Top_Mis,int StringLength,int Plus_Hits,int Minus_Hits,READ & R)
{
	if(Plus_Hits)
	{
		Top_BQ=BQSumX(MF.Hit_Array[0],R.Quality,Top_Mis,StringLength);
		return QSumX(MF.Hit_Array[0],R.Quality,Top_Mis,StringLength);
	}
	else
	{
		char Rev_Qual[100];
		Reverse_Quality(Rev_Qual,R,StringLength);
		Top_BQ=BQSumX(MC.Hit_Array[0],Rev_Qual,Top_Mis,StringLength);
		return QSumX(MC.Hit_Array[0],Rev_Qual,Top_Mis,StringLength);
	}
}

bool Do_Mismatch_Scan(READ& RawR,bool batmeth1,unsigned char* Original_Text,MEMX & MF,MEMX & MC,LEN & L,BWT* fwfmi,BWT* revfmi,int Start_Mis,int End_Mis,int & Last_Mis,int & Head_Top_Count,Hit_Info & H,int & Quality_Score,READ & R,BATREAD & B,FILE* Mishit_File,FILE* Single_File,unsigned Conversion_Factor,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments)
{

	H.Loc=0;H.Indel=0;H.Score=0;H.QScore=-1;H.Cigar[0]=0;
	Last_Mis=Scan(batmeth1,MF,MC,End_Mis,L,fwfmi,revfmi,Start_Mis,Head_Top_Count,(BOOST>=5) ? 2:maxhits);
	int Plus_Hits=MF.Hit_Array_Ptr-1,Minus_Hits=MC.Hit_Array_Ptr-1;
	int Sub_OptMF=MF.Hit_Array_Ptr,Sub_OptMC=MC.Hit_Array_Ptr,Sub_OptMIS=0;
	int Sub_Plus_Hits,Sub_Minus_Hits;
	float Top_Score=FLT_MAX,Sub_Opt_Score=FLT_MAX,Sub_BQScore=FLT_MAX;
	float Top_BQScore=FLT_MAX;bool Close_Hits=false;
	Top_Penalty=0;
	if(Last_Mis==0)
	{
		SARange SA=MF.Exact_Match_Forward[L.LH-1];
		if(SA.Start)//Get stat for later mapQ calculation..
		{
			if (SA.Skip)
				Top_Penalty++;
			else
			Top_Penalty=SA.End-SA.Start;
		}
		SA=MC.Exact_Match_Forward[L.LH-1];
		if(SA.Start)
		{
			if (SA.Skip)
				Top_Penalty++;
			else
			Top_Penalty+=(SA.End-SA.Start);
		}
	}

	if(Plus_Hits+Minus_Hits==1 && Last_Mis!=End_Mis && !HEURISTIC)
	{
		bool Deep_Scan=true;
		if(BOOST>=3)
		{
			if(BOOST>=4)
				Deep_Scan=false;
			else
			{
				if(Last_Mis>=2)
					Deep_Scan=false;

			}

		}

		if(Deep_Scan)
		{
			Sub_OptMIS= Scan(batmeth1,MF,MC,Last_Mis+1,L,fwfmi,revfmi,Last_Mis+1,Head_Top_Count,(BOOST>=2)? 1:maxhits);//2);
		}
		else
		{
			Sub_OptMIS=-1;
		}

		if(Sub_OptMIS != -1) 
		{
			Close_Hits=true;
			Sub_Plus_Hits=MF.Hit_Array_Ptr-Plus_Hits-2;Sub_Minus_Hits=MC.Hit_Array_Ptr-Minus_Hits-2;
			if(!Check_Subopt(Plus_Hits,Minus_Hits,Last_Mis,Sub_OptMIS,R,L.STRINGLENGTH,MF,MC,Top_Score,Top_BQScore,Sub_Opt_Score,Sub_BQScore,Quality_Score))
			{
				if(Recover_With_SW(RawR,Original_Text,revfmi,Plus_Hits,Minus_Hits,R,B,L.STRINGLENGTH,MF,MC,Quality_Score,Conversion_Factor,Alignments,Good_Alignments,H)) 
				{
					H.Status=SW_RECOVERED;
					Last_Mis=H.Mismatch;Top_Score=H.Score;
					return true;
				}
				else
					H.Status=UNRESOLVED_HIT;
					Last_Mis=H.Mismatch;
					return true;
			}
		}
		MF.Hit_Array_Ptr=Plus_Hits+1,MC.Hit_Array_Ptr=Minus_Hits+1;
	}
	if (Last_Mis == End_Mis || Sub_OptMIS== -1)
	{
		Top_Score= Calc_Top_Score(MF,MC,Top_BQScore,Last_Mis,L.STRINGLENGTH,Plus_Hits,Minus_Hits,R);
		Quality_Score=30;
	}
	Alignment Aln;Aln.Realigned=NO;
	if(Last_Mis!= -1)
	{
		H.Loc=UINT_MAX;
	}	
	else 
	{
	       H.Status=UNMAPPED;
	}

	char Multi_Hit[MULTIHIT_COUNT],Unique='R';
	H.MH=Multi_Hit+1;Multi_Hit[0]='\t';
	bool RESCUE_MULTI=true;H.Sub_Opt_Hit=0;
	char SW_Status=NO_SW_SCAN;//In multi scan can differentialte hit sufficiantly..

	if(Last_Mis >=0)//If mismatch hits..
	{
		//if(!HEURISTIC) MAX_SW=INT_MAX;
		if(Plus_Hits+Minus_Hits==1)//Unique hits 
		{
			if(Unique_Hits(Plus_Hits,Minus_Hits,MF.Hit_Array[0],MC.Hit_Array[0]))
			{
				//R.Real_Len=150;//QW added;
				if(1 || L.STRINGLENGTH<R.Real_Len)//if long read then extend on the fly..
				{
					if(Extend_With_SW(RawR,Original_Text,revfmi,Plus_Hits,Minus_Hits,R,B,L.STRINGLENGTH,MF,MC,Quality_Score,Conversion_Factor,Alignments,Good_Alignments,H))
					{
						Last_Mis=H.Mismatch;Top_Score=H.Score;Top_BQScore=H.BQScore;
						if(Close_Hits)
							H.Status=UNIQUEHIT;
						else
							H.Status=SHARP_UNIQUEHIT;
					}
					else
						assert(false);
				}
				else
				{
				Get_Info(revfmi,MF,MC,L.STRINGLENGTH,H,Conversion_Factor);//obtain hit details in H;
				Unique='U';Multi_Hit[0]=0;
				if(Close_Hits)
					H.Status=UNIQUEHIT;
				else
					H.Status=SHARP_UNIQUEHIT;
				}
			}
			else
			{
				if(Recover_With_SW(RawR,Original_Text,revfmi,Plus_Hits,Minus_Hits,R,B,L.STRINGLENGTH,MF,MC,Quality_Score,Conversion_Factor,Alignments,Good_Alignments,H)) 
				{
					H.Status=SW_RECOVERED;
					Last_Mis=H.Mismatch;Top_Score=H.Score;Top_BQScore=H.BQScore;
					return true;
				}
				Get_Info(revfmi,MF,MC,L.STRINGLENGTH,H,Conversion_Factor);//obtain hit details in H;
				Last_Mis=H.Mismatch;
				H.Sub_Opt_Hit=H.Loc;H.Status=MULTI_HIT;
				Quality_Score=0;
			}
		}
		else if (RESCUE_MULTI && Phred_Check_Multi(Plus_Hits,Minus_Hits,Last_Mis,R,L.STRINGLENGTH,MF,MC,Top_Score,Quality_Score,SW_Status))
		{
			if(SW_Status==NO_SW_SCAN && Unique_Hits(Plus_Hits,Minus_Hits,MF.Hit_Array[0],MC.Hit_Array[0]))
			{
				//assert(false);
				if(L.STRINGLENGTH<R.Real_Len)//if long read then extend on the fly..
				{
					if(Extend_With_SW(RawR,Original_Text,revfmi,Plus_Hits,Minus_Hits,R,B,L.STRINGLENGTH,MF,MC,Quality_Score,Conversion_Factor,Alignments,Good_Alignments,H))
					{
						Last_Mis=H.Mismatch;Top_Score=H.Score;Top_BQScore=H.BQScore;
						H.Status=MULTI_HIT;
					}
					else
						assert(false);
				}
				else
				{
					Get_Info(revfmi,MF,MC,L.STRINGLENGTH,H,Conversion_Factor);//obtain hit details in H;
					Unique='U';Multi_Hit[0]=0;
					H.Mismatch=Last_Mis;H.Status=MULTI_HIT;
					H.Score= Top_Score;
					H.BQScore=Top_BQScore;
				}
			}
			else
			{
				if(Recover_With_SW(RawR,Original_Text,revfmi,Plus_Hits,Minus_Hits,R,B,L.STRINGLENGTH,MF,MC,Quality_Score,Conversion_Factor,Alignments,Good_Alignments,H)) 
				{
					H.Status=SW_RECOVERED;
					Last_Mis=H.Mismatch;Top_Score=H.Score;Top_BQScore=H.BQScore;
				}
				else 
				{
					Get_Info(revfmi,MF,MC,L.STRINGLENGTH,H,Conversion_Factor);//obtain hit details in H;
					Last_Mis=H.Mismatch;
					H.Sub_Opt_Hit=H.Loc;H.Status=MULTI_HIT;
				}
			}
		}
		else 
		{
			//Print_Mishit(R,Mishit_File);
			//Print_Blank_Line(Single_File,R);
		}

		H.Mismatch=Last_Mis;
		H.Score= Top_Score;
		H.BQScore=Top_BQScore;
		H.Sub_BQScore=Sub_BQScore;
		//Alignment Tmp_A;
		//Copy_H_to_A(Tmp_A,H);
		//Alignments.push(Tmp_A);
	}
	else
	{
		MAX_SW=MAX_SW_sav;
	}
	return true;
}

Alignment RealignFast(READ & RawR,unsigned char* Original_Text,Hit_Info &  H,BATREAD & Read,int StringLength, READ & R,int OFF,int Filter,bool Do_Filter)
{
	Alignment A;
	if(!FASTSW)
	{
		A.Score=INT_MAX;
		return A; 
	}
	char Org_String[ORGSTRINGLENGTH];
	char Org_String_Ori[200];
	char *Quality,Rev_Qual[MAXTAG];
	Cigar_Info Cig_Info;


	char Real_String[R.Real_Len],*Current_Tag;
	if(R.Real_Len>=StringLength)
	{
		R.Tag_Copy[R.Real_Len]=0;
		R.Quality[R.Real_Len]=0;
		if(H.Sign=='+')
		{
			Read2Bin(Real_String,R.Tag_Copy,R.Real_Len);
			Quality=R.Quality;
		}
		else
		{
			assert(false);
		}
		StringLength=R.Real_Len;
	}
	Current_Tag=R.Tag_Copy;

	Get_Bases(Original_Text/*Original_Text_Ori*/,H.Org_Loc,StringLength+INDELGAP,Org_String_Ori);
	Get_Bases(Original_Text,H.Org_Loc,StringLength+INDELGAP,Org_String);
	for(int i=0;i<StringLength+INDELGAP;i++)
	{
		*(Org_String+i)="ACGT"[*(Org_String+i)];
	}
	*(Org_String+StringLength+INDELGAP)=0;
	
	for(int i=0;i<StringLength+INDELGAP;i++)
	{
		*(Org_String_Ori+i)="ACGT"[*(Org_String_Ori+i)];
	}
	*(Org_String_Ori+StringLength+INDELGAP)=0;

	A=Fast_SW(Org_String,Current_Tag,Quality,OFF,'+',Org_String_Ori,RawR);
	//A=Fast_SW(Org_String,Current_Tag,Quality,OFF,'+');
	if(A.SW_Score<match*9*StringLength/10) 
		A.Score=INT_MAX;
	A.Loc=H.Org_Loc+1;// moxian test //after compare, remove plus 1.

	A.Sign=H.Sign;
	return A;
	
}

Alignment RealignFastMinus(READ& RawR,unsigned char* Original_Text,Hit_Info &  H,BATREAD & Read,int StringLength, READ & R,int OFF,int Filter,bool Do_Filter)
{
	Alignment A;
	if(!FASTSW)
	{
		A.Score=INT_MAX;
		return A; 
	}
	char Org_String[200];
	char Org_String_Ori[200];
	char *Quality;

	char Real_String[R.Real_Len],*Current_Tag;
	if(R.Real_Len>=StringLength)
	{
		R.Tag_Copy[R.Real_Len]=0;
		R.Quality[R.Real_Len]=0;
		if(H.Sign=='+')
		{
			assert(false);
		}
		else
		{
			Quality=R.Quality;
			H.Org_Loc++;
		}
		StringLength=R.Real_Len;
	}
	Current_Tag=R.Tag_Copy;

	Get_Bases(Original_Text,H.Org_Loc-INDELGAP,StringLength+INDELGAP,Org_String);//get ref bases and convert..
	Get_Bases(Original_Text/*Original_Text_Ori*/,H.Org_Loc-INDELGAP,StringLength+INDELGAP,Org_String_Ori);//momo
	char Org_Rev[StringLength+INDELGAP+1];
	for(int i=StringLength+INDELGAP-1,j=0;i>=0;i--,j++)
	{
		Org_Rev[j]="ACGT"[3-Org_String[i]];
	}
	Org_Rev[StringLength+INDELGAP]=0;
	
	char Org_Rev_Ori[StringLength+INDELGAP+1];
	for(int i=StringLength+INDELGAP-1,j=0;i>=0;i--,j++)
	{
		Org_Rev_Ori[j]="ACGT"[3-Org_String_Ori[i]];
	}
	Org_Rev_Ori[StringLength+INDELGAP]=0;

	A=Fast_SW(Org_Rev,Current_Tag,Quality,OFF,'-',Org_Rev_Ori,RawR);
	if(A.SW_Score<match*9*StringLength/10) 
		A.Score=INT_MAX;
	A.Loc+=H.Org_Loc;//;-1 after mapping location compare,I minus 1 //moxian changed
	A.Sign=H.Sign;
	return A;
}

Alignment RealignX(READ& RawR,unsigned char* Original_Text,Hit_Info &  H,BATREAD & Read,int StringLength, READ & R,bool Dont_Push_To_Q,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,char* Cigar,int & Clip_H,int & Clip_T,int & Filter,bool Do_Filter)
{
	s_align* Aln;
	Alignment A;
	char Org_String[500];
	char *Quality,Rev_Qual[MAXTAG];
	Cigar_Info Cig_Info;
	char Org_String_Ori[500];

	char Real_String[R.Real_Len],*Current_Tag;
	char Real_String_raw[R.Real_Len],*Current_Tag_raw;
	if(R.Real_Len>=StringLength)
	{
		R.Tag_Copy[R.Real_Len]=0;
		R.Quality[R.Real_Len]=0;
		if(H.Sign=='+')
		{
			Read2Bin(Real_String,R.Tag_Copy,R.Real_Len);
			Read2Bin(Real_String_raw,RawR.Tag_Copy,R.Real_Len);
			Quality=R.Quality;
		}
		else
		{
			Read2RevCBin(Real_String,R.Tag_Copy,R.Real_Len);
			Read2RevCBin(Real_String_raw,RawR.Tag_Copy,R.Real_Len);
			Reverse_Quality(Rev_Qual,R,R.Real_Len);
			H.Org_Loc-=(R.Real_Len-StringLength)+INDELGAP-1;
			//Offset=R.Real_Len-StringLength;
			Quality=Rev_Qual;
		}
		StringLength=R.Real_Len;
	}
	Current_Tag=Real_String;Current_Tag_raw=Real_String_raw;

	int Jump=0;if(H.Sign=='-') Jump=0 +JUMP;
		
	int8_t* mata_tmp; mata_tmp = (int8_t*)calloc(25, sizeof(int8_t));
	char state=RawR.state;if(H.Sign=='-') state=RawR.state=='1'?'2':'1';
	if(state=='1')
		init_SSW_BS_c2t(mata_tmp);
	else if(state=='2')
		init_SSW_BS_g2a(mata_tmp);
	else init_SSW(mata_tmp);
	
	s_profile* p = ssw_init((int8_t*)Current_Tag_raw, StringLength, mata_tmp, n, 1);

	Get_Bases(Original_Text,H.Org_Loc+Jump,StringLength+INDELGAP,Org_String);
	//Get_Bases(Original_Text_Ori,H.Org_Loc+Jump,StringLength+INDELGAP,Org_String_Ori);//momo
	Aln=mengyao_ssw_core(Org_String/*_Ori*/,StringLength, Current_Tag/*_raw*/,StringLength+INDELGAP,Filter,0/*DP*/, p);
	//if(Aln->score1 >= ACC_SCORE)
	if(Aln->score1 >= Filter)
	{
		A.Clip_H=Aln->read_begin1;A.Clip_T=0;
		if(Aln->read_end1!=StringLength-1) A.Clip_T=StringLength-1-Aln->read_end1;

		A.SW_Score=Aln->score1;
		ssw_cigar_processQ(H.Sign,RawR,Org_String_Ori,Aln,Cig_Info,Org_String,Aln->ref_begin1,Current_Tag,Aln->read_begin1,StringLength,Quality,A.Cigar,A.Clip_H,A.Clip_T);
		A.Loc=H.Org_Loc+Jump+Aln->ref_begin1;//+Offset;
		A.Score= -Cig_Info.Score;
		A.QScore=Cig_Info.QScore;
		A.BQScore=Cig_Info.BQScore;
		A.Mismatch=Cig_Info.Mis;
		A.Indel=Cig_Info.Indel_Count;
		A.Sign=H.Sign;
		A.Realigned=1;
		if(Do_Filter && BOOST && Aln->score1 >Filter) 
		{
			Filter=Aln->score1;
		}
		if(!Dont_Push_To_Q)
		{
			Good_Alignments.push(A);
		}
	}
	else
	{
		A.Loc=INT_MAX;
		if(BOOST && Aln->score1>=0)
		{
			A.Score= -INT_MIN;
			A.Realigned=1;
			if(!Dont_Push_To_Q)
			{
				Good_Alignments.push(A);
			}
		}
	}
	align_destroy(Aln);
	init_destroy(p); 
	return A;
	
}

inline void Copy_H_to_A(Alignment & A,Hit_Info & H)
{
	A.Mismatch=H.Mismatch;
	A.Indel=H.Indel;
	A.Score=-H.Score;
	A.BQScore=H.BQScore;
	A.QScore=H.QScore;
	A.SW_Score=H.SW_Score;
	A.Loc=H.Loc+1;
	A.Clip_H=H.Clip_H;
	A.Clip_T=H.Clip_T;
	if(H.Cigar[0])
		strcpy(A.Cigar,H.Cigar);
	else
	{
		H.Cigar[0]=0;
		H.Cigar[1]='Z';
		A.Cigar[1]='Z';
		A.Cigar[0]=0;
	}
}

inline void Copy_A_to_H(Alignment & A,Hit_Info & H)
{
	H.Mismatch=A.Mismatch;
	H.Indel=A.Indel;
	H.Score= -A.Score;
	H.QScore= A.QScore;
	H.BQScore= A.BQScore;
	H.SW_Score= A.SW_Score;
	H.Loc=H.Org_Loc= A.Loc-1;
	H.Clip_H=A.Clip_H;H.Clip_T=A.Clip_T;
	if(A.Cigar[0])
		strcpy(H.Cigar,A.Cigar);
	else
	{
		H.Cigar[0]=0;		
		H.Cigar[1]='Z';
		A.Cigar[1]='Z';
	}

}



bool Extend_With_SW(READ& RawR,unsigned char* Original_Text,BWT *revfmi,int Plus_Hits,int Minus_Hits,READ & R,BATREAD & BR, int StringLength,MEMX & MF,MEMX & MC,int & Quality_Score,unsigned Conversion_Factor,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,Hit_Info & H)
{
	int Hits=0,Clip_T,Clip_H;
	int Filter=0;
	if(Plus_Hits)
	{
		assert(!Minus_Hits);
		for(int i=0;MF.Hit_Array[i].Start;i++)
		{
			SARange SA=MF.Hit_Array[i];
			assert(SA.End-SA.Start>=0);
			Hits+=(SA.End-SA.Start+1);
			Alignment A;
			//int Anumber=0;//wrong
			//time_t Start_Time,End_Time;//wrong
			//time(&Start_Time);//wrong
			for (int j=0;j<=(SA.End-SA.Start);j++)
			{
				unsigned Loc=SA2Loc(revfmi,SA,j,Conversion_Factor);
				H.Loc =H.Org_Loc= Loc;
				H.Sign='+';
				Hit_Info H2=H;
				A=RealignFast(RawR,Original_Text,H2,BR,StringLength,R,INT_MAX,Filter,true);
				if(A.Score==INT_MAX)
				{
					RealignX(RawR,Original_Text,H,BR,StringLength,R,false,Good_Alignments,NULL,Clip_H,Clip_T,Filter,true);
				}
				else
				{
					assert(A.Score<=0);
					//Anumber++;//wrong
					A.Realigned=1;A.Clip_T=A.Clip_H=0;
					Good_Alignments.push(A);
					//if(Anumber >= 500 ) break;//wrong  || j > 500
				}
				//time(&End_Time);//wrong
				//if(difftime(End_Time,Start_Time) > 5) break;//wrong 120
			}
		}
	}
	if(Minus_Hits)
	{
		assert(!Plus_Hits);
		for(int i=0;MC.Hit_Array[i].Start;i++)
		{
			SARange SA=MC.Hit_Array[i];
			Hits+=(SA.End-SA.Start+1);
			assert(SA.End-SA.Start>=0);
		//	int Anumber=0;//wrong
		//	time_t Start_Time,End_Time;//wrong
		//	time(&Start_Time);//wrong
			for (int j=0;j<=(SA.End-SA.Start);j++)
			{

				unsigned Loc=SA2Loc(revfmi,SA,j,Conversion_Factor);
				H.Loc =H.Org_Loc= Loc;H.Sign='-';
				Hit_Info H2=H;
				H2.Loc =H2.Org_Loc= Loc-(R.Real_Len-SEEDSIZE);
				Alignment B=RealignFastMinus(RawR,Original_Text,H2,BR,StringLength,R,INT_MAX,Filter,true);
				if(B.Score==INT_MAX)
				{
					Alignment A=RealignX(RawR,Original_Text,H,BR,StringLength,R,false,Good_Alignments,NULL,Clip_H,Clip_T,Filter,true);
				}
				else
				{
					assert(B.Score<=0);
				//	Anumber++;
					B.Realigned=1;B.Clip_T=B.Clip_H=0;
					Good_Alignments.push(B);
				//	if(Anumber >= 500) break; // || j > 500
				}
			//	time(&End_Time);//wrong
			//	if(difftime(End_Time,Start_Time) > 5) break;//wrong //120
			}
		}
	}
	if(!Good_Alignments.empty())
	{
		Alignment A=Good_Alignments.top();
		assert(Good_Alignments.size()==1);//there is a unique best alignment
		Copy_A_to_H(A,H);
		Alignments.push(A);
		return true;
	}
	else
	{
		return false;
	}
}
//Tries to recover sw hits from multi hits. returns false if unmappable..
bool Recover_With_SW(READ& RawR,unsigned char* Original_Text,BWT *revfmi,int Plus_Hits,int Minus_Hits,READ & R,BATREAD & BR, int StringLength,MEMX & MF,MEMX & MC,int & Quality_Score,unsigned Conversion_Factor,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,Hit_Info & H)
{
	int Hits=0;
	int Filter=0;
	if(Plus_Hits+Minus_Hits==1)
	{
		if(Plus_Hits)//Skip past sentinel..
		{
			if(MF.Hit_Array_Ptr>1)
				for(int i=1;MF.Hit_Array[i+1].Start;i++)
				{
					MF.Hit_Array[i]=MF.Hit_Array[i+1];
				}
		}
		else
		{
			if(MC.Hit_Array_Ptr>1)
				for(int i=1;MC.Hit_Array[i+1].Start;i++)
				{
					MC.Hit_Array[i]=MC.Hit_Array[i+1];
				}
		}
	}

	//might be suboptimal only hits..
	if(!Plus_Hits)
	{
		if(MF.Hit_Array_Ptr>1)
			for(int i=0;MF.Hit_Array[i+1].Start;i++)
			{
				MF.Hit_Array[i]=MF.Hit_Array[i+1];
			}
		Plus_Hits++;
	}
	if(!Minus_Hits)
	{
		if(MC.Hit_Array_Ptr>1)
			for(int i=0;MC.Hit_Array[i+1].Start;i++)
			{
				MC.Hit_Array[i]=MC.Hit_Array[i+1];
			}
		Minus_Hits++;
	}

	int Clip_T,Clip_H;
	if(Plus_Hits)
	{
		for(int i=0;MF.Hit_Array[i].Start;i++)
		{
			SARange SA=MF.Hit_Array[i];
			assert(SA.End-SA.Start>=0);
			Hits+=(SA.End-SA.Start+1);
//			int Anumber=0;
//			time_t Start_Time,End_Time;//wrong
//			time(&Start_Time);//wrong
			for (int j=0;j<=(SA.End-SA.Start);j++)
			{
				unsigned Loc=SA2Loc(revfmi,SA,j,Conversion_Factor);
				H.Loc =H.Org_Loc= Loc;
				H.Sign='+';
				Hit_Info H2=H;
				Alignment A;
				A=RealignFast(RawR,Original_Text,H2,BR,StringLength,R,INT_MAX,Filter,true);
				if(A.Score==INT_MAX)
				{
					RealignX(RawR,Original_Text,H,BR,StringLength,R,false,Good_Alignments,NULL,Clip_H,Clip_T,Filter,true);
				}
				else
				{
					assert(A.Score<=0);
//					Anumber++;
					A.Realigned=1;
					Good_Alignments.push(A);
//					if(Anumber >= 500 ) break; //|| j > 500
				}
//				time(&End_Time);//wrong
//				if(difftime(End_Time,Start_Time) > 5) break;//wrong 120
			}
		}
	}
	if(Minus_Hits)
	{
		for(int i=0;MC.Hit_Array[i].Start;i++)
		{
			SARange SA=MC.Hit_Array[i];
			Hits+=(SA.End-SA.Start+1);
			assert(SA.End-SA.Start>=0);
//			int Anumber=0;
//			time_t Start_Time,End_Time;//wrong
//			time(&Start_Time);//wrong
			for (int j=0;j<=(SA.End-SA.Start);j++)
			{
				unsigned Loc=SA2Loc(revfmi,SA,j,Conversion_Factor);
				H.Loc =H.Org_Loc= Loc;
				H.Sign='-';
				Hit_Info H2=H;
				H2.Loc =H2.Org_Loc= Loc-(R.Real_Len-SEEDSIZE);
				Alignment B=RealignFastMinus(RawR,Original_Text,H2,BR,StringLength,R,INT_MAX,Filter,true);
				if(B.Score==INT_MAX)
				{
					Alignment A=RealignX(RawR,Original_Text,H,BR,StringLength,R,false,Good_Alignments,NULL,Clip_H,Clip_T,Filter,true);
					//RealignX(H,BR,StringLength,R,false,Alignments,Good_Alignments,NULL,Clip_H,Clip_T,Filter,true);
				}
				else
				{
					assert(B.Score<=0);
//					Anumber++;
					B.Realigned=1;B.Clip_T=B.Clip_H=0;
					Good_Alignments.push(B);
//					if(Anumber >= 500 ) break; //|| j > 500
				}
//				time(&End_Time);//wrong
//				if(difftime(End_Time,Start_Time) > 5) break;//wrong 120
			}
		}
	}
	if(!Good_Alignments.empty())
	{
		Alignment A=Good_Alignments.top();
		if(Good_Alignments.size()==1)//there is a unique best alignment
		{
			A.Sub_Opt_Score=INT_MAX;
		}
		else
		{
			Good_Alignments.pop();
			Alignment B=Good_Alignments.top();
			Alignments.push(B);
			Good_Alignments.push(A);//
			A.Sub_Opt_Score= -B.Score;
		}
		Copy_A_to_H(A,H);
		Alignments.push(A);
		return true;
	}
	else
	{
		return false;
	}
}

unsigned SA2Loc(BWT *revfmi,SARange S,int Pos,unsigned Conversion_Factor)
{
	if (S.Start==S.End) 
	{
		return S.Start;
	}
	else
	{
		assert (S.Start);
		return Conversion_Factor-BWTSaValue(revfmi,S.Start+Pos);
	}
	
}

void Launch_Threads(int NTHREAD, void* (*Map_t)(void*),Thread_Arg T)
{
	Threading* Thread_Info=(Threading*) malloc(sizeof(Threading)*NTHREAD);
	int Thread_Num=0;
	pthread_attr_t Attrib;
	pthread_attr_init(&Attrib);
	pthread_attr_setdetachstate(&Attrib, PTHREAD_CREATE_JOINABLE);

	for (int i=0;i<NTHREAD;i++)
	{
		T.ThreadID=i;
		Thread_Info[i].Arg=T;
		//if(!(Thread_Info[i].r=pthread_create(&Thread_Info[i].Thread,NULL,Map_t,(void*) &Thread_Info[i].Arg))) Thread_Num++;
		Thread_Info[i].r=pthread_create(&Thread_Info[i].Thread,&Attrib,Map_t,(void*) &Thread_Info[i].Arg);
		if(Thread_Info[i].r) {fprintf(stderr,"Launch_Threads():Cannot create thread..\n");exit(-1);} else Thread_Num++;
	}
	fprintf(stderr,"%d Threads runnning ...\n",Thread_Num);
	pthread_attr_destroy(&Attrib);

	for (int i=0;i<NTHREAD;i++)
	{
		pthread_join(Thread_Info[i].Thread,NULL);
	}
}

void Mode_Parameters(BATPARAMETERS BP)
{
	if (BP.SCANMODE==VERYSENSITIVE)
	{
		MODE=VERYSENSITIVE;
		CUT_MAX_SWALIGN=INT_MAX;
	}
	else if(BP.SCANMODE==SENSITIVE)
	{
		MODE=SENSITIVE;
		MAX_SW=1000;
		CUT_MAX_SWALIGN=1000;
	}
	else if(BP.SCANMODE==FAST)
	{
		MODE=FAST;
		CUT_MAX_SWALIGN=1000;
	}
	else if(BP.SCANMODE==VERYFAST)
	{
		MODE=VERYFAST;
		//MAX_SW=200;
		CUT_MAX_SWALIGN=1000;
	}

}

void Set_Force_Indel(bool & Force_Indel,int Last_Mis,Hit_Info & H,int Avg_Q)
{
	if(H.Indel || Last_Mis>1)
	{
		if(MODE<=FAST)
		{
			if(H.Indel+Last_Mis>1) 
			{
				if(Last_Mis)
				{
					int Avg_Score=(H.QScore)/(Last_Mis);
					if(Avg_Score>=Avg_Q)
						Force_Indel=true;
				}
			}
		}
		else
		{
			int Mis_Limit;
			if(MODE==VERYSENSITIVE) Mis_Limit=0; else Mis_Limit=1;
			if(Last_Mis>Mis_Limit) 
			{
				if(Last_Mis)
				{
					int Avg_Score=(H.QScore)/(Last_Mis);
					if(Avg_Score>=Avg_Q)
						Force_Indel=true;
				}
			}
			if(H.Indel)
			{
				Force_Indel=true;
			}
		}
	}
}

int Calculate_Average_Quality(READ & R)
{
	int Avg=0;
	for(int i=0;i<R.Real_Len;i++)
	{
		Avg+=R.Quality[i]-QUALITYCONVERSIONFACTOR;
	}
	Avg=Avg/R.Real_Len;
	return Avg/3;
}


void Set_Affinity()
{
	cpu_set_t Set;
	CPU_ZERO(&Set);

	if(sched_getaffinity(0,sizeof(cpu_set_t),&Set)<0)
	{
		fprintf(stderr,"Affinity could not be get..\n");
	}
	else
	{
		for (int i=0;i<CPU_SETSIZE;i++)
		{
			if(CPU_ISSET(i,&Set))
			{
				//printf("Bound to %d\n",i);
				CPU_ZERO(&Set);
				CPU_SET(i, &Set);
				if(sched_setaffinity(0, sizeof(Set), &Set)<0)
				{
					fprintf(stderr,"Affinity could not be set..\n");
				}
				return;
			}
		}
	}

}

bool Report_Single(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,READ & RawR,unsigned char* Original_Text,char source,READ & R,FILE* Single_File,const int StringLength,BATREAD & Read,bool & Print_Status,int Clip_H,int Clip_T,Alignment & A)
{
	Ann_Info Ann;
	Hit_Info H;
	assert(A.QScore!=INT_MAX);
	H.Org_Loc=A.Loc;H.Loc = A.Loc;H.Sign=A.Sign;H.QScore=A.QScore;H.Status=UNIQUEHIT;
	//A.Score= -A.Score;
	Location_To_Genome(H.Loc,Ann);H.Chr=Ann.Name;
	assert(A.Realigned);
	if (H.Loc+StringLength <= Ann.Size)
	{
		Cigar_Check_And_Print(RawR,Final_Alignments,Original_Text,source,H,Read,StringLength,Single_File,R,true,30,A,A.Clip_H,A.Clip_T,A.Cigar);Print_Status=true;
	}
}

void Map_One_Seg(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,READ & RawR,bool batmeth1,RQINDEX & RQHALF,RQINDEX & RQ,unsigned char* Original_Text,unsigned Entries,char source,BWT *fwfmi,BWT *revfmi,READ & R,BATREAD & B,unsigned & Conversion_Factor,MEMX & MF,MEMX & MC,MEMX & MFLH,MEMX & MCLH,MEMX & MFLT,MEMX & MCLT,MEMX & MFH,MEMX & MCH,MEMX & MFT,MEMX & MCT,LEN & L,LEN & L_Main,LEN & L_Half,LEN & L_Third,unsigned & Actual_Tag,FILE* Single_File,FILE* Mishit_File,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,Pair* & Pairs,bool PRINT,Hit_Info & H,int & Quality_Score,int Segment_Length,int SEG_SIZE)
	
{
		READ RTemp=R;BATREAD BTemp=B;
		char T_EOS=R.Tag_Copy[SEG_SIZE+1],T_EOQ=R.Quality[SEG_SIZE+1];
		READ RawRTemp=RawR;
		char T_EOS_raw=RawR.Tag_Copy[SEG_SIZE+1],T_EOQ_raw=RawR.Quality[SEG_SIZE+1];
		if(SEG_SIZE)
		{
			R.Tag_Copy[SEG_SIZE+1]=0;
			RawR.Tag_Copy[SEG_SIZE+1]=0;
		}
		R.Real_Len=0;
		for(;R.Tag_Copy[R.Real_Len]!=0 && R.Tag_Copy[R.Real_Len]!='\n' && R.Tag_Copy[R.Real_Len]!='\r';R.Real_Len++);
		RawR.Real_Len=R.Real_Len;
		if(R.Tag_Copy[R.Real_Len] == '\n' || R.Quality[R.Real_Len] == '\r'  ) R.Tag_Copy[R.Real_Len]='\0';
		R.Quality[R.Real_Len]='\0';RawR.Quality[R.Real_Len]='\0';
		if(Segment_Length)
		{
			R.Real_Len=RawR.Real_Len=Segment_Length;
		}
		
		int Avg_Q=std::min(13,Calculate_Average_Quality(R));
		//int Avg_Q=Calculate_Average_Quality(R);
		Conversion_Factor=revfmi->textLength-L.STRINGLENGTH;
		IN.Positive_Head=R.Tag_Copy;
		R.Tag_Number=1;R.Read_Number=Actual_Tag;

		Process_Read(RawR,R,B,MF,MC);
		MF.Strand='+';MF.Larger_Than_Ten=0;MC.Strand='-';MC.Larger_Than_Ten=0;
		MF.Extend=MC.Extend=false;

		Process_Read(RawR,R,B,MFLH,MCLH);
		MFLH.Strand='+';MFLH.Larger_Than_Ten=0;MCLH.Strand='-';MCLH.Larger_Than_Ten=0;
		MFLH.L=MCLH.L=L_Half;

		Process_Read(RawR,R,B,MFLT,MCLT);
		MFLT.Strand='+';MFLT.Larger_Than_Ten=0;MCLT.Strand='-';MCLT.Larger_Than_Ten=0;
		MFLT.L=MCLT.L=L_Half;

		READ R_Head;BATREAD B_Head;READ Raw_R_Head;
		for (int i=0;i<L.STRINGLENGTH/3;i++) R_Head.Tag_Copy[i]=R.Tag_Copy[i]; 
		for (int i=0;i<L.STRINGLENGTH/3;i++) Raw_R_Head.Tag_Copy[i]=RawR.Tag_Copy[i]; 
		B_Head.StringLength=L.STRINGLENGTH/3;B_Head.NCount=0;B_Head.IGNOREHEAD=0;
		Process_Read(Raw_R_Head,R_Head,B_Head,MFH,MCH);
		MFH.Strand='+';MFH.Larger_Than_Ten=0;MCH.Strand='-';MCH.Larger_Than_Ten=0;MFH.Extend=MCH.Extend=false;
		MFH.L=MCH.L=L_Third;

		READ R_Tail;BATREAD B_Tail;READ Raw_R_Tail;
		for (int i=0;i<L.STRINGLENGTH/3;i++) R_Tail.Tag_Copy[i]=R.Tag_Copy[i+L.STRINGLENGTH-L.STRINGLENGTH/3]; 
		for (int i=0;i<L.STRINGLENGTH/3;i++) Raw_R_Tail.Tag_Copy[i]=RawR.Tag_Copy[i+L.STRINGLENGTH-L.STRINGLENGTH/3];
		B_Tail.StringLength=L.STRINGLENGTH/3;B_Tail.NCount=0;B_Tail.IGNOREHEAD=0;
		if(B_Tail.StringLength!=0) Process_Read(Raw_R_Tail,R_Tail,B_Tail,MFT,MCT);
		MFT.Strand='+';MFT.Larger_Than_Ten=0;MCT.Strand='-';MCT.Larger_Than_Ten=0;MFT.Extend=MCT.Extend=false;
		MFT.L=MCT.L=L_Third;


		FreeQ(Alignments);
		FreeQ(Good_Alignments);
		if (R.NCount > NCOUNT) 
		{
			//Print_Blank_Line(Single_File,R);
			R.Tag_Copy[SEG_SIZE+1]=T_EOS;R.Quality[SEG_SIZE+1]=T_EOQ;
			RawR.Tag_Copy[SEG_SIZE+1]=T_EOS_raw;RawR.Quality[SEG_SIZE+1]=T_EOQ_raw;
			R=RTemp=R;B=BTemp;
			RawR=RawRTemp;
			return;
		}

		int Last_Mis;
		int Head_Top_Count,Tail_Top_Count;//# of top hits in H/T
		int Head_Subopt_Count,Tail_Subopt_Count;//# of top hits in H/T
		Quality_Score=QUAL_UNMAPPED;H.Status=UNMAPPED;
		if(Do_Mismatch_Scan(RawR,batmeth1,Original_Text,MF,MC,L,fwfmi,revfmi,0,Inter_MM,Last_Mis,Head_Top_Count,H,Quality_Score,R,B,Mishit_File,Single_File,Conversion_Factor,Alignments,Good_Alignments))
		{ 
			assert(Last_Mis==-1 || H.Cigar[0]|| H.Cigar[1]=='Z'||L.STRINGLENGTH==R.Real_Len);
			assert(L.STRINGLENGTH==R.Real_Len || Last_Mis== -1 || H.QScore != -1);
			bool Force_Indel=false;
			Set_Force_Indel(Force_Indel,Last_Mis,H,Avg_Q);
			int Err=0;
			//if(Last_Mis>=Inter_MM-1 && TESTMODE)  Last_Mis=Inter_MM;
			//if(H.Indel || H.Score >=gap_extension+gap_open)  Force_Indel=true;
			if(MODE>=SENSITIVE && 0)
			{
				if(H.Score >= gap_extensionP+gap_openP && Last_Mis!=Inter_MM)  Force_Indel=true;
			}
			if(Last_Mis==Inter_MM) Force_Indel=false;

			if(BEST || (H.Status == UNMAPPED) || Last_Mis==Inter_MM || Force_Indel)
			{
				Max_MM_GAP_Adjust=Max_MM_GAP/2;
				Err=Do_Indel(RawR,batmeth1,RQHALF,RQ,Original_Text,Entries,fwfmi,revfmi,MFLH,MCLH,MFLT,MCLT,MFH,MCH,MFT,MCT,L.STRINGLENGTH,Pairs,Single_File,R,Alignments,H);
				if(Alignments.empty() && (Last_Mis<0) && (Inter_MM<Max_MM))//No hits so far..
				{
					assert(Last_Mis!=Inter_MM);
					Do_Mismatch_Scan(RawR,batmeth1,Original_Text,MF,MC,L,fwfmi,revfmi,Inter_MM+1,Max_MM,Last_Mis,Head_Top_Count,H,Quality_Score,R,B,Mishit_File,Single_File,Conversion_Factor,Alignments,Good_Alignments);
				}
			}
			if(!PRINT && !Good_Alignments.empty())
			{
				R.Tag_Copy[SEG_SIZE+1]=T_EOS;R.Quality[SEG_SIZE+1]=T_EOQ;
				R=RTemp=R;B=BTemp;
				RawR=RawRTemp;
				return;
			}
			else
			{	
				std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> TAlignments;
				TAlignments=Alignments;
				if(!Report_SW_Hits(Final_Alignments,RawR,Original_Text,source,Err,R,Single_File,L.STRINGLENGTH,B,H,Quality_Score,Alignments,Good_Alignments,Force_Indel,PRINT))
				{
				//	Print_Blank_Line(Single_File,R);
				}
				Alignments=TAlignments;
			}
		}
		R.Tag_Copy[SEG_SIZE+1]=T_EOS;R.Quality[SEG_SIZE+1]=T_EOQ;
		RawR.Tag_Copy[SEG_SIZE+1]=T_EOS_raw;RawR.Quality[SEG_SIZE+1]=T_EOQ_raw;
		R=RTemp=R;B=BTemp;RawR=RawRTemp=RawR;
}

void Get_Basic_MapQ(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,Alignment & C1, Alignment & C2,int & MapQ)
{
	MapQ=1;//printf("\n%d %d\n",C1.Score,C2.Score);
	if(!Good_Alignments.empty())
	{
		C1=Good_Alignments.top();C1.Score= -C1.Score;
		Good_Alignments.pop();
		if(!Good_Alignments.empty())
		{
			C2=Good_Alignments.top();C2.Score= -C2.Score;
			//if(abs(C1.Score-C2.Score)<=3) //moxian 10
			//	MapQ=0;
		}
		Good_Alignments.push(C1);
	}
	else
		MapQ= -1;
}

void Adjust_Alignments(READ& RawR,unsigned char* Original_Text,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,int Offset,READ & RTemp, BATREAD & BTemp)
{
	std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> A;

	while(!Alignments.empty())
	{
		Pop_And_Realign(RawR,Original_Text,Alignments,A,Offset,RTemp,BTemp);
	}
	Alignments=A;
}

void Pop_And_Realign(READ& RawR,unsigned char* Original_Text,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & A,int Offset,READ & RTemp, BATREAD & BTemp)
{
	int Filter=0,ClipT,ClipH;
	char CIG2[MAX_SIGLEN];
	Hit_Info H;

	Alignment Aln=Alignments.top();
	Alignments.pop();
	if(Aln.Realigned==NO)
		Aln.Clip_H=Aln.Clip_T=0;

	H.Org_Loc=Aln.Loc;H.Sign=Aln.Sign;
	if(H.Sign=='+')
	{
		H.Org_Loc-=Offset;
		if(Aln.Clip_H<=2)
			H.Org_Loc-=Aln.Clip_H;
	}
	else
	{
		H.Org_Loc+=Offset;
		if(Aln.Clip_T<=2)
			H.Org_Loc+=Aln.Clip_T;
	}
	RealignX(RawR,Original_Text,H,BTemp,75,RTemp,false,A,CIG2,ClipH,ClipT);
}

bool SW_List(READ& RawR,unsigned char* Original_Text,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Good_Alignments,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Alignments,int Offset,READ & RTemp, BATREAD & BTemp,bool & List_Exceeded,int & LS)
{
	int Enum_Hits=0,Top_Hits=0;
	int List_Size=Good_Alignments.size();
	int Top_Score;
	LS+=List_Size;
	bool Top_Scanned=false;
	for(int i=0;Enum_Hits<MAX_PER_LIST && !Good_Alignments.empty();i++)
	{
		if(List_Size>=MAX_PER_LIST && !Top_Scanned)
		{
			if(i==0)
				Top_Score=Good_Alignments.top().Score;
			else if(abs(Good_Alignments.top().Score-Top_Score)>10)
			{
				Top_Scanned=true;
				Enum_Hits=Top_Hits;
			}
				//break;
		}
		else
			Enum_Hits++;
		LS--;
		Pop_And_Realign(RawR,Original_Text,Good_Alignments,Alignments,Offset,RTemp,BTemp);
	}
	if(Enum_Hits==MAX_PER_LIST)
	{
		List_Exceeded=true;
		return true;
	}
	else
	{
		List_Exceeded=false;
		return false;
	}
}
/* Given a base name, will find file names of related FM Index */
void Build_Names(const char* Genome_Name,FMFILES & F,BATPARAMETERS & BP)//build FM index file names...
{
	int Last_Dash=0;
	char* tempoptarg= (char*)Genome_Name;
	char* Name=(char*)Genome_Name;
				for(;Name[0]!=0;Name++)
				{
					if (Name[0]=='/') 
					{
						Last_Dash++;Genome_Name=Name;
					}
				}

	char* Command_Line_Buffer;

			Command_Line_Buffer=(char*)malloc(6000);
				F.REVBWTINDEX = (char*)Command_Line_Buffer;
				//printf("%s\n",F.REVBWTINDEX);
				if(Last_Dash) Last_Dash=Genome_Name-tempoptarg+1; else Genome_Name--;
				strncpy(F.REVBWTINDEX,tempoptarg,Last_Dash);
				F.REVBWTINDEX[Last_Dash+0]='r';F.REVBWTINDEX[Last_Dash+1]='e';F.REVBWTINDEX[Last_Dash+2]='v';
				strcpy(F.REVBWTINDEX+Last_Dash+3,Genome_Name+1);
				strcat(F.REVBWTINDEX+Last_Dash+3,".bwt"); 
//printf("%s\n",F.REVBWTINDEX);
				F.BWTFILE=F.REVBWTINDEX+500;
				strncpy(F.BWTFILE,tempoptarg,Last_Dash);
				strcpy(F.BWTFILE+Last_Dash,Genome_Name+1);
				strcat(F.BWTFILE+Last_Dash,".bwt"); 


				F.REVOCCFILE = F.BWTFILE+500;
				strncpy(F.REVOCCFILE,tempoptarg,Last_Dash);
				F.REVOCCFILE[Last_Dash+0]='r';F.REVOCCFILE[Last_Dash+1]='e';F.REVOCCFILE[Last_Dash+2]='v';
				strcpy(F.REVOCCFILE+Last_Dash+3,Genome_Name+1);
				strcat(F.REVOCCFILE+Last_Dash+3,".fmv"); 


				F.OCCFILE=F.REVOCCFILE+500;			
				strncpy(F.OCCFILE,tempoptarg,Last_Dash);
				strcpy(F.OCCFILE+Last_Dash,Genome_Name+1);
				strcat(F.OCCFILE+Last_Dash,".fmv"); 

				F.SAFILE=F.OCCFILE+500;			
				strncpy(F.SAFILE,tempoptarg,Last_Dash);
				strcpy(F.SAFILE+Last_Dash,Genome_Name+1);
				strcat(F.SAFILE+Last_Dash,".sa");

				F.REVSAFILE = F.SAFILE+500;
				strncpy(F.REVSAFILE,tempoptarg,Last_Dash);
				F.REVSAFILE[Last_Dash+0]='r';F.REVSAFILE[Last_Dash+1]='e';F.REVSAFILE[Last_Dash+2]='v';
				strcpy(F.REVSAFILE+Last_Dash+3,Genome_Name+1);
				strcat(F.REVSAFILE+Last_Dash+3,".sa"); 

				F.BINFILE=F.REVSAFILE+500;			
				strncpy(F.BINFILE,tempoptarg,Last_Dash);
				strcpy(F.BINFILE+Last_Dash,Genome_Name+1);
				strcat(F.BINFILE+Last_Dash,".pac");

				if(!BP.USELOCATION)
				{

					F.LOCATIONFILE=F.BINFILE+500;			
					strncpy(F.LOCATIONFILE,tempoptarg,Last_Dash);
					strcpy(F.LOCATIONFILE+Last_Dash,Genome_Name+1);
					strcat(F.LOCATIONFILE+Last_Dash,".ann.location");
				}

                                F.BLKFILE = F.LOCATIONFILE+500;
                                strncpy(F.BLKFILE,tempoptarg,Last_Dash);
                                strcpy(F.BLKFILE+Last_Dash,Genome_Name+1);
                                strcat(F.BLKFILE+Last_Dash,".blk.");

                                F.INDFILE = F.BLKFILE+500;
                                strncpy(F.INDFILE,tempoptarg,Last_Dash);
                                strcpy(F.INDFILE+Last_Dash,Genome_Name+1);
                                strcat(F.INDFILE+Last_Dash,".ind.");

                                F.RANGEFILE = F.INDFILE+500;
                                strncpy(F.RANGEFILE,tempoptarg,Last_Dash);
                                strcpy(F.RANGEFILE+Last_Dash,Genome_Name+1);
                                strcat(F.RANGEFILE+Last_Dash,".range");

                                F.NLOCATIONFILE = F.RANGEFILE+500;
                                strncpy(F.NLOCATIONFILE,tempoptarg,Last_Dash);
                                strcpy(F.NLOCATIONFILE+Last_Dash,Genome_Name+1);
                                strcat(F.NLOCATIONFILE+Last_Dash,".N.location");
}
// G-->A
inline void ReplaceGtoA(READ & R) //char* Read
{
	int i;
        for (i=0;i<R.Real_Len;i++)
        {
                if (R.Tag_Copy[i] == 'G' || R.Tag_Copy[i] == 'g') R.Tag_Copy[i]='A';
        }

        for (i=0;i<R.Real_Len;i++)
        {
                if (R.Raw_Tag_Copy[i] == 'G' || R.Raw_Tag_Copy[i] == 'g') R.Raw_Tag_Copy[i]='A';
        }
}
//C-->T
inline void ReplaceCtoT(READ & R)
{
	int i;
        for (i=0;i<R.Real_Len;i++)
        {
                if (R.Tag_Copy[i]  == 'C' || R.Tag_Copy[i] == 'c') R.Tag_Copy[i]='T';
        }

        for (i=0;i<R.Real_Len;i++)
        {
                if (R.Raw_Tag_Copy[i]  == 'C' || R.Raw_Tag_Copy[i] == 'c') R.Raw_Tag_Copy[i]='T';
        }
}
int Get_Match(const char* cig)
{
	//const char* cig=cigar.c_str();
	
	int n=0;char temp[20];
	int totalM_len=0;
	
	while(*cig!='\0')
	{
		if(*cig>='0' && *cig<='9')
		{
			temp[n]=*cig;
			cig++;n++;
		}else if(*cig=='M')
		{
			temp[n]='\0';
			int length=atoi(temp);
			totalM_len+=length;
			cig++;n=0;
		}else 
		{
			cig++;n=0;
		}
	}

	return totalM_len;
}
int Get_ED(std::string & S)//edit distance
{
	int ed=0;
	int i;
	for(i=0;i<S.size();i++)//momomo
	{
		if(S[i]=='I' || S[i]=='D') ed++;
	}
	return ed;
}
int Get_ED(const char* cig)//edit distance
{
	std::string S = cig;
	int ed=0;
	int i;
	for(i=0;i<S.size();i++)//momomo
	{
		if(S[i]=='I' || S[i]=='D') ed++;
	}
	return ed;
}
void MEM_STRUC(MEMX & MF, MEMX & MC,MEMX & MCLH,MEMX & MFLH, MEMX & MFLT,MEMX & MCLT,MEMX & MFH,MEMX & MCH,MEMX & MFT,MEMX & MCT,MEMLOOK & MLook,LEN & L)
{
	MF.L=MC.L=MFLH.L=MCLH.L=MFLT.L=MCLT.L=MFH.L=MCH.L=MFT.L=MCT.L=L;
/*	int LOOKUPSIZE=3;
	MF.Lookupsize=LOOKUPSIZE;MC.Lookupsize=LOOKUPSIZE;MFLH.Lookupsize=LOOKUPSIZE;MCLH.Lookupsize=LOOKUPSIZE;
	MFLT.Lookupsize=LOOKUPSIZE;MCLT.Lookupsize=LOOKUPSIZE;
	MFH.Lookupsize=LOOKUPSIZE;MCH.Lookupsize=LOOKUPSIZE;MFT.Lookupsize=LOOKUPSIZE;MCT.Lookupsize=LOOKUPSIZE;
*/	// initialise memory structures 
	Copy_MEM(MLook,MF,MC,MAX_MISMATCHES);
	Copy_MEM(MLook,MFLH,MCLH,MAX_MISMATCHES);
	Copy_MEM(MLook,MFLT,MCLT,MAX_MISMATCHES);
	Copy_MEM(MLook,MFH,MCH,MAX_MISMATCHES);
	Copy_MEM(MLook,MFT,MCT,MAX_MISMATCHES);
}
