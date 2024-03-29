﻿#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <numeric>

#include "Message.h"
#include "UctSearch.h"

using namespace std;


bool debug_message = false;


////////////////////////////////////
//  エラーメッセージの出力の設定  //
////////////////////////////////////
void
SetDebugMessageMode(const bool flag)
{
	debug_message = flag;
}

bool
GetDebugMessageMode()
{
	return debug_message;
}

///////////////////////
//  探索の情報の表示  //
///////////////////////
void
PrintPlayoutInformation(const uct_node_t *root, const po_info_t *po_info, const int finish_time, const int pre_simulated)
{
	double winning_percentage = (double)root->win / root->move_count;
	double finish_time_sec = finish_time / 1000.0;
	// KLDを表示
	float kld = 0;
	for (int i = 0; i < root->child_num; i++) {
		const int move_count = root->child[i].move_count;
		if (move_count > 0) {
			const float p = (float)move_count / root->move_count;
			kld += p * std::log(p / (root->child[i].nnrate + FLT_EPSILON));
		}
	}
	cout << "All Playouts       :  " << setw(7) << root->move_count << endl;
	cout << "Pre Simulated      :  " << setw(7) << pre_simulated << endl;
	cout << "Thinking Time      :  " << setw(7) << finish_time_sec << " sec" << endl;
	cout << "Winning Percentage :  " << setw(7) << (winning_percentage * 100) << "%" << endl;
	cout << "KLD Infomation     :  " << setw(7) << kld << endl;
	if (finish_time_sec != 0.0) {
		cout << "Playout Speed      :  " << setw(7) << (int)(po_info->count / finish_time_sec) << " PO/sec " << endl;
	}
}

//////////////////////
//  探索時間の出力  //
/////////////////////
void
PrintPlayoutLimits(const double time_limit, const int playout_limit)
{
	cout << "Time Limit    : " << time_limit << " Sec" << endl;
	cout << "Playout Limit : " << playout_limit << " PO" << endl;
}

////////////////////////////////////////
//  再利用した探索回数の出力          //
////////////////////////////////////////
void
PrintReuseCount(const int count)
{
	if (!debug_message) return;

	cout << "Reuse : " << count << " Playouts" << endl;
}
