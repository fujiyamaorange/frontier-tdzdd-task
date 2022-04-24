# frontier-tdzdd-task

### コンパイル

```
g++ frontier.cpp -o frontier.out -I include/
```

### 実行

```
./frontier.out graph-data/<グラフ>
```

example

```
./frontier.out graph-data/grid2x2.grh
```

### 画像ファイルを生成

```
dot -Tpng test.dot -o test.png
```
