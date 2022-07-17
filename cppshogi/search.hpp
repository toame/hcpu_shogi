﻿/*
  Apery, a USI shogi playing engine derived from Stockfish, a UCI chess playing engine.
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2018 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad
  Copyright (C) 2011-2018 Hiraoka Takuya

  Apery is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Apery is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef APERY_SEARCH_HPP
#define APERY_SEARCH_HPP

#include "move.hpp"
#include "usi.hpp"

// 時間や探索深さの制限を格納する為の構造体
struct LimitsType {
	LimitsType() {
		nodes = time[Black] = time[White] = inc[Black] = inc[White] = movesToGo = moveTime = mate = infinite = ponder = 0;
	}
	bool useTimeManagement() const { return !(mate | moveTime | nodes | infinite); }

	int time[ColorNum], inc[ColorNum], movesToGo, moveTime, mate, infinite, ponder;
	s64 nodes;
	Timer startTime;
};

struct Searcher {
    // static メンバ関数からだとthis呼べないので代わりに thisptr を使う。
    // static じゃないときは this を入れることにする。
    STATIC Searcher* thisptr;
	STATIC LimitsType limits;
	STATIC StateListPtr states;

    STATIC OptionsMap options;

    STATIC void init();

    STATIC void setOption(std::istringstream& ssCmd);
};

// 入玉勝ちかどうかを判定
template <bool CheckInCheck = true>
bool nyugyoku(const Position& pos) {
	// return false;
	// CSA ルールでは、一 から 六 の条件を全て満たすとき、入玉勝ち宣言が出来る。
	// 判定が高速に出来るものから順に判定していく事にする。

	// 一 宣言側の手番である。

	// この関数を呼び出すのは自分の手番のみとする。ponder では呼び出さない。

	// 六 宣言側の持ち時間が残っている。

	// 持ち時間が無ければ既に負けなので、何もチェックしない。

	// 五 宣言側の玉に王手がかかっていない。
	if (CheckInCheck)
		if (pos.inCheck())
			return false;

	const Color us = pos.turn();
	// 敵陣のマスク
	const Bitboard opponentsField = (us == Black ? inFrontMask<Black, Rank4>() : inFrontMask<White, Rank6>());

	// 二 宣言側の玉が敵陣三段目以内に入っている。
	if (!pos.bbOf(King, us).andIsAny(opponentsField))
		return false;

	// 四 宣言側の敵陣三段目以内の駒は、玉を除いて10枚以上存在する。
	const int ownPiecesCount = (pos.bbOf(us) & opponentsField).popCount() - 1;
	if (ownPiecesCount < 10)
		return false;

	// 三 宣言側が、大駒5点小駒1点で計算して
	//     先手の場合28点以上の持点がある。
	//     後手の場合27点以上の持点がある。
	//     点数の対象となるのは、宣言側の持駒と敵陣三段目以内に存在する玉を除く宣言側の駒のみである。
	const int ownBigPiecesCount = (pos.bbOf(Rook, Dragon, Bishop, Horse) & opponentsField & pos.bbOf(us)).popCount();
	const int ownSmallPiecesCount = ownPiecesCount - ownBigPiecesCount;
	const Hand hand = pos.hand(us);
	const int val = ownSmallPiecesCount
		+ hand.numOf<HPawn>() + hand.numOf<HLance>() + hand.numOf<HKnight>()
		+ hand.numOf<HSilver>() + hand.numOf<HGold>()
		+ (ownBigPiecesCount + hand.numOf<HRook>() + hand.numOf<HBishop>()) * 5;

	// 合計を数える
	const Bitboard BlackField = inFrontMask<Black, Rank4>();
	const Bitboard WhiteField = inFrontMask<White, Rank6>();

	const int BlackBigPiecesCount = (pos.bbOf(Rook, Dragon, Bishop, Horse) & pos.bbOf(Color(Black))).popCount();
	const int WhiteBigPiecesCount = (pos.bbOf(Rook, Dragon, Bishop, Horse) & pos.bbOf(Color(White))).popCount();

	const int BlackPiecesCount = (pos.bbOf(Color(Black))).popCount() - 1;
	const int WhitePiecesCount = (pos.bbOf(Color(White))).popCount() - 1;

	const int BlackSmallPiecesCount = BlackPiecesCount - BlackBigPiecesCount;
	const int WhiteSmallPiecesCount = WhitePiecesCount - WhiteBigPiecesCount;

	const Hand Blackhand = pos.hand(Color(Black));
	const Hand Whitehand = pos.hand(Color(White));

	const int valB = BlackSmallPiecesCount
		+ Blackhand.numOf<HPawn>() + Blackhand.numOf<HLance>() + Blackhand.numOf<HKnight>()
		+ Blackhand.numOf<HSilver>() + Blackhand.numOf<HGold>()
		+ (BlackBigPiecesCount + Blackhand.numOf<HRook>() + Blackhand.numOf<HBishop>()) * 5;
	const int valW = WhiteSmallPiecesCount
		+ Whitehand.numOf<HPawn>() + Whitehand.numOf<HLance>() + Whitehand.numOf<HKnight>()
		+ Whitehand.numOf<HSilver>() + Whitehand.numOf<HGold>()
		+ (WhiteBigPiecesCount + Whitehand.numOf<HRook>() + Whitehand.numOf<HBishop>()) * 5;



#if defined LAW_24
	if (val < 31)
		return false;
#else
	if (val < (us == Black ? 28 : (valB + valW - 27)))
		return false;
#endif

	return true;
}

#endif // #ifndef APERY_SEARCH_HPP
