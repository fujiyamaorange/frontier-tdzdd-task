#include <tdzdd/DdSpec.hpp>
#include <tdzdd/DdStructure.hpp>
#include <tdzdd/DdEval.hpp>

#include <vector>
#include <fstream>
#include <iostream>

// グラフを表すクラス Graph
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
  // 計算状態が配列
  Graph const G;

public:
  PathZDD(Graph G) : G(G)
  {
    setArraySize(G.numEdges() + 1);
  }

  int getRoot(int *mate) const
  {
    // 初期状態を記述
    // mateはint型の配列
    // 辺の本数
    mate[0] = 100000;
    for (int i = 1; i < G.numEdges() + 1; i++)
    {
      mate[i] = i;
      // std::cout << mate[i] << std::endl;
    }

    std::cout << "mate配列の中身確認" << std::endl;
    for (int i = 0; i < G.numEdges() + 1; i++)
    {
      std::cout << mate[i] << std::endl;
    }

    // std::map<int, std::set<int> > frontier;

    std::cout << "G.getStart(): ";
    std::cout << G.getStart() << std::endl;

    std::cout << "G.getTerminal(): ";
    std::cout << G.getTerminal() << std::endl;
    return G.numEdges();
  }

  int getChild(int *mate, int level, int value) const
  {
    // level: ZDDの変数番号(根節点が最も大きい)
    // 終端節点は特殊(0-終端:0 1-終端:-1)
    // value: 枝の種類(0 or 1)

    std::cout << "level: ";
    std::cout << level << std::endl;
    // 後ろから探索していく→辺の両端の点を取得する
    std::pair<int, int> pair = G.getEdge(G.numEdges() - level);
    int u = pair.first;
    int v = pair.second;
    std::cout << "level: ";
    std::cout << level << std::endl;
    std::cout << "u: ";
    std::cout << u << std::endl;
    std::cout << "v: ";
    std::cout << v << std::endl;

    if (value == 1)
    {
      // 中点の枝刈り
      if (mate[u] == 0 || mate[v] == 0)
      {
        std::cout << "中点！" << std::endl;
        return 0;
      }
      // 閉路の枝刈り
      if (mate[u] == v && mate[v] == u)
      {
        std::cout << "閉路！" << std::endl;
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
        // mate[u] = 0;
        // mate[mate[u]] = v;
        // mate[v] = mate[u];
      }
      else if (mate[u] == u && mate[v] != v)
      {
        // 既存パスの延長
        // 逆バージョン
        int tmp = mate[u];
        mate[u] = mate[v];
        mate[mate[v]] = tmp;
        mate[v] = 0;

        // mate[v] = 0;
        // mate[mate[v]] = u;
        // mate[u] = mate[v];
      }
      else if (mate[mate[u]] == u && mate[mate[v]] == v)
      {
        // 2つのパスの融合
        // int w = mate[u];
        // int x = mate[v];
        mate[mate[u]] = mate[v];
        mate[mate[v]] = mate[u];
        mate[u] = 0;
        mate[v] = 0;
      }
      else if (mate[mate[v]] == v && mate[mate[u]] == u)
      {
        // 2つのパスの融合
        // 逆バージョン
        // int w = mate[v];
        // int x = mate[u];
        mate[mate[v]] = mate[u];
        mate[mate[u]] = mate[v];
        mate[v] = 0;
        mate[u] = 0;
      }
    }

    // sが中点の枝刈り
    if (mate[G.getStart()] == 0)
    {
      std::cout << "sが中点！" << std::endl;
      return 0;
    }

    // TODO: s-tパスが孤立する枝刈り
    // sが孤立したとき
    if (level == 0)
    {
      // 最後まで行ったとき
      if (mate[G.getStart()] == G.getStart())
      {
        std::cout << "sが孤立！" << std::endl;
        return 0;
      }

      // tが孤立したとき
      if (mate[G.getTerminal() == G.getTerminal()])
      {
        std::cout << "tが孤立" << std::endl;
        return 0;
      }
    }

    // TODO: sとt以外の頂点が短点となってフロンティアから抜ける枝刈り

    // TODO: 終端節点チェック(解の完成)
    if (mate[G.getStart()] == G.getTerminal() && mate[G.getTerminal()] == G.getStart())
    {
      int success = 1;
      for (int i = 1; i < G.numVertices() + 1; i++)
      {
        if (i == G.getStart() || i == G.getTerminal())
          continue;
        if (mate[i] != 0 && mate[i] != i)
        {
          success = 0;
          break;
        }
      }
      // 解の完成
      if (success == 1)
        return -1;

      return 0;
    }

    // 冗長なパスあり
    if (mate[G.getStart()] == G.getTerminal() && mate[G.getTerminal()] == G.getStart())
    {
      if (mate[v] == u && mate[u] == v)
      {
        std::cout << "冗長なパス！" << std::endl;
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
  G.print();

  PathZDD pathZdd(G);
  tdzdd::DdStructure<2> dd(pathZdd);
  std::cout << "dd.zddCardinality(): ";
  std::cout << dd.zddCardinality() << std::endl;
  dd.dumpDot();
}
