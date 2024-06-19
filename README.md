# edu_2024-DigitalImageProcessing
2024年度 ディジタル映像処理及び演習 の授業で作成したプログラムたち。

## 仕様てきな
- 言語: C++
- 使用ライブラリ: OpenCV

## ディレクトリ構造(理想)
```
01_2024-04-01
├dst
│ └(出力ファイル)
├src
│ └(素材データ)
└(ソースコード)
```

## VSCode Coderunner の設定
```json
{
    "code-runner.executorMap": {
        "cpp": "cd $dir && g++ $fileName -std=c++11 `pkg-config --cflags --libs opencv4` && ./a.out",
    }
}
```
