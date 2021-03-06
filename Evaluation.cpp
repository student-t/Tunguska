#include "Evaluation.h"
#include "MoveGen.h"
#include "FenParser.h"
#include "Zobrist.h"

#include <iostream>
#include <fstream>

#define PAWN_SQ {0, 0, 0, 0,  0,  0,  0,  0, 5, 10, 10,-20,-20, 10, 10,  5, 5, -5,-10,  0,  0,-10, -5,  5, 0,  0,  0, 20, 20,  0,  0,  0, 5,  5, 10, 25, 25, 10,  5,  5, 10, 10, 20, 30, 30, 20, 10, 10, 50, 50, 50, 50, 50, 50, 50, 50, 0,  0,  0,  0,  0,  0,  0,  0}						
#define KNIGHT_SQ {-50,-40,-30,-30,-30,-30,-40,-50,-40, -20,  0,  5,  5,  0, -20, -40,-30,  5, 10, 15, 15, 10,  5, -30, -30,  0, 15, 20, 20, 15,  0,-30,-30,  5, 15, 20, 20, 15,  5,-30,-30,  0, 10, 15, 15, 10,  0, -30, -40, -20,  0,  0,  0,  0,-20,-40,-50,-40,-30,-30,-30,-30,-40,-50}							
#define BISHOP_SQ {-20,-10,-10,-10,-10,-10,-10,-20,-10,  5,  0,  0,  0,  0,  5,-10,-10, 10, 10, 10, 10, 10, 10,-10,-10,  0, 10, 10, 10, 10,  0,-10,-10,  5,  5, 10, 10,  5,  5,-10,-10,  0,  5, 10, 10,  5,  0,-10,-10,  0,  0,  0,  0,  0,  0,-10,-20,-10,-10,-10,-10,-10,-10,-20}							  
#define ROOK_SQ {-5, 0, 5, 10, 10, 5, 0, -5, 0,  0,  5,  10,  10,  5,  0, 0, 0,  0,  5,  10,  10,  5,  0, 0, 0,  0,  5,  10,  10,  5,  0, 0, 0,  0,  5,  10,  10,  5,  0, 0, 0,  0,  5,  10,  10,  5,  0,  0, 20, 20, 20, 20, 20, 20, 20, 20, 0,  0,  0,  0,  0,  0,  0,  0}
#define QUEEN_SQ {-20,-10,-10, -5, -5,-10,-10,-20,-10,  0,  5,  0,  0,  0,  0,-10,-10,  5,  5,  5,  5,  5,  0,-10,0,   0,  5,  5,  5,  5,  0, -5,-5,   0,  5,  5,  5,  5,  0, -5,-10,  0,  5,  5,  5,  5,  0,-10,-10,  0,  0,  0,  0,  0,  0,-10,-20,-10,-10, -5, -5,-10,-10,-20}							
#define KING_SQ {20, 30, 10,  0,  0, 10, 30, 20,20, 20,  0,  0,  0,  0, 20, 20,-10,-20,-20,-20,-20,-20,-20,-10,-20,-30,-30,-40,-40,-30,-30,-20,-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30,-30,-40,-40,-50,-50,-40,-40,-30}							 
#define KING_END_SQ {-50, -40, 0, 0, 0,	0, -40,	-50, -20, 0, 10, 15, 15, 10, 0,	-20, 0,	10,	20,	20,	20,	20,	10,	0, 0, 10, 30, 40, 40, 30, 10, 0, 0,	10,	20,	40,	40,	20,	10,	0, 0, 10, 20, 20, 20, 20, 10, 0, -10, 0, 10, 10, 10, 10, 0,	-10, -50, -10,	0,	0,	0,	0, -10, -50}

//idx 0/1 => light/dark squares bishop
static const int KNB_K_MATE[2][64]= {
		{-5,-5,-10,-10,-20,-30,-30,-50,
		  0,0,-5,-5,-10,-20,-30,-30,
		  5,5,0,0,-5,-10,-10,-20,
		  10,10,5,5,0,-5,-5,-10,
		  -10,-5,-5,0,5,5,10,10,
		  -20,-10,-10,-5,0,0,5,5,
		  -30,-30,-20,-10,-5,-5,0,0,
		 -50,-30,-30,-20,-10,-10,-5,-5},

		{-50,-30,-30,-20,-10,-10,-5,-5,
		-30,-30,-20,-10,-5,-5,0,0,
		-20,-10,-10,-5,0,0,5,5,
		-10,-5,-5,0,5,5,10,10,
		10,10,5,5,0,-5,-5,-10,
		5,5,0,0,-5,-10,-10,-20,
		0,0,-5,-5,-10,-20,-30,-30,
		-5,-5,-10,-10,-20,-30,-30,-50}
};

int Evaluation::PIECE_VALUES[14] = {0, 0, PAWN_VAL, -PAWN_VAL, KNIGHT_VAL, -KNIGHT_VAL, BISHOP_VAL,
					-BISHOP_VAL, ROOK_VAL, -ROOK_VAL, QUEEN_VAL, -QUEEN_VAL, KING_VAL, -KING_VAL};
					
int Evaluation::PIECE_SQUARES_MG[14][64] = {{}, {}, 
										PAWN_SQ, PAWN_SQ,
									 	KNIGHT_SQ, KNIGHT_SQ,
									 	BISHOP_SQ, BISHOP_SQ,
										ROOK_SQ, ROOK_SQ,
										QUEEN_SQ, QUEEN_SQ,
										KING_SQ, KING_SQ};

int Evaluation::PIECE_SQUARES_END[14][64] = {{}, {},
											PAWN_SQ, PAWN_SQ,
											KNIGHT_SQ, KNIGHT_SQ,
											BISHOP_SQ, BISHOP_SQ,
											ROOK_SQ, ROOK_SQ,
										 	QUEEN_SQ, QUEEN_SQ,
										  	KING_END_SQ, KING_END_SQ};	
//Bishops
static const int BISHOP_PAIR_MG = 25;
static const int BISHOP_PAIR_EG = 50;
static const int BISHOP_PAWN_PENALTY_MG = -8;
static const int BISHOP_PAWN_PENALTY_EG = -12;

//Open and semiopen files
static const int R_OPEN_MG = 14;
static const int R_SOPEN_MG = 7;
static const int Q_OPEN_MG = 6;
static const int Q_SOPEN_MG = 3;
static const int R_OPEN_EG = 20;
static const int R_SOPEN_EG = 10;
static const int Q_OPEN_EG = 8;
static const int Q_SOPEN_EG = 4;
static const int OPENFILES_BONUS_MG[2][2] = {{R_OPEN_MG, R_SOPEN_MG},{Q_OPEN_MG, Q_SOPEN_MG}}; 
static const int OPENFILES_BONUS_EG[2][2] = {{R_OPEN_EG, R_SOPEN_EG},{Q_OPEN_EG, Q_SOPEN_EG}};

//Pawns
static const int ISOLATED_PAWN_PENALTY_MG[8] = {-5, -7, -10, -10, -10, -10, -7, -5};
static const int ISOLATED_PAWN_PENALTY_EG[8] = {-10, -14, -20, -20, -20, -20, -14, -10};
static const int PASSED_PAWN_BONUS_MG[2][8] = {{0, 2, 5, 10, 20, 40, 70, 120}, {120, 70, 40, 20, 10, 5, 2, 0}};
static const int PASSED_PAWN_BONUS_EG[2][8] = {{0, 5, 10, 20, 40, 90, 150, 200}, {200, 150, 90, 40, 20, 10, 5, 0}};
static const int DOUBLED_ISOLATED_PAWN_MG = -10;
static const int DOUBLED_ISOLATED_PAWN_EG = -20;

static const int PAWN_CONNECTED_BONUS_MG[2][64] = {
    { 0, 0, 0, 0, 0, 0, 0, 0,
      2, 2, 2, 3, 3, 2, 2, 2,
      4, 4, 5, 6, 6, 5, 4, 4,
      7, 8,10,12,12,10, 8, 7,
     11,14,17,21,21,17,14,11,
     16,21,25,33,33,25,21,16,
     32,42,50,55,55,50,42,32,
      0, 0, 0, 0, 0, 0, 0, 0},
      
    { 0, 0, 0, 0, 0, 0, 0, 0,
     32,42,50,55,55,50,42,32,
     16,21,25,33,33,25,21,16,
     11,14,17,21,21,17,14,11,
      7, 8,10,12,12,10, 8, 7,
      4, 4, 5, 6, 6, 5, 4, 4,
      2, 2, 2, 3, 3, 2, 2, 2,
      0, 0, 0, 0, 0, 0, 0, 0}
};

static const int PAWN_CONNECTED_BONUS_EG[2][64] = {
    { 0, 0, 0, 0, 0, 0, 0, 0,
      4, 4, 5, 6, 6, 5, 4, 4,
      7, 8,10,12,12,10, 8, 7,
     11,14,17,21,21,17,14,11,
     16,21,25,33,33,25,21,16,
     26,31,35,43,43,35,31,26,
	 52,62,70,86,86,70,62,52,
      0, 0, 0, 0, 0, 0, 0, 0},
      
    { 0, 0, 0, 0, 0, 0, 0, 0,
     52,62,70,86,86,70,62,52,
	 26,31,35,43,43,35,31,26,
     16,21,25,33,33,25,21,16,
     11,14,17,21,21,17,14,11,
     7, 8,10,12,12,10, 8, 7,
     4, 4, 5, 6, 6, 5, 4, 4,
     0, 0, 0, 0, 0, 0, 0, 0}
};

int Evaluation::MIRROR64[64];
static AttackInfo attackInfo;

void Evaluation::initAll(){
	for (int i = 0; i < 64; i++){
		int r = i/8;
		int c = i % 8;
		int sqBlack = (7 - r)* 8 + c;
		MIRROR64[i] = sqBlack;
	}
}	

void Evaluation::materialBalance(const Board& board, int& mg, int& eg){
	for (int i = 0; i < 64; i++){
		mg+= PIECE_VALUES[board.board[i]];
		eg+= PIECE_VALUES[board.board[i]];
	}
}

void Evaluation::pieceSquaresBalance(const Board& board, int& mg, int& eg){
	//white
	U64 pieces = board.bitboards[Board::WHITE];
	while(pieces){
		int sq = numberOfTrailingZeros(pieces);
		mg+= PIECE_SQUARES_MG[board.board[sq]][sq];
		eg+= PIECE_SQUARES_END[board.board[sq]][sq];
		pieces&= pieces - 1;
	}
	
	//black
	pieces = board.bitboards[Board::BLACK];
	while(pieces){
		int sq = numberOfTrailingZeros(pieces);
		mg-= PIECE_SQUARES_MG[board.board[sq]][MIRROR64[sq]];
		eg-= PIECE_SQUARES_END[board.board[sq]][MIRROR64[sq]];
		pieces&= pieces - 1;
	}
}

void Evaluation::evalPawns(const Board& board, int& mg, int& eg){
	
	int s = 1;
	for (int side = 0; side < 2; side++){
		
		int pawnSide = Board::PAWN | side;
		U64 pawnBB = board.bitboards[pawnSide];
		U64 pawns = board.bitboards[pawnSide];
		U64 oppPawns = board.bitboards[side^1 | Board::PAWN];
	
		while (pawns){
			int sq = numberOfTrailingZeros(pawns);
			int file = sq & 7;

			bool isolated = false;
			
			//isolated
			if ((BitBoardGen::ADJACENT_FILES[file] & pawnBB) == 0){
				mg+= s * ISOLATED_PAWN_PENALTY_MG[file];
				eg+= s * ISOLATED_PAWN_PENALTY_EG[file];
				isolated = true;
			}
			
			//passed
			U64 frontSpan = BitBoardGen::FRONT_SPAN[side][sq];
			
			if ((frontSpan & oppPawns) == 0){
				int r = sq >> 3;
				mg+= s * PASSED_PAWN_BONUS_MG[side][r];
				eg+= s * PASSED_PAWN_BONUS_EG[side][r];
			}
			
			//connected
			if (BitBoardGen::PAWN_CONNECTED[side][sq] & pawnBB){
				mg+= s * PAWN_CONNECTED_BONUS_MG[side][sq];
				eg+= s * PAWN_CONNECTED_BONUS_EG[side][sq];
			}
			pawns&= pawns - 1;

			//test for isolated and doubled
			if (isolated && (BitBoardGen::BITBOARD_FILES[file] & pawns)){
				mg+= s * DOUBLED_ISOLATED_PAWN_MG;
				eg+= s * DOUBLED_ISOLATED_PAWN_EG;
			}
		}
		s = -1;
	}
}

bool Evaluation::materialDraw(const Board& board){

	if (board.bitboards[Board::WHITE_PAWN] || board.bitboards[Board::BLACK_PAWN])
		return false;

	bool hasWQ = board.bitboards[Board::WHITE_QUEEN];
	bool hasBR = board.bitboards[Board::BLACK_ROOK];
	bool hasBQ = board.bitboards[Board::BLACK_QUEEN];
	bool hasWR = board.bitboards[Board::WHITE_ROOK];

	//no queen or rooks
	if (!hasWQ && !hasBQ && !hasWR && !hasBR){

		bool hasWB = board.bitboards[Board::WHITE_BISHOP];
		bool hasWN = board.bitboards[Board::WHITE_KNIGHT];
		bool hasBB = board.bitboards[Board::BLACK_BISHOP];
		bool hasBN = board.bitboards[Board::BLACK_KNIGHT];
		int wn = BitBoardGen::popCount(board.bitboards[Board::WHITE_KNIGHT]);
		int bn = BitBoardGen::popCount(board.bitboards[Board::BLACK_KNIGHT]);
		int wb = BitBoardGen::popCount(board.bitboards[Board::WHITE_BISHOP]);
		int bb = BitBoardGen::popCount(board.bitboards[Board::BLACK_BISHOP]);

		//no bishops
		if(!hasWB && !hasBB){
			// 2 or less knights
			if (wn < 3 && bn < 3)
				return true;
		}
		//has bishops but no knights
		else if (!hasWN && !hasBN){
			if (abs(wb - bb) < 2)
				return true;
		}//has bishops and knights
		else if((wn < 3 && !hasWB) || (wb == 1 && !hasWN)){
			if ((bn < 3 && !hasBB) || (bb == 1 && !hasBN)) {
				return true;
			}
		}
		//has rooks, no queen
	} else if (!hasWQ && !hasBQ) {
		int wr = BitBoardGen::popCount(board.bitboards[Board::WHITE_ROOK]);
		int br = BitBoardGen::popCount(board.bitboards[Board::BLACK_ROOK]);
		int wn = BitBoardGen::popCount(board.bitboards[Board::WHITE_KNIGHT]);
		int bn = BitBoardGen::popCount(board.bitboards[Board::BLACK_KNIGHT]);
		int wb = BitBoardGen::popCount(board.bitboards[Board::WHITE_BISHOP]);
		int bb = BitBoardGen::popCount(board.bitboards[Board::BLACK_BISHOP]);

        if (wr == 1 && br == 1) {
            if ((wn + wb) < 2 && (bn + bb) < 2)	{ 
            	return true; 
            }
        } else if (wr == 1 && !hasBR) {
            if ((wn + wb == 0) && (((bn + bb) == 1) || ((bn + bb) == 2))) { 
            	return true; 
            }
        } else if (br == 1 && !hasWR) {
            if ((bn + bb == 0) && (((wn + wb) == 1) || ((wn + wb) == 2))) {
            	return true; 
         	}
        }
    }
    return false;
}
									
void Evaluation::pieceOpenFile(const Board& board, int& mg, int& eg){
	
	const int pieceTypes[2] = {Board::ROOK, Board::QUEEN};
	U64 pawnBoth = board.bitboards[Board::WHITE] | board.bitboards[Board::BLACK];
	int s = 1;

	for (int side = 0; side < 2; side++){
		for (int p = 0; p < 2; p++){
			
			int opp = side^1;
			U64 pieces = board.bitboards[pieceTypes[p] | side];
			U64 pawnSide = board.bitboards[Board::PAWN | side];
			U64 pawnOpp = board.bitboards[Board::PAWN | opp];
			
			while (pieces){
				int sq = numberOfTrailingZeros(pieces);
				int file = sq & 7;

				if ((pawnBoth & BitBoardGen::BITBOARD_FILES[file]) == 0){
					mg+= s * OPENFILES_BONUS_MG[p][0];
					eg+= s * OPENFILES_BONUS_EG[p][0];
				}
				else if ((pawnSide & BitBoardGen::BITBOARD_FILES[file]) == 0){
					mg+= s * OPENFILES_BONUS_MG[p][1];
					eg+= s * OPENFILES_BONUS_EG[p][1];
				}
				pieces&= pieces - 1;
			}
		}
		s = -1;
	}
}

//Save attack info here!
void Evaluation::kingAttack(const Board& board, int& mg){
	mg+= kingAttackedSide(board, Board::BLACK) - kingAttackedSide(board, Board::WHITE);
}

// Return opp attack eval on side king
int Evaluation::kingAttackedSide(const Board& board, int side){
	int opp = side^1;
	U64 king = board.bitboards[Board::KING | side];
	int kingSq = numberOfTrailingZeros(king);
	U64 region = BitBoardGen::BITBOARD_KING_REGION[side][kingSq];
	U64 occup = board.bitboards[side] | board.bitboards[opp];
	U64 enemyOrEmpty = ~board.bitboards[opp]; //include opp king square
	
	int numAttackers = 0;
	int attackVal = 0;
	//Attack weights
	const int ROOK_AW = 40;
	const int QUEEN_AW = 80;
	const int BISHOP_AW = 20;
	const int KNIGHT_AW = 20;
	//Weight of attack by numAttackers, 0 or 1 attacker => 0 weight
	const int ATTACK_W[] = {0, 0, 50, 75, 88, 94, 97, 99};

	//rooks
	U64 rooks = board.bitboards[Board::ROOK | opp];
	U64 rookTargs = 0;
	int nrooks = 0;
	
	while (rooks){	
		int from = numberOfTrailingZeros(rooks);
		U64 tmpTarg = MoveGen::rookAttacks(board, occup, from, 0);
		rookTargs|= tmpTarg;

		if (tmpTarg & region)
			nrooks++;
		rooks&= rooks - 1;
	}
	numAttackers+= nrooks;
	attackVal+= nrooks * ROOK_AW;
	
	//attack info
	attackInfo.rooks[side] = rookTargs;
	
	//queens
	U64 queens = board.bitboards[Board::QUEEN | opp];
	U64 queenTargs = 0;
	int nqueens = 0;
	
	while (queens){	
		int from = numberOfTrailingZeros(queens);
		U64 tmpTarg = MoveGen::rookAttacks(board, occup, from, 0) | MoveGen::bishopAttacks(board, occup, from, 0); 
		queenTargs|= tmpTarg;

		if (tmpTarg & region)	
			nqueens++;
		queens &= queens - 1;
	}
	
	numAttackers+= nqueens;
	attackVal+= nqueens * QUEEN_AW;
	
	//attack info
	attackInfo.queens[side] = queenTargs;
	
	//bishops
	U64 bishops = board.bitboards[Board::BISHOP | opp];
	U64 bishopTargs = 0;
	int nbishops = 0;
	
	while (bishops){
		int from = numberOfTrailingZeros(bishops);
		U64 tmpTarg = MoveGen::bishopAttacks(board, occup, from, 0);
		bishopTargs|=  tmpTarg;

		if (tmpTarg & region)
			nbishops++;
		bishops&= bishops - 1;
	}
	
	numAttackers+= nbishops;
	attackVal+= nbishops * BISHOP_AW;
	
	//attack info
	attackInfo.bishops[side] = bishopTargs;
	
	//knights
	long knights = board.bitboards[Board::KNIGHT | opp];
	long knightTargs = 0;
	int nknights = 0;
	
	while (knights){
		int from = numberOfTrailingZeros(knights);
		long tmpTarg = BitBoardGen::BITBOARD_KNIGHT_ATTACKS[from] & enemyOrEmpty;
		knightTargs|= tmpTarg;

		if (tmpTarg & region)
			nknights++;
		knights&= knights - 1;
	}
	
	numAttackers+= nknights;
	attackVal+= nknights * KNIGHT_AW;
	
	//attack info
	attackInfo.knights[side] = knightTargs;
	
	numAttackers = numAttackers > 7 ? 7 : numAttackers;
	attackVal*= ATTACK_W[numAttackers];
	
	return attackVal/100;
}													

static const int king_shelter_bonus1 = 8;
static const int king_shelter_bonus2 = 4;
//one sq ahead region
static const int king_shelter_attacked_penalty1[] = {0, -8, -16, -48};
//two squares ahead region
static const int king_shelter_attacked_penalty2[] = {0, -4, -8, -24};
//opp connected pawns attacking shelter
static const int king_connected_attack_penalty = -5;

// if has shelter, evaluate?
void Evaluation::kingShelter(const Board& board, int& mg){
	
	int s = 1;
	for (int side = 0; side < 2; side++){
		
		int opp = side^1;
		int ks = numberOfTrailingZeros(board.bitboards[Board::KING | side]);
		U64 front1 = BitBoardGen::BITBOARD_KING_AHEAD[side][ks][0];
		U64 front2 = BitBoardGen::BITBOARD_KING_AHEAD[side][ks][1];
		U64 myPawns = board.bitboards[Board::PAWN | side];
		U64 oppPawns = board.bitboards[Board::PAWN | opp];
		
		//defense pawns
		U64 shelter = front1 & myPawns;
		mg+= s * BitBoardGen::popCount(shelter) * king_shelter_bonus1;
		
		shelter = front2 & myPawns;
		mg+= s * BitBoardGen::popCount(shelter) * king_shelter_bonus2;
		
		//enemy pawns
		U64 oppPawnsAttacking = 0;
		shelter = front1 & oppPawns;
		oppPawnsAttacking|= shelter;
		mg+= s * king_shelter_attacked_penalty1[BitBoardGen::popCount(shelter)];
		
		shelter = front2 & oppPawns;
		oppPawnsAttacking|= shelter;
		mg+= s * king_shelter_attacked_penalty2[BitBoardGen::popCount(shelter)];

		while(oppPawnsAttacking){
			int sq = numberOfTrailingZeros(oppPawnsAttacking);
			if (BitBoardGen::PAWN_CONNECTED[opp][sq] & oppPawns){
				mg+= s * king_connected_attack_penalty;
			}
			oppPawnsAttacking&= oppPawnsAttacking - 1;
		}
		s = -1;
	}
}

Board Evaluation::mirrorBoard(const Board& board){
	Board mBoard;
	int tmpCastle = 0;
	int tmpEP = 0;

	//Castle
	if (BoardState::white_can_castle_ks(board.state)) tmpCastle |= BoardState::BK_CASTLE;
	if (BoardState::white_can_castle_qs(board.state)) tmpCastle |= BoardState::BQ_CASTLE;
	if (BoardState::black_can_castle_ks(board.state)) tmpCastle |= BoardState::WK_CASTLE;
	if (BoardState::black_can_castle_qs(board.state)) tmpCastle |= BoardState::WQ_CASTLE;

	//Ep square
	if (BoardState::epSquare(board.state) != 0)
		tmpEP = Evaluation::MIRROR64[BoardState::epSquare(board.state)];

	int mPieces[] = {0, 0, Board::BLACK_PAWN, Board::WHITE_PAWN, Board::BLACK_KNIGHT, Board::WHITE_KNIGHT, 
		Board::BLACK_BISHOP, Board::WHITE_BISHOP, Board::BLACK_ROOK, Board::WHITE_ROOK, 
		Board::BLACK_QUEEN, Board::WHITE_QUEEN, Board::BLACK_KING, Board::WHITE_KING};
	
	//mirror pieces
	for (int i = 0; i < 64; i++)
		mBoard.board[i] = mPieces[board.board[Evaluation::MIRROR64[i]]];

	//setup bitboards
	for (int s = 0; s < 64; s++){
		if (mBoard.board[s] == Board::EMPTY)
			continue;
		mBoard.bitboards[mBoard.board[s]] |= BitBoardGen::ONE << s;
		mBoard.bitboards[mBoard.board[s] & 1] |= BitBoardGen::ONE << s;
	}

	int side = BoardState::currentPlayer(board.state);
	mBoard.histPly = board.histPly;
	mBoard.state = BoardState::new_state(tmpEP, BoardState::halfMoves(board.state), side ^ 1, tmpCastle);
	mBoard.zKey = Zobrist::getKey(mBoard);
	mBoard.ply = 0;
	mBoard.hashTable = NULL;

	return mBoard;
}

int Evaluation::countMaterial(const Board& board, int piece){
	return BitBoardGen::popCount(board.bitboards[piece]);
}

int Evaluation::materialValueSide(const Board& board, int side){
	int mat = 0;

	for (int i = 0; i < 64; i++){
		if (board.board[i] == Board::EMPTY)
			continue;
		if ((board.board[i] & 1) == side)
			mat += PIECE_VALUES[board.board[i]];
	}
	return abs(mat) - KING_VAL;
}


//Phase
static const int pawn_phase = 0;
static const int knight_phase = 1;
static const int bishop_phase = 1;
static const int rook_phase = 2;
static const int queen_phase = 4;
static const int total_phase = 24;

int Evaluation::get_phase(const Board& board){
	int phase = total_phase;
	
	for (int side = 0; side < 2; side++){	
		phase-= countMaterial(board, Board::PAWN | side) * pawn_phase;
		phase-= countMaterial(board, Board::KNIGHT | side) * knight_phase;
		phase-= countMaterial(board, Board::BISHOP | side) * bishop_phase;
		phase-= countMaterial(board, Board::ROOK | side) * rook_phase;
		phase-= countMaterial(board, Board::QUEEN | side) * queen_phase;
	}
	return (phase * 256 + (total_phase / 2)) / total_phase;
}

void Evaluation::testEval(std::string test_file){
	std::ifstream file(test_file);
    std::string line; 
	int n = 0;

	printf("Running eval test\n");

    while (std::getline(file, line)){
    	Board board = FenParser::parseFEN(line);
    	Board mBoard = mirrorBoard(board);
    	bool eq = evaluate(board, BoardState::currentPlayer(board.state)) == evaluate(mBoard, BoardState::currentPlayer(mBoard.state));
    	n++;
    	if (!eq){
    		std::cout << "Eval test fail at FEN: " << line << std::endl;
    		return;
    	}
    }
    printf("Evaluation ok: tested %d positions.\n", n);
}

void Evaluation::evalBishops(const Board& board, int& mg, int& eg){

	int wb_cnt = BitBoardGen::popCount(board.bitboards[Board::WHITE_BISHOP]);
	int bb_cnt = BitBoardGen::popCount(board.bitboards[Board::BLACK_BISHOP]);

	//Bishop pair
	std::pair<int, int> w_ev = wb_cnt > 1 ? std::make_pair(BISHOP_PAIR_MG, BISHOP_PAIR_EG) : std::make_pair(0, 0);
	std::pair<int, int> b_ev = bb_cnt > 1 ? std::make_pair(BISHOP_PAIR_MG, BISHOP_PAIR_EG) : std::make_pair(0, 0);
	mg+= w_ev.first - b_ev.first;
	eg+= w_ev.second - b_ev.second;
	
	/*
	//Pawns of same color penalty
	int s = 1;
	for (int side = 0; side < 2; side++){
		U64 bsh = board.bitboards[Board::BISHOP | side];
	
		while(bsh){
			int color =  BitBoardGen::COLOR_OF_SQ[numberOfTrailingZeros(bsh)];
			U64 pawns = board.bitboards[Board::PAWN | side] & BitBoardGen::LIGHT_DARK_SQS[color];
			int cnt =  BitBoardGen::popCount(pawns);
			mg+= s * BISHOP_PAWN_PENALTY_MG * cnt;
			eg+= s * BISHOP_PAWN_PENALTY_EG * cnt;
			bsh&= bsh - 1;
		}
		s = -1;
	}
	*/
}

//Double rooks on 7th bonus (extra bonus if side has queen)
static const int D_ROOK_7_MG = 40;
static const int D_ROOK_7_EG = 80;
static const int D_ROOK_7_Q = 20;

void Evaluation::evalRooks(const Board& board, int& mg, int& eg){
	int s = 1;
	const U64 seventh_rank[2] = {BitBoardGen::BITBOARD_RANKS[6], BitBoardGen::BITBOARD_RANKS[1]};
	
	//double rooks on 7th
	for (int side = 0; side < 2; side++){
		U64 rooks = board.bitboards[Board::ROOK | side];
		
		if (BitBoardGen::popCount(rooks & seventh_rank[side]) > 1){
			mg+= s * D_ROOK_7_MG;
			eg+= s * D_ROOK_7_EG;
			
			//More bonus if side has queen
			mg+= board.bitboards[Board::QUEEN | side] ? s * D_ROOK_7_Q : 0;
			eg+= board.bitboards[Board::QUEEN | side] ? s * D_ROOK_7_Q : 0;
		}
		s = -1;
	}
}

//Mobility
static const int MOB_N[2][9] = {{-75, -56, -9, -2, 6, 15, 22, 30, 36}, {-76, -54, -26, -10, 5, 11, 26, 28, 29}};
static const int MOB_B[2][14] = {{-48, -21 ,16, 26, 37, 51, 54, 63, 65, 71, 79, 81, 92, 97}, {-58, -19, -2, 12, 22, 42, 54, 58, 63, 70, 74, 86, 90, 94}};
static const int MOB_R[2][15] = {{-56, -25, -11, -5, -4, -1, 8, 14, 21, 23, 31, 32, 43, 49, 59}, {-78, -18, 26, 55, 70, 81, 109, 120, 128, 143, 154, 160, 165, 168, 169}};
static const int MOB_Q[2][28] = {{-40, -25, 2, 4, 14, 24, 25, 40, 43, 47, 54, 56, 60, 70, 72, 73, 75, 
					77, 85, 94, 99, 108, 112, 113, 118, 119, 123, 128}, 
					{-35, -12, 7, 19, 37, 55, 62, 76, 79, 87, 94, 102, 111, 116, 118, 122,
					128, 130, 133, 136, 140, 157, 158, 161, 174, 177, 191, 199}};
					
static int dirs[2][2] = {{7, 64 - 9}, {9, 64 - 7}};
static int diffs[2][2] = {{7, -9}, {9, -7}};
void Evaluation::mobility(const Board& board, int& mg, int& eg){
	
	int s = 1;
	for (int side = 0; side < 2; side++){
		
		int opp = side^1;
		U64 oppPawnBB = board.bitboards[Board::PAWN | opp];
		U64 oppPawnAttacks = 0;
		
		if (oppPawnBB){
			for (int i = 0; i < 2; i++){
				int *dir = dirs[i];
				int *diff = diffs[i];
				U64 wFile = BitBoardGen::WRAP_FILES[i];
				oppPawnAttacks|= BitBoardGen::circular_lsh(oppPawnBB, dir[opp]) & ~wFile;
			}
		}
		
		U64 king = board.bitboards[Board::KING | side];
		U64 blockedPawns = 0;
		if (side == Board::WHITE)
			blockedPawns = ((board.bitboards[Board::WHITE_PAWN] << 8) & board.bitboards[Board::BLACK]) >> 8;
		else
			blockedPawns = ((board.bitboards[Board::BLACK_PAWN] >> 8) & board.bitboards[Board::WHITE]) << 8;
		
		U64 mobilityArea = ~(oppPawnAttacks | king | blockedPawns);
		
		//knights
		int n = BitBoardGen::popCount(attackInfo.knights[side] & mobilityArea);
		mg+= s * MOB_N[0][n];
		eg+= s * MOB_N[1][n];
		
		//bishops
		n = BitBoardGen::popCount(attackInfo.bishops[side] & mobilityArea);
		mg+= s * MOB_B[0][n];
		eg+= s * MOB_B[1][n];

		//rooks
		n = BitBoardGen::popCount(attackInfo.rooks[side] & mobilityArea);
		mg+= s * MOB_R[0][n];
		eg+= s * MOB_R[1][n];
		
		//queens
		n = BitBoardGen::popCount(attackInfo.queens[side] & mobilityArea);
		mg+= s * MOB_Q[0][n];
		eg+= s * MOB_Q[1][n];

		s = -1;
	}
}

ending_type Evaluation::get_ending(const Board& board){
	
	//KBN vs K
	for (int side = 0; side < 2; side++){
		int opp = side^1;
		if (BitBoardGen::popCount(board.bitboards[side]) == 1){
			if (BitBoardGen::popCount(board.bitboards[opp]) == 3){
				if (board.bitboards[Board::KNIGHT | opp] && board.bitboards[Board::BISHOP | opp]){
					return KNB_K;
				}
			}
		}
	}
	return OTHER_ENDING;
}

//K vs KBN
//k7/3bn3/8/4K3/8/8/8/8 b - -
int Evaluation::evalKBN_K(const Board& board, int side){

	int mg = 0;
	int eg = 0;
	int bishop_color = -1;
	
	if (board.bitboards[Board::WHITE_BISHOP]){
		U64 bishop = board.bitboards[Board::WHITE_BISHOP];
		bishop_color = BitBoardGen::COLOR_OF_SQ[numberOfTrailingZeros(bishop)];
	} else {
		U64 bishop = board.bitboards[Board::BLACK_BISHOP];
		bishop_color = BitBoardGen::COLOR_OF_SQ[numberOfTrailingZeros(bishop)];
	}
	
	assert(bishop_color > 0);
	int w_ks = numberOfTrailingZeros(board.bitboards[Board::WHITE_KING]);
	int b_ks = numberOfTrailingZeros(board.bitboards[Board::BLACK_KING]);
	
	materialBalance(board, mg, eg);
	mg+= KNB_K_MATE[bishop_color][w_ks];
	eg+= KNB_K_MATE[bishop_color][w_ks];
	mg-= KNB_K_MATE[bishop_color][b_ks];
	eg-= KNB_K_MATE[bishop_color][b_ks];
	
	int phase = get_phase(board);
	int eval = ((mg * (256 - phase)) + (eg * phase))/256;
	
	return side == Board::WHITE ? eval : -eval;
}

//maybe add same line as opp king/queen bonus? Same idea for rook
int Evaluation::evaluate(const Board& board, int side){

	/* TODO
	if (materialDraw())
		if (pawns){ return scale eval }
	*/
	if (materialDraw(board))
		return 0;
	else if (get_ending(board) == KNB_K)
		return evalKBN_K(board, side);
	
	int mg = 0;
	int eg = 0;
	
	materialBalance(board, mg, eg);
	pieceSquaresBalance(board, mg, eg);
	evalPawns(board, mg, eg);
	pieceOpenFile(board, mg, eg);
	kingAttack(board, mg);
	kingShelter(board, mg);
	evalBishops(board, mg, eg);
	evalRooks(board, mg, eg);	//CHANGE: 2 rooks or rook + queen
	// mobility(board, mg, eg);
	
	int phase = get_phase(board);
	int eval = ((mg * (256 - phase)) + (eg * phase))/256;
	
	return side == Board::WHITE ? eval : -eval;
}

/*
int main(){
	BitBoardGen::initAll();
	Zobrist::init_keys();
	Evaluation::initAll();
	Board board = FenParser::parseFEN("2kr1b2/1p3pr1/2p5/1p4n1/3N1Q1p/8/2P2P1n/2KR1B2 w - -");
	board.print();
	
	return 0;
}
*/