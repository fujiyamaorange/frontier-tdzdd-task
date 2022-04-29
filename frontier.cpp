#include <tdzdd/DdSpec.hpp>
#include <tdzdd/DdStructure.hpp>
#include <tdzdd/DdEval.hpp>

#include <vector>
#include <fstream>
#include <iostream>
#include <map>
#include <iostream>
#include <fstream>
#include <string>

std::map<int, std::set<int> > frontier;
int countBranchCut;
int countGoal;

class Graph
{
private:
  int const s;
  int const t;
  int const n;
  int const m;
  std::vector<std::pair<int, int> > const edge_list;

public:
  Graph(int s, int t, int n, int m, std::vector<std::pair<int, int> > edge_list)
      : s(s), t(t), n(n), m(m), edge_list(edge_list)
  {
  }

  std::pair<int, int> getEdge(int e) const
  {
    return std::make_pair(edge_list[e].first, edge_list[e].second);
  }

  int getStart() const
  {
    return s;
  }

  int getTerminal() const
  {
    return t;
  }

  // 頂点
  int numVertices() const
  {
    return n;
  }

  // 辺
  int numEdges() const
  {
    return m;
  }

  // frontier[i[にはiの時にフロンティアから抜ける点を格納する
  void constructFrontier() const
  {
    std::set<int> tmpset;
    std::set<int> nokori;
    for (int i = 0; i < numEdges(); i++)
    {
      int u = getEdge(i).first;
      int v = getEdge(i).second;
      tmpset.insert(u);
      tmpset.insert(v);

      // std::cout << "tmpsetの確認開始" << std::endl;
      // std::set<int>::iterator t = tmpset.begin();
      // while (t != tmpset.end())
      // {
      //   std::cout << *t++ << std::endl;
      // }
      // std::cout << "tmpsetの確認終了" << std::endl;

      // これから出てくる点の集合をnokoriにまとめる
      for (int j = i + 1; j < numEdges(); j++)
      {
        int x = getEdge(j).first;
        int y = getEdge(j).second;
        nokori.insert(x);
        nokori.insert(y);
      }
      // std::cout << "nokoriの確認開始" << std::endl;
      // std::set<int>::iterator n = nokori.begin();
      // while (n != nokori.end())
      // {
      //   std::cout << *n++ << std::endl;
      // }
      // std::cout << "nokoriの確認終了" << std::endl;

      if (nokori.size() <= 0)
        continue;

      std::set<int>::iterator k = tmpset.begin();
      for (; k != tmpset.end(); k++)
      {
        decltype(tmpset)::iterator it = nokori.find(*k);
        if (it == nokori.end())
        {
          // 見つからなかった場合
          frontier[i].insert(*k);
          tmpset.erase(*k);
        }
      }

      nokori.clear();
    }

    // std::cout << "フロンティアの確認開始" << std::endl;
    // for (int i = 0; i < numEdges(); i++)
    // {
    //   std::cout << i;
    //   std::cout << "の時のフロンティア" << std::endl;
    //   std::set<int>::iterator k = frontier[i].begin();
    //   for (; k != frontier[i].end(); k++)
    //   {
    //     std::cout << *k << std::endl;
    //   }
    // }
    // std::cout << "フロンティアの確認終了" << std::endl;
  }

  void printEdges() const
  {
    // std::cout << "#edges: " << m << std::endl;
    for (auto it = edge_list.begin(); it != edge_list.end(); ++it)
    {
      std::cout << (*it).first << ", " << (*it).second << std::endl;
    }
  }

  void print() const
  {
    std::cout << "#Vertices: " << numVertices() << std::endl;
    std::cout << "#Edges: " << numEdges() << std::endl;
    std::cout << "Start, Terminal = " << s << ", " << t << std::endl;
    std::cout << "Edge List: " << std::endl;
    printEdges();
  }
};

// TdZdd で表現すべき部分
class PathZDD : public tdzdd::PodArrayDdSpec<PathZDD, int, 2>
{
  Graph const G;

public:
  PathZDD(Graph G) : G(G)
  {
    setArraySize(G.numVertices() + 1);
  }

  int getRoot(int *mate) const
  {
    // 初期状態を記述

    // 1-index
    mate[0] = 100000;
    for (int i = 1; i < G.numVertices() + 1; i++)
    {
      mate[i] = i;
    }

    return G.numEdges();
  }

  int getChild(int *mate, int level, int value) const
  {
    // level: ZDDの変数番号(根節点が最も大きい)
    // 終端節点は特殊(0-終端:0 1-終端:-1)
    // value: 枝の種類(0 or 1)

    // 後ろから探索していく→辺の両端の点を取得する
    std::pair<int, int> pair = G.getEdge(G.numEdges() - level);
    int u = pair.first;
    int v = pair.second;

    if (value == 1)
    {
      // 中点の枝刈り
      if (mate[u] == 0 || mate[v] == 0)
      {
        countBranchCut++;
        return 0;
      }
      // 閉路の枝刈り
      if (mate[u] == v && mate[v] == u)
      {
        countBranchCut++;
        return 0;
      }

      if (mate[u] == u && mate[v] == v)
      {
        // 新しいペアの出現
        mate[u] = v;
        mate[v] = u;
      }
      else if (mate[u] != u && mate[v] == v)
      {
        // 既存パスの延長
        int tmp = mate[v];
        mate[v] = mate[u];
        mate[mate[u]] = tmp;
        mate[u] = 0;
      }
      else if (mate[u] == u && mate[v] != v)
      {
        // 既存パスの延長-逆バージョン
        int tmp = mate[u];
        mate[u] = mate[v];
        mate[mate[v]] = tmp;
        mate[v] = 0;
      }
      else if (mate[u] != u && mate[v] != v)
      {
        // 2つのパスの融合
        mate[mate[u]] = mate[v];
        mate[mate[v]] = mate[u];
        mate[u] = 0;
        mate[v] = 0;
      }
    }

    // sが中点の枝刈り
    if (mate[G.getStart()] == 0)
    {
      countBranchCut++;
      return 0;
    }

    // sとt以外の頂点が短点となってフロンティアから抜ける枝刈り
    std::set<int> thisTimeFrontier = frontier[G.numEdges() - level];
    std::set<int>::iterator f = thisTimeFrontier.begin();
    for (; f != thisTimeFrontier.end(); f++)
    {
      // sにつながる辺がなくなった時
      if (*f == G.getStart() && mate[*f] == G.getStart())
      {
        countBranchCut++;
        return 0;
      }

      if (*f == G.getStart() || *f == G.getTerminal())
        continue;

      // 端点である
      if (mate[*f] != 0 && mate[*f] != *f)
      {
        countBranchCut++;
        return 0;
      }

      // 効率的な実装のために(スライド6回目16p)
      if (mate[*f] == 0 || mate[*f] == *f)
      {
        if (*f != G.getStart())
        {
          mate[*f] = 0;
        }
      }
    }

    // 最後まで行ったとき
    if (level == 1)
    {
      // sが孤立
      if (mate[G.getStart()] == G.getStart())
      {
        countBranchCut++;
        return 0;
      }

      // tが孤立したとき
      if (mate[G.getTerminal()] == G.getTerminal())
      {
        countBranchCut++;
        return 0;
      }
    }

    // 終端節点チェック(解の完成)
    if (mate[G.getStart()] == G.getTerminal() && mate[G.getTerminal()] == G.getStart())
    {
      int success = 1;
      for (int i = 1; i < G.numVertices() + 1; i++)
      {
        if (i == G.getStart() || i == G.getTerminal())
          continue;
        if (mate[i] != 0 && mate[i] != i)
        {
          return 0;
        }
      }
      // 解の完成
      return -1;
    }

    // 冗長なパスあり
    if (mate[G.getStart()] == G.getTerminal() && mate[G.getTerminal()] == G.getStart())
    {
      if (mate[v] == u && mate[u] == v)
      {
        countBranchCut++;
        return 0;
      }
    }

    // 終端節点チェック→枝刈りなら0，解が完成なら-1を返す
    return level - 1;
  }
};

// 入力ファイルの読み込み
Graph readGraph(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cerr << "Please input a graph file" << std::endl;
    exit(-1);
  }
  std::ifstream ifs(argv[1]);
  if (!ifs)
  {
    std::cerr << "Error: cannot open file " << argv[1] << std::endl;
    exit(-1);
  }

  int s, t;
  if (ifs)
    ifs >> s >> t;

  std::vector<std::pair<int, int> > edge_list;
  int u, v;
  int n = 0;
  while (ifs >> u >> v)
  {
    edge_list.push_back(std::make_pair(u, v));
    if (n < u)
      n = u;
    if (n < v)
      n = v;
  }
  Graph G(s, t, n, edge_list.size(), edge_list);

  return G;
}

int main(int argc, char **argv)
{
  Graph G = readGraph(argc, argv);
  // G.print();

  PathZDD pathZdd(G);
  // フロンティアを構築
  G.constructFrontier();
  tdzdd::DdStructure<2> dd(pathZdd);
  std::cout << "dd.zddCardinality(): ";
  std::cout << dd.zddCardinality() << std::endl;
  // dd.dumpDot();
}
