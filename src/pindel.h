/* 
 * This File is part of Pindel; a program to locate genomic variation. 
 * https://trac.nbic.nl/pindel/
 * 
 *   Copyright (C) 2011 Kai Ye
 * 
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PINDEL_H
#define	PINDEL_H

#include <iostream>

// System header files
#include <string>
#include <vector>
#include <set>
#include <map>

// Samtools header files
#include "khash.h"
#include "sam.h"

#include "user_defined_settings.h"

// Forward declarations
class LineReader;

extern std::set<std::string> g_sampleNames; // EWL: could make this a singleton instead

/*
 * Global variables defined in pindel.cpp
 */

extern char Match[256];
extern char Match2N[256];
extern char Convert2RC[256];
extern char Convert2RC4N[256];
extern char Cap2LowArray[256];
extern unsigned int DSizeArray[15];
extern short g_reportLength;
extern std::string CurrentChrMask;
extern unsigned int NumberOfSIsInstances;
extern unsigned int NumberOfDIInstances;
extern unsigned int NumberOfTDInstances;
extern unsigned int NumberOfDeletionsInstances;
extern unsigned int g_numberOfInvInstances;
extern unsigned int NumRead2ReportCutOff;
extern int NumRead2ReportCutOff_BP;
extern char Cap2LowArray[256];
extern unsigned int g_maxPos;
extern unsigned int CONS_Chr_Size;
extern bool FirstChr;
extern int g_binIndex;
extern short g_MinClose;
extern unsigned int WINDOW_SIZE;
extern int g_maxInsertSize;

extern const std::string Pindel_Version_str;

/*
 * For search functions
 */
extern unsigned int BoxSize;
extern int MIN_IndelSize_Inversion;
extern int Max_Length_NT;

/*
 * Constants
 */
const char SENSE = '+';
const char ANTISENSE = '-';
const char FORWARD = '+';
const char BACKWARD = '-';
const std::string N_str = "N";
const char N_char = 'N';
const char BD_char = 'A';
const short Max_uint8_t = 100;
const unsigned g_SpacerBeforeAfter = 10000000;
const char Plus = '+';
const char Minus = '-';
const char FirstCharReadName = '@';
const short Max_short = 128;
const unsigned int NumberOfReadsPerBuffer = 1000; // estimate later


/*
 * Data structures
 */
struct UniquePoint {
	short LengthStr;
	unsigned int AbsLoc;
	char Direction; // forward reverse
	char Strand; // sense antisense
	short Mismatches;

	UniquePoint( const short lengthStr, const unsigned int absLoc, const char direction, const char strand, const short mismatches );
	UniquePoint() { UniquePoint(0,0,'N','N',0); };
	friend std::ostream& operator<<(std::ostream& os, const UniquePoint& up );
};

class SortedUniquePoints { 

public:
	void push_back( const UniquePoint& up ) { m_positions.push_back( up ); }// make inline?
	unsigned int size() const { return m_positions.size(); }
	unsigned int MaxLen() const;
	bool empty() const { return m_positions.empty(); } 
	const UniquePoint& operator[](const unsigned int pos) const { return m_positions[pos]; } 
	UniquePoint& operator[](const unsigned int pos) { return m_positions[pos]; }
	void clear() { m_positions.clear(); }
	void swap(SortedUniquePoints& otherPV) { m_positions.swap( otherPV.m_positions ); };

private:
	std::vector< UniquePoint> m_positions;
};

struct RP_READ {
    std::string ReadName;
    std::string ChrNameA;
    std::string ChrNameB;
    char DA;
    char DB;
    unsigned PosA;
    unsigned PosB;
    short Distance;
    short MQA;
    short MQB;
    std::string Tag;
};

struct SPLIT_READ {
	SPLIT_READ() {
		FragName = "";
        FarFragName = "";
		Name = "";
		UnmatchedSeq = "";
		MatchedD = 0;
		MatchedRelPos = 0;
		MS = 0;
		InsertSize = 0;
		Tag = "";
        Thickness = 0;
		ReadLength = 0;
		ReadLengthMinus = 0;
		MAX_SNP_ERROR = 0;
		TOTAL_SNP_ERROR_CHECKED = 0;
		TOTAL_SNP_ERROR_CHECKED_Minus = 0;
		BP = 0;
		Left = 0;
		Right = 0;
		BPLeft = 0;
		BPRight = 0; 
		IndelSize = 0;
		Investigated = false; // whether far end has been searched for
        UniqueRead = false;
		NT_str = "";
		NT_size = 0;
		Used = false;
		CloseEndLength = 0;
		LeftMostPos = 0;
	}
	std::string FragName;
    std::string FarFragName;
	std::string Name;

	void setUnmatchedSeq( const std::string unmatchedSeq ) { 
		const double EPSILON = 0.00001; // to compensate for downrounding off errors (0.04 = 0.03999999 on some computers)
	
		UnmatchedSeq = unmatchedSeq; 
		ReadLength = UnmatchedSeq.size();
		ReadLengthMinus = ReadLength - 1;

		UserDefinedSettings* userSettings = UserDefinedSettings::Instance();
	
		MAX_SNP_ERROR = (short)(((double)ReadLength * userSettings->Seq_Error_Rate) + EPSILON);
		TOTAL_SNP_ERROR_CHECKED_Minus = MAX_SNP_ERROR + userSettings->ADDITIONAL_MISMATCH;
		TOTAL_SNP_ERROR_CHECKED = TOTAL_SNP_ERROR_CHECKED_Minus + 1;
	}
	std::string getUnmatchedSeq() const { return UnmatchedSeq; }	


	char MatchedD; // rename AnchorStrand?
	unsigned int MatchedRelPos;
	short MS; // rename MappingQuality ?
	short InsertSize;
	std::string Tag; // rename SampleName ?
    unsigned Thickness;
	SortedUniquePoints UP_Close; // partial alignment of the unmapped reads close to the mapped read
	SortedUniquePoints UP_Far;
   short getReadLength() const { return ReadLength; }
	short getReadLengthMinus() const { return ReadLengthMinus; }
	short getMAX_SNP_ERROR() const { return MAX_SNP_ERROR; }
	short getTOTAL_SNP_ERROR_CHECKED() const { return TOTAL_SNP_ERROR_CHECKED; }
	short getTOTAL_SNP_ERROR_CHECKED_Minus() const { return TOTAL_SNP_ERROR_CHECKED_Minus; }

	short BP;
	int Left;
	int Right;
	unsigned int BPLeft;
	unsigned int BPRight;
	unsigned int IndelSize;
   bool UniqueRead;
	std::string InsertedStr;
	std::string NT_str;
	unsigned short NT_size;
	bool Used;
	bool Investigated;
	short CloseEndLength;
	int LeftMostPos;
    std::map <std::string, int> ReadCountPerSample;

	unsigned int getLastAbsLocCloseEnd() const;
	bool goodFarEndFound() const;
	bool hasCloseEnd() const;
	unsigned int MaxLenCloseEnd() const;
	unsigned int MaxLenFarEnd() const;
	friend std::ostream& operator<<(std::ostream& os, const SPLIT_READ& splitRead);

private:
	short ReadLength;
	short ReadLengthMinus;
	short MAX_SNP_ERROR; // = (short)(Temp_One_Read.UnmatchedSeq.size() * Seq_Error_Rate);
	short TOTAL_SNP_ERROR_CHECKED; // = MAX_SNP_ERROR + ADDITIONAL_MISMATCH + 1;
	short TOTAL_SNP_ERROR_CHECKED_Minus; // = MAX_SNP_ERROR + ADDITIONAL_MISMATCH;
	std::string UnmatchedSeq;
};

struct SupportPerSample {
	int NumPlus;
	int NumMinus;
	int NumUPlus;
	int NumUMinus;

	SupportPerSample() { NumPlus=0; NumMinus=0; NumUPlus=0; NumUMinus=0; }
};

struct Indel4output {
	Indel4output() {
		BPLeft = 0;
		BPRight = 0;
		IndelSize = 0;
		Start = 0;
		End = 0;
		RealStart = 0;
		RealEnd = 0;
		NT_size = 0;
		WhetherReport = false;
		IndelStr = "";
		Support = 0;
	}
	unsigned int BPLeft;
	unsigned int BPRight;
	unsigned int IndelSize;
	unsigned int Start;
	unsigned int End;
	unsigned int RealStart;
	unsigned int RealEnd;
	short NT_size;
	bool WhetherReport;
	std::string IndelStr;
	unsigned short Support;
};

struct LI_Pos {
	LI_Pos() {
		Plus_Pos = 0;
		Minus_Pos = 0;
		WhetherReport = false;
	}
	unsigned Plus_Pos;
	unsigned Minus_Pos;
	std::vector<unsigned> Plus_Reads; // put index here
	std::vector<unsigned> Minus_Reads;
	bool WhetherReport;
};

struct Rest_Pos {
	Rest_Pos() {
		Strand = 'X';
		Pos = 0;
	}
	char Strand;
	unsigned Pos;
	std::vector<unsigned> Pos_Reads; // put index here
};

struct bam_info {
	bam_info() {
		BamFile = "";
		InsertSize = 0;
		Tag = "";
	}
	std::string BamFile; // EW: change to FileName?
	int InsertSize;
	std::string Tag;
};

struct flags_hit {
	flags_hit() {
		mapped = false;
		unique = false;
		sw = false;
		edits = 0;
		suboptimal = false;
	}
	bool mapped;
	bool unique;
	bool sw;
	int edits;
	bool suboptimal;
};

/*
 * Function definitions
 */
void ReadInOneChr(std::ifstream & inf_Seq, std::string & TheInput, const std::string & ChrName);
void parse_flags_and_tags(const bam1_t * b, flags_hit * flags);
int32_t bam_cigar2len(const bam1_core_t * c, const uint32_t * cigar);
void build_record(const bam1_t * mapped_read, const bam1_t * unmapped_read, void *data);

#ifdef __cplusplus
extern "C" {
int32_t bam_get_tid(const bam_header_t * header, const char *seq_name);
int32_t bam_aux2i(const uint8_t * s);
void bam_init_header_hash(bam_header_t * header);
}
#endif

std::vector<std::string>
ReverseComplement(const std::vector<std::string> &input);
std::string ReverseComplement(const std::string & InputPattern);
std::string Cap2Low(const std::string & input);
void GetRealStart4Insertion(const std::string & TheInput,
		const std::string & InsertedStr, unsigned int &RealStart,
		unsigned int &RealEnd);
void GetRealStart4Deletion(const std::string & TheInput,	unsigned int &RealStart, unsigned int &RealEnd);
bool ReportEvent(const std::vector<SPLIT_READ> &Deletions, const unsigned int &Pre_S, const unsigned int &Pre_E);
void GetCloseEnd(const std::string & CurrentChr, SPLIT_READ & Temp_One_Read);
void GetFarEnd_OtherStrand(const std::string & CurrentChr, SPLIT_READ & Temp_One_Read, const short &RangeIndex);
void GetFarEnd_SingleStrandDownStream(const std::string & CurrentChr, SPLIT_READ & Temp_One_Read, const short &RangeIndex);
void GetFarEnd_SingleStrandUpStream(const std::string & CurrentChr, SPLIT_READ & Temp_One_Read, const short &RangeIndex);
void GetFarEnd_SingleStrandDownStreamInsertions(const std::string & CurrentChr, SPLIT_READ & Temp_One_Read, const short &RangeIndex);
void GetFarEnd(const std::string & CurrentChr, SPLIT_READ & Temp_One_Read,	const int &start, const int &end);
void OutputDeletions(const std::vector<SPLIT_READ> &Deletions,
		const std::string & TheInput, const unsigned int &C_S,
		const unsigned int &C_E, const unsigned int &RealStart,
		const unsigned int &RealEnd, std::ofstream & DeletionOutf);

void OutputTDs(const std::vector<SPLIT_READ> &TDs,
		const std::string & TheInput, const unsigned int &C_S,
		const unsigned int &C_E, const unsigned int &RealStart,
		const unsigned int &RealEnd, std::ofstream & TDOutf);

void OutputInversions(const std::vector<SPLIT_READ> &Inv,
		const std::string & TheInput, const unsigned int &C_S,
		const unsigned int &C_E, const unsigned int &RealStart,
		const unsigned int &RealEnd, std::ofstream & InvOutf);

void OutputSIs(const std::vector<SPLIT_READ> &SIs,
		const std::string & TheInput, const unsigned int &C_S,
		const unsigned int &C_E, const unsigned int &RealStart,
		const unsigned int &RealEnd, std::ofstream & SIsOutf);

short CompareTwoReads(const SPLIT_READ & First, const SPLIT_READ & Second);
std::string Reverse(const std::string & InputPattern);
std::string Cap2Low(const std::string & input);

void CheckBoth(const SPLIT_READ & OneRead, const std::string & TheInput,
		const std::string & CurrentReadSeq,
		const std::vector<unsigned int> PD_Plus[],
		const std::vector<unsigned int> PD_Minus[], const short BP_Start,
		const short BP_End, const short CurrentLength,
		SortedUniquePoints &UP);
void GetIndelTypeAndRealStart(const std::string & TheInput,
		const unsigned int &BPLeft, const unsigned int &IndelSize,
		const std::string & IndelStr, std::string & IndelType,
		unsigned int &RealStart, const bool & WhetherD);
void CleanUniquePoints(SortedUniquePoints &Input_UP);

bool readTransgressesBinBoundaries(SPLIT_READ & read,	const unsigned int &upperBinBorder);

void saveReadForNextCycle(SPLIT_READ & read,	std::vector<SPLIT_READ> &futureReads);

bool readInSpecifiedRegion(const SPLIT_READ & read, const SearchRegion* region );
void reportBreakDancerEvent( const std::string& chromosomeName, const int leftPosition, const int rightPosition, 
	                          const int svSize, const std::string& svType, const int svCounter);
void updateReadAfterCloseEndMapping( SPLIT_READ& Temp_One_Read );

LineReader *getLineReaderByFilename(const char *filename);

// note: in future, want to move to having a pointer to a Chromosome structure instead of a ChrID-string

class Genome;

class Chromosome {

friend class Genome;

public:
	Chromosome( const std::string& name, const std::string& sequence ) { m_name = name; m_sequence = sequence; };
	const std::string& getName() const { return m_name; }
	const std::string& getSeq() const { return m_sequence; }
	const unsigned int getCompSize() const { return m_sequence.size(); }
	const unsigned int getBiolSize() const { return m_sequence.size() - 2 * g_SpacerBeforeAfter; }

private:
	std::string m_name;
	std::string m_sequence;
};

extern const Chromosome g_dummyChromosome;

class Genome {
public:
	const Chromosome* addChromosome( Chromosome* newChromosome );
	unsigned int chrNameToChrIndex( const std::string chromosomeName );
	const Chromosome& getChr( unsigned int index ) const { if (index<m_chromosomes.size()) return *m_chromosomes[ index ]; else return g_dummyChromosome; }
	const Chromosome* loadChromosome( std::ifstream& fastaFile );

private:
	std::vector< Chromosome* > m_chromosomes;
};

extern Genome g_genome;

class SearchWindow {

public:
	SearchWindow( const Chromosome* chromosome, const unsigned int regionStart, const unsigned int regionEnd );
	SearchWindow(const SearchWindow& other); 

	unsigned int getStart() const { return m_currentStart; }
	unsigned int getEnd() const { return m_currentEnd; }
	void setStart( const unsigned int newStart ) { m_currentStart = newStart; }	
	void setEnd( const unsigned int newEnd ) { m_currentEnd = newEnd; }
	const std::string& getChromosomeName() const { return m_chromosome->getName(); }
	const Chromosome* getChromosome() const { return m_chromosome; }
	unsigned int getSize() const { return 1 + m_currentEnd - m_currentStart; }
	bool encompasses( const std::string& chromosomeName, const unsigned int position ) const;
	SearchWindow makePindelCoordinateCopy() const { SearchWindow temp( m_chromosome, m_currentStart + g_SpacerBeforeAfter, m_currentEnd + g_SpacerBeforeAfter ); return temp; }; 
	SearchWindow& operator=(const SearchWindow& other );

protected:
	const Chromosome* m_chromosome;
	unsigned int m_currentStart;
	unsigned int m_currentEnd;
};

class LoopingSearchWindow: public SearchWindow {

public:
	LoopingSearchWindow(const SearchRegion* region, const int chromosomeSize, const int binSize, const Chromosome* chromosome );
	void next();
	std::string display() const;
	bool finished() const;

private:
	void updateEndPositions();
	unsigned int m_globalStart;
	unsigned int m_globalEnd;
	unsigned int m_officialStart;
	unsigned int m_officialEnd;
	unsigned int m_displayedStart;
	unsigned int m_displayedEnd;
	const unsigned int m_BIN_SIZE;
};



#endif /* PINDEL_H */
