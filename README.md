# frontier-tdzdd-task

### コンパイル
```
g++ frontier.cpp -o frontier -I include/
```

### 実行
```
./frontier graph-data/<グラフ>
```
example
```
./frontier graph-data/grid2x2.grh
```

### 画像ファイルを生成
```
dot -Tpng test.dot -o test.png   
```
