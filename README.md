# テンション・オセロ (Tension Othello)

挟んだ石がすぐには裏返らない、リアルタイムのオセロ。仕様は [specs/001-new-othello/spec.md](specs/001-new-othello/spec.md)。

## ビルド (Windows / Visual Studio + CMake)

DxLib(https://dxlib.xsrv.jp/ の Visual C++ 用)をダウンロードし、`third_party/dxlib/DxLibNoneModel_NoneSoftDrawCode_VC/` に `DxLib.h` などが来るように展開してから(`third_party/` は Git 管理外です):

```
cmake -S . -B build
cmake --build build --config Release
```

- ゲーム: `build/Release/sandwich_othello.exe`(assets/ は自動でコピーされます)
- エンジンテスト: `build/Release/engine_tests.exe`(DxLib なしでもビルド可)

## 操作

- タイトル: スタート / Enter
- ゲーム: 緑の点のマスを左クリックで配置(黒=あなた、白=AI)。R でリスタート、Esc で終了
- ウィンドウは枠をドラッグしてリサイズできます
