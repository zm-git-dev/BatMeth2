#include <iostream> 
#include "print-s.h"
#include <cstdio>
#include "stdlib.h"
#include "assert.h"
#include <string.h> 

const int SOFTCLIPLENGTH=10;
extern int INDELGAP;
extern bool STACK_LOWQ;
int Calc_MapQ(Hit_Info & H,Alignment & A,int Clip_Count);
extern int Top_Penalty;
extern int JUMP;
extern int MAX_MISMATCHES;
int Get_ED(std::string & S);

int Find_Cigar(char* Current_Tag_raw,READ & RawR,unsigned char* Original_Text,char* Cigar,Hit_Info & H,char* Current_Tag,int StringLength,READ & R,int & Clip_H,int & Clip_T)
{
	s_align* Aln;
	Ann_Info Ann;
	char Org_String[StringLength+50];
	char Org_String_Ori[StringLength+50];
	Cigar_Info Cig_Info;

	int Jump=0;if(H.Sign=='-') Jump= 0+JUMP;
	int8_t* mata_tmp; mata_tmp = (int8_t*)calloc(25, sizeof(int8_t));
	char state=RawR.state;if(H.Sign=='-') state=RawR.state=='1'?'2':'1';
	if(state=='1')
		init_SSW_BS_c2t(mata_tmp);
	else if(state=='2')
		init_SSW_BS_g2a(mata_tmp);
	else init_SSW(mata_tmp);
	s_profile* p = ssw_init((int8_t*)Current_Tag_raw, StringLength, mata_tmp, n, 1);
	Get_Bases(Original_Text,H.Loc+Jump,StringLength+INDELGAP,Org_String);
	Get_Bases(Original_Text_Ori,H.Loc+Jump,StringLength+INDELGAP,Org_String_Ori);
	Aln=mengyao_ssw_core(Org_String_Ori,StringLength, Current_Tag_raw,StringLength+INDELGAP,0,0/*DP*/, p);///
	if(Aln->score1 >= ACC_SCORE)
	{
		H.SW_Score=Aln->score1;H.SW_Sub_Opt_Score=Aln->score2;
		Clip_H=Aln->read_begin1;Clip_T=0;
		if(Aln->read_end1!=StringLength-1) Clip_T=StringLength-1-Aln->read_end1;
		ssw_cigar_processQ(H.Sign,RawR,Org_String_Ori,Aln,Cig_Info,Org_String,Aln->ref_begin1,Current_Tag,Aln->read_begin1,StringLength,R.Quality,NULL,Clip_H,Clip_T);
		ssw_cigar_print(H.Sign,Aln->read_begin1,RawR,Org_String_Ori+Aln->ref_begin1,Aln,Cigar,Cig_Info,Org_String+Aln->ref_begin1,Current_Tag+Aln->read_begin1,Clip_H,Clip_T,StringLength);
		H.Score= -Cig_Info.Score;
		H.QScore=Cig_Info.QScore;
		H.BQScore=Cig_Info.BQScore;
		H.Mismatch=Cig_Info.Mis;
		H.Indel=Cig_Info.Indel_Count;
		//A.Sign=H.Sign;

		H.Loc=H.Loc+Jump+Aln->ref_begin1-1;
		align_destroy(Aln);
		init_destroy(p); 
		return (Cig_Info.Length+Clip_T);
	}
	align_destroy(Aln);
	init_destroy(p); 
	return 0;
}

bool Quality_Passed(Cigar_Info & C,int StringLength)
{
	if(C.Mis>5) return false;
	if(C.Length<StringLength)
	{
		if(StringLength-C.Length+C.Mis >5) return false;
		//if(StringLength-C.Length>10) return false;

	}
	return true;
}

void Reverse_Tag(char* Dest,const READ & R,int StringLength)
{
	for (int i=StringLength-1;i>=0;i--)
	{
		*Dest=Char_To_CharC[R.Tag_Copy[i]];Dest++;
	}
	*Dest=0;
}

void Read2Bin(char* Dest,char* Source,int StringLength)
{
	for (int i=0;i<StringLength;i++)
	{
		*Dest=Char_To_Code[Source[i]];Dest++;
	}
	*Dest=0;
}

void Read2RevCBin(char* Dest,char* Source,int StringLength)
{
	for (int i=StringLength-1;i>=0;i--)
	{
		*Dest=Char_To_CodeC[Source[i]];Dest++;
	}
	*Dest=0;
}

void Cigar_Check_And_Print(READ & RawR,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,unsigned char* Original_Text,char source,Hit_Info &  H,BATREAD & Read,int StringLength,FILE* Single_File,READ & R,bool Dont_Check_Quality,int Quality_Score,Alignment A,int Clip_H,int Clip_T,char* CIG)
{
	H.Loc=H.Org_Loc;
	Print_Sam(RawR,Final_Alignments,Original_Text,source,Single_File,R,H,StringLength,Quality_Score,A,Clip_H,Clip_T,CIG);
}

void Print_Sam(READ & RawR,std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,unsigned char* Original_Text,char source,FILE* Single_File,READ & R,Hit_Info & H,int StringLength,int Quality_Score,Alignment A,int TClip_H,int TClip_T,char* TCIG)
{
	int Flag=0;
	int Skip=0;//StringLength;
	char Printed_CIG[MAX_SIGLEN];
	char* Qual=R.Quality,Rev_Qual[MAXTAG];char *CIG;
	char* Tag=R.Tag_Copy,Rev_Tag[MAXTAG];
	assert(H.Loc!=UINT_MAX && A.Score >=0);
	//int Read_Len=R.Real_Len;
	int Clip_H=TClip_H,Clip_T=TClip_T;
	if(TCIG)
	{
		if(TCIG[0])
		{
			CIG=TCIG;
			if(A.Loc)
				H.SW_Score=A.SW_Score;
			H.SW_Sub_Opt_Score=0;
		}
		else //Should be bad Cigar
		{
			CIG=Printed_CIG;
			TCIG=NULL;
		}
	}
	else
	{
		assert(false);
		CIG=Printed_CIG;
	}

	int Sub_Opt_Score=0;

	if(R.Real_Len>=StringLength)
	{
		R.Tag_Copy[R.Real_Len]=0;
		R.Quality[R.Real_Len]=0;
		char Real_String[R.Real_Len];char Real_String_raw[R.Real_Len];
		if(H.Sign=='+')
		{
			if(!TCIG)
			{
				Read2Bin(Real_String,R.Tag_Copy,R.Real_Len);
				Read2Bin(Real_String_raw,RawR.Tag_Copy,R.Real_Len);
				Skip=Find_Cigar(Real_String_raw,RawR,Original_Text,CIG,H,Real_String,R.Real_Len,R,Clip_H,Clip_T);
			}

		}
		else
		{
			Reverse_Quality(Rev_Qual,R,R.Real_Len);
			Reverse_Tag(Rev_Tag,R,R.Real_Len);
			for(int i=0;i<R.Real_Len;i++)
			{
				R.Tag_Copy[i]=Rev_Tag[i];
				R.Quality[i]=Rev_Qual[i];
			}
			if(!TCIG)
			{
				Read2Bin(Real_String,R.Tag_Copy,R.Real_Len);Read2Bin(Real_String_raw,RawR.Tag_Copy,R.Real_Len);
				H.Loc-=(R.Real_Len-StringLength)+INDELGAP-1;
				Skip=Find_Cigar(Real_String_raw,RawR,Original_Text,CIG,H,Real_String,R.Real_Len,R,Clip_H,Clip_T);
			}
		}

		if(TCIG)
		{
			if(A.Loc)
			{
				H.Score= A.Score;
				H.QScore=A.QScore;
				H.BQScore=A.BQScore;
				H.Mismatch=A.Mismatch;
				H.Indel=A.Indel;
			}
		}
		else
			H.Score= -H.Score;
	}
	else
	{
		assert(false);
		sprintf(CIG,"%dM",StringLength);
		R.Tag_Copy[StringLength]=0;
		R.Quality[StringLength]=0;
	}
	
        if(Quality_Score)
        {
                Quality_Score=Calc_MapQ(H,A,Clip_H+Clip_T);
        }

	if(!CIG[0])
	{
		sprintf(CIG,"%dM",R.Real_Len);
		//\\Quality_Score=0;
		//fprintf (stdout,"\nCigar Error:%s\n",R.Description);
	}

	Ann_Info Ann;
	Location_To_Genome(H.Loc,Ann);H.Chr=Ann.Name;
	H.Loc++;
	if (H.Loc+Skip > Ann.Size) 
	{
		return;
	}

	std::string cig;
	//int ed = Get_ED(cig.append(CIG) );
	//if( H.Mismatch <=MAX_MISMATCHES ) //&& R.Real_Len*0.02+1 >=ed
	{
		A.source=source;A.Chr=H.Chr;A.Sign=H.Sign;A.Loc=H.Loc;strcpy(A.Cigar,CIG);A.Mismatch=H.Mismatch;A.QualityScore=Quality_Score;
		Final_Alignments.push(A);
		//fprintf(Single_File,"%c\t%s\t%c\t%u\t%d\t%d\t%s\n",source,H.Chr,H.Sign,H.Loc,0,R.Real_Len,CIG);//Quality_Score,%d\t
	}//funlockfile(Single_File);
	
}

/*
void Print_Sam(std::priority_queue <Alignment,std::vector <Alignment>,Comp_Alignment> & Final_Alignments,unsigned char* Original_Text,char source,FILE* Single_File,READ & R,Hit_Info & H,int StringLength,int Quality_Score,Alignment A,int TClip_H,int TClip_T,char* TCIG)
{
		int Flag=0;
	int Skip=0;//StringLength;
	char Printed_CIG[MAX_SIGLEN];
	char* Qual=R.Quality,Rev_Qual[MAXTAG];char *CIG;
	char* Tag=R.Tag_Copy,Rev_Tag[MAXTAG];
	assert(H.Loc!=UINT_MAX && A.Score >=0);
	//int Real_Len=0;
	int Clip_H=TClip_H,Clip_T=TClip_T;
	if(TCIG)
	{
		if(TCIG[0])
		{
			CIG=TCIG;
			if(A.Loc)
				H.SW_Score=A.SW_Score;
			H.SW_Sub_Opt_Score=0;
		}
		else
		{
			CIG=Printed_CIG;
			TCIG=NULL;
		}
	}
	else
	{
		assert(false);
		CIG=Printed_CIG;
	}

	int Sub_Opt_Score=0;

	if(R.Real_Len>=StringLength)
	{
		R.Tag_Copy[R.Real_Len]=0;
		R.Quality[R.Real_Len]=0;
		char Real_String[R.Real_Len];
		if(H.Sign=='+')
		{
			if(!TCIG)
			{
				Read2Bin(Real_String,R.Tag_Copy,R.Real_Len);
				Skip=Find_Cigar(Original_Text,CIG,H,Real_String,R.Real_Len,R,Clip_H,Clip_T);
			}

		}
		else
		{
			Reverse_Quality(Rev_Qual,R,R.Real_Len);
			Reverse_Tag(Rev_Tag,R,R.Real_Len);
			for(int i=0;i<R.Real_Len;i++)
			{
				R.Tag_Copy[i]=Rev_Tag[i];
				R.Quality[i]=Rev_Qual[i];
			}
			if(!TCIG)
			{
				Read2Bin(Real_String,R.Tag_Copy,R.Real_Len);
				H.Loc-=(R.Real_Len-StringLength)+INDELGAP-1;
				Skip=Find_Cigar(Original_Text,CIG,H,Real_String,R.Real_Len,R,Clip_H,Clip_T);
			}
		}

		if(TCIG)
		{
			if(A.Loc)
			{
				H.Score= A.Score;
				H.QScore=A.QScore;
				H.BQScore=A.BQScore;
				H.Mismatch=A.Mismatch;
				H.Indel=A.Indel;
			}
		}
		else
			H.Score= -H.Score;
	}
	else
	{
		assert(false);
		sprintf(CIG,"%dM",StringLength);
		R.Tag_Copy[StringLength]=0;
		R.Quality[StringLength]=0;
	}

	if(Quality_Score)
	{
		Quality_Score=Calc_MapQ(H,A,Clip_H+Clip_T);
	}

	if(!CIG[0])
	{
		sprintf(CIG,"%dX",R.Real_Len);
		Quality_Score=0;
		fprintf (stdout,"\nCigar Error:%s\n",R.Description);
	}

	Ann_Info Ann;
	Location_To_Genome(H.Loc,Ann);H.Chr=Ann.Name;
	H.Loc++;
	if (H.Loc+Skip > Ann.Size) 
	{
		return;
	}

	if(H.Sign=='-') 
	{
		Flag=16; 
		Qual=Rev_Qual;
		Tag=Rev_Tag;
	}
	if(Sub_Opt_Score!=INT_MAX)
		fprintf(Single_File,"%s\t%d\t%s\t%u\t%d\t%s\t*\t0\t0\t%s\t%s\tNM:i:%d\tMM:i:0\tAS:i:%d\tSS:i:%d\tQS:i:%d\tSW:i:%d\tSO:i:%d\n",R.Description+1,Flag,H.Chr,H.Loc,Quality_Score,CIG,Tag,Qual,H.Mismatch,H.Score,Sub_Opt_Score,H.QScore,H.SW_Score,H.SW_Sub_Opt_Score);
	else
		fprintf(Single_File,"%s\t%d\t%s\t%u\t%d\t%s\t*\t0\t0\t%s\t%s\tNM:i:%d\tMM:i:0\tAS:i:%d\tQS:i:%d\tSW:i:%d\tSO:i:%d\n",R.Description+1,Flag,H.Chr,H.Loc,Quality_Score,CIG,Tag,Qual,H.Mismatch,H.Score,H.QScore,H.SW_Score,H.SW_Sub_Opt_Score);
}
*/
void Print_Mishit(READ & R,FILE* Mishit_File)
{
	if(PRINT_MISHIT) fprintf(Mishit_File,"@%s\n%s\n+\n%s\n",R.Description+1,R.Tag_Copy,R.Quality);	
}

void Print_Blank_Line(FILE* Single_File,READ & R)
{
	//int Real_Len=0;
	//for(;R.Tag_Copy[Real_Len]!=0 && R.Tag_Copy[Real_Len]!='\n';Real_Len++);
	R.Tag_Copy[R.Real_Len]=R.Quality[R.Real_Len]=0;
	fprintf(Single_File,"");//moxian
	//fprintf(Single_File,"%s\t4\t*\t0\t0\t*\t*\t0\t0\t%s\t%s\n",R.Description+1,R.Tag_Copy,R.Quality);
}

int Calc_MapQ(Hit_Info & H,Alignment & A,int Clip_Count)
{
	//printf("%d\t%d\n",H.Score,H.QScore);//moxian test
	int Quality_Score;
	const int QUAL_START=60,Q_GAP=10;
	//int BOPEN=gap_open,BEXT=gap_extension;// BOPEN=6,BEXT=3,MATCH_BONUS=0;//2;
	//int BOPEN=gap_open,BEXT=gap_extension;// BOPEN=6,BEXT=3,MATCH_BONUS=0;//2;
	if(H.Status==SW_RECOVERED)
	{
		int Sub_Opt_Score=A.Sub_Opt_Score;
		int MapQ=H.BQScore;
		if(Clip_Count)
		{
			int Score_Add=std::min((Clip_Count)*MN,BOPEN+BEXT*(Clip_Count-1));
			MapQ-=Score_Add;
		}	
		if(A.Score<A.Sub_Opt_Score-Q_GAP/3 )
		{
			Quality_Score=std::max(1,QUAL_START/2+MapQ);//-std::min(Top_Penalty,QUAL_START/2));
		}
		else
		{
			if(STACK_LOWQ)
				Quality_Score=1;
			else
				Quality_Score=0;
		}
		//Quality_Score=0;
	}
	//Unique hits..
	else if(H.Status==UNIQUEHIT)
	{
		int Sub_Opt_Score=INT_MAX;
		int MapQ=H.BQScore;
		int Offset=0;
		if(Clip_Count)
		{
			int Score_Add=std::min((Clip_Count)*MN,BOPEN+BEXT*(Clip_Count-1));
			MapQ-=Score_Add;
		}	
		//if(H.Indel) MapQ+=BOPEN;
		Quality_Score=std::max(1,QUAL_START-Offset+MapQ-std::min(Top_Penalty,QUAL_START/3));

	}
	else if(H.Status==SHARP_UNIQUEHIT)
	{
		int Sub_Opt_Score=INT_MAX;
		int MapQ=H.BQScore;
		int Offset=0;
		if(Clip_Count)
		{
			int Score_Add=std::min((Clip_Count)*MN,BOPEN+BEXT*(Clip_Count-1));
			MapQ-=Score_Add;
			//Offset=5;
		}	
		//if(H.Indel) MapQ+=BOPEN;
		Quality_Score=std::max(1,QUAL_START-Offset+MapQ-std::min(Top_Penalty,QUAL_START/3));
	}
	//Multiple hits..
	else 
	{
		int Sub_Opt_Score=H.Sub_Opt_Score;
		assert(H.Status==MULTI_HIT);
		assert(H.Sub_Opt_Score!=INT_MAX);
		int MapQ=H.BQScore;
		if(Clip_Count)
		{
			int Score_Add=std::min((Clip_Count)*MN,BOPEN+BEXT*(Clip_Count-1));
			MapQ-=Score_Add;
		}	
		if(A.Score<A.Sub_Opt_Score-Q_GAP/3 )
		{
			Quality_Score=std::max(1,QUAL_START/2+MapQ);//-std::min(Top_Penalty,QUAL_START/2));
		}
		else
		{
			if(STACK_LOWQ)
				Quality_Score=1;
			else
				Quality_Score=0;
		}
		//Quality_Score=0;
	}
	return Quality_Score;
}

/*int Calc_MapQX(Hit_Info & H,Alignment & A,int Clip_Count)
{
	int Quality_Score;
	const int QUAL_START=210,Q_GAP=10;
	if(H.Status==SW_RECOVERED)
	{
		int Sub_Opt_Score=A.Sub_Opt_Score;
		int MapQ= -H.Score;
		if(Clip_Count)
		{
			int Score_Add=std::min((Clip_Count)*MN,BOPEN+BEXT*(Clip_Count-1));
			MapQ-=Score_Add;
		}	
		//MapQ= MapQ/2; 
		if(A.Score<A.Sub_Opt_Score-Q_GAP/2 )
		{
			Quality_Score=std::max(1,QUAL_START/3+MapQ);
		}
		else if(A.Score<A.Sub_Opt_Score-Q_GAP )
		{
			Quality_Score=std::max(1,QUAL_START/3+MapQ);
		}
		else
		{
			Quality_Score=0;
		}
		Quality_Score=0;
	}
	//Unique hits..
	else if(H.Status==UNIQUEHIT)
	{
		int Sub_Opt_Score=INT_MAX;
		int MapQ= -H.Score+H.BQScore;
		if(Clip_Count)
		{
			int Score_Add=std::min((Clip_Count)*MN,BOPEN+BEXT*(Clip_Count-1));
			MapQ+=Score_Add;
		}	
		if(H.Indel==1) MapQ+=BOPEN;
		//MapQ= MapQ/2; 
		Quality_Score=std::max(1,QUAL_START+MapQ);

	}
	else if(H.Status==SHARP_UNIQUEHIT)
	{
		int Sub_Opt_Score=INT_MAX;
		int MapQ= -H.Score+H.BQScore;
		if(Clip_Count)
		{
			int Score_Add=std::min((Clip_Count)*MN,BOPEN+BEXT*(Clip_Count-1));
			MapQ+=Score_Add;
		}	
		if(H.Indel==1) MapQ+=BOPEN;
		//MapQ= MapQ/2; 
		Quality_Score=std::max(1,QUAL_START+MapQ);

	}
	//Multiple hits..
	else 
	{
		int Sub_Opt_Score=H.Sub_Opt_Score;
		assert(H.Status==MULTI_HIT);
		assert(H.Sub_Opt_Score!=INT_MAX);
		int MapQ= -H.Score;
		if(Clip_Count)
		{
			int Score_Add=std::min((Clip_Count)*MN,BOPEN+BEXT*(Clip_Count-1));
			MapQ-=Score_Add;
		}	
		//MapQ= MapQ/2; 
		if(H.Score<H.Sub_Opt_Score-Q_GAP/2 )
		{
			Quality_Score=std::max(1,QUAL_START/3+MapQ);
		}
		else if(A.Score<A.Sub_Opt_Score-Q_GAP )
		{
			Quality_Score=std::max(1,QUAL_START/2+MapQ);
		}
		else
		{
			Quality_Score=0;
		}
		Quality_Score=0;
	}
	return Quality_Score;
}*/
