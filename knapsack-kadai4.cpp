#include <tdzdd/DdSpec.hpp>
#include <tdzdd/DdStructure.hpp>
#include <tdzdd/DdEval.hpp>

class KnapsackZdd : public tdzdd::DdSpec<KnapsackZdd, int, 2>
{
  // インスタンス変数: 問題の入力
  int const n;
  int const *w;
  int const W;

public:
  KnapsackZdd(int n, int *w, int W) : n(n), w(w), W(W)
  {
    // 追記
  }

  int getRoot(int &state) const
  {
    // 初期状態
    // 計算状態を格納する
    state = 0;
    // ZDDの変数番号を返す
    return n;
  }

  int getChild(int &state, int level, int value) const
  {
    // 子節点の計算状態を計算しその節点のレベル(変数番号)を返す
    // value: 枝の種類(0 or 1)
    if (value == 1)
      state += w[n - level];
    level--;

    if (state > W)
      // 0-終端の場合は0を返す
      return 0;
    if (level == 0)
      // 1-終端の場合は-1を返す
      return -1;

    return level;
  }
};

// 評価の仕様を定義する
class MaxElement : public tdzdd::DdEval<MaxElement, int>
{
  int const n;
  int const *c;

public:
  MaxElement(int n, int *c) : n(n), c(c) {}

  // 終端節点の評価値を定義
  void evalTerminal(int &val, bool one) const
  {
    val = one ? 0 : INT_MIN;
  }

  // valuesに全ての子節点の評価値が格納されている
  void evalNode(int &val, int level, tdzdd::DdValues<int, 2> const &values) const
  {
    // levelとvaluesから現在の節点の評価値を計算しvalに格納する
    val = std::max(values.get(0), values.get(1) + c[n - level]);
  }
};

int main(int argc, char **argv)
{
  int n = 6, W = 15;
  int w[] = {4, 4, 3, 5, 8, 2};
  // インスタンス化
  KnapsackZdd knapsack(n, w, W);
  int v[] = {5, 8, 4, 6, 12, 4};
  // ZDDを生成
  tdzdd::DdStructure<2> dd(knapsack);
  std::cout << "解の数: ";
  std::cout << dd.zddCardinality() << std::endl;
  std::cout << "最適解: ";
  // evaluate関数を実行する
  std::cout << dd.evaluate(MaxElement(n, v)) << std::endl;

  for (auto it = dd.begin(); it != dd.end(); ++it)
  {
    // 実行可能回を1行ずつ出力
    std::cout << "実行解: ";
    for (auto itr = (*it).begin(); itr != (*it).end(); ++itr)
    {
      std::cout << n - *itr + 1 << " ";
    }
    std::cout << std::endl;
  }
}
