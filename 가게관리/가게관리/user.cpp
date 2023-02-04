#define P_MAX 600
#define BS_MAX 30000

#include <unordered_map>
#include <queue>
#include <memory.h>
//#include <iostream>

using namespace std;

unordered_map <int, int> B_hash, S_hash, P_hash;
int B_idx, S_idx, P_idx;

struct {
	int PId;
	int Price;
	int Curr_Q; 
	int init_Q;
	bool sold;  // ���0�ΰ��� ��Ҷ�������, �ǸŵȰ����� �����ϱ� ����
} B_DB[BS_MAX + 1];
struct sel_info {
	int BId;
	//int Price;
	int Sold_Q;
}; vector <sel_info>  S_DB[BS_MAX + 1];

int P_Qtt[P_MAX + 1]; // ��ǰ�� ���� ������

struct pdt{
	int BId;
	int Price;
	int Curr_Q;
};
struct comp {
	bool operator ()(pdt a, pdt b) {
		if (a.Price == b.Price) return a.BId > b.BId;
		return a.Price > b.Price;
	}
};
priority_queue <pdt, vector<pdt>, comp> P_PQ[P_MAX + 1];

void init() {
	// �ʱ�ȭ �ڵ�//
	B_hash.clear(); S_hash.clear(); P_hash.clear();
	int prev_M = max(B_idx, S_idx);
	for (int i = 0; i < prev_M; i++) {
		B_DB[i] = {}; S_DB[i] = {};
	}
	B_idx = S_idx = 1;
	// ### ����, �Ǹ� idx�� 1����. �����ǸԵ� ���µ� ������ ��� �Ÿ�����

	memset(P_Qtt, 0, sizeof(P_Qtt));
	for (int i = 0; i < P_idx; i++) {
		P_PQ[i] = {};
	}
	P_idx = 1;

}

int buy(int bId, int mProduct, int mPrice, int mQuantity) {
	// && �����ʹ� �׻��ؽ����̾ƴ� �������� DB�� �����ҰŴ� //
	B_hash[bId] = B_idx;
	B_DB[B_idx] = { mProduct, mPrice, mQuantity, mQuantity };
	B_idx++;

	int pidx = P_hash[mProduct];
	if (pidx == 0) { // ���ο� ��ǰ�� ��쿡��
		pidx = P_idx++;
		P_hash[mProduct] = pidx;
	}
	P_Qtt[pidx] += mQuantity;
	P_PQ[pidx].push({bId, mPrice, mQuantity});

	return P_Qtt[pidx];
}

int cancel(int bId) {
	int bidx = B_hash[bId];
	if (bidx == 0) return -1; // �����̷� ������ �����Ѵ�.
	if (B_DB[bidx].Curr_Q != B_DB[bidx].init_Q)  return -1;// ��� ���� or �̹� ����߰ų�, �ǸŵȰŸ�

	// DB���� ��� ���� ����
	int del_Q = B_DB[bidx].Curr_Q;
	B_DB[bidx].Curr_Q = 0;

	// ��ǰ�� ���� �ֽ�ȭ
	int pidx = P_hash[B_DB[bidx].PId];
	P_Qtt[pidx] -= del_Q;

	return P_Qtt[pidx];
} //test

int sell(int sId, int mProduct, int mPrice, int mQuantity) {
	int result = mQuantity * mPrice;
	// ������ ����
	int pidx = P_hash[mProduct];
	if (P_Qtt[pidx] < mQuantity) return -1; /// ��� �Ǹŷ����� ������

	pdt temp;
	P_Qtt[pidx] -= mQuantity; // ��� ���� �̸� ������
	// PQ���� ������ ���� �� �Ǹ� vector
	S_hash[sId] = S_idx;

	while (mQuantity != 0) {
		temp = P_PQ[pidx].top();
		P_PQ[pidx].pop();

		int bidx = B_hash[temp.BId];
		// �̹� ��� or �ȷȰų� ���� ���� �ֽ�ȭ �ݿ��� �ȵǾ� �ִٸ�
		if (B_DB[bidx].Curr_Q == 0) continue;
		if (temp.Curr_Q != B_DB[bidx].Curr_Q) {
			// �ֽ�ȭ ���� PQ push
			P_PQ[pidx].push({temp.BId,temp.Price, B_DB[bidx].Curr_Q });
			continue;
		}
		// ���� ���� �´ٸ� �Ǹ��Ѵ�.
		if (mQuantity >= temp.Curr_Q) {
			result -= temp.Price * temp.Curr_Q; /// �Ǹż��� ���
			B_DB[bidx].Curr_Q = 0;
			B_DB[bidx].sold = true;
			// ���ͻ���
			S_DB[S_idx].push_back({ temp.BId,temp.Curr_Q });
			mQuantity -= temp.Curr_Q;
		}
		else {
			result -= temp.Price * mQuantity;
			B_DB[bidx].Curr_Q -= mQuantity;
			// �ֽ�ȭ ���� push, ���ͻ���
			P_PQ[pidx].push({ temp.BId,temp.Price, B_DB[bidx].Curr_Q });
			S_DB[S_idx].push_back({ temp.BId,mQuantity });
			mQuantity = 0;
		}
	}
	
	S_idx++;
	return result;
}

int refund(int sId) {
	int sidx = S_hash[sId];
	if (sidx == 0) return -1;
	if (S_DB[sidx].empty()) return -1;

	int result=0;
	for (auto& itr : S_DB[sidx]) {
		int bidx = B_hash[itr.BId];
		int pidx = P_hash[B_DB[bidx].PId];
		// ����
		B_DB[bidx].sold = false;
		B_DB[bidx].Curr_Q += itr.Sold_Q;
		P_Qtt[pidx] += itr.Sold_Q;
		P_PQ[pidx].push({itr.BId, B_DB[bidx].Price,B_DB[bidx].Curr_Q });

		result += itr.Sold_Q;
	}
	S_DB[sidx].clear();
	return result;
}