# Solomon: Simple Off-LOading Macros Orchestrating multiple Notations

[![ci](https://github.com/ymiki-repo/solomon/actions/workflows/ci.yml/badge.svg)](https://github.com/ymiki-repo/solomon/actions/workflows/ci.yml)

| | English | 日本語 |
| ---- | ---- | ---- |
| Fortran（**v2.0.0から追加**） | [README](README_f90.md) | [このファイル](README_f90_jp.md) |
| C/C++ | [README](README.md) | [README](README_jp.md) |

## 概要

* GPU向け指示文（OpenACC, OpenMP target）統合マクロ
* 現在のバージョン: v2.0.1（`solomon/solomon.hpp` 内の `SOLOMON_VERSION` として定義）
* 詳細については [Miki & Hanawa (2024, IEEE Access, vol. 12, pp. 181644-181665)](https://doi.org/10.1109/ACCESS.2024.3509380) を参照してください
* 使用した際には [Miki & Hanawa (2024, IEEE Access, vol. 12, pp. 181644-181665)](https://doi.org/10.1109/ACCESS.2024.3509380) を引用してください
* Released under the MIT license, see LICENSE.txt
* Copyright (c) 2024 Yohei MIKI

## 目次

* [概要](#概要)
* [意義](#意義)
  * [背景](#背景)
  * [開発方針と特徴](#開発方針と特徴)
* [変更履歴（Changelog）](#変更履歴changelog)
  * [v2.0.1（2026-11-10）](#v2012026-11-10)
  * [v2.0.0（2026-11-04）](#v2002026-11-04)
* [使い方](#使い方)
  * [Solomon を用いたコードの開発方法](#solomon-を用いたコードの開発方法)
  * [Solomon を使ったコードのコンパイル方法](#solomon-を使ったコードのコンパイル方法)
  * [v1.x との後方互換性（v2.0.0 以降）](#v1x-との後方互換性v200-以降)
  * [Solomon の拡張方法（コードジェネレータを用いた更新方法）](#solomon-の拡張方法コードジェネレータを用いた更新方法)
  * [プロファイラタグ（NVTX, rocTX, ITT）](#プロファイラタグnvtx-roctx-itt)
  * [エディタ支援（シンタックスハイライトと clang-format）](#エディタ支援シンタックスハイライトと-clang-format)
* [サンプルコード](#サンプルコード)
  * [diffusion: メモリ律速な問題の実装例](#diffusion-メモリ律速な問題の実装例)
  * [declare: 宣言の指示文の使用例](#declare-宣言の指示文の使用例)
  * [サンプルの実行方法](#サンプルの実行方法)
* [Solomon が提供する API](#solomon-が提供する-api)
  * [使用可能な指示文](#使用可能な指示文)
  * [使用可能な指示節・指示句](#使用可能な指示節指示句)
  * [使用可能なランタイム関数](#使用可能なランタイム関数)
* [謝辞](#謝辞)

## 意義

### 背景

* 指示文を用いたGPU化は，CUDA/HIP/SYCLなどの低レベルな開発環境を用いたGPU化に比べて移植工数が抑えられます
* GPU化に使われる代表的な指示文としては OpenACC と OpenMP target がありますが，両者にはトレードオフがあります
  * OpenACC
    * 広く使われている指示文であり，先行して開発されてきたことから機能・ドキュメントがより充実しています
    * AMD，Intel は OpenACC をサポートするコンパイラを提供していないため，実質的にNVIDIA製GPU向けの指示文となっています
      * HPE Cray コンパイラによる AMD GPU 向けの OpenACC サポートなどもあります
  * OpenMP target
    * 主要GPUベンダー（NVIDIA，AMD，Intel）全社が OpenMP target 対応コンパイラを提供しています
    * OpenACC の全ての機能に対応できてはいません
* 指示文を用いたGPU化においては，こうしたトレードオフを踏まえた上でどの指示文を用いるかを決断する必要があります

### 開発方針と特徴

* ベンダーロックインからの解放を実現するため，Solomon の開発では下記項目の実現を目標としています
  * 指示文的な記法を用い，OpenACC と OpenMP target 両方を統一的な記法から利用
  * 指示文を用いたGPU化の初心者，OpenACC あるいは OpenMP target を用いた実装に慣れている開発者の双方にとっての学習コストの低減
* Solomon はプリプロセッサマクロを用いた指示文への展開機能までを提供します
  * 実際のバイナリ生成はGPUベンダー製のコンパイラなどに任せるため，コンパイラの性能向上や機能拡張の恩恵をそのまま受けられます
  * 単なるマクロの集積なので，更新が停滞したとしてもユーザーコードが動かなくなるような悪影響はありません
  * ユーザーサイドでマクロを付け足すことも簡単です

## 変更履歴（Changelog）

### v2.0.1（2026-11-10）

`solomon/` のマクロの展開は v2.0.0 と同じです（ヘッダは `solomon.hpp` の `SOLOMON_VERSION` を除いて clang-format で書式を整えただけです）．このリリースはサンプル，同梱の道具，文書，リポジトリの検査を変えます．

* **継続的インテグレーション**: GitHub Actions で，サンプルをホストのコンパイラ（GCC と gfortran．C/C++ のサンプルは Clang でも．逐次とホストの OpenMP）でビルドして展開結果を確かめ，シェルスクリプトに shellcheck を掛け，移行スクリプトを dry-run で試します．GPU は使いません．結果は各 README の冒頭のバッジに出ます．同じ検査は `bash misc/ci/run_tests.sh` で手元でも実行できます（bash 4.4 以降と GNU make）
* **サンプルの Makefile の `SOLOMON_DIR`**: `:` や glob の文字（`[ ] * ?`）を含むパスでは，`make` がメッセージを出して止まるようになりました
* **Intel コンパイラでの Fortran のサンプル**: 事前コンパイルのオプション `-Xs "-device ..."` を，C のサンプルと同じくリンクのときだけ渡します
* **サンプルのジョブスクリプト**: `rocm-smi` / `nvidia-smi` が無いときは `numactl --show` が示す NUMA ノードを使い，diffusion は `DETECT_NUMA=1` のとき先に `numactl` の有無を確かめます．`module load` に失敗したときは `MODULE_USE` も確かめるよう促します．`RUN_TIMEOUT=0` は受け付けません（正の上限が要ります）．結果の移動に失敗するとジョブは失敗します．bash の POSIX モード（bash である `sh` など）で実行したジョブスクリプトがロックファイル `sh/.lock` に書けないときは，何も出さずに終わらずにそれを知らせます
* **サンプルの Makefile**: すべてのサンプルの Makefile が，GNU make の組み込みの規則と同じく，`TARGET_ARCH` をコンパイラに，`LDFLAGS` / `LDLIBS` をリンカに渡します．サンプル自身が要るライブラリ（diffusion の `-lm`）は，コマンド行で `LDLIBS` を与えても残ります
* **移行スクリプト**: 書き換えずに残した v1.x の名前（ビルドのファイルの変数と代入，`#pragma` や `!$omp` などの生の指示文の行）を知らせます．行末の空白の後で続く `#pragma` を GCC と同じく続けて読み，UTF-8 のバイト順マークの後の `#include` と `#pragma` も見つけ，たどり着けないファイルを知らせます
* **文書**: 空の引数を指示節の上限に数えないという注記に，33 個以上続く場合への参照を足しました．必須の指示節の無いまま直接書いた OpenMP の data 系の指示文，OpenACC に対応物の無い指示節（`OMP_TARGET_CLAUSE_LINEAR(...)`，`OMP_TARGET_CLAUSE_LASTPRIVATE(...)`），ifx の `interop` 構文，`SOLOMON_SYNCHRONIZE(...)` に指示節のマクロを渡すとコンパイラが止まることの説明を足しました
* **同梱のファイル**: `.editorconfig` が JSON，YAML，TOML，gnuplot，Vim script，Emacs Lisp のファイルも扱い，`solomon/util/jl/` の生成器はシンボリックリンクから実行しても自分の隣にヘッダを書きます

### v2.0.0（2026-11-04）

* **Fortran サポート**: `spp.sh` / `fortran.mk` による前処理を経て，C/C++ と同じマクロが Fortran コードでも使えるようになりました（この README）
* **名前空間の整理**: ユーザー向けマクロを `SOLOMON_` / `SOLOMON_CLAUSE_` 接頭辞に改名しました．v1.x の綴りは `-DSOLOMON_WITH_SHORT_NAMES` でそのまま使え，移行スクリプトも同梱しています．下請けの `PRAGMA(...)` / `PRAGMA_STR(x)` / `PRAGMA_STR1(x)` / `SHOW_PRAGMA(...)` は内部名になり，v1.x の名前は `-DSOLOMON_WITH_SHORT_NAMES` のときだけ定義されます（[v1.x との後方互換性](#v1x-との後方互換性v200-以降)）
* **簡易記法を2階層に再設計**: 並列化の階層は CUDA の block / thread に倣います．`SOLOMON_OFFLOAD_OUTER_LOOP(...)` / `SOLOMON_PARALLELIZE_INNER_LOOP(...)` と `SOLOMON_CLAUSE_NUM_BLOCKS(n)` / `SOLOMON_CLAUSE_NUM_THREADS(n)` により，gang↔teams / vector↔thread の対応を指定できます（`SOLOMON_CLAUSE_BLOCK` / `SOLOMON_CLAUSE_THREAD` は OpenACC だけのヒントで，OpenMP target では無視されます）
* **OpenACC でのブロック系の指示節の意味**（v1.x から変更）: `SOLOMON_CLAUSE_BLOCK` / `SOLOMON_CLAUSE_NUM_BLOCKS(n)`（v1.x の `AS_BLOCK` / `NUM_BLOCKS(n)`）は OpenACC で `worker` / `num_workers(n)` ではなく `gang` / `num_gangs(n)` を出力するようになりました．`SOLOMON_CLAUSE_NUM_GRIDS(n)`（v1.x の `NUM_GRIDS(n)`）は OpenMP target でも `num_teams(n)` を出力します（[v1.x との後方互換性](#v1x-との後方互換性v200-以降)）
* **シリアルフォールバック**: OpenACC・OpenMP のいずれも有効でない場合，全指示文が除去されシリアルコードとしてコンパイルされます（`-D_OPENMP` の手動指定は不要になりました）．このとき `SOLOMON_APPEND_ARGS(...)` と v1.x の名前 `APPEND_ARGS(...)` も空に展開され，引数は消えます
* **デバイス常駐変数**: `SOLOMON_DECLARE_ON_DEVICE(...)`（link 意味論には `SOLOMON_DECLARE_ON_DEVICE_LINKED(...)`）
* **逐次オフロード領域**: `SOLOMON_OFFLOAD_SERIAL`（`acc serial` ↔ 裸の `omp target` の相互変換）
* **関数のオフロード宣言の改善**: `SOLOMON_DECLARE_OFFLOADED` の出力不正を修正し，名前形式 `SOLOMON_CLAUSE_TARGETS(...)` を追加しました
* **ベンダー中立プロファイラタグ**: `-DSOLOMON_TAGGED_PROFILE(_AUTO)` で NVTX / rocTX / ITT のタグを発行します（[プロファイラタグ](#プロファイラタグnvtx-roctx-itt)）
* **GCC での自動のプロファイラタグの行番号**: GCC の前処理系は長いファイルの後ろの方のタグに誤った行番号を書くことがあるため，`spp.sh` と `fortran.mk` は `-ftrack-macro-expansion=0` を GCC の `cpp` に渡します．g++ / gcc でビルドする C/C++ は [プロファイラタグ](#プロファイラタグnvtx-roctx-itt) を参照してください
* **エディタ支援**: vim / emacs / VS Code 向けシンタックスハイライトと `.clang-format` を同梱しています（[エディタ支援](#エディタ支援シンタックスハイライトと-clang-format)）
* **サンプルのジョブスクリプト**: 手順を `sh/common/` に共通化し，システムごとには `sh/<system>/config.sh` に設定を書くだけで済むようにしました（[サンプルの実行方法](#サンプルの実行方法)）．`verify.sh` は結果を判定し（誤った結果ではジョブが失敗します），同じサンプルのディレクトリで別のジョブスクリプトが実行中のときと，前の実行の生成物が残っているときは止まります
* **Fortran の前処理の道具**: `spp.sh` と `fortran.mk` は，OpenACC / OpenMP が有効か，ソースをどう読むかを，渡されたフラグのままコンパイラ自身に尋ねます．コンパイラが答えないとき（知らないフラグなど）と，ソースを固定形式として読むときはエラーで止まります．`spp.sh` のソースは最後の引数に置きます（[Solomon を使ったコードのコンパイル方法](#solomon-を使ったコードのコンパイル方法)）
* **移行スクリプト**: 既定では `solomon.hpp` を読み込むソースだけを書き換えます（`--all` で渡したソースすべてを書き換えます）（[v1.x との後方互換性](#v1x-との後方互換性v200-以降)）
* **キューの待ち**: `SOLOMON_CLAUSE_WAIT_QUEUE(...)` で，指定したキューの完了を待ってから指示文を始められます（OpenACC の `wait(...)`．OpenMP target では無視されます）
* **`wait` 節のまとめ**: 同じ指示文の `wait` 節を，OpenACC の規定どおり `device_type` の区切りごとにまとめます（`wait` と `wait(1)` は `wait` に，`wait(1)` と `wait(2)` は `wait(1, 2)` に．`devnum:` / `queues:` の待ちは残します）
* **`simd` だけが取る節の Solomon の記法**: `SOLOMON_CLAUSE_SIMDLEN(length)`・`SOLOMON_CLAUSE_SAFELEN(length)`・`SOLOMON_CLAUSE_ALIGNED(...)`・`SOLOMON_CLAUSE_NONTEMPORAL(...)`・`SOLOMON_CLAUSE_IF_SIMD(condition)` を追加しました．`SOLOMON_CLAUSE_INDEPENDENT` で `simd` が付く所に出ます（OpenMP のみ）．nvfortran が拒否する `SOLOMON_CLAUSE_NONTEMPORAL(...)` は，そのコンパイラではわざと捨てます（[使用可能な指示節・指示句](#使用可能な指示節指示句)）
* **v1.1.0 の指示節の並べ替えの記述の訂正**: 自動の並べ替えで先頭へ動くのは `SOLOMON_CLAUSE_INDEPENDENT`（及びその同義語）だけで，OpenMP のバックエンドでは `simd`，OpenACC では `independent` です（v2.0.0 以降．v1.x は OpenMP のバックエンドだけ）．2 つ目以降は捨てます．ほかの指示節はすべて記述した順のまま出力されるので，生成される指示文は入力順に依ることがあります（OpenACC では指示節の順序は意味を持ちません）
* **ランタイム関数**: デバイスを選ぶ関数とストリームを得る関数の 6 個（`get_num_devices_for_offloading()` など．v1.0.0 からあります）を README に載せました（[使用可能なランタイム関数](#使用可能なランタイム関数)）．デバイスの 3 関数を縮退モード（OpenMP のルーチン）とシリアルモード（`0` / 効果なし）でも定義しました（これまではビルドが失敗していました）
* **不具合の修正**（以下の指示文の出力が変わります．以前の出力に依存したコードは振る舞いが変わりえます）:
  * `ACC_CLAUSE_WAIT(...)` で `wait` 節とその後ろに書いた指示節が消えていた不具合を直しました．消えていた `async` などが効くようになるため，待ちを明示していないコードは振る舞いが変わりえます．また，`OMP_TARGET_CLAUSE_DEPEND_IN(...)` は OpenACC 使用時に引数なしの `wait`（全てのキューを待つ）になります．`ACC_CLAUSE_WAIT(...)` の引数に空の要素があると（余分なカンマ．`ACC_CLAUSE_WAIT(1, )` など），OpenACC と OpenMP target 使用時に Solomon のエラーで止まります
  * `SOLOMON_CLAUSE_ASYNC_QUEUE(...)` の後ろに書いた指示節が OpenACC 以外で消えていた不具合を直しました
  * data 構文に渡した `async` / `wait` / `device_type` と `update` に渡した `device_type` が捨てられていた不具合を直しました（OpenACC 3.2 以降．GCC 13 の OpenACC では data 構文の `async` / `wait` がコンパイルエラーになります）．消えていた `async` が効くようになり，data 構文の出口の copyout も非同期になるため，待ちを書かずにホストで結果を読んでいたコードは振る舞いが変わりえます
  * 引数なしの `SOLOMON_CLAUSE_ASYNC()` / `ACC_CLAUSE_ASYNC()` と，OpenACC 使用時の `OMP_TARGET_CLAUSE_NOWAIT` が `async()` を出し，NVIDIA HPC SDK の C/C++ コンパイラ以外が拒否していた不具合を直しました（`async` を出します）
  * `PRAGMA_ACC_UPDATE` に渡した `ACC_CLAUSE_SELF(...)` が OpenMP target 使用時に捨てられ，デバイスからホストへの転送が黙って消えていた不具合を直しました（`from(...)` になります）
  * 計算構文に渡した `ACC_CLAUSE_SELF(...)` が OpenMP target 使用時に捨てられ，領域がホストでなくデバイスで動いていた不具合を直しました（`if(target: .not.(condition))` になります．同じ指示文に `if` 節を書くと OpenMP target のコンパイラが止めます）．計算構文に渡した引数 2 個以上の `ACC_CLAUSE_SELF(...)`（`PRAGMA_ACC_UPDATE(...)` の並び）は，OpenMP target 使用時に Solomon のエラーで止まります
  * OpenMP の記法の data 系の指示文が OpenACC 使用時に指示節の無い指示文になり，黙って意味が変わるかコンパイラが止めていた不具合を直しました（`use_device_*` だけの target data は `acc host_data` になり `PRAGMA_ACC_END_HOST_DATA` で閉じます．ほかの OpenACC の対応物の無い形は Solomon のエラーで止まります）
  * OpenMP target 使用時，OpenMP の指示文に必須の指示節（`map`，`update` では `to` / `from`）になる指示節を持たない OpenACC の `enter data`，`exit data`，`update` が必須の指示節の無い OpenMP の指示文になり，gfortran と ifx が拒否していた不具合を直しました（指示文を出しません．`PRAGMA_ACC_DATA(...)` / `PRAGMA_ACC_HOST_DATA(...)` は従来どおり）
  * この README が載せていた書き方 `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_CLAUSE_TO(...))` / `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_CLAUSE_FROM(...))` が v1.x から壊れた `map` 節を出していた不具合を直しました（入れ子の節ごとに `map(to : ...)` / `map(from : ...)`，OpenACC では `copyin(...)` / `copyout(...)` になります）．入れ子の節の前に `OMP_TARGET_MAP_MODIFIER_*` で map の修飾子を書けるようにし，`update` 用に `OMP_TARGET_CLAUSE_TO_PRESENT(...)` / `OMP_TARGET_CLAUSE_FROM_PRESENT(...)` を足しました．入れ子の節や修飾子の前に生の並びを書くと（例: `OMP_TARGET_CLAUSE_MAP(a, OMP_TARGET_CLAUSE_TO(b))`），Solomon のエラーで止まります（35 番目の要素まで調べます）．移行: 入れ子でない並びの 3 番目以降の要素が，1 引数で呼べない関数形式のマクロの名前（例: `#define fm2(x, y) ...` の下の `OMP_TARGET_CLAUSE_MAP(a, b, fm2)`）だと前処理が止まり，1 引数で呼ぶとトップレベルのカンマを含む形に展開される関数形式のマクロの名前（例: `#define fc1(x) x, x`）だと内部のマクロの名前を挙げる Solomon のエラーでビルドが止まるようになりました．その要素を 1 番目か 2 番目に置いてください（例: `OMP_TARGET_CLAUSE_MAP(fm2, a, b)`）
  * 縮退モード（gfortran / ifx の `-fopenmp` など）で `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)` / `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...)` が `!$omp parallel do` / `!$omp parallel do simd` になります．これまで 1 チーム（実質は逐次）で動いていたループがスレッド並列になるため，ループに競合があると表に出ます
  * 縮退モードで `OMP_TARGET_CLAUSE_IF_TARGET(...)` を捨てるようにしました（これまでは `if (target : ...)` を出し，nvfortran 以外のコンパイラが拒否していました）
  * `PRAGMA_OMP_TARGET_SIMD(...)` は v1.x から OpenMP target で指示節をすべて捨てていました．指示節を出すようにしました
  * 指示節の並びに書いた Solomon の指示節マクロでない名前（綴りの誤りなど）は，後ろに書いた指示節とともに黙って消えていました．Solomon のエラーで止まるようにしました（Fortran では，コンパイラが拒否する名前を指示文に残します）
  * 32 個目の指示節が展開されず（内部のマクロ名が残り，コンパイラが拒否していました），33 個目以降の候補が捨てられていました．選ばれる指示節が 32 個以上のときは内部のマクロ名が残りましたが，指示文に合わない指示節が混じる並び（例: `PRAGMA_ACC_PARALLEL(...)` に `ACC_CLAUSE_COLLAPSE(...)` 31 個と `ACC_CLAUSE_PRIVATE(...)` 2 個）と並べ替えの無い指示文（例: `PRAGMA_ACC_ROUTINE(...)` の 33 個目の `ACC_CLAUSE_BIND(...)`）では残りが黙って消えていました．32 個まで出し，候補が 33 個以上なら Solomon のエラーで止まるようにしました（空の引数は数えません）
  * OpenACC 使用時，必須の節が 1 つも無い OpenACC の data 系の指示文（例: `ACC_CLAUSE_IF(...)` だけ）が，その指示節だけで出ていました（例: `acc data if (n)`）．g++ 13 は `data` / `host_data` を黙って受理し，`enter data` / `exit data` / `update` を拒否しました．gfortran 13 は `data` / `host_data` / `enter data` を受理し，`update` を拒否しました．nvc++ 26.9 は拒否しました．Solomon のエラーで止まるようにしました
  * declare の指示節が 1 つも無い `PRAGMA_ACC_DECLARE(...)` は，`_OPENACC` によらず Solomon のエラーで止まるようにしました．OpenACC はどの版でも declare に指示節を 1 つ以上求めます（2.6・2.7 の 2.13 節の構文 "declare clause-list"．3.0 以降は制限としても明記）
  * map の修飾子の誤用（`OMP_TARGET_CLAUSE_MAP(...)` の外の `OMP_TARGET_MAP_MODIFIER_*`，入れ子の節の無い修飾子，7 個以上の修飾子）は内部名を指示文に残し，clang では `map` 節が黙って消えていました．Solomon のエラーで止まるようにしました．1 つにまとめた `wait` の節の引数が 32 個を超えるときも Solomon のエラーで止まります
  * 引数なしの `SOLOMON_SYNCHRONIZE()` / `PRAGMA_ACC_WAIT()`（と OpenACC 使用時の `PRAGMA_OMP_TARGET_TASKWAIT()`）が `acc wait (),` を，引数なしの `PRAGMA_OMP_FLUSH()` が `omp flush (),` を，v1.1.0（2026-03-21 時点の `main`）では，測ったどのコンパイラ・言語モードでも（gcc / g++，clang / clang++，NVIDIA HPC SDK の C/C++ コンパイラ．GNU モードも厳密モードも．`PRAGMA_OMP_FLUSH()` は OpenMP を有効にしたとき）出していました．gcc / g++ と clang はどちらも拒否し，NVIDIA HPC SDK の C/C++ コンパイラは `acc wait (),` を受理して `omp flush (),` を拒否しました（2025-12-05 に公開した v1.1.0 は `acc wait` / `omp flush` を出しており，2026-02 の v1.1.0 の更新で前者が `acc wait ()` になり，2026-03 の更新で両方の壊れた形が入りました）．`acc wait` / `omp flush` を出すようにしました（gcc / g++ の厳密モードでは次の項目の制約が付きます）
  * gcc / g++ の厳密モードで C23 / C++20 より前の版（`-std=c99`，`-std=c++17` など）では，引数が名前付きの引数を 2 個以上とる関数形式マクロの名前で終わる `ACC_CLAUSE_WAIT(...)`・`ACC_CLAUSE_ASYNC(...)`・`ACC_CLAUSE_SELF(...)`・`SOLOMON_SYNCHRONIZE(...)`・`PRAGMA_ACC_WAIT(...)`・`PRAGMA_OMP_FLUSH(...)`（例: `#define fm2(a, b) ...` の下の `ACC_CLAUSE_WAIT(fm2)`．v1.1.0 では前処理がエラーなく通りました）の前処理が止まるようになりました．その名前を括弧で囲むか最後に置かない，または GNU モードか C23 / C++20 を使ってください（ほかの構成は影響を受けません）．この 6 つのマクロは引数がちょうど 34 個のときもそこで止まります
  * `PRAGMA_ACC_UPDATE(...)` に渡した引数なしの `ACC_CLAUSE_SELF()` が必須の節に数えられ，ほかの指示節の無い update が，OpenACC では `update self()`（g++ は拒否し，nvc++ は受理しました），OpenMP target では指示節の無い `target update`（g++ と clang++ は拒否し，nvc++ 26.9 は受理しました）になっていました．OpenACC では必須の節の Solomon のエラーで止まり，OpenMP target では出力しないようにしました
  * OpenACC のバックエンドで `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)`・`PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...)`・`PRAGMA_OMP_TARGET_SIMD(...)` が `loop` 構文を出さず，`OFFLOAD_BY_OPENACC_PARALLEL` では全 gang がループ全体を実行し（コンパイラの自動並列化が無いと結果が誤ります），`loop` の指示節（`collapse` など）が捨てられていました．`loop independent gang` / `loop independent` を出すように直しました．冗長に，または 1 gang で動いていたループが並列に走るので，ループに競合があると表に出ます
  * OpenACC のバックエンドで `OMP_TARGET_CLAUSE_MAP_DELETE(...)` が `finalize` の無い `delete`（参照数を 1 減らすだけ）になり，データがデバイスに残って後の転送が飛ばされていました．exit data に `finalize` を付けるように直しました．`finalize` はその exit data の `copyout` / `delete` すべてに効きます（ほかの変数を 2 回以上 enter data しているときは，`OMP_TARGET_CLAUSE_MAP_DELETE(...)` を別の exit data に書いてください）．回避のために `ACC_CLAUSE_FINALIZE` を足していた場合は外してください（`finalize finalize` を GCC 13 が拒否します）
  * OpenMP のバックエンドで，`ACC_CLAUSE_DELETE(...)`（したがって `PRAGMA_ACC_EXIT_DATA_DELETE(...)` と `SOLOMON_FREE_FROM_DEVICE(...)`）が `map(delete: ...)` ではなく `map(release: ...)` を出すようにしました（データを消すのではなく，OpenACC と同じく参照数を 1 減らします）
  * OpenACC のバックエンドで `PRAGMA_OMP_TARGET_TASKWAIT(...)` が指示節を `acc wait` に渡していました（内部の名前が残りました）．`acc wait`（すべての待ち）を出すように直しました
  * 縮退モードで `OMP_TARGET_CLAUSE_THREAD_LIMIT(num)` を `num_threads(num)` にしなくなりました（`OMP_TARGET_CLAUSE_NUM_THREADS(...)` と併記すると `num_threads` が 2 つになっていました）．`SOLOMON_CLAUSE_NUM_THREADS(n)` と `ACC_CLAUSE_VECTOR_LENGTH(n)` は今までどおり `num_threads(n)` になります．ただし `PRAGMA_OMP_TARGET_TEAMS(...)` はこれらを `thread_limit(n)` として受け取ります
  * OpenMP のバックエンドで，`PRAGMA_ACC_ROUTINE(...)` / `SOLOMON_DECLARE_OFFLOADED(...)` に渡した `ACC_CLAUSE_DEVICE_TYPE(...)` と `ACC_CLAUSE_NOHOST` も捨てるようにしました（コンパイラが拒否する `declare target` を出していました）．OpenACC のバックエンドでは，`OMP_TARGET_CLAUSE_DEVICE_TYPE(nohost)` が `nohost` になり，`host` / `any` は捨てます．ほかの語は Solomon のエラーで止まります
  * 引数なしの `OMP_TARGET_CLAUSE_INDIRECT()`・`OMP_TARGET_CLAUSE_ORDERED()`・`OMP_CLAUSE_ORDERED()`・`OMP_CLAUSE_PARTIAL()`・`OMP_CLAUSE_DESTROY()` が，`indirect()` / `ordered()` / `partial()` / `destroy()` ではなく `indirect` / `ordered` / `partial` / `destroy` を出すようにしました
  * `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO(...)`（`_DO_SIMD` も）が，ほかの target の複合構文と同じく `copyin` を捨てるようにしました
  * ホストの OpenMP の指示文が，受け取れる指示節を黙って捨てていました: `PRAGMA_OMP_MASKED(...)`・`PRAGMA_OMP_SCAN(...)`・`PRAGMA_OMP_SCOPE(...)`・`PRAGMA_OMP_TILE(...)`・`PRAGMA_OMP_WORKSHARE(...)`・`PRAGMA_OMP_DEPOBJ(...)`（と `masked` を含む結合形）の指示節，`sections` の指示節（`nowait` を含む），`taskloop` の `in_reduction`，`single` の `firstprivate`，`do` の `nowait`，`critical` の名前と `hint`，`interop` の `destroy`．これらを出すようにしました．コンパイラがそこでは拒否する `simd` の `firstprivate` と `distribute` の `linear` は捨てるようにしました．NVIDIA HPC SDK（nvfortran）では，`PRAGMA_OMP_WORKSHARE(...)` の `OMP_CLAUSE_NOWAIT` は今までどおり捨てます（nvfortran 26.9 は `workshare nowait` を拒否します）．指示節や名前が消えて通っていた入力のうち，次は止まるようになります: `PRAGMA_OMP_SCOPE(...)` の `firstprivate` / `allocate`（OpenMP 5.2 の機能で，Clang 22 と Intel 2026.1 が拒否します），名前の無い `PRAGMA_OMP_CRITICAL(OMP_CLAUSE_HINT(h))`（GCC・Clang・Intel のコンパイラが拒否します），`PRAGMA_OMP_DEPOBJ(o)` のような裸の名前（Solomon のエラー．`OMP_PASS_LIST(o)` で渡してください），`PRAGMA_OMP_END_CRITICAL` で閉じた名前つきの `PRAGMA_OMP_CRITICAL(OMP_PASS_LIST(name))`（終わりにも同じ名前が要るのでコンパイラが拒否します．新しい `PRAGMA_OMP_END_CRITICAL_WITH_NAME(name)` で閉じてください）．`PRAGMA_OMP_DEPOBJ(...)` の `update` 節を書く `OMP_CLAUSE_DEPOBJ_UPDATE(...)` を足しました．そこに渡した `OMP_CLAUSE_UPDATE`（`atomic` の節）は，消えずに引数の無い `update` になり，コンパイラが拒否します
  * `PRAGMA_OMP_FLUSH(...)` の先頭に Solomon の指示節マクロを渡すと，その展開が括弧の中に入っていました．Solomon のエラーで止まるようにしました（2 番目以降では今も展開が並びに残り，コンパイラが拒否します）．この検査により，2 番目の引数が `::`・`/`・`(` で始まると前処理が止まります．そのような名前は先頭に書いてください
  * v1.1.0 の縮退モードでは，`SOLOMON_SYNCHRONIZE()` / `PRAGMA_ACC_WAIT(...)`（v1.x の `SYNCHRONIZE()`）が定義の無い名前 `PRAGMA_OMP_TARGET_TASKWAIT(...)` を残し，`PRAGMA_OMP_TARGET_TASKWAIT(...)` は内部の名前を残して，ビルドが止まっていました．どれも何も出さないようにしました
  * 名前に `simd` を含む指示文（例: `PRAGMA_OMP_DO_SIMD(...)`）に渡した `OMP_CLAUSE_SIMD`（と OpenMP のバックエンドの `SOLOMON_CLAUSE_INDEPENDENT` / `OMP_TARGET_CLAUSE_SIMD`）は，コンパイラが拒否する `simd simd` を出さずに捨てるようにしました
  * `PRAGMA_OMP_TEAMS(...)`・`..._TEAMS_DISTRIBUTE(...)`・`..._TEAMS_LOOP(...)` に渡した `OMP_CLAUSE_IF(...)`（`teams` の `if` は OpenMP 6.0 の節）は，Fortran では版によらず書いたとおりに出します．指示文としては正しい書き方ですが，6.0 より前のコンパイラは拒否することがあります（gfortran 13 と nvfortran 26.9 は拒否し，ifx 2026.1 は受け付けて正しく動きます）．C/C++ では `_OPENMP >= 202411` のときだけ出します（`OMP_CLAUSE_IF(...)` の行を参照）
  * `PRAGMA_OMP_ORDERED(...)` が，`doacross` を拒否するコンパイラのために `OMP_CLAUSE_DEPEND(source)` / `OMP_CLAUSE_DEPEND(sink: ...)` を出すようにしました（`OMP_CLAUSE_DOACROSS(...)` の行を参照）
  * `OMP_TARGET_CLAUSE_COPYIN(...)`（OpenMP の `copyin`．`threadprivate` の変数の値を写す）が，OpenACC バックエンドでは data 節の `copyin`（デバイスへの転送）になっていた．OpenMP target バックエンドが複合構文から捨てるのと同じく，OpenACC バックエンドでも捨てる（転送の書き方は OpenMP target の節の表を参照）
  * OpenACC で，`_SIMD` の付く target の指示文と `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)` に `SOLOMON_CLAUSE_INDEPENDENT` か `OMP_TARGET_CLAUSE_SIMD` を渡すと，`independent` が 2 つ出ていた（GCC と Clang は拒否する）．1 つの指示文に `independent` 節は 1 つだけ（OpenMP のバックエンドでは `simd` 節を 1 つだけ）出し，先頭に置く

<details>
<summary>v1.1.0（2025-12-05）</summary>

この項は v2.0.0 で書いた v1.x のまとめで，`main` の 2026-03-21（`94ae137`）までの更新を含みます．

* 指示節の自動並べ替え機能を追加しました（生成される指示文が入力順に依らず一意になります）
* 非同期実行用の `ASYNC_QUEUE(id)` / `WAIT_QUEUE(id)` を追加しました
* `PRINT_GENERATED_PRAGMA` を追加しました（生成された指示文をコンパイル時メッセージとして出力します）
* 非推奨となった `use_device_ptr` に代わり `use_device_addr` を使用するようにしました
* バグ修正: `PRAGMA_ACC_WAIT`，`AS_THREAD`/`AS_BLOCK`/`AS_GRID`，OpenACC の atomic 指示文の欠落など

（表記は当時の v1.x の綴りです）

</details>

<details>
<summary>v1.0.0（2024-08-22）</summary>

* 初版リリース: OpenACC と OpenMP target を単一ソースで切り替えるマクロ集（[Miki & Hanawa 2024, IEEE Access](https://doi.org/10.1109/ACCESS.2024.3509380) の出版版）
* OpenACC 記法・OpenMP target 記法の相互変換（acc2omp / omp2acc）と簡易記法
* サンプルコード（nbody, diffusion）とコードジェネレータを同梱

</details>

## 使い方

### Solomon を用いたコードの開発方法

1. Solomon のモジュールファイルをインクルードする

   各ソースファイルの冒頭でSolomonのインクルードファイルをインクルードしてください
   続いて，各モジュール，プログラムの冒頭で`USE_SOLOMON_RUNTIME`としてください

   ```Fortran
   ! file main.f90
   #include <solomon.hpp>

   program main
     USE_SOLOMON_RUNTIME
     use sub
     implicit none
     ! ...
   end program main
   ```

   ```Fortran
   ! file sub.f90
   #include <solomon.hpp>

   module sub
     USE_SOLOMON_RUNTIME
     implicit none
   contains
     ! ...
   end module sub
   ```

2. OpenACC/OpenMP target 指示文のかわりに Solomon が提供するマクロを挿入する

   * GPU化を始めたばかりの方には，`SOLOMON_OFFLOAD(...)` などの簡易記法の利用をおすすめします
   * OpenACC あるいは OpenMP target を用いた開発に慣れている方は，OpenACC/OpenMP 的な記法を使うこともできます
     * OpenMP 的記法を使った場合には，`PRAGMA_OMP_TARGET_*` や `OMP_TARGET_CLAUSE_*` のように `_TARGET_` をつけたものだけが OpenACC 使用時における変換対象となります（例えば `PRAGMA_OMP_ATOMIC(...)` は `!$omp atomic __VA_ARGS__` へと変換されるため，`!$acc atomic __VA_ARGS__` には変換されません）
       * CPU 向けの OpenMP 指示文（`TARGET` の付かないもの）は，OpenACC 使用時にもそのまま出力されます．ホスト側の OpenMP のスレッドから offload を呼ぶ使い方（例えば `PRAGMA_OMP_PARALLEL(...)` の中で `SOLOMON_OFFLOAD(...)` を使う）を想定したもので，offload する領域の中に CPU 向けの OpenMP 指示文を混ぜる使い方は想定していません
       * この使い方では OpenACC に加えて OpenMP も有効にしてください（NVIDIA HPC SDK では `-acc` に加えて `-mp` を付ける．Fortran では `SOLOMON_FLAGS` や `spp.sh` の引数にも付ける）．有効にしないとこれらのマクロが定義されず，コンパイルエラーになります
     * OpenACC のバックエンドでは，OpenMP の記法のループ系の指示文（`PRAGMA_OMP_TARGET_PARALLEL_DO(...)`・`PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)` など）に終わりが要ります．`SOLOMON_END_OFFLOAD` で閉じることを推奨します．`PRAGMA_OMP_END_TARGET_<name>`（v2.0.0 以降．OpenMP のバックエンドでは何も出しません．ループ構文の終わりの指示文は省略できるため）でも閉じられます
     * `PRAGMA_OMP_TARGET_DATA(...)` という記法は使わないでください
       * OpenACC 使用時，`OMP_TARGET_CLAUSE_USE_DEVICE_PTR/ADDR(...)`（と `if`）だけの `PRAGMA_OMP_TARGET_DATA(...)` は `host_data use_device(...)` になります（`PRAGMA_ACC_END_HOST_DATA` で閉じてください）．指示節がどれも OpenACC の対応物を持たない OpenMP の data 系の指示文（例: 汎用の `OMP_TARGET_CLAUSE_MAP(...)` だけ）と，`use_device_*` をほかの data 節（汎用の `OMP_TARGET_CLAUSE_MAP(...)` を含む）と 1 つに書いた target data は Solomon のエラーで止まります．入れ子の指示文に分けて書いてください（v2.0.0 以降）．`OMP_TARGET_CLAUSE_DEVICE(...)` は無視されます（OpenACC は現在のデバイスを使います）
       * 演算加速器（GPU）からアクセスするデータについては `SOLOMON_DATA_ACCESS_BY_DEVICE(...)` か `PRAGMA_ACC_DATA(...)`，ホスト（CPU）からアクセスするデータについては `SOLOMON_DATA_ACCESS_BY_HOST(...)` か `PRAGMA_ACC_HOST_DATA(...)` をお使いください
       * `PRAGMA_OMP_TARGET_DATA(...)` は `data` にも `host_data` にもなるので，OpenACC のバックエンドに `PRAGMA_OMP_END_TARGET_DATA` はありません（OpenMP target 専用）．代わりに `SOLOMON_DATA_ACCESS_BY_DEVICE(...)` … `PRAGMA_ACC_END_DATA`，または `SOLOMON_USE_DEVICE_DATA_FROM_HOST(...)` … `PRAGMA_ACC_END_HOST_DATA` で書いてください（どちらの終わりも OpenMP target のデータ領域も閉じます）
     * `PRAGMA_ACC_DECLARE(...)` という記法は使わないでください（OpenMP target バックエンドへは変換されません）
       * デバイス常駐変数には `SOLOMON_DECLARE_ON_DEVICE(...)` を，link 意味論が必要な場合には `SOLOMON_DECLARE_ON_DEVICE_LINKED(...)` をお使いください（v2.0.0 以降）
     * Fortran では手続内に挿入した `SOLOMON_DECLARE_OFFLOADED(...)`（や `PRAGMA_ACC_ROUTINE(...)`）は自己完結するため，`SOLOMON_DECLARE_OFFLOADED_END` の挿入は不要です（空に展開されるため書いても無害です）（v2.0.0 以降）
       * 挿入位置は宣言部の `use`／`implicit` 文より**後**にしてください（Intel ifx など，`implicit none` より前の指示文を受理しないコンパイラがあります）
   * GPU実行時には無視してほしい指示文については，`SOLOMON_IF_NOT_OFFLOADED(arg)` の中に記入してください
     * <details><summary> 実装例: `arg` については，縮退モード（OpenACC と OpenMP target 両方を無効化した場合）のみ実体化されます</summary>

       ```Fortran
       SOLOMON_OFFLOAD(SOLOMON_CLAUSE_PRIVATE(i,j))
       do i=1, num
         SOLOMON_IF_NOT_OFFLOADED(PRAGMA_OMP_SIMD())
         do j=1, 16
           ! computation
         end do
       end do
       SOLOMON_END_OFFLOAD
       ```

       * OpenACC 使用時

         ```Fortran
         !$acc kernels
         !$acc loop private(i,j)
         do i=1, num

           do j=1, 16
             ! computation
           end do
         end do
         !$acc end kernels
         ```

       * OpenMP target 使用時

         ```Fortran
         !$omp target teams loop private(i,j)
         do i=1, num

           do j=1, 16
             ! computation
           end do
         end do
         ```

       * 縮退モード

         ```Fortran
         !$omp parallel do private(i,j)
         do i=1, num
           !$omp simd
           do j=1, 16
             ! computation
           end do
         end do
         ```

     </details>

   * 指示文に付与する指示節・指示句については，下記の例のようにカンマ区切りで渡してください：

      ```Fortran
      SOLOMON_OFFLOAD(SOLOMON_CLAUSE_INDEPENDENT, ACC_CLAUSE_VECTOR_LENGTH(128), OMP_TARGET_CLAUSE_COLLAPSE(3))
      ```

      * 簡易記法，OpenACC/OpenMP的記法を混ぜても問題ありません
      * ~~`SOLOMON_CLAUSE_INDEPENDENT`（およびこれに対応する `ACC_CLAUSE_INDEPENDENT` や `OMP_TARGET_CLAUSE_SIMD`）については，全ての指示節・指示句の先頭に記載してください~~
        * **[更新 v1.1.0]** この制約は自動的に処理されるようになりました．OpenMP のバックエンド（`simd` が指示文名の一部になる）では，どの順番で記述されていても，`SOLOMON_CLAUSE_INDEPENDENT`（及びその同義語）を Solomon が自動的に先頭に並び替えます．OpenACC でも `independent` を先頭に並べ替えます（v2.0.0 以降．OpenACC では指示節の順序は意味を持ちません: OpenACC 3.3 の 2.1 節）．`simd` / `independent` 節は 1 つだけ出力するので，指示文自身が付ける所に `SOLOMON_CLAUSE_INDEPENDENT` を書いても（例: `PRAGMA_OMP_TARGET_SIMD(...)`）害はありません（NVIDIA HPC SDK では，名前に `simd` を持つ OpenMP target の指示文に `simd` 節が続くことがありますが，受理されます）．`ACC_CLAUSE_DEVICE_TYPE(...)` より後ろに書いた指示節はその device 用なので，動かさず，まとめもしません（OpenACC 3.3 の 2.4 節）．ほかの指示節は，どのバックエンドでも記述した順のままです
        * `simd` だけが取る節（`OMP_CLAUSE_SIMDLEN(...)`・`OMP_CLAUSE_SAFELEN(...)`・`OMP_CLAUSE_ALIGNED(...)`・`OMP_CLAUSE_NONTEMPORAL(...)`）は，名前に `simd` の無い指示文では並べ替えで `simd` が付いても捨てます．`OMP_CLAUSE_IF(simd: ...)` は残り（`if` はほかの構文の節でもあるため），`simd` が付かない所（`teams loop` の形では常に）ではコンパイラが拒否します．Solomon の記法では，代わりに `SOLOMON_CLAUSE_SIMDLEN(length)`・`SOLOMON_CLAUSE_SAFELEN(length)`・`SOLOMON_CLAUSE_ALIGNED(...)`・`SOLOMON_CLAUSE_NONTEMPORAL(...)`・`SOLOMON_CLAUSE_IF_SIMD(condition)` を `SOLOMON_CLAUSE_INDEPENDENT` と一緒に書いてください．`simd` が付く所に出て，付かない所では捨てます（簡易記法の表を参照）
      * 指示文に対応していない指示節・指示句については，Solomon が自動的に無視します
        * OpenMP target 使用時，OpenMP の指示文に必須の指示節（`map`，`update` では `to` / `from`）になる指示節を持たない OpenACC の `enter data`，`exit data`，`update`（例: `attach` だけ，`if` だけ）は，指示文そのものを出しません（v2.0.0 以降）．`PRAGMA_ACC_DATA(...)` / `PRAGMA_ACC_HOST_DATA(...)` は必須の指示節の無い `!$omp target data` を残し，gfortran が拒否します（nvfortran と ifx は受理します）．OpenMP target でも通すときは，`map` になる指示節（`PRAGMA_ACC_DATA(...)`．例: `ACC_CLAUSE_COPYIN(...)`）か `use_device_*` になる指示節（`PRAGMA_ACC_HOST_DATA(...)`．例: `ACC_CLAUSE_USE_DEVICE(...)`）を 1 つ以上入れてください
        * OpenMP target 使用時，OpenMP の data 系の指示文を直接書いたもの（`PRAGMA_OMP_TARGET_DATA(...)`，`PRAGMA_OMP_TARGET_ENTER_DATA(...)`，`PRAGMA_OMP_TARGET_EXIT_DATA(...)`，`PRAGMA_OMP_TARGET_UPDATE(...)`）で，その指示文に必須の指示節が 1 つも残らないもの（例: `OMP_TARGET_CLAUSE_IF(...)` だけ，`ACC_CLAUSE_PRESENT(...)` だけ，その指示文が取らない指示節だけ）は，必須の指示節の無いまま書いたとおりに出します．gfortran は拒否し，nvfortran は受理し，ifx は `target data` と `target update` を受理して `target enter data` と `target exit data` を拒否します
        * OpenACC 使用時，指示節がどれもその指示文の必須の節でない OpenACC の data 系の指示文（`data`，`host_data`，`enter data`，`exit data`，`update`．例: `ACC_CLAUSE_IF(...)` だけ）は，Solomon のエラーで止まります（v2.0.0 以降）
        * declare の指示節（`copy`，`copyin`，`copyout`，`create`，`present`，`deviceptr`，`device_resident`，`link`）が 1 つも無い `PRAGMA_ACC_DECLARE(...)` は，`_OPENACC` によらず `Solomon: the OpenACC specification requires at least one clause on a declare directive (...)`（Fortran は `SOLOMON_ERROR_OPENACC_REQUIRES_A_CLAUSE_ON_DECLARE_DIRECTIVE`）で止まります（v2.0.0 以降）．OpenACC はどの版でも declare に指示節を 1 つ以上求めます（2.6・2.7 の 2.13 節の構文 "declare clause-list"．3.0 以降は制限としても明記）
        * Solomon の map の修飾子の誤用もビルドを止めます（v2.0.0 以降）: `OMP_TARGET_CLAUSE_MAP(...)` の外に書いた `OMP_TARGET_MAP_MODIFIER_*`，後ろに入れ子の指示節の無い修飾子，入れ子の指示節の後ろの入れ子の指示節でない並び，1 つの `OMP_TARGET_CLAUSE_MAP(...)` の 7 個以上の修飾子．GCC・Clang・Intel のコンパイラ（C/C++）は `error: Solomon: ...` を，NVIDIA HPC SDK（C/C++）は `invalid text in pragma` を出します．Fortran では `SOLOMON_ERROR_` で始まる名前が指示文に残り，Fortran のコンパイラがそこで構文エラーを出します
        * 指示節には Solomon の指示節マクロだけを書けます（v2.0.0 以降）．Solomon の指示節マクロでない名前（綴りを誤った `SOLOMON_CLAUSE_*` や，`collapse(2)` のような生の指示節）を書くと，ビルドが止まります: GCC・Clang・Intel のコンパイラ（C/C++）は `error: Solomon: undefined name in the clause list: <name>` を，NVIDIA HPC SDK（C/C++）はマクロの行で `invalid text in pragma` を出します．Fortran では名前の前に `SOLOMON_ERROR_UNDEFINED_CLAUSE` が置かれ，Fortran のコンパイラがそこで構文エラーを出します．そのような名前の直後に演算子・文字列・数・括弧で始まる要素（例: `*p`，`(x)`，`SOLOMON_CLAUSE_TARGETS(...)`）を置くと，代わりに前処理がトークンの貼り合わせのエラーで止まることがあります．指示節マクロの間のカンマが抜けたときも `undefined name in the clause list` で止まり，指示節の代わりに数と内部の名前（`SOLOMON_INTERNAL_TAG_*`）が並びます．シリアル（OpenACC も OpenMP も無効）では名前を調べません．空の引数は無視します（指示節の上限の数にも入りません）．`OMP_CLAUSE_*` はホストの OpenMP の指示文（`PRAGMA_OMP_*`）用で，OpenACC の指示文に渡すことは想定していません: OpenMP を有効にしない OpenACC の構成では定義の無い名前として止まります（OpenACC と OpenMP target の両方で使う指示節には `OMP_TARGET_CLAUSE_*` か `SOLOMON_CLAUSE_*` を使ってください）．Fortran では，定義の無い名前の後ろに整数の要素を続けると（`async, 1` など），前処理が内部のマクロについてのエラー（例: `macro "SOLOMON_INTERNAL_PICKUP_FLAG_MATCHING_0" passed 2 arguments, but takes just 1`）で止まるか，後ろに Solomon の指示節のマクロが続くときは，書いた名前ではなく内部の名前（例: `SOLOMON_INTERNAL_TAG_ACC_PARALLEL`）の前に `SOLOMON_ERROR_UNDEFINED_CLAUSE` が置かれ，Fortran のコンパイラがそこで構文エラーを出すことがあります．Solomon の指示節マクロだけを書いてください
      * 空の判定: `ACC_CLAUSE_WAIT(...)`・`ACC_CLAUSE_ASYNC(...)`・`ACC_CLAUSE_SELF(...)`（とほかの記法の同じ名前の指示節，`OMP_CLAUSE_ORDERED(...)`・`OMP_CLAUSE_PARTIAL(...)`・`OMP_CLAUSE_DESTROY(...)`・`OMP_TARGET_CLAUSE_INDIRECT(...)`，指示文の `SOLOMON_SYNCHRONIZE(...)`・`PRAGMA_ACC_WAIT(...)`・`PRAGMA_OMP_FLUSH(...)`）は引数が空かを調べます．gcc / g++ の厳密モードで C23 / C++20 より前の版（`-std=c99`，`-std=c++17` など）では，引数の最後の字句が名前付きの引数を 2 個以上とる関数形式マクロの名前だと前処理が止まり（例: `#define fm2(a, b) ...` の下の `fm2`），引数がちょうど 34 個（トップレベルのカンマが 33 個）のときも止まります（33 個と 35 個以上は通ります）．その名前を括弧で囲むか最後に置かない，引数の数を変える，または GNU モードか C23 / C++20 を使ってください．ほかの構成（GNU モード，clang 系，C23・C++20，Fortran）にこの制約はありません
   * OpenACC/OpenMP target間の互換性向上のため，（個々のマクロを直接使うよりも）下記に示す統合マクロの使用をおすすめします

     | 推奨実装 | 対応実装（非推奨） |
     | ---- | ---- |
     | **`SOLOMON_OFFLOAD(...)`** <br> `PRAGMA_ACC_KERNELS_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL_LOOP(...)` | <br> `PRAGMA_ACC_KERNELS(...) PRAGMA_ACC_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL(...) PRAGMA_ACC_LOOP(...)` |
     | **`SOLOMON_DECLARE_DATA_ON_DEVICE(...)`** <br> `PRAGMA_ACC_DATA_PRESENT(...)` | <br> `PRAGMA_ACC_DATA(ACC_CLAUSE_PRESENT(...))` |
     | `OMP_TARGET_CLAUSE_MAP_TO(...)` | `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_CLAUSE_TO(...))` |

     * 1 つの `OMP_TARGET_CLAUSE_MAP(...)` の中で，生の指示節の字面と Solomon のマクロを混ぜないでください．map-type の修飾子は入れ子の節の前に `OMP_TARGET_MAP_MODIFIER_*` で書きます（例: `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_MAP_MODIFIER_PRESENT, OMP_TARGET_CLAUSE_TO(a))`）．`OMP_TARGET_CLAUSE_MAP(present, OMP_TARGET_CLAUSE_TO(a))` や `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_CLAUSE_TO(present : a))` のような混在には対応していません．入れ子の節と修飾子は先頭に書きます．その前に生の並びを書くと Solomon のエラーで止まります（v2.0.0 以降．35 番目の要素まで調べます）

   * `SOLOMON_OFFLOAD(...)` よりも細粒度でGPU化・並列化を指示したい場合（多重ループ中の個々のループに指示文を付与したい場合）には，下記のマクロが使えます
     * OpenACC/OpenMP target間の互換性向上のため，統合マクロの使用が推奨です（OpenACC/OpenMP的記法を使って直接実装した際には，両指示文の設計の違いから異なるバックエンドへ正しく変換される保証がありません）

     | 使用可能なマクロ | 出力 | 使用されるバックエンド | 備考 |
     | ---- | ---- | ---- | ---- |
     | **`SOLOMON_OFFLOAD_OUTER_LOOP(...)`** | `!$acc parallel [...] !$acc loop gang [...]` <br> `!$omp target teams distribute [...]` <br> `!$omp parallel do [...]` | OpenACC <br> OpenMP target <br> OpenMP（縮退モード） | 多重ループの外側ループに指定 <br> スレッド数などは `SOLOMON_CLAUSE_NUM_THREADS(n)` や `SOLOMON_CLAUSE_NUM_BLOCKS(n)` などで示唆する <br> `SOLOMON_CLAUSE_BLOCK`, `ACC_CLAUSE_GANG` などは重複指定となってしまうため付与してはいけない |
     | **`SOLOMON_PARALLELIZE_INNER_LOOP(...)`** | `!$acc loop vector [...]` <br> `!$omp parallel do [...]` <br> 無視される（外側ループが並列化済みのため） | OpenACC <br> OpenMP target <br> OpenMP（縮退モード） | 多重ループの内側ループに指定 <br> `SOLOMON_CLAUSE_THREAD`, `ACC_CLAUSE_VECTOR` などは重複指定となってしまうため付与してはいけない |

     ```Fortran
     SOLOMON_OFFLOAD_OUTER_LOOP(SOLOMON_CLAUSE_NUM_BLOCKS(16384), SOLOMON_CLAUSE_NUM_THREADS(256))
     do i = 1, N_out
       ! common computation
       SOLOMON_PARALLELIZE_INNER_LOOP()
       do j = 1, N_in
         ! further computation
       end do
     end do
     SOLOMON_END_OFFLOAD_OUTER_LOOP
     ```

   * カーネルの非同期実行および同期処理については下記のマクロが提供されているので，用途に合わせて使い分けてください
     * OpenACC/OpenMP target間の互換性向上のため，非同期実行および同期処理についてもSolomonが提供する簡易記法の使用をおすすめします
     * バックエンドに依らず非同期処理を実行したい場合には`SOLOMON_CLAUSE_ASYNC(...)`および`SOLOMON_SYNCHRONIZE(...)`を用いてください．キューIDの指定は無視される場合がある点にご注意ください
     * キューIDを指定した細やかな非同期処理を実行したい場合には`SOLOMON_CLAUSE_ASYNC_QUEUE(id)`および`SOLOMON_WAIT_QUEUE(id)`を用いてください．非同期実行がなされない場合がある点にご注意ください
     * 指定したキューの完了を待ってから指示文を始めたい場合には`SOLOMON_CLAUSE_WAIT_QUEUE(...)`を加えてください（`SOLOMON_CLAUSE_ASYNC_QUEUE(id)`と組で使います）．次の起動の前に全てのキューを待つ場合には，その前に`SOLOMON_SYNCHRONIZE()`を置いてください

     | 使用可能なマクロ | 出力 | 使用されるバックエンド | 備考 |
     | ---- | ---- | ---- | ---- |
     | **`SOLOMON_CLAUSE_ASYNC(...)`** <br> `ACC_CLAUSE_ASYNC(...)` <br> `OMP_TARGET_CLAUSE_NOWAIT` | <br> `async(__VA_ARGS__)` <br> `nowait` | <br> OpenACC <br> OpenMP | 両バックエンドで共通して非同期実行される <br> OpenACC においてはキューIDを指定可能 <br> OpenMP においてはキューIDは無視される <br> 引数なしでは OpenACC で括弧の無い `async` になる（`async(acc_async_noval)` と同じ） |
     | **`SOLOMON_SYNCHRONIZE(...)`** <br> `PRAGMA_ACC_WAIT(...)` <br> `PRAGMA_OMP_TARGET_TASKWAIT(...)` | <br> `!$acc wait __VA_ARGS__` <br> `!$omp taskwait __VA_ARGS__` | <br> OpenACC <br> OpenMP | 両バックエンドで共通して同期処理が実行される．`SOLOMON_CLAUSE_ASYNC(...)` に対応して記述する <br> OpenACC（`SOLOMON_SYNCHRONIZE`，`PRAGMA_ACC_WAIT`）: 引数なしは `!$acc wait`，引数ありは `!$acc wait (__VA_ARGS__)` になる．引数はキューIDの並び（wait-argument）だけで，指示節は渡せない（`SOLOMON_CLAUSE_ASYNC(3)` などの指示節のマクロを渡しても Solomon は見分けず，`!$acc wait (...)` の中に残ってコンパイラが Solomon の内部名（`SOLOMON_INTERNAL_TAG_...`）を含むエラーで止まる） <br> OpenMP target（`SOLOMON_SYNCHRONIZE`，`PRAGMA_ACC_WAIT`）: 引数は数によらず無視され `!$omp taskwait` になる <br> `PRAGMA_OMP_TARGET_TASKWAIT(...)`: OpenMP target では引数（指示節）がそのまま `!$omp taskwait` に渡り，`PRAGMA_OMP_TARGET_TASKWAIT(1, 2)` のようなキューIDは前処理エラーになる．OpenACC 使用時には `!$acc wait` になる <br> 縮退モード（演算加速器を用いないCPU実行）: 3 つとも何も出さない |
     | **`SOLOMON_CLAUSE_ASYNC_QUEUE(id)`** <br> `ACC_CLAUSE_ASYNC(id)` | <br> `async(id)` <br> N/A（OpenMP target 使用時には無視される） | <br> OpenACC <br> OpenMP | OpenACC でのみキューIDを指定して非同期実行 <br> キューIDの指定が必須 <br> OpenMP ではキューIDを指定した非同期実行がサポートされていないため，無視される |
     | **`SOLOMON_CLAUSE_WAIT_QUEUE(...)`** <br> `ACC_CLAUSE_WAIT(...)` | <br> `wait(__VA_ARGS__)` <br> N/A（OpenMP target 使用時には無視される） | <br> OpenACC <br> OpenMP | OpenACC でのみ，指定したキューIDの処理の完了を待ってから指示文を始める（v2.0.0 以降） <br> キューIDを 1 個以上渡す（`SOLOMON_WAIT_QUEUE(id)` と違い並びを渡せる）．空にすると OpenACC では全てのキューを待つ `wait` になるが，全てを待つときは指示文の前に `SOLOMON_SYNCHRONIZE()` を置くことを推奨する <br> OpenMP では無視される．`SOLOMON_CLAUSE_ASYNC(...)` と組み合わせると OpenMP target では待たない <br> 同じ指示文の複数の `wait` 節はまとめる（`ACC_CLAUSE_WAIT(...)` の行を参照） |
     | **`SOLOMON_WAIT_QUEUE(id)`** <br> `PRAGMA_ACC_WAIT(id)` | <br> `acc wait (id)` <br> N/A（OpenMP target 使用時には無視される） | <br> OpenACC <br> OpenMP | OpenACC でのみキューIDを指定して同期処理を実行．`SOLOMON_CLAUSE_ASYNC_QUEUE(id)` に対応して記述する <br> キューIDの指定が必須 <br> OpenMP ではキューIDを指定した同期処理がサポートされていないため，無視される |

### Solomon を使ったコードのコンパイル方法

* コンパイラオプションを用いて，OpenACC または OpenMP target を有効化してください
  * 用いるコンパイラでのOpenACC または OpenMP target の機能を有効化するオプションを確認してください
    * NVIDIA HPC SDKでのOpenACCを用いてGPU化するためのオプションは`-acc=gpu -gpu=[target GPU architecture]`です
    * NVIDIA HPC SDKでのOpenMP targetを用いてGPU化するためのオプションは`-mp=gpu -gpu=[target GPU architecture]`です
    * AMD ROCmでのOpenMP targetを用いてGPU化するためのオプションは`-fopenmp --offload-arch=[target GPU architecture]`です
    * Intel oneAPIでのOpenMP targetを用いてGPU化するためのオプションは`-fiopenmp -fopenmp-targets=spir64_gen -Xs "-device [target GPU architecture]"`です

* Solomon のパス（`solomon.hpp` があるディレクトリ）を確認し，コンパイル時に `-I/path/to/solomon` などとして指定してください
  * コンパイルするディレクトリからの相対パス(`../../../solomon`など)による指定，絶対パス( `/usr/local/solomon/include`など)はどちらでも受け付けられます
  * 拡散方程式サンプルコードの場合には，コンパイルするディレクトリ（`samples/F/diffusion/Makefile`が置いてある`samples/F/diffusion/`）からのSolomonヘッダーファイル`solomon/solomon.hpp` への相対パスは`../../../solomon`です

* Solomon の動作モードを決めて，そのプリプロセスフラグを下記から選んでください

  | プリプロセスフラグ | 使用されるバックエンド | 備考 |
  | ---- | ---- | ---- |
  | `-DSOLOMON_OFFLOAD_BY_OPENACC` | OpenACC | デフォルトでは `kernels` 構文を使用 |
  | `-DSOLOMON_OFFLOAD_BY_OPENACC -DSOLOMON_OFFLOAD_BY_OPENACC_PARALLEL` | OpenACC | デフォルトでは `parallel` 構文を使用 |
  | `-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET` | OpenMP target | デフォルトでは `loop` 指示文を使用（コンパイラの `_OPENMP` が 201811（OpenMP 5.0）以上のとき）．それ未満（例: gfortran 13）では `distribute` を使用 |
  | `-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE` | OpenMP target | デフォルトでは `distribute` 指示文を使用 |
  | | 縮退モード | OpenMP を用いたマルチコアCPU向けのスレッド並列 |
  | | シリアルモード | OpenACC・OpenMP のいずれも有効化されていない場合は，全ての指示文が除去されシリアルコードとしてコンパイルされます（v2.0.0 以降） |

  * `-DSOLOMON_OFFLOAD_BY_*` は，それを有効にするコンパイラのフラグ（OpenACC は `-acc` / `-fopenacc`，OpenMP target は `-mp=gpu` / `-fopenmp` など．`_OPENMP` が 201307（OpenMP 4.0）以上であること）と組で初めて効きます．フラグが無いと取り消され，縮退モード（OpenMP が有効なとき）かシリアルモードでビルドされます（`fortran.mk` と `spp.sh` は注意メッセージを出します）
  * `solomon.hpp` の後では，`SOLOMON_OFFLOAD_BY_OPENACC` / `SOLOMON_OFFLOAD_BY_OPENMP_TARGET` は使われているバックエンドのものだけが定義されたまま残ります．接頭辞なしの `OFFLOAD_BY_OPENACC` / `OFFLOAD_BY_OPENMP_TARGET` はどちらの綴りで指定しても同じ規則で定義されるので，使われているバックエンドの判定にはこちらを使ってください．`SOLOMON_OFFLOAD_BY_OPENACC_PARALLEL` / `SOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE` は，属するバックエンドが使われるときだけ有効になり，`solomon.hpp` の後にも定義されたまま残ります．接頭辞なしの綴り `OFFLOAD_BY_OPENACC_PARALLEL` / `OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE` は，直接指定したときも `solomon.hpp` が接頭辞つきから定義したときも，バックエンドによらず定義されたまま残ります（v1.x と同じ）．使われているバックエンドの判定には使えません

* **半自動コード生成・コンパイル方法**

  1. Makefile中で以下のようにFortranのコンパイラとそのオプション，さらにコンパイルの対象のファイル(.f90)を実施している箇所を探してください

     ```Makefile
     FC = nvfortran
     FLAGS = -O3

     %.o: %.f90
          $(FC) -c $(FLAGS) $< -o $@
     ```

  2. これらの箇所をそれぞれ以下のように変更してください

     ```Makefile
     SOLOMON_DIR = ../../../solomon
     FC = nvfortran -acc=gpu
     FLAGS = -O3 -DSOLOMON_OFFLOAD_BY_OPENACC
     INC = -I$(SOLOMON_DIR)

     SOLOMON_FC    = $(FC)
     SOLOMON_FLAGS = $(FLAGS) $(INC)

     %.o: spp/%.f90
          $(FC) -c $(FLAGS) $< -o $@

     .SECONDARY:

     include $(SOLOMON_DIR)/fortran.mk
     ```

     * Makefile のレシピの行（`$(FC) -c ...`）はタブで始めます．このページの例は空白で示しているので，写すときはタブに置き換えてください
     * `.SECONDARY:` は `spp/*.f90` を残します．無いと `make` はこれらを中間ファイルとして扱い，ビルドの後に消します
     * Solomonのパスを`SOLOMON_DIR`として指定
     * OpenACC または OpenMP target の機能を有効化するオプションを `FC`（例のとおり）か `FLAGS` に追加
     * コンパイルオプションの変数`FLAGS`にSolomonの動作モードのプリプロセスフラグを追加
     * `FC`および`FLAGS`の内容を変数`SOLOMON_FC`と`SOLOMON_FLAGS`にそれぞれ指定
       * `fortran.mk` は `SOLOMON_FC` と `SOLOMON_FLAGS` を用いてコンパイラを起動し `_OPENACC` / `_OPENMP` を自動検出するため，OpenACC/OpenMP を有効化するフラグ（`-acc=gpu`, `-mp=gpu`, `-fopenmp`, `-fiopenmp` など）を `SOLOMON_FC` または `SOLOMON_FLAGS` のいずれかに必ず含めてください
       * `_OPENACC` / `_OPENMP` のいずれも検出されなかった場合は注意メッセージ（`solomon: note: ...`）が表示され，全ての指示文はシリアルコードに展開されます．オフロードを意図していた場合は有効化フラグを追加してください（v2.0.0 以降）
       * `fortran.mk` は，`-DSOLOMON_OFFLOAD_BY_OPENACC` か `-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET` で求めたバックエンドが `_OPENACC` / `_OPENMP` を検出できずに無効になったときにも注意メッセージを表示します（`SOLOMON_FLAGS` に書いた `-D_OPENACC=...` / `-D_OPENMP=...` も検出に数えます．v2.0.0 以降）
       * `fortran.mk` と `spp.sh` は，渡されたフラグのままコンパイラ自身に尋ねます．小さな探りのファイルをコンパイラに前処理させて `_OPENACC` / `_OPENMP` を（`spp.sh` の C/C++ では言語と規格（`-std=`）も．システムの `cpp` には，コンパイラが答えた規格の `-std=` を渡します．`cpp` がその規格を知らないときは知っている次に古いものを渡し，`__cplusplus`・`__STDC_VERSION__` はコンパイラの値にします）知り，2 行の自由形式のプログラムをコンパイルさせて Fortran の形式を知ります．コンパイラが探りを前処理しない（コンパイラが拒む知らないフラグ，読まない拡張子．ifx は知らないフラグを警告して無視し，処理を続けます）か，自由形式のプログラムをコンパイルできないときは，コンパイラの文言（最初の行．それが位置だけなら次の行も）を添えてエラーで止まります．そのフラグでコンパイラが固定形式として読むソースは止めます（固定形式は未対応．`fortran.mk` は自由形式の `.f90` だけを扱います）．このためにソース 1 つあたりコンパイラを 1〜2 回多く呼びます（v2.0.0 以降）
       * 探りは現在のディレクトリで動かし，探りの入力と出力は専用の作業ディレクトリに置きます．そのためフラグの相対パスや `@file` はそのままの意味で効き，出力の名前から決まるファイル（`-MD`・`-MMD`・`-save-temps` など）は作業ディレクトリに出ます．フラグが名前を指定したファイル（`-MF x.d`・`-Wp,-MMD,x.d`・`-MJ x.json`）は，探りでもその名前に書かれます．現在のディレクトリの `_solomon` で始まる名前は予約です（探りの間に書かれたものは消します）
       * `SOLOMON_FC` と `SOLOMON_FLAGS` は，ほかのレシピと同じくシェルの語として読みます．引用した値（例: `-DMSG='"a b"'`）と `-I <dir>`・`-D <macro>` の形も使えます．`SOLOMON_FC` が空のとき，コンパイラ・`cpp`・`awk`・`sed` が見つからないとき，フラグが読めないとき（引用符の閉じ忘れ，値の無い `-I`・`-D`），ソースが通常のファイルでないとき，出力のパスがディレクトリであるとき（規則が走るとき．そのディレクトリがソースより新しいと make は規則を走らせません）は，`fortran.mk` はエラー（`solomon: error: ...`）で止まります（v2.0.0 以降）．recipe は `SHELL` に POSIX sh（make の既定の `/bin/sh`，bash，dash）を前提にします．zsh のように語を分けないシェルではエラーで止まります
       * 前処理系のオプション（`-I`・`-D`・`-U`・`-include` など）は，`SOLOMON_FC`（`spp.sh` では `-compiler=` も）の語の中に書いたものも `cpp` に渡します．`cpp` が定義する予約されていない名前のマクロ（`linux`・`unix` など）は，コンパイラも定義するときだけ残します（gfortran では `unix` という名前の Fortran の変数がそのまま残ります．nvfortran と ifx はこれらを定義します）
         * これらは書いた順（`SOLOMON_FC`・`-compiler=` の語が先，その後にフラグ）で，Solomon が自分で足すオプションの後に `cpp` へ渡します．`#include "..."` はコンパイラと同じく，ソースのディレクトリを先に探します．gcc と同じく，`-U X` と `-D X` は後に書いた方が効きます（ifx の前処理は順序によらず `-U` を優先するので，1 つのマクロに両方を書かないでください）
         * `cpp` に届く綴りは，`-I`・`-D`・`-U`・`-include`・`-imacros`・`-isystem`・`-idirafter`・`-iquote`（値を次の語に書く形とつなげた形），`-Xpreprocessor <option>`，`-Wp,<options>`，`--include=` / `--include` です．ほかの綴り（`--define-macro=` など）は届きません．`-include`・`-imacros` で指定したファイルがカレントに無いときは，ソースのディレクトリを先に探します（Fortran では gfortran と ifx は `-include` を無視し，nvfortran は拒否します）
       * nvfortran のときは，`fortran.mk` と `spp.sh` は `cpp` に `-D__SOLOMON_NVCOMPILER__` も渡し，nvfortran 26.9 が拒否する `PRAGMA_OMP_WORKSHARE(...)` の `OMP_CLAUSE_NOWAIT` を捨てます
       * GCC の `cpp` は，文字列の外の非 ASCII（日本語の注釈など）を `spp/*.f90` で `\Uxxxxxxxx` に書き換えます．コンパイルには影響しません
       * `cpp` は Fortran の注釈の中でも C の規則で注釈と引用符を読みます: 注釈の中の `/*`（例: `! see /* this`）は前処理を止め（`unterminated comment`），注釈の中の 1 つだけの `'`（例: `! don't`）は行ごとに警告を出します（出力は変わりません）
       * `spp.sh` と `fortran.mk` は `PATH` に `cpp`（GCC か LLVM の C の前処理系）・`awk`・`sed` を要します．それらかコンパイラが見つからないとエラーで止まります
     * ソースファイル中の Fortran の文字列連結（`//`）はプリプロセス時に保護されるため，自由に使用できます（v2.0.0 以降，`fortran.mk`・`spp.sh` の両方）
       * この保護のために `__SOLOMON_FC_CONCAT__` というトークンを Solomon が予約しています．ユーザーコード中に書かないでください
     * コンパイルの対象のファイルをsppディレクトリの下にある.f90に変更
     * Solomonの補助Makefileである`fortran.mk`をincludeするように絶対パスあるいは相対パスで指定
     * `fortran.mk` は `spp/<name>.f90` を `<name>.f90` から作ります．サブディレクトリのソースも扱えます（`spp/sub/x.f90` は `sub/x.f90` から）．ソースの引用符の include（`#include "..."`）は，ソースのディレクトリ，次に `-I` のディレクトリの順に，ソースそのものを前処理したときと同じく探します（`../` を含むパスも）．include されたファイルの中の引用符の include は，そのファイルのディレクトリ，ソースのディレクトリ，`-I` のディレクトリの順に探します（直接の前処理はソースのディレクトリを探しません）．作業ファイルは `spp/` の下の専用ディレクトリに置いて，プリプロセスが失敗しても消します（v2.0.0 以降）
       * `make` は空白を含むファイル名を扱えず，バックスラッシュ（`\`）か二重引用符（`"`）を含むパスは `#line` に書けないため，`fortran.mk` はこれらをエラーで止めます（空白を含む名前には `spp.sh` を使ってください）

  3. 以上の変更により，makeコマンドでSolomonを用いて実装したコードから，バックエンドとしてOpenACCまたはOpenMP targetを用いてGPU化したプログラムが生成され，コンパイル・リンクされるようになります

* 手動コード生成・コンパイル方法
  * 前述の半自動コード生成・コンパイル方法において `include $(SOLOMON_DIR)/fortran.mk` によって簡略化していた手順を，全て自分で実施するという内容です
    * 下記手順（2, 3）については，事前に手順1で出力先ディレクトリ（以下の例では`spp`）を作成した上で，簡易スクリプト（試作版）を用いて実行することもできます：

      ```sh
      export SOLOMON_DIR=/path/to/solomon
      "${SOLOMON_DIR}"/spp.sh -compiler=nvfortran -acc=gpu -mp=gpu -I"${SOLOMON_DIR}" -DSOLOMON_OFFLOAD_BY_OPENACC main.f90 > spp/main.f90 # NVIDIA HPC SDK の場合
      "${SOLOMON_DIR}"/spp.sh -compiler=amdflang -fopenmp -I"${SOLOMON_DIR}" -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET main.f90 > spp/main.f90 # AMD ROCm の場合
      "${SOLOMON_DIR}"/spp.sh -compiler=ifx -fiopenmp -I"${SOLOMON_DIR}" -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET main.f90 > spp/main.f90 # Intel oneAPI の場合
      ```

      * `-compiler=...` の引数およびコンパイラフラグは使用する環境に応じて適宜置き換えてください
      * `spp.sh` のソースは最後の引数です．`-compiler=`・`-I`・`-D`（`-I <dir>`・`-D <macro>` の形も）はどこに書いても読み，それ以外の引数はそのままコンパイラに渡します（ソースが最後に無いと，最後の引数を示してエラーで止まります．名前が `-` で始まるソースは `./<name>` の形で渡してください．v2.0.0 以降）
      * `spp.sh` は C と C++ のソースも扱います．言語はコンパイラに従い（`g++`・`mpicxx` などでは `.c` のソースも C++），`-std=` はコンパイラのものです（上の探りの注記を参照）
      * nvc / nvc++ 26.9 では，`spp.sh` に渡すフラグから `-MD` / `-MMD` を外してください．付けるとコンパイラが `-E` の前処理の結果を書かず，`spp.sh` が止まります（外しても前処理の結果は変わりません）
      * `-compiler=...`, `-I...`, `-D...` 以外のコンパイラフラグ（`-acc=gpu`, `-mp=gpu`, `-fopenmp`, `-fiopenmp` など）は `_OPENACC` / `_OPENMP` の自動検出時にコンパイラへ渡されるため，`-D_OPENACC=...` / `-D_OPENMP=...` を手動で指定する必要はありません（v2.0.0 以降．従来の `-compiler="nvfortran -acc=gpu"` という書き方も引き続き使用できます）
      * GPU アーキテクチャを指定するフラグ（`-gpu=...`, `--offload-arch=...`, `-Xs "-device ..."`）はマクロの検出には不要です．渡すと探りでも使われます（その分時間がかかります）．最終的な compile/link 時に指定してください
      * `"${SOLOMON_DIR}"/spp.sh` については，Solomonのパスに`PATH`を通した上で`spp.sh`として実行することもできます
      * `spp.sh` はプリプロセスの結果を標準出力に書きます．作業ファイルはカレントディレクトリの `spp/` の下の専用ディレクトリに置き（`spp/` が無ければ作ります），終了時に消すので，ソースのディレクトリにはファイルを作りません．引用符の include（`#include "..."`）は `fortran.mk` と同じ順に探します（v2.0.0 以降）
      * バックスラッシュ（`\`）か二重引用符（`"`）を含むソースのパスは，`#line` に書けないためエラーで止めます
      * `spp.sh` も `fortran.mk` と同じ 2 種類の注意メッセージ（`solomon: note: ...`）を標準エラーに出します．標準出力と終了コードは変わりません（v2.0.0 以降）

  1. Fortranのソースファイルのディレクトリの下にSolomonによる処理結果を保存する専用のディレクトリ `spp` を以下のコマンドで作成してください

     ```sh
     mkdir -p spp
     ```

  2. 用いるコンパイラが対応できるOpenACC または OpenMP target のversionを表す年月の文字列を以下の2つのコマンドで確認してください

     ```sh
     echo "OPENACC=_OPENACC OPENMP=_OPENMP" > spp/solomon.F
     nvfortran -E -acc=gpu -mp=gpu spp/solomon.F # NVIDIA HPC SDK の場合
     amdflang -E -fopenmp spp/solomon.F # AMD ROCm の場合
     ifx -E -fiopenmp spp/solomon.F # Intel oneAPI の場合
     ```

     * 2-4行目のコンパイラおよびコンパイルオプションは，使用する環境に応じて適宜置き換えてください
     * GPU アーキテクチャを指定するフラグ（`-gpu=...`, `--offload-arch=...`, `-Xs "-device ..."`）は，`_OPENACC` / `_OPENMP` の値が OpenACC/OpenMP の有効化フラグのみで決まるためここでは不要です（実際の compile/link 時（手順 4）に指定してください）
     * コンパイラのそれぞれの機能が有効化できる場合には，この出力としてOpenACCとOpenMPのversionを表す年月の文字列が`OPENACC=201711 OPENMP=202011` のように表示されます
       * 有効化できない場合は`OPENACC=_OPENACC OPENMP=202011` のように元の文字列のまま表示されます

  3. FortranのソースファイルをC言語用のプリプロセッサを用いた以下の3つのコマンドでプリプロセスを実行してください（文字列連結 `//` を退避し，`cpp -P` を通してから，`//` を戻して `#pragma` を `!$` にします）

     ```sh
     sed 's,//,__SOLOMON_FC_CONCAT__,g' mysrc.f90 > spp/mysrc.src.f90
     cpp -P -DSOLOMON_FORTRAN -D_OPENACC=201711 -I../../../solomon -DSOLOMON_OFFLOAD_BY_OPENACC spp/mysrc.src.f90 > spp/mysrc.i.f90
     sed -e 's/^#pragma solomon_fprof //' -e 's/^#pragma /!$/g' -e 's,__SOLOMON_FC_CONCAT__,//,g' spp/mysrc.i.f90 > spp/mysrc.f90
     ```

     * これらのコマンドは正本である `fortran.mk` の規則の要約で，そのまま使う手順ではありません（`spp/` の中の作業ファイルを同じ名前の include が拾うことがあり，clang の `cpp` は `.f90` の入力を Fortran のコンパイラに渡します）．`fortran.mk` か `spp.sh` を使ってください．規則はさらに作業ファイルの先頭に `#line 1 "mysrc.f90"` を書き（自動のプロファイラのタグが `mysrc.f90` を指すように），作業ファイルを専用のディレクトリの連なりの底に 1 つだけ置いて`-x c`（`clang -E` を起動する `cpp` にも作業ファイルを C として読ませるため）とソースのディレクトリの `-iquote` を `cpp` に渡します（`cpp` が GCC のものなら `-ftrack-macro-expansion=0` も渡します．[プロファイラタグ](#プロファイラタグnvtx-roctx-itt) を参照）．nvfortran のときは `-D__SOLOMON_NVCOMPILER__=1`（`__NVCOMPILER` の値）も渡し，`cpp` の予約されていない名前のマクロは，コンパイラも定義するとき以外は取り消します（`-Ulinux -Uunix`）

  4. 最後に，以下のようにコンパイルを実行してください

     ```sh
     nvfortran -O3 -acc=gpu -gpu=[target GPU architecture (e.g., cc90)] spp/mysrc.f90 -o myprog # NVIDIA HPC SDK（OpenACC）の場合
     nvfortran -O3 -mp=gpu -gpu=[target GPU architecture (e.g., cc90)] spp/mysrc.f90 -o myprog # NVIDIA HPC SDK（OpenMP target）の場合
     amdflang -O3 -fopenmp --offload-arch=[target GPU architecture (e.g., gfx942)] spp/mysrc.f90 -o myprog  # AMD ROCm（OpenMP target）の場合
     ifx -O3 -fiopenmp -fopenmp-targets=spir64_gen -Xs "-device [target GPU architecture (e.g., pvc)]" spp/mysrc.f90 -o myprog  # Intel oneAPI（OpenMP target）の場合
     ```

* 実際に生成される指示文を確認する方法
  * 中間ファイル `spp/*.f90` が生成されているので，このファイルを開くと実際に生成された指示文が確認できます
  * プリプロセスフラグとして `-DPRINT_GENERATED_PRAGMA` を追加すると，生成された指示文が，その直前の注釈の行 `!$message("#pragma " "...")` として `spp/*.f90` にもう一度書かれます．Fortran のコンパイラはコンパイル時に何も表示しません

* 使用例：[拡散方程式用の Makefile](samples/F/diffusion/Makefile)
  * 半自動コード生成・コンパイル方法の手順例です

### v1.x との後方互換性（v2.0.0 以降）

* v2.0.0 では，ユーザーコードや他ライブラリとの名前衝突を避けるため，ユーザー向けマクロを全て改名しました:
  * 指示文マクロには `SOLOMON_` 接頭辞が付きます（例: `OFFLOAD(...)` は `SOLOMON_OFFLOAD(...)` になりました）
  * 引数トークンには `SOLOMON_CLAUSE_` 接頭辞が付きます（例: `AS_INDEPENDENT` は `SOLOMON_CLAUSE_INDEPENDENT`，`COLLAPSE(n)` は `SOLOMON_CLAUSE_COLLAPSE(n)` になりました）
* v1.x 向けに書かれたコードは，`SOLOMON_WITH_SHORT_NAMES` を定義する（例: コンパイルフラグに `-DSOLOMON_WITH_SHORT_NAMES` を追加する）ことで従来の接頭辞なしの綴りが有効になり，そのまま動作します（デフォルト: OFF）
* `_Pragma` を組み立てる下請けの `PRAGMA(...)`，`PRAGMA_STR(x)`，`PRAGMA_STR1(x)`，`SHOW_PRAGMA(...)` は，v2.0.0 では内部名（`SOLOMON_INTERNAL_` 接頭辞）になり，デフォルトでは定義されません．`SOLOMON_WITH_SHORT_NAMES` を定義すると v1.x の名前 `PRAGMA(...)` が使えます．`PRAGMA_STR(x)`・`PRAGMA_STR1(x)`・`SHOW_PRAGMA(...)` は，v1.x と同じくさらに `PRINT_GENERATED_PRAGMA` を定義したときだけ定義されます．自前の指示文マクロの部品にしている場合は，定義（`solomon/short_names.hpp`）を自分のコードにコピーしてください
* シリアル実行（OpenACC・OpenMP のいずれも有効でない場合）では，`SOLOMON_APPEND_ARGS(...)` と v1.x の名前 `APPEND_ARGS(...)` は空に展開され，引数は消えます（例: `call f(0 SOLOMON_APPEND_ARGS(1, 2))` は `call f(0)` になります）
* `OMP_TARGET_CLAUSE_MAP(...)`（OpenACC と OpenMP target のとき．縮退モードとシリアルモードでは節を出さないので起きません）: 入れ子でない並びの 3 番目以降の要素が，引数 1 個では呼べない関数形式マクロの名前のとき（例: `#define fm2(x, y) ...` と `OMP_TARGET_CLAUSE_MAP(a, b, fm2)`）は前処理が失敗し，引数 1 個で呼ぶとトップレベルのカンマを含む形に展開される関数形式マクロの名前のとき（例: `#define fc1(x) x, x`）は内部のマクロの名前を挙げる Solomon のエラーでビルドが止まるようになりました．その要素を 1 番目か 2 番目に置いてください
* 設定マクロには新しい綴り `-DSOLOMON_OFFLOAD_BY_*` が追加されました．従来の `-DOFFLOAD_BY_*` は無条件で使用し続けられます（`SOLOMON_WITH_SHORT_NAMES` の定義は不要です）
* `AS_BLOCK` / `NUM_BLOCKS(n)`（`SOLOMON_CLAUSE_BLOCK` / `SOLOMON_CLAUSE_NUM_BLOCKS(n)`）は OpenACC での意味が変わりました: `gang` / `num_gangs(n)` を出力します（v1.x: `worker` / `num_workers(n)`）．ブロックを OpenACC では gang に，OpenMP target では team にそろえるための変更です．コンパイルは通りますが OpenACC での並列化の対応付けが変わるので，v1.x の意味を OpenACC で保つには代わりに `ACC_CLAUSE_WORKER` / `ACC_CLAUSE_NUM_WORKERS(n)` を書いてください．同様に，`NUM_GRIDS(n)`（`SOLOMON_CLAUSE_NUM_GRIDS(n)`．`SOLOMON_CLAUSE_NUM_BLOCKS(n)` の別名）は OpenMP target でも `num_teams(n)` を出力するようになった（v1.x では捨てられていた）ため，`NUM_GRIDS(n)` と `NUM_BLOCKS(m)` を併用すると同じ指示節が 2 つ出力されます（例は [misc/migrate/MIGRATION.md](misc/migrate/MIGRATION.md)）．移行スクリプトは，これらを含むファイルについて注意を表示します（書き換えずに残す生の指示文の行にだけある場合も）
* 新旧 API の完全な対応表は [misc/migrate/MIGRATION.md](misc/migrate/MIGRATION.md) を参照してください
* 移行スクリプト `misc/migrate/solomon_migrate_v1_to_v2.sh` を同梱しています．v1.x のソースコード（およびファイル引数として明示したビルドスクリプト）を v2.0.0 の綴りに書き換えます．デフォルトは dry-run（diff 表示のみ）で，`--apply` を付けるとバックアップ（`*.v1.bak`）を残して書き換えます．デフォルトでは `solomon.hpp` を読み込むソースだけを書き換えます（それ以外は，`solomon.hpp` を読み込まずに v1.x の名前を使う利用者のヘッダも含めて，飛ばして表示します．`--all` を付けるとそれらも書き換えます）．bash と GNU sed が必要で，bash 4 以降と GNU の道具で確かめています

### Solomon の拡張方法（コードジェネレータを用いた更新方法）

* Solomon では，1つの指示文ごとに 32 個の指示節・指示句（候補）を受け付ける仕様となっています（指示文に合わない指示節と Solomon の指示節マクロでない名前も数えます．空の引数は数えません．ただし 33 個以上続く場合は下記）．超えるとビルドが止まります（v2.0.0 以降）: GCC・Clang・Intel のコンパイラ（C/C++）は `error: Solomon: too many clauses for one directive (limit 32)` を，NVIDIA HPC SDK（C/C++）はマクロの行で `invalid text in pragma` を出します．Fortran では指示文に `SOLOMON_ERROR_TOO_MANY_CLAUSES_FOR_ONE_DIRECTIVE` が残り，Fortran のコンパイラがそこで構文エラーを出します
  * 候補 2 個に数えられる指示節があります: OpenMP target の引数ありの `ACC_CLAUSE_SELF(...)`（縮退モードでは計算構文のものだけ．引数なしの `ACC_CLAUSE_SELF()` は 1 個）と，OpenACC の `OMP_TARGET_CLAUSE_MAP(...)` の `present` の付いた入れ子の指示節（`OMP_TARGET_MAP_MODIFIER_PRESENT` の後ろのもの，入れ子の `OMP_TARGET_CLAUSE_TO_PRESENT(...)` / `OMP_TARGET_CLAUSE_FROM_PRESENT(...)`．update に直接書いたものは 1 個）．縮退モードの `SOLOMON_CLAUSE_NUM_THREADS(...)`（`ACC_CLAUSE_VECTOR_LENGTH(...)` も）も 2 個に数えます
  * OpenACC では，`SOLOMON_OFFLOAD_OUTER_LOOP(...)` と `SOLOMON_PARALLELIZE_INNER_LOOP(...)` は `loop` に `gang` / `vector` を足すので，指示節を 31 個まで受け付けます．同じく `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)` は 30 個まで（`gang` と `independent` を足す），`OMP_TARGET_CLAUSE_MAP_DELETE(...)` を含む exit data は 31 個まで（`finalize` を足す）です
  * ほかの引数の前に続けて書いた空の引数は，33 個ごとに候補 1 個に数えられます（33〜65 個で 1 個，66〜98 個で 2 個，…）
  * 多数の指示節を並べる場合と，`SOLOMON_DECLARE_DATA_ON_DEVICE(...)` / `PRAGMA_ACC_DATA_PRESENT(...)` に長い変数の並びを渡す場合は，`!$acc` / `!$omp` の行が 132 桁を超えます．gfortran では `-ffree-line-length-none` を付けてください
  * 1 つの指示文の `wait` の節をまとめた引数は合計 32 個までです．超えると `error: Solomon: too many arguments of the wait clauses merged into one (limit 32)`（Fortran は `SOLOMON_ERROR_TOO_MANY_MERGED_WAIT_ARGUMENTS`）でビルドが止まります．1 つの `wait` 節に書いた引数はまとめの対象でないので，この上限を受けません
  * 32 という上限値では不足する場合には，下記の 4 つのジェネレータを同じ値で実行し直して上限値を増やしてください（`dedup_wait.jl` はまとめた `wait` の引数の上限も，`nest_map.jl` は 1 つの `OMP_TARGET_CLAUSE_MAP(...)` の入れ子の指示節の数も上げます）．ジェネレータは Julia のスクリプトで，パッケージ `ArgParse` が要ります（`julia -e 'import Pkg; Pkg.add("ArgParse")'`）

    ```sh
    cd solomon/util # このディレクトリには，jl/*.jl が格納されています
    julia jl/check_clause.jl --max 64 # この例は，上限値を 64 に増やす場合です
    julia jl/sort_clause.jl --max 64 --level 1
    julia jl/dedup_wait.jl --max 64
    julia jl/nest_map.jl --max 64 --mods 6
    ```

  * ジェネレータはどのディレクトリから実行しても `solomon/util/impl/` に書き出します．不正な値（`--max 0` など）を与えると，ヘッダを書かずに終了コード 1 で止まります．`sort_clause.jl`・`dedup_wait.jl`・`nest_map.jl` のどれかを `check_clause.jl` より小さい値で生成したままにすると，ビルドが `#error "Solomon: regenerate check_clause.jl, sort_clause.jl, dedup_wait.jl, and nest_map.jl in solomon/util/jl with the same --max (README)"` で止まります（v2.0.0 以降）
  * 同様の制限が他の内部マクロに存在することがありますが，同様の手順で上限値を増やせます
  * 提供済みのコードジェネレータは下記の通りで，すべて `solomon/util/jl/` に配置されています

  | ジェネレータ | 目的 | デフォルト最大値 |
  | ------------ | ---- | ---------------- |
  | `check_clause.jl` | 節チェックマクロの生成 | 32 |
  | `num_args.jl` | 入力数読取マクロの生成 | 1024 |
  | `pickup_clause.jl` | 節フィルタリングマクロの生成 | 99 |
  | `retrieve_args.jl` | 入力フィルタリングマクロの生成 | 128 |
  | `sort_clause.jl` | 節ソートマクロの生成 | 32 |
  | `dedup_wait.jl` | 1 つの指示文の `wait` 節をまとめるマクロの生成 | 32 |
  | `is_empty.jl` | 空の判定に使うカンマ検出マクロの生成 | 32 |
  | `nest_map.jl` | `OMP_TARGET_CLAUSE_MAP(...)` の入れ子の指示節を扱うマクロの生成 | 32 |

### プロファイラタグ（NVTX, rocTX, ITT）

Solomon はベンダー中立なプロファイラタグを発行できます（v2.0.0 以降）．バックエンド（NVIDIA NVTX / AMD rocTX / Intel ITT）は使用中のコンパイラから自動選択されるため，ベンダー固有名がコードに現れることはありません（強制したい場合は `SOLOMON_TAGGED_PROFILE_WITH_NVTX` / `SOLOMON_TAGGED_PROFILE_WITH_ROCTX` / `SOLOMON_TAGGED_PROFILE_WITH_ITT` を定義してください）．自動で見分けるのは NVIDIA HPC SDK（と `nvcc`）と Intel のコンパイラだけです（NVTX と ITT）．AMD コンパイラはホスト側パスに識別マクロを定義しないため rocTX は自動選択できません．AMD 環境では常に `SOLOMON_TAGGED_PROFILE_WITH_ROCTX` を明示的に定義してください．GCC と Clang（ほかの LLVM 系のコンパイラを含む）も自動では選ばれないので，`..._WITH_*` を定義してください．

* `-DSOLOMON_TAGGED_PROFILE` で手動タグ用マクロが有効になります（他の Solomon マクロと同様，行末セミコロンなしで記述します）:
  * `SOLOMON_PROFILE_RANGE_BEGIN("name")` / `SOLOMON_PROFILE_RANGE_END`: 名前付き区間
  * `SOLOMON_PROFILE_MARK("name")`: 瞬間イベント
* `-DSOLOMON_TAGGED_PROFILE_AUTO` を定義すると，さらに Solomon の指示文マクロに `"FILE:LINE MACRO"` 形式のタグが自動で付きます:
  * データ転送・同期系（`SOLOMON_MEMCPY_*`, `SOLOMON_MALLOC_ON_DEVICE`, `SOLOMON_FREE_FROM_DEVICE`, `SOLOMON_SYNCHRONIZE` など）は操作全体を覆う区間になります
  * カーネル起動系ループマクロ（`SOLOMON_OFFLOAD`, `SOLOMON_OFFLOAD_OUTER_LOOP`）はマクロからループの終端が見えないため瞬間マークのみです．カーネル実行時間の計測には手動区間マクロ（またはプロファイラ自身のカーネルトレース）をお使いください
  * `SOLOMON_OFFLOAD_SERIAL`〜`SOLOMON_END_OFFLOAD_SERIAL` は実区間になります
  * g++ / gcc 14 以前の前処理系では，Solomon の指示文が多い翻訳単位の後ろの方の自動のタグの行番号が，Solomon のヘッダの行になることがあります（タグのマクロの名前と指示文は正しい）．GCC だけのオプション `-ftrack-macro-expansion=0` を C/C++ のコンパイルのフラグに足すと直ります（clang などはこのオプションを知らずエラーで止めるので，GCC のときだけ足してください）．Fortran では `spp.sh` と `fortran.mk` が，`cpp` が GCC のものなら自動で付けます
* リンク: NVTX は NVIDIA HPC SDK のオフロードフラグ（`-acc=gpu`，`-mp=gpu`，`-cuda`）があれば追加フラグ不要です．それらが無いとき（ホスト向けの `-mp` など）は `-cudalib=nvtx3` を追加してください．選ばれたバックエンドのヘッダが見つからないときは，追加するフラグを示すエラーでコンパイルが止まります．rocTX はコンパイル時に `-I$ROCM_PATH/include`，リンク時に `-L$ROCM_PATH/lib -lroctx64` を追加してください．ITT はコンパイル時に `-I$VTUNE_PROFILER_DIR/sdk/include`，リンク時に `-L$VTUNE_PROFILER_DIR/sdk/lib64 -littnotify` を追加してください（`VTUNE_PROFILER_DIR` は VTune の `env/vars.sh` が設定します）
* Fortran: 同梱ヘルパーを同じコンパイラ・フラグでコンパイルしてリンクしてください（例: `nvc -c -mp=gpu -DSOLOMON_TAGGED_PROFILE /path/to/solomon/profile/solomon_profile.c`．define なしでコンパイルすると no-op になります）．自動タグの `FILE` は `spp.sh` や `make` に渡したソースのパスになります（v2.0.0 以降．132 桁を超えるタグの行は `&` で継続します）

### エディタ支援（シンタックスハイライトと clang-format）

Solomon は単なるプリプロセッサマクロのため，エディタは既定ではハイライトしません．`SOLOMON_*` マクロと2段記法（`PRAGMA_ACC_*`, `PRAGMA_OMP_*`, `ACC_CLAUSE_*`, `OMP_CLAUSE_*`, `OMP_TARGET_*`）を指示文らしく見せる設定ファイルを `misc/editor/` に同梱しています（v2.0.0 以降）:

* vim / neovim: `misc/editor/vim/after/syntax/{c,cpp,fortran}.vim` を `~/.vim/after/syntax/` へコピー（または symlink）してください（neovim は `~/.config/nvim/after/syntax/`）
* emacs: `init.el` に `(load "/path/to/solomon/misc/editor/emacs/solomon-highlight.el")` を追加してください
* VS Code: フォルダ `misc/editor/vscode/solomon-highlight/` を `~/.vscode/extensions/` へコピー（または symlink）し，VS Code を再起動してください
  * これはビルド不要のローカル injection grammar（テキスト2ファイル）です．VS Code は `settings.json` だけでは正規表現ベースのハイライトを追加できないため，展開済みローカル拡張として読み込ませる方式を採っています（マーケットプレイスへの公開は行いません）
  * 指示文とコードの中の名前を色付けします．コメントと文字列の中は色付けしません
* clang-format: リポジトリ直下の `.clang-format` に Solomon の指示文マクロを `StatementMacros`/`AttributeMacros` として登録してあり，整形時に指示文行が崩れません．参考設定（強制ではありません）なので，マクロのリストをご自身の `.clang-format` へコピーしてお使いください

## サンプルコード

### diffusion: メモリ律速な問題の実装例

* [3次元拡散方程式](samples/F/diffusion)
  * [OpenACC を用いた実装（名古屋大学の星野哲也氏による実装）](https://github.com/hoshino-UTokyo/lecture_openacc) を改変したものです
  * Solomon 化以外に，簡単な性能最適化やコード改編も施してあります
  * C 版と Fortran 版があります
  * 各ディレクトリで `make` によりコンパイルできます

### declare: 宣言の指示文の使用例

* [オフロード領域から呼ぶ手続とデバイスに常駐する変数](samples/F/declare)
  * `SOLOMON_DECLARE_OFFLOADED(...)` を手続の宣言部と，呼ぶ側での `SOLOMON_CLAUSE_TARGETS(...)` による名前の形で使い，モジュールで `SOLOMON_DECLARE_ON_DEVICE(...)` も使います
  * 結果が正しければ `1000000.0` を表示して終了コード 0 で終わります（違えば終了コード 1 で止まります）．サンプルのディレクトリで `sbatch sh/whirlpool/verify.sh` を実行すると，NVIDIA HPC SDK で 4 つのモデルをビルドして実行します
  * オフロードするときは，`SOLOMON_DECLARE_ON_DEVICE(...)` で宣言した変数のホスト側の値がデバイスで上書きされないことも確かめます（ユニファイドメモリやホストへのフォールバックではこの確認を `SHARED_HOST_DEVICE_MEMORY` の定義で外してください）
  * NVIDIA HPC SDK（26.9）は同じファイルで定義した手続を宣言が無くてもデバイスから呼べるようにするため，このコンパイラでは `SOLOMON_DECLARE_ON_DEVICE(...)` の抜けは検出できますが，手続の宣言の抜けは検出できません

### サンプルの実行方法

* サンプル（`samples/F/diffusion/`，`samples/F/declare/`）は `sh/` 以下にジョブスクリプトを持っています
  * `sh/common/`: 全システムに共通の手順（`verify.sh` は全モデルの結果を確認し，`iterate.sh` は性能を測定します）．そこにあるほかのスクリプト（`guard.sh`，diffusion では `setup.sh` / `sweep.sh`）は手順が読み込む部品で，ジョブスクリプトからは実行しません
  * `sh/<system>/`（`local`，`whirlpool`，`miyabi-g`．declare のサンプルは `whirlpool` だけ）: `config.sh` にそのシステムの設定（コンパイラ，GPU のベンダーとアーキテクチャ，module 名）を，各ジョブスクリプトにスケジューラの指示行を書き，`sh/common/` の同名の手順を実行します
* ジョブスクリプトはサンプルのディレクトリで実行（投入）してください: 例えば `cd samples/F/diffusion && sbatch sh/whirlpool/verify.sh`，ジョブスケジューラが無ければ `bash sh/local/verify.sh`
  * `verify.sh` は結果を判定し，誤った結果があるとジョブが失敗します（v2.0.0 以降）: diffusion はすべての大きさ（32，37，64，…．37 は時間ステップ数が奇数になり，1 ステップ前のまま残った結果を不合格にします）で `Error < 2.0e-4` を求めます
  * ジョブスクリプトは引数を取りません（設定は `config.sh` にあります）．現在のディレクトリに `config.sh` が無いとき，Slurm と PBS のジョブスクリプトはまず `SLURM_SUBMIT_DIR` / `PBS_O_WORKDIR` へ移ります
  * `DETECT_NUMA=1` のとき，ジョブスクリプトは `CUDA_VISIBLE_DEVICES`（`ROCR_VISIBLE_DEVICES`）を `config.sh` の `GPU_ID` にしますが，既に値があれば（ジョブスケジューラやシェルで設定した場合など）それを保ちます（そのときは 1 行出し，`GPU_ID` は使いません）
  * diffusion の 1 回の実行は `config.sh` の `RUN_TIMEOUT`（既定 600s．`timeout` の時間の書き方，例えば 10m）で打ち切り，超えた実行は失敗に数えます（`timed out after RUN_TIMEOUT=...`）．whirlpool（RTX PRO 4000 Blackwell）では最も遅い実行（N=512，最適化なし）が約 100 秒かかるので，遅い GPU では大きくしてください
  * ジョブスクリプトは bash 4.1 以降，Environment Modules，`timeout`（GNU coreutils），`numactl` を要します（各手順が使うコマンドを最初に確かめ，見つからなければ止まります）．`flock`（util-linux）があれば下のロックに使います（無ければ警告を出してロックなしで動きます）
  * 同じサンプルのディレクトリで 2 つのジョブスクリプト（`verify.sh` と `iterate.sh` など）を同時に投入しないでください．2 つ目は `another job script is running` で止まります（1 つ目が `sh/.lock` を持っています）．同時に動かすときはサンプルのディレクトリの写しを使ってください（`samples/F/` の外に写したときは Solomon の場所が要ります: ジョブスクリプトに見えるように `SOLOMON_DIR=/path/to/solomon` を export してください）
  * 前の実行の生成物がサンプルのディレクトリに残っていると，ジョブスクリプトは止まります．外へ移してから実行し直してください（消しはしません）
* システムを追加するときは，`sh/local/`（または同じジョブスケジューラのシステムのディレクトリ）を `sh/<your-system>/` にコピーし，`config.sh` の値とスケジューラの指示行を書き換えてください
* Miyabi-G の課金グループ名は，`sh/miyabi-g/` の各ジョブスクリプトの `#PBS -W group_list=` の行（スケジューラの指示行ではシェルの変数を使えません）と，`samples/C/nbody/sh/miyabi-g/config.sh` の `PBS_GROUP`（`module use` のパスがこれを参照します）に書かれています．まとめて書き換えるには，`YOUR_GROUP` を利用者の課金グループ名にしてから Solomon のトップディレクトリで次を実行してください: `sed -i -E 's/^(#PBS -W group_list=|PBS_GROUP=).*/\1YOUR_GROUP/' samples/*/*/sh/miyabi-g/*.sh`

### サンプルを手で作る

* サンプルのディレクトリ（`samples/F/diffusion/`，`samples/F/declare/`）で `make` を実行してください．nvfortran で `./run` を作ります．対象の GPU の既定は `GPU_ARCH=90`（H100）です．ほかの GPU はコマンド行で指定してください（例: `make GPU_ARCH=120`）
* ほかのコンパイラは `AMDFLANG=1` か `IFX=1` で選びます（例: `make AMDFLANG=1 GPU_ARCH=gfx90a`，`make IFX=1 GPU_ARCH=pvc`）．AMD と Intel の GPU には `GPU_ARCH` の既定がありません．選べるコンパイラは 1 つまでで，OpenACC（`USE_OPENACC=1`）には NVIDIA HPC SDK が要ります．`NVHPC=0` でほかのコンパイラも選ばないと，`$(FC)`（指定しなければ gfortran）でオフロードしないホストの実行ファイルを作ります．ホストでも Makefile は OpenMP target のオフロードを要求するので，OpenMP のフラグが無いと `fortran.mk` がソースごとに「OpenMP target のバックエンドは無効」という note を出し，指示文はシリアルのコードに展開されます．OpenMP のフラグ（例: `FC='gfortran -fopenmp'`）を与えると，target 領域はホストで実行されます（ホストへのフォールバック）
* `SOLOMON_DIR`（既定 `../../../solomon`）は Solomon の `solomon/` ディレクトリです．サンプルのディレクトリを別の場所に置いたときは，コマンド行か環境変数で与えてください（例: `make SOLOMON_DIR=/path/to/solomon`）．export した `SOLOMON_DIR` も使われます．パスに空白，引用符，`$`，バッククォート，`\ ( ) ; & | < > : [ ] * ?` のどれも含めないでください．そこに `fortran.mk` が無ければ `make` が止まります
* make の変数やヘッダ（Solomon のものも）を変えると，前処理・コンパイル・リンクをやり直します．`make clean` は要りません．比べるのは変数の値だけです（`settings.txt` に記録し，`make clean` で消えます）．同じ名前の別のコンパイラ（別の module や `PATH`）に切り替えたときは `make clean` を実行してください
* diffusion: `./run N` で実行します．`N` は 1 次元あたりの格子点の数です（3 <= N で，N^3 が既定の整数に収まる，つまり N <= 1290）．引数が無い・多い・不正なときは使い方を標準エラーに出してコード 1 で止まります．`OPT_LEVEL` は 0（最適化のフラグなし），1（`-O1`），2（`-O2`），3（`-O3`），4（既定．nvfortran では `-Ofast`，ほかでは `-O3 -ffast-math`）をとります．それ以外の値では `make` がエラーで止まります．`USE_MANAGED=1` / `USE_UNIFIED=1` は `-gpu=mem:managed` / `-gpu=mem:unified` を渡します（`mem:` を知らない NVIDIA HPC SDK では `-gpu=managed` / `-gpu=unified`）
* diffusion: 実行ごとに，カレントディレクトリの `diffusion_benchmark_<compiler><MODEL_ID>.csv` に 1 行を足します．`<compiler>` は `FC` のファイル名です（例: `nvfortran`）
* declare: ホスト（GPU を使わない gfortran `-fopenmp` など）や統合メモリでは，デバイスに常駐する変数の検査が，これらの場合を挙げたメッセージで失敗します．そのときは `FC` に足して `SHARED_HOST_DEVICE_MEMORY` を定義してください（例: ホストでは `make NVHPC=0 FC='gfortran -fopenmp -DSHARED_HOST_DEVICE_MEMORY'`，統合メモリでは `make GPU_ARCH=120 FC='nvfortran -gpu=mem:unified -DSHARED_HOST_DEVICE_MEMORY'`）

## Solomon が提供する API

### 使用可能な指示文

* <details><summary>演算に関する指示文</summary>

  | 入力 | 出力 | バックエンドとして用いる指示文 |
  | ---- | ---- | ---- |
  | **`SOLOMON_OFFLOAD(...)`** <br> `PRAGMA_ACC_KERNELS_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL_LOOP(...)` <br> `PRAGMA_OMP_TARGET_TEAMS_LOOP(...)` <br> `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO(...)` | <br> `!$acc kernels __VA_ARGS__` <br> `!$acc loop __VA_ARGS__` <br> `!$acc parallel __VA_ARGS__` <br> `!$acc loop __VA_ARGS__` <br> `!$omp target teams loop __VA_ARGS__` <br> `!$omp target teams distribute parallel do __VA_ARGS__` | <br> OpenACC (kernels) <br><br> OpenACC (parallel) <br><br> OpenMP (loop: `_OPENMP` が 201811 以上) <br> OpenMP (distribute) |
  | **`SOLOMON_OFFLOAD_OUTER_LOOP(...)`** | `!$acc parallel __VA_ARGS__ !$acc loop gang __VA_ARGS__` <br> `!$omp target teams distribute __VA_ARGS__` <br> `!$omp parallel do __VA_ARGS__` | OpenACC <br> OpenMP target <br> OpenMP（縮退モード） |
  | **`SOLOMON_PARALLELIZE_INNER_LOOP(...)`** | `!$acc loop vector __VA_ARGS__` <br> `!$omp parallel do __VA_ARGS__` <br> 無視される（外側ループが並列化済みのため） | OpenACC <br> OpenMP target <br> OpenMP（縮退モード） |
  | **`SOLOMON_OFFLOAD_SERIAL(...)`** <br> `PRAGMA_ACC_SERIAL(...)` <br> `PRAGMA_OMP_TARGET(...)` | <br> `!$acc serial __VA_ARGS__` <br> `!$omp target __VA_ARGS__` | <br> OpenACC <br> OpenMP <br> 直後の構造化ブロックをデバイス上で単一スレッド実行する．reduction 結果をデバイス上に保持したい場合などに有用（v2.0.0 以降） |
  | **`SOLOMON_END_OFFLOAD_SERIAL`** | `!$acc end serial` <br> `!$omp end target` | OpenACC <br> OpenMP <br> serial 領域を終端する（v2.0.0 以降） |
  | **`SOLOMON_SYNCHRONIZE(...)`** <br> `PRAGMA_ACC_WAIT(...)` <br> `PRAGMA_OMP_TARGET_TASKWAIT(...)` | <br> `!$acc wait __VA_ARGS__`（キューIDだけ） <br> `!$omp taskwait`（`SOLOMON_SYNCHRONIZE`，`PRAGMA_ACC_WAIT`: 引数は無視される） <br> `!$omp taskwait __VA_ARGS__`（`PRAGMA_OMP_TARGET_TASKWAIT`: 指示節が渡る） | <br> OpenACC <br> OpenMP <br> OpenMP |
  | **`SOLOMON_WAIT_QUEUE(id)`** <br> `PRAGMA_ACC_WAIT(id)` | <br> `!$acc wait (id)` | <br> OpenACC (only) |
  | **`SOLOMON_DECLARE_OFFLOADED(...)`** <br> `PRAGMA_ACC_ROUTINE(...)` <br> `PRAGMA_OMP_DECLARE_TARGET(...)` | <br> `!$acc routine __VA_ARGS__` <br> `!$omp declare target __VA_ARGS__` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_DECLARE_OFFLOADED_END`** <br> `PRAGMA_OMP_END_DECLARE_TARGET` | <br> （何も出力しない） | <br> OpenMP (only) <br> Fortran では手続内の `SOLOMON_DECLARE_OFFLOADED(...)` が自己完結するため本マクロは空に展開される．書いても無害だが不要（v2.0.0 以降） |
  | **`SOLOMON_CLAUSE_TARGETS(...)`** | `(__VA_ARGS__)` | OpenACC/OpenMP <br> 手続を名前で指定する: `SOLOMON_DECLARE_OFFLOADED(SOLOMON_CLAUSE_TARGETS(func), ...)` は `!$acc routine (func) ...` / `!$omp declare target (func)` に展開される（v2.0.0 以降）．`SOLOMON_CLAUSE_TARGETS(...)` は引数のどの位置に書いてもよく，名前の並びは常に指示文名の直後に置かれる |
  | **`SOLOMON_DECLARE_ON_DEVICE(...)`** | `!$acc declare create(__VA_ARGS__)` <br> `!$omp declare target (__VA_ARGS__)` | OpenACC <br> OpenMP <br> モジュール・手続の宣言部でデバイス常駐変数を宣言する（v2.0.0 以降） |
  | **`SOLOMON_DECLARE_ON_DEVICE_LINKED(...)`** | `!$acc declare link(__VA_ARGS__)` <br> `!$omp declare target link(__VA_ARGS__)` | OpenACC <br> OpenMP <br> link 意味論でデバイス常駐変数を宣言する（v2.0.0 以降）．nvfortran（26.9）は OpenMP target で `link`（`declare target` の）を実装しておらず，警告（`OpenMP feature, LINK, not yet implemented`）だけを出して宣言なしでコンパイルする |
  | **`SOLOMON_ATOMIC(...)`** <br> `PRAGMA_ACC_ATOMIC(...)` <br> `PRAGMA_OMP_TARGET_ATOMIC(...)` | <br> `!$acc atomic __VA_ARGS__` <br> `!$omp atomic __VA_ARGS__` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_ATOMIC_UPDATE`** <br> `PRAGMA_ACC_ATOMIC_UPDATE` <br> `PRAGMA_OMP_TARGET_ATOMIC_UPDATE` | <br> `!$acc atomic update` <br> `!$omp atomic update` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_ATOMIC_READ`** <br> `PRAGMA_ACC_ATOMIC_READ` <br> `PRAGMA_OMP_TARGET_ATOMIC_READ` | <br> `!$acc atomic read` <br> `!$omp atomic read` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_ATOMIC_WRITE`** <br> `PRAGMA_ACC_ATOMIC_WRITE` <br> `PRAGMA_OMP_TARGET_ATOMIC_WRITE` | <br> `!$acc atomic write` <br> `!$omp atomic write` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_ATOMIC_CAPTURE`** <br> `PRAGMA_ACC_ATOMIC_CAPTURE` <br> `PRAGMA_OMP_TARGET_ATOMIC_CAPTURE` | <br> `!$acc atomic capture` <br> `!$omp atomic capture` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_END_OFFLOAD`** | `!$acc end kernels` <br> `!$acc end parallel` | OpenACC (kernels) <br> OpenACC (parallel) |
  | **`SOLOMON_END_OFFLOAD_OUTER_LOOP`** | `!$acc end parallel` | OpenACC (only) |
  | **`PRAGMA_ACC_END_PARALLEL`** | `!$acc end parallel` | OpenACC (only) |
  | **`PRAGMA_ACC_END_KERNELS`** | `!$acc end kernels` | OpenACC (only) |
  | **`PRAGMA_ACC_END_SERIAL`** | `!$acc end serial` | OpenACC (only) |
  | **`PRAGMA_ACC_END_DATA`** | `!$acc end data` <br> `!$omp end target data` | OpenACC <br> OpenMP |
  | **`PRAGMA_ACC_END_HOST_DATA`** | `!$acc end host_data` <br> `!$omp end target data` | OpenACC <br> OpenMP |
  | **`PRAGMA_ACC_END_ROUTINE`** | （何も出力しない） | OpenMP (only) <br> Fortran では空に展開される（手続内の `PRAGMA_ACC_ROUTINE(...)` には終わりの指示文が要らないため） |
  | **`PRAGMA_OMP_END_PARALLEL`** | `!$omp end parallel` | OpenMP (only) |
  | **`PRAGMA_OMP_END_TEAMS`** | `!$omp end teams` | OpenMP (only) |
  | **`PRAGMA_OMP_END_SIMD`** | `!$omp end simd` | OpenMP (only) |
  | **`PRAGMA_OMP_END_SINGLE`** | `!$omp end single` | OpenMP (only) |
  | **`PRAGMA_OMP_END_SECTIONS`** | `!$omp end sections` | OpenMP (only) |
  | **`PRAGMA_OMP_END_CRITICAL`** | `!$omp end critical` | OpenMP (only) <br> 名前の無い `critical` を閉じる |
  | **`PRAGMA_OMP_END_CRITICAL_WITH_NAME(name)`** | `!$omp end critical(name)` | OpenMP (only) <br> `critical`（`PRAGMA_OMP_CRITICAL(OMP_PASS_LIST(name), ...)` で開いたもの）を閉じる（Fortran では同じ名前が要る．v2.0.0 以降） |
  | **`PRAGMA_OMP_END_PARALLEL_SECTIONS`** | `!$omp end parallel sections` | OpenMP (only) |
  | **`PRAGMA_OMP_END_DECLARE_TARGET`** | （何も出力しない） | OpenMP (only) <br> Fortran では空に展開される（手続内の `PRAGMA_OMP_DECLARE_TARGET(...)` には終わりの指示文が要らないため） |
  | **`PRAGMA_OMP_END_TARGET_PARALLEL`** | `!$acc end kernels` <br> `!$acc end parallel` <br> `!$omp end target parallel` | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP |
  | **`PRAGMA_OMP_END_TARGET_SIMD`** | `!$acc end kernels` <br> `!$acc end parallel` <br> `!$omp end target simd` | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP |
  | **`PRAGMA_OMP_END_TARGET_TEAMS`** | `!$acc end kernels` <br> `!$acc end parallel` <br> `!$omp end target teams` | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP |
  | **`PRAGMA_OMP_END_TARGET_PARALLEL_DO`** | `!$acc end kernels` <br> `!$acc end parallel` <br> （なし） | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP（ループ構文の終わりの指示文は省略できる） <br> OpenACC 使用時に同じ名前の OpenMP の記法のループ系の指示文を閉じる．代わりに `SOLOMON_END_OFFLOAD` を推奨する（v2.0.0 以降） |
  | **`PRAGMA_OMP_END_TARGET_PARALLEL_DO_SIMD`** | `!$acc end kernels` <br> `!$acc end parallel` <br> （なし） | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP（ループ構文の終わりの指示文は省略できる） <br> OpenACC 使用時に同じ名前の OpenMP の記法のループ系の指示文を閉じる．代わりに `SOLOMON_END_OFFLOAD` を推奨する（v2.0.0 以降） |
  | **`PRAGMA_OMP_END_TARGET_PARALLEL_LOOP`** | `!$acc end kernels` <br> `!$acc end parallel` <br> （なし） | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP（ループ構文の終わりの指示文は省略できる） <br> OpenACC 使用時に同じ名前の OpenMP の記法のループ系の指示文を閉じる．代わりに `SOLOMON_END_OFFLOAD` を推奨する（v2.0.0 以降） |
  | **`PRAGMA_OMP_END_TARGET_TEAMS_DISTRIBUTE`** | `!$acc end kernels` <br> `!$acc end parallel` <br> （なし） | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP（ループ構文の終わりの指示文は省略できる） <br> OpenACC 使用時に同じ名前の OpenMP の記法のループ系の指示文を閉じる．代わりに `SOLOMON_END_OFFLOAD` を推奨する（v2.0.0 以降） |
  | **`PRAGMA_OMP_END_TARGET_TEAMS_DISTRIBUTE_SIMD`** | `!$acc end kernels` <br> `!$acc end parallel` <br> （なし） | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP（ループ構文の終わりの指示文は省略できる） <br> OpenACC 使用時に同じ名前の OpenMP の記法のループ系の指示文を閉じる．代わりに `SOLOMON_END_OFFLOAD` を推奨する（v2.0.0 以降） |
  | **`PRAGMA_OMP_END_TARGET_TEAMS_LOOP`** | `!$acc end kernels` <br> `!$acc end parallel` <br> （なし） | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP（ループ構文の終わりの指示文は省略できる） <br> OpenACC 使用時に同じ名前の OpenMP の記法のループ系の指示文を閉じる．代わりに `SOLOMON_END_OFFLOAD` を推奨する（v2.0.0 以降） |
  | **`PRAGMA_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO`** | `!$acc end kernels` <br> `!$acc end parallel` <br> （なし） | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP（ループ構文の終わりの指示文は省略できる） <br> OpenACC 使用時に同じ名前の OpenMP の記法のループ系の指示文を閉じる．代わりに `SOLOMON_END_OFFLOAD` を推奨する（v2.0.0 以降） |
  | **`PRAGMA_OMP_END_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD`** | `!$acc end kernels` <br> `!$acc end parallel` <br> （なし） | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP（ループ構文の終わりの指示文は省略できる） <br> OpenACC 使用時に同じ名前の OpenMP の記法のループ系の指示文を閉じる．代わりに `SOLOMON_END_OFFLOAD` を推奨する（v2.0.0 以降） |

  </details>

  * <details><summary>抽象化マクロ</summary>

    | 入力 | （中間）出力 | バックエンドとして用いる指示文 |
    | ---- | ---- | ---- |
    | `PRAGMA_ACC_LAUNCH_DEFAULT(...)` <br> `PRAGMA_OMP_TARGET_LAUNCH_DEFAULT(...)` | `PRAGMA_ACC_KERNELS(__VA_ARGS__)` <br> `PRAGMA_ACC_PARALLEL(__VA_ARGS__)` <br> `PRAGMA_OMP_TARGET_TEAMS(__VA_ARGS__)` | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP |
    | `PRAGMA_ACC_OFFLOADING_DEFAULT(...)` <br> `PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(...)` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__) PRAGMA_ACC_LOOP(__VA_ARGS__)` <br> `PRAGMA_OMP_TARGET_TEAMS_LOOP(__VA_ARGS__)` <br> `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO(__VA_ARGS__)` | OpenACC <br> OpenMP (loop) <br> OpenMP (distribute) |

    </details>

  * <details><summary>OpenACC</summary>

    | 入力 | OpenACC 使用時の出力 | OpenMP target 使用時の出力 |
    | ---- | ---- | ---- |
    | `PRAGMA_ACC_PARALLEL(...)` | `!$acc parallel __VA_ARGS__` | `PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(__VA_ARGS__)` |
    | `PRAGMA_ACC_KERNELS(...)` | `!$acc kernels __VA_ARGS__` | `PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(__VA_ARGS__)` |
    | `PRAGMA_ACC_SERIAL(...)` | `!$acc serial __VA_ARGS__` | `PRAGMA_OMP_TARGET(__VA_ARGS__)`（v2.0.0 以降） |
    | `PRAGMA_ACC_LOOP(...)` | `!$acc loop __VA_ARGS__` | N/A（OpenMP target 使用時には無視される） |
    | `PRAGMA_ACC_CACHE(...)` | `!$acc cache(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） |
    | `PRAGMA_ACC_ATOMIC(...)` | `!$acc atomic __VA_ARGS__` | `PRAGMA_OMP_TARGET_ATOMIC(__VA_ARGS__)` |
    | `PRAGMA_ACC_WAIT(...)` | `!$acc wait __VA_ARGS__` | `PRAGMA_OMP_TARGET_TASKWAIT()` |
    | `PRAGMA_ACC_ROUTINE(...)` | `!$acc routine __VA_ARGS__` | `PRAGMA_OMP_DECLARE_TARGET(__VA_ARGS__)` |
    | `PRAGMA_ACC_DECLARE(...)` | `!$acc declare __VA_ARGS__` | N/A（OpenMP target 使用時には無視される．`SOLOMON_DECLARE_ON_DEVICE(...)` をお使いください） |

    </details>

  * <details><summary>OpenMP target</summary>

    | 入力 | OpenMP target 使用時の出力 | OpenACC 使用時の出力 | 縮退モード（演算加速器を用いないCPU実行）での出力 |
    | ---- | ---- | ---- | ---- |
    | `PRAGMA_OMP_TARGET(...)` | `!$omp target __VA_ARGS__` | `PRAGMA_ACC_SERIAL(__VA_ARGS__)`（v2.0.0 以降） | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_TARGET_PARALLEL(...)` | `!$omp target parallel __VA_ARGS__` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_DO(...)` | `!$omp target parallel do __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_DO(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_DO_SIMD(...)` | `!$omp target parallel do simd __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_DO_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_LOOP(...)` | `!$omp target parallel loop __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_LOOP(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_SIMD(...)` | `!$omp target simd __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)`（v2.0.0 以降） | `PRAGMA_OMP_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS(...)` | `!$omp target teams __VA_ARGS__` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS(__VA_ARGS__)`（縮退モードでも teams のまま．NVIDIA HPC SDK（`-mp`）では使えない．gfortran ではチーム数を指定しなければ並列にならない） |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)` | `!$omp target teams distribute __VA_ARGS__` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__) PRAGMA_ACC_LOOP(ACC_CLAUSE_GANG, ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)`（`SOLOMON_OFFLOAD_OUTER_LOOP(...)` と同じ．`ACC_CLAUSE_GANG` を渡さないこと．v2.0.0 以降） | `PRAGMA_OMP_PARALLEL_DO(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...)` | `!$omp target teams distribute simd __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)`（v2.0.0 以降） | `PRAGMA_OMP_PARALLEL_DO_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_LOOP(...)` | `!$omp target teams loop __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_DO(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO(...)` | `!$omp target teams distribute parallel do __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_DO(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD(...)` | `!$omp target teams distribute parallel do simd __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_DO_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_ATOMIC(...)` | `PRAGMA_OMP_ATOMIC(__VA_ARGS__)` | `PRAGMA_ACC_ATOMIC(__VA_ARGS__)` | `PRAGMA_OMP_ATOMIC(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TASKWAIT(...)` | `PRAGMA_OMP_TASKWAIT(__VA_ARGS__)` | `PRAGMA_ACC(wait)`: `acc wait`（指示節は捨てる．v2.0.0 以降） | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_DECLARE_TARGET(...)` | `!$omp declare target __VA_ARGS__` | `PRAGMA_ACC_ROUTINE(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_BEGIN_DECLARE_TARGET(...)` | `!$omp declare target __VA_ARGS__`（Fortran には `begin declare target` が無い（OpenMP 5.2 の 7.8.2 節）ので，`PRAGMA_OMP_DECLARE_TARGET(...)` と同じになる．`SOLOMON_DECLARE_OFFLOADED(...)` と同じく手続の仕様部に書く） | `PRAGMA_ACC_ROUTINE(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_END_DECLARE_TARGET` | （Fortran では何も出力しない） | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） |

    </details>

  * <details><summary>OpenMP</summary>

    | 入力 | 出力 |
    | ---- | ---- |
    | `PRAGMA_OMP_THREADPRIVATE(...)` | `!$omp threadprivate(__VA_ARGS__)` |
    | `PRAGMA_OMP_SCAN(...)` | `!$omp scan __VA_ARGS__` |
    | `PRAGMA_OMP_DECLARE_SIMD(...)` | `!$omp declare simd __VA_ARGS__` |
    | `PRAGMA_OMP_TILE(...)` | `!$omp tile __VA_ARGS__` |
    | `PRAGMA_OMP_UNROLL(...)` | `!$omp unroll __VA_ARGS__` |
    | `PRAGMA_OMP_PARALLEL(...)` | `!$omp parallel __VA_ARGS__` |
    | `PRAGMA_OMP_TEAMS(...)` | `!$omp teams __VA_ARGS__` |
    | `PRAGMA_OMP_SIMD(...)` | `!$omp simd __VA_ARGS__` |
    | `PRAGMA_OMP_MASKED(...)` | `!$omp masked __VA_ARGS__` |
    | `PRAGMA_OMP_SINGLE(...)` | `!$omp single __VA_ARGS__` |
    | `PRAGMA_OMP_WORKSHARE(...)` | `!$omp workshare __VA_ARGS__` <br> NVIDIA HPC SDK（nvfortran）では `OMP_CLAUSE_NOWAIT` を捨てる（nvfortran 26.9 は `nowait` を `workshare` で拒否する．nvfortran は `fortran.mk` と `spp.sh` が見分ける）．ほかのコンパイラでは `workshare nowait` を出す |
    | `PRAGMA_OMP_SCOPE(...)` | `!$omp scope __VA_ARGS__` <br> `firstprivate`・`allocate` は `scope` では OpenMP 5.2 の機能で，Clang 22・Intel 2026.1 は拒否する |
    | `PRAGMA_OMP_SECTIONS(...)` | `!$omp sections __VA_ARGS__` |
    | `PRAGMA_OMP_SECTION` | `!$omp section` |
    | `PRAGMA_OMP_DO(...)` | `!$omp do __VA_ARGS__` |
    | `PRAGMA_OMP_DISTRIBUTE(...)` | `!$omp distribute __VA_ARGS__` |
    | `PRAGMA_OMP_LOOP(...)` | `!$omp loop __VA_ARGS__` |
    | `PRAGMA_OMP_TASK(...)` | `!$omp task __VA_ARGS__` |
    | `PRAGMA_OMP_TASKLOOP(...)` | `!$omp taskloop __VA_ARGS__` |
    | `PRAGMA_OMP_TASKYIELD` | `!$omp taskyield` |
    | `PRAGMA_OMP_INTEROP(...)` | `!$omp interop __VA_ARGS__` |
    | `PRAGMA_OMP_CRITICAL(...)` | `!$omp critical __VA_ARGS__` <br> 名前は `OMP_PASS_LIST(name)` で渡す: `PRAGMA_OMP_CRITICAL(OMP_PASS_LIST(name), OMP_CLAUSE_HINT(hint))` → `critical (name) hint(hint)`（名前はどこに書いても先頭に置かれる） <br> `OMP_CLAUSE_HINT` に `omp_sync_hint_none` 以外を渡すときは名前が要る（OpenMP 5.2 §15.2）．名前が無いと GCC・Clang・Intel のコンパイラが拒否する <br> 名前つきの `critical` は `PRAGMA_OMP_END_CRITICAL_WITH_NAME(name)` で閉じる．`PRAGMA_OMP_END_CRITICAL` は名前の無いものを閉じる |
    | `PRAGMA_OMP_BARRIER` | `!$omp barrier` |
    | `PRAGMA_OMP_TASKGROUP(...)` | `!$omp taskgroup __VA_ARGS__` |
    | `PRAGMA_OMP_TASKWAIT(...)` | `!$omp taskwait __VA_ARGS__` |
    | `PRAGMA_OMP_FLUSH(...)` | 引数なしは `!$omp flush`，引数ありは後ろに括弧で並びが付く `(__VA_ARGS__)`（`__VA_ARGS__` は変数の並びだけで，memory-order 節は渡せない．Solomon の指示節マクロ（`OMP_CLAUSE_ACQ_REL` など）を先頭に書くと `Solomon: PRAGMA_OMP_FLUSH takes a list of variables; a memory-order clause cannot be passed` で止まり，2 番目以降に書くとその展開が並びに残ってコンパイラが拒否する．2 番目の引数は `/`（共通ブロック名）・`(` で始められない（この検査が字句を貼るので前処理が止まる）．そのような名前は先頭に書くか，`PRAGMA_OMP(flush (...))` で書く） |
    | `PRAGMA_OMP_DEPOBJ(...)` | `!$omp depobj __VA_ARGS__` <br> depend object を先頭に `OMP_PASS_LIST(obj)` で渡し，後ろに `OMP_CLAUSE_DEPEND(...)`・`OMP_CLAUSE_DESTROY(...)`・`OMP_CLAUSE_DEPOBJ_UPDATE(...)` のどれかを書く: `omp depobj (obj) depend(...)`．裸の名前は定義の無い名前として止まる．`OMP_CLAUSE_UPDATE` は `atomic` の節で，ここに渡すとビルドが止まる（gfortran と ifx は引数の無い `update` を拒否する．nvfortran 26.9 は `depobj` を読み飛ばす） |
    | `PRAGMA_OMP_ATOMIC(...)` | `!$omp atomic __VA_ARGS__` |
    | `PRAGMA_OMP_ATOMIC_UPDATE` | `!$omp atomic update` |
    | `PRAGMA_OMP_ATOMIC_READ` | `!$omp atomic read` |
    | `PRAGMA_OMP_ATOMIC_WRITE` | `!$omp atomic write` |
    | `PRAGMA_OMP_ATOMIC_CAPTURE` | `!$omp atomic capture` |
    | `PRAGMA_OMP_ORDERED(...)` | `!$omp ordered __VA_ARGS__` |
    | `PRAGMA_OMP_DO_SIMD(...)` | `!$omp do simd __VA_ARGS__` |
    | `PRAGMA_OMP_DISTRIBUTE_SIMD(...)` | `!$omp distribute simd __VA_ARGS__` |
    | `PRAGMA_OMP_DISTRIBUTE_PARALLEL_DO(...)` | `!$omp distribute parallel do __VA_ARGS__` |
    | `PRAGMA_OMP_DISTRIBUTE_PARALLEL_DO_SIMD(...)` | `!$omp distribute parallel do simd __VA_ARGS__` |
    | `PRAGMA_OMP_TASKLOOP_SIMD(...)` | `!$omp taskloop simd __VA_ARGS__` |
    | `PRAGMA_OMP_PARALLEL_DO(...)` | `!$omp parallel do __VA_ARGS__` |
    | `PRAGMA_OMP_PARALLEL_LOOP(...)` | `!$omp parallel loop __VA_ARGS__` |
    | `PRAGMA_OMP_PARALLEL_SECTIONS(...)` | `!$omp parallel sections __VA_ARGS__` |
    | `PRAGMA_OMP_PARALLEL_DO_SIMD(...)` | `!$omp parallel do simd __VA_ARGS__` |
    | `PRAGMA_OMP_MASKED_TASKLOOP(...)` | `!$omp masked taskloop __VA_ARGS__` |
    | `PRAGMA_OMP_MASKED_TASKLOOP_SIMD(...)` | `!$omp masked taskloop simd __VA_ARGS__` |
    | `PRAGMA_OMP_PARALLEL_MASKED_TASKLOOP(...)` | `!$omp parallel masked taskloop __VA_ARGS__` |
    | `PRAGMA_OMP_PARALLEL_MASKED_TASKLOOP_SIMD(...)` | `!$omp parallel masked taskloop simd __VA_ARGS__` |
    | `PRAGMA_OMP_TEAMS_DISTRIBUTE(...)` | `!$omp teams distribute __VA_ARGS__` |
    | `PRAGMA_OMP_TEAMS_DISTRIBUTE_SIMD(...)` | `!$omp teams distribute simd __VA_ARGS__` |
    | `PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_DO(...)` | `!$omp teams distribute parallel do __VA_ARGS__` |
    | `PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD(...)` | `!$omp teams distribute parallel do simd __VA_ARGS__` |
    | `PRAGMA_OMP_TEAMS_LOOP(...)` | `!$omp teams loop __VA_ARGS__` |

    </details>

* <details><summary>メモリ操作，データ転送</summary>

  | 入力 | 出力 | バックエンドとして用いる指示文 |
  | ---- | ---- | ---- |
  | **`SOLOMON_ALLOCATE_ON_DEVICE(...)`** <br> `PRAGMA_ACC_ENTER_DATA_CREATE(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA_MAP_ALLOC(...)` | <br> `!$acc enter data create(__VA_ARGS__)` <br> `!$omp target enter data map(alloc: __VA_ARGS__)` | <br> OpenACC <br> OpenMP <br> `SOLOMON_MALLOC_ON_DEVICE(...)`（C/C++ 版に掲載）も同じ意味で使用可能 |
  | **`SOLOMON_DEALLOCATE_ON_DEVICE(...)`** <br> `PRAGMA_ACC_EXIT_DATA_DELETE(...)` | <br> `!$acc exit data delete(__VA_ARGS__)` <br> `!$omp target exit data map(release: __VA_ARGS__)` | <br> OpenACC <br> OpenMP <br> デバイス上のデータの参照数を 1 減らす（ほかの `SOLOMON_ALLOCATE_ON_DEVICE(...)` が残っている間はデータが残る）．`SOLOMON_FREE_FROM_DEVICE(...)`（C/C++ 版に掲載）も同じ意味で使用可能 |
  | `PRAGMA_OMP_TARGET_EXIT_DATA_MAP_DELETE(...)` | `!$acc exit data delete(__VA_ARGS__) finalize` <br> `!$omp target exit data map(delete: __VA_ARGS__)` | OpenACC <br> OpenMP <br> 参照数によらずデバイス上のデータを消す |
  | **`SOLOMON_MEMCPY_D2H(...)`** <br> `PRAGMA_ACC_UPDATE_HOST(...)` <br> `PRAGMA_OMP_TARGET_UPDATE_FROM(...)` | <br> `!$acc update host(__VA_ARGS__)` <br> `!$omp target update from(__VA_ARGS__)` <br> （async や device_type などの指示節は渡せない．引数は括弧の中に入る．指示節は PRAGMA_ACC_UPDATE に渡す） | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_MEMCPY_H2D(...)`** <br> `PRAGMA_ACC_UPDATE_DEVICE(...)` <br> `PRAGMA_OMP_TARGET_UPDATE_TO(...)` | <br> `!$acc update device(__VA_ARGS__)` <br> `!$omp target update to(__VA_ARGS__)` <br> （async や device_type などの指示節は渡せない．引数は括弧の中に入る．指示節は PRAGMA_ACC_UPDATE に渡す） | <br> OpenACC <br> OpenMP |
  | `PRAGMA_ACC_ENTER_DATA(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA(...)` | `!$acc enter data __VA_ARGS__` <br> `!$omp target enter data __VA_ARGS__` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_ENTER_DATA_COPYIN(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA_MAP_TO(...)` | `!$acc enter data copyin(__VA_ARGS__)` <br> `!$omp target enter data map(to: __VA_ARGS__)` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_EXIT_DATA(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA(...)` | `!$acc exit data __VA_ARGS__` <br> `!$omp target exit data __VA_ARGS__` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_EXIT_DATA_COPYOUT(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA_MAP_FROM(...)` | `!$acc exit data copyout(__VA_ARGS__)` <br> `!$omp target exit data map(from: __VA_ARGS__)` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_UPDATE(...)` <br> `PRAGMA_OMP_TARGET_UPDATE(...)` | `!$acc update __VA_ARGS__` <br> `!$omp target update __VA_ARGS__` | OpenACC <br> OpenMP <br> データの更新には `SOLOMON_MEMCPY_H2D(list)` / `SOLOMON_MEMCPY_D2H(list)` を第一に推奨します．指示節が要るときは `PRAGMA_OMP_TARGET_UPDATE(OMP_TARGET_CLAUSE_TO(list))` / `PRAGMA_OMP_TARGET_UPDATE(OMP_TARGET_CLAUSE_FROM(list))` を使います．これらを `OMP_TARGET_CLAUSE_MAP(...)` で包まないでください（update 指示文に `map` 節は無く，包んだ節は捨てられます） |
  | **`SOLOMON_DATA_ACCESS_BY_DEVICE(...)`** <br> `PRAGMA_ACC_DATA(...)` <br> `PRAGMA_OMP_TARGET_DATA(...)` | <br> `!$acc data __VA_ARGS__` <br> `!$omp target data __VA_ARGS__` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_DATA_ACCESS_BY_HOST(...)`** <br> `PRAGMA_ACC_HOST_DATA(...)` <br> `PRAGMA_OMP_TARGET_DATA(...)` | <br> `!$acc host_data __VA_ARGS__` <br> `!$omp target data __VA_ARGS__` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_USE_DEVICE_DATA_FROM_HOST(...)`** <br> `PRAGMA_ACC_HOST_DATA_USE_DEVICE(...)` <br> `PRAGMA_OMP_TARGET_DATA_USE_DEVICE_ADDR(...)` | <br> `!$acc host_data use_device(__VA_ARGS__)` <br> `!$omp target data use_device_addr(__VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`SOLOMON_DECLARE_DATA_ON_DEVICE(...)`** <br> `PRAGMA_ACC_DATA_PRESENT(...)` | <br> `!$acc data present(__VA_ARGS__)` <br> `!$omp target data map(alloc: __VA_ARGS__) if(.false.)`（`PRAGMA_ACC_END_DATA` で閉じる．OpenMP target 使用時の領域は何もしない（`if(.false.)` でホストが対象のデバイスになる）．end target data の指示文と対にするためだけにある） | <br> OpenACC <br> OpenMP |

  </details>

  * <details><summary>OpenACC</summary>

    | 入力 | 出力 | OpenMP target 使用時の出力 |
    | ---- | ---- | ---- |
    | `PRAGMA_ACC_DATA(...)` | `!$acc data __VA_ARGS__` | `PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_ENTER_DATA(...)` | `!$acc enter data __VA_ARGS__` | N/A（`map` になる指示節が無いとき）．それ以外は `PRAGMA_OMP_TARGET_ENTER_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_EXIT_DATA(...)` | `!$acc exit data __VA_ARGS__` | N/A（`map` になる指示節が無いとき）．それ以外は `PRAGMA_OMP_TARGET_EXIT_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_HOST_DATA(...)` | `!$acc host_data __VA_ARGS__` | `PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_UPDATE(...)` | `!$acc update __VA_ARGS__` | N/A（`to` / `from` になる指示節が無いとき）．それ以外は `PRAGMA_OMP_TARGET_UPDATE(__VA_ARGS__)` |

    </details>

  * <details><summary>OpenMP target</summary>

    | 入力 | 出力 | OpenACC 使用時の出力 | 縮退モード（演算加速器を用いないCPU実行）での出力 |
    | ---- | ---- | ---- | ---- |
    | `PRAGMA_OMP_TARGET_DATA(...)` | `!$omp target data __VA_ARGS__` | `!$acc data SOLOMON_ERROR_NO_OPENACC_COUNTERPART_IN_DATA_CLAUSES`（OpenACC の対応物を持つ指示節が無いとき．止まる）．それ以外は `PRAGMA_ACC_DATA(...)` または `PRAGMA_ACC_HOST_DATA(...)`（`use_device_*` だけのとき） | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_TARGET_ENTER_DATA(...)` | `!$omp target enter data __VA_ARGS__` | `!$acc enter data SOLOMON_ERROR_NO_OPENACC_COUNTERPART_IN_DATA_CLAUSES`（OpenACC の対応物を持つ指示節が無いとき．止まる）．それ以外は `PRAGMA_ACC_ENTER_DATA(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_TARGET_EXIT_DATA(...)` | `!$omp target exit data __VA_ARGS__` | `!$acc exit data SOLOMON_ERROR_NO_OPENACC_COUNTERPART_IN_DATA_CLAUSES`（OpenACC の対応物を持つ指示節が無いとき．止まる）．それ以外は `PRAGMA_ACC_EXIT_DATA(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_TARGET_UPDATE(...)` | `!$omp target update __VA_ARGS__` | `!$acc update SOLOMON_ERROR_NO_OPENACC_COUNTERPART_IN_DATA_CLAUSES`（OpenACC の対応物を持つ指示節が無いとき．止まる）．それ以外は `PRAGMA_ACC_UPDATE(__VA_ARGS__)` | N/A（縮退モードでは無視される） |

    </details>

### 使用可能な指示節・指示句

* <details><summary>簡易記法</summary>

  | 入力 | 出力 | バックエンドとして用いる指示文 | 備考 |
  | ---- | ---- | ---- | ---- |
  | **`SOLOMON_CLAUSE_INDEPENDENT`** <br> `ACC_CLAUSE_INDEPENDENT` <br> `OMP_TARGET_CLAUSE_SIMD` | <br> `independent` <br> `simd` | <br> OpenACC <br> OpenMP | `simd` は `teams loop` の形では付かない（`loop` が `simd` を取らないため） |
  | **`SOLOMON_CLAUSE_SIMDLEN(length)`** <br> `OMP_CLAUSE_SIMDLEN(length)` | <br> `simdlen(length)` | <br> OpenMP (only) | `SOLOMON_CLAUSE_INDEPENDENT` で `simd` が付く所にだけ出し（例: `SOLOMON_OFFLOAD(SOLOMON_CLAUSE_INDEPENDENT, SOLOMON_CLAUSE_SIMDLEN(8))` は `!$omp target teams distribute parallel do simd simdlen(8)`，縮退モードでは `!$omp parallel do simd simdlen(8)`），`simd` が付かない所（OpenACC，`teams loop` の形，シリアルモード）では無視する．名前に `simd` を含むループの指示文（`PRAGMA_OMP_PARALLEL_DO_SIMD(...)` など）も受け取るが，`PRAGMA_OMP_DECLARE_SIMD(...)` では黙って捨てる（そこでは `OMP_CLAUSE_SIMDLEN(length)` などの OpenMP の記法で書く）．OpenACC の `vector_length` ではない（スレッドブロックあたりのスレッド数は `SOLOMON_CLAUSE_NUM_THREADS(n)` で指定する）．`SOLOMON_CLAUSE_INDEPENDENT` も渡すこと．渡さないと `simd` の無い指示文に節が残り，コンパイラが拒否する．nvfortran 26.9 は `simdlen`・`safelen`・`aligned` を未実装の警告つきで受理し，無視する |
  | **`SOLOMON_CLAUSE_SAFELEN(length)`** <br> `OMP_CLAUSE_SAFELEN(length)` | <br> `safelen(length)` | <br> OpenMP (only) | `SOLOMON_CLAUSE_SIMDLEN(length)` と同じ．反復は `simd` の前にスレッドやチームに分けられるので，反復の間に依存があるループは `safelen` を付けてもこの形では安全にならない．そのような内側のループは `PRAGMA_OMP_SIMD(OMP_CLAUSE_SAFELEN(n))` で書く |
  | **`SOLOMON_CLAUSE_ALIGNED(...)`** <br> `OMP_CLAUSE_ALIGNED(...)` | <br> `aligned(__VA_ARGS__)` | <br> OpenMP (only) | `SOLOMON_CLAUSE_SIMDLEN(length)` と同じ |
  | **`SOLOMON_CLAUSE_NONTEMPORAL(...)`** <br> `OMP_CLAUSE_NONTEMPORAL(...)` | <br> `nontemporal(__VA_ARGS__)` | <br> OpenMP (only) | `SOLOMON_CLAUSE_SIMDLEN(length)` と同じ．nvfortran は `nontemporal` を拒否するので，わざと捨てる（ヒントの節なので結果は変わらない） |
  | **`SOLOMON_CLAUSE_IF_SIMD(condition)`** <br> `OMP_CLAUSE_IF(simd: condition)` | <br> `if(simd: condition)` | <br> OpenMP (only) | `SOLOMON_CLAUSE_SIMDLEN(length)` と同じ．条件が偽のときは SIMD を使わずに 1 反復ずつ実行する．ifx 2026.1 は `!$omp parallel do simd`（縮退モード）では拒否し，`!$omp target teams distribute parallel do simd` では内部エラーで止まる |
  | **`SOLOMON_CLAUSE_SEQUENTIAL`** <br> `ACC_CLAUSE_SEQ` | <br> `seq` | <br> OpenACC (only) | |
  | **`SOLOMON_CLAUSE_NUM_THREADS(n)`** <br> `ACC_CLAUSE_VECTOR_LENGTH(n)` <br> `OMP_TARGET_CLAUSE_THREAD_LIMIT(n)` | <br> `vector_length(n)` <br> `thread_limit(n)` | <br> OpenACC <br> OpenMP | 縮退モードでは，`SOLOMON_CLAUSE_NUM_THREADS(n)` と `ACC_CLAUSE_VECTOR_LENGTH(n)` は `num_threads(n)`，つまり CPU のスレッド数になり（GPU 向けのブロックの大きさはコア数を超えることがある），`thread_limit(n)`（`PRAGMA_OMP_TARGET_TEAMS(...)` のとき）になる．`OMP_TARGET_CLAUSE_THREAD_LIMIT(n)` は `thread_limit(n)` のままで，そこでは `PRAGMA_OMP_TARGET_TEAMS(...)` だけが受け取る．同じ指示文に `OMP_TARGET_CLAUSE_NUM_THREADS(...)` を併記しない |
  | **`SOLOMON_CLAUSE_NUM_BLOCKS(n)`** <br> `ACC_CLAUSE_NUM_GANGS(n)` <br> `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` | <br> `num_gangs(n)` <br> `num_teams(n)` | <br> OpenACC <br> OpenMP | v2.0.0 で `ACC_CLAUSE_NUM_WORKERS(n)` から `ACC_CLAUSE_NUM_GANGS(n)` へと変更 |
  | ~~`SOLOMON_CLAUSE_NUM_GRIDS(n)`~~ <br> ~~`ACC_CLAUSE_NUM_GANGS(n)`~~ <br> ~~`OMP_TARGET_CLAUSE_NUM_TEAMS(n)`~~ | <br> ~~`num_gangs(n)`~~ <br> ~~`num_teams(n)`~~ | <br> OpenACC <br> OpenMP | v2.0.0 で非推奨．以降は `SOLOMON_CLAUSE_NUM_BLOCKS(n)` を使用 |
  | **`SOLOMON_CLAUSE_THREAD`** <br> `ACC_CLAUSE_VECTOR` | <br> `vector` | <br> OpenACC (only) | |
  | **`SOLOMON_CLAUSE_BLOCK`** <br> `ACC_CLAUSE_GANG` | <br> `gang` | <br> OpenACC (only) | v2.0.0 で `ACC_CLAUSE_WORKER` から `ACC_CLAUSE_GANG` へと変更 |
  | ~~`SOLOMON_CLAUSE_GRID`~~ <br> ~~`ACC_CLAUSE_GANG`~~ | <br> ~~`gang`~~ | <br> OpenACC (only) | v2.0.0 で非推奨．以降は `SOLOMON_CLAUSE_BLOCK` を使用 |
  | **`SOLOMON_CLAUSE_COLLAPSE(n)`** <br> `ACC_CLAUSE_COLLAPSE(n)` <br> `OMP_TARGET_CLAUSE_COLLAPSE(n)` | <br> `collapse(n)` <br> `collapse(n)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_ASYNC(...)`** <br> `ACC_CLAUSE_ASYNC(...)` <br> `OMP_TARGET_CLAUSE_NOWAIT` | <br> 引数なしは `async`，引数ありは後ろに括弧で付く `(__VA_ARGS__)` <br> `nowait` | <br> OpenACC <br> OpenMP | 引数なしでは OpenACC で括弧の無い `async` になる（`async(acc_async_noval)` と同じ） <br> ROCm 7.2.4 の amdflang（flang 22）は `nowait` を `target teams loop` と `target teams distribute parallel do` に付けると拒否する（`not yet implemented`）．ROCm 10.0 は受理する |
  | **`SOLOMON_CLAUSE_ASYNC_QUEUE(id)`** <br> `ACC_CLAUSE_ASYNC(id)` | <br> `async(id)` | <br> OpenACC (only) | |
  | **`SOLOMON_CLAUSE_WAIT_QUEUE(...)`** <br> `ACC_CLAUSE_WAIT(...)` | <br> 引数なしは `wait`，引数ありは後ろに括弧で並びが付く `(__VA_ARGS__)` | <br> OpenACC (only) | |
  | **`SOLOMON_CLAUSE_REDUCTION(...)`** <br> `ACC_CLAUSE_REDUCTION(...)` <br> `OMP_TARGET_CLAUSE_REDUCTION(...)` | <br> `reduction(__VA_ARGS__)` <br> `reduction(__VA_ARGS__)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_IF(condition)`** <br> `ACC_CLAUSE_IF(condition)` <br> `OMP_TARGET_CLAUSE_IF(condition)` | <br> `if(condition)` <br> `if(condition)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_PRIVATE(...)`** <br> `ACC_CLAUSE_PRIVATE(...)` <br> `OMP_TARGET_CLAUSE_PRIVATE(...)` | <br> `private(__VA_ARGS__)` <br> `private(__VA_ARGS__)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_FIRSTPRIVATE(...)`** <br> `ACC_CLAUSE_FIRSTPRIVATE(...)` <br> `OMP_TARGET_CLAUSE_FIRSTPRIVATE(...)` | <br> `firstprivate(__VA_ARGS__)` <br> `firstprivate(__VA_ARGS__)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_DEVICE_PTR(...)`** <br> `ACC_CLAUSE_DEVICEPTR(...)` <br> `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(...)` | <br> `deviceptr(__VA_ARGS__)` <br> `is_device_ptr(__VA_ARGS__)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_COPY_BEFORE_AND_AFTER_EXEC(...)`** <br> `ACC_CLAUSE_COPY(...)` <br> `OMP_TARGET_CLAUSE_MAP_TOFROM(...)` | <br> `copy(__VA_ARGS__)` <br> `map(tofrom: __VA_ARGS__)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_COPY_H2D_BEFORE_EXEC(...)`** <br> `ACC_CLAUSE_COPYIN(...)` <br> `OMP_TARGET_CLAUSE_MAP_TO(...)` | <br> `copyin(__VA_ARGS__)` <br> `map(to: __VA_ARGS__)` | <br> OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_COPY_D2H_AFTER_EXEC(...)`** <br> `ACC_CLAUSE_COPYOUT(...)` <br> `OMP_TARGET_CLAUSE_MAP_FROM(...)` | <br> `copyout(__VA_ARGS__)` <br> `map(from: __VA_ARGS__)` | <br> OpenACC <br> OpenMP | |

  </details>

* <details><summary>OpenACC</summary>

  | 入力 | 出力 | OpenMP target 使用時の出力 | 備考 |
  | ---- | ---- | ---- | ---- |
  | `ACC_CLAUSE_IF(condition)` | `if(condition)` | `OMP_TARGET_CLAUSE_IF(condition)` | |
  | `ACC_CLAUSE_SELF(...)` | 引数なしは `self`，引数ありは後ろに括弧で付く `(__VA_ARGS__)` | 計算構文では引数なしは `if (target : .false.)`，引数ありは `if(target: .not.(condition))`（OpenMP target 使用時は同じ指示文に `if` 節を書けない）．`OMP_TARGET_CLAUSE_FROM(__VA_ARGS__)`（`PRAGMA_ACC_UPDATE`．引数なしの `ACC_CLAUSE_SELF()` は捨てる） | 引数なしでは OpenACC で括弧の無い `self` になる．NVIDIA HPC SDK 26.9 と GCC 13 は計算構文の `self` を受け付けない（OpenACC 2.7） <br> 引数なしの `ACC_CLAUSE_SELF()` は `PRAGMA_ACC_UPDATE(...)` の必須の節に数えない: ほかの指示節が無いと，OpenACC では必須の節の Solomon のエラーで止まり，OpenMP target では出力されない（v2.0.0 以降） <br> 引数 2 個以上は `PRAGMA_ACC_UPDATE(...)` の並び．計算構文に渡すと OpenMP target 使用時に Solomon のエラーで止まる（計算構文の `self` は条件 1 つ） |
  | `ACC_CLAUSE_DEFAULT(mode)` | `default(mode)` | N/A（OpenMP target 使用時には無視される） | data 構文（`PRAGMA_ACC_DATA(...)`）の `default` は gfortran 13 と nvfortran 26.9 が拒否する |
  | `ACC_CLAUSE_DEFAULT_NONE` | `default(none)` | `OMP_TARGET_CLAUSE_DEFAULTMAP_NONE` | data 構文の `default` は gfortran 13 と nvfortran 26.9 が拒否する |
  | `ACC_CLAUSE_DEFAULT_PRESENT` | `default(present)` | `OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT` | data 構文の `default` は gfortran 13 と nvfortran 26.9 が拒否する <br> OpenMP target 使用時，gfortran 13 と nvfortran 26.9（`-mp=gpu`）は `defaultmap(present)` を拒否する（`OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT` の行を参照） |
  | `ACC_CLAUSE_DEVICE_TYPE(...)` | `device_type(__VA_ARGS__)` | N/A（OpenMP のバックエンドでは無視される） | data 構文と `update` にも付く（NVIDIA HPC SDK 26.9 の Fortran は拒否する） <br> 1 つの並びを 2 つの指示文に渡すマクロ（`SOLOMON_OFFLOAD`，`SOLOMON_OFFLOAD_OUTER_LOOP`，`PRAGMA_ACC_KERNELS_LOOP` / `PRAGMA_ACC_PARALLEL_LOOP`，OpenACC 使用時の `PRAGMA_OMP_TARGET_*` の計算の指示文）では，並びが計算構文と `loop` 構文の両方に渡るので，`device_type` の後ろの指示節は両方でそのデバイス用になる（`kernels loop` と同じ） <br> `device_type` の後ろには，その指示節を受け取る指示文で `device_type` の後ろに書ける指示節だけを書く（OpenACC 3.3）: 計算構文は `async`，`wait`，`num_gangs`，`num_workers`，`vector_length`，`loop` は `collapse`，`gang`，`worker`，`vector`，`seq`，`independent`，`auto`，`tile`，data 構文と `update` は `async`，`wait`，`routine` は `gang`，`worker`，`vector`，`seq`，`bind`．それ以外の指示節（`private`，`reduction`，`copyin` など）はどの `device_type` よりも前に書く．2 つの指示文に渡すマクロは各指示節を受け取れる指示文にだけ渡すので，`device_type` の後ろには両方の一覧の指示節を書ける: `SOLOMON_OFFLOAD(SOLOMON_CLAUSE_COLLAPSE(2), ACC_CLAUSE_DEVICE_TYPE(nvidia), ACC_CLAUSE_NUM_GANGS(4))` は `kernels device_type(nvidia) num_gangs(4)` と `loop collapse(2) device_type(nvidia)` になる <br> OpenMP のバックエンド（OpenMP target と縮退モード）では `device_type` は捨てられ，その後ろの指示節は全デバイス向けになる（`PRAGMA_ACC_ROUTINE(...)` / `SOLOMON_DECLARE_OFFLOADED(...)` でも） |
  | `ACC_CLAUSE_ASYNC(...)` | 引数なしは `async`，引数ありは後ろに括弧で付く `(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_NOWAIT` | data 構文（`PRAGMA_ACC_DATA`，`SOLOMON_DATA_ACCESS_BY_DEVICE`）にも付く（OpenACC 3.2 以降．GCC 13 は拒否する）．OpenMP target の `target data` には付かない（`nowait` / `depend` が無い）ので，待ちが要るときは data 構文の前に `SOLOMON_SYNCHRONIZE()` を置く <br> data 構文の `async` は data 構文自身の転送（出口の copyout を含む）を非同期にする．data 構文の後でホストが結果を読む前に待ちを置く <br> 引数なしでは OpenACC で括弧の無い `async` になる（`async(acc_async_noval)` と同じ） <br> gcc / g++ の厳密モードで C23 / C++20 より前の版では，[Solomon を用いたコードの開発方法](#solomon-を用いたコードの開発方法) の空の判定の注記を参照 |
  | `ACC_CLAUSE_WAIT(...)` | 引数なしは `wait`，引数ありは後ろに括弧で並びが付く `(__VA_ARGS__)` | N/A（引数なし．指示節は捨てられる）．引数ありは `OMP_TARGET_CLAUSE_DEPEND_IN(__VA_ARGS__)` | 引数なしは OpenACC で `wait`（全てのキューを待つ）．OpenMP target では無視されるので，待ちが要るときは前に `SOLOMON_SYNCHRONIZE()` を置く <br> OpenMP target では引数が `depend(in : ...)` の対象になるので変数を渡す．キュー番号（整数）は OpenACC でしか通らない．バックエンドによらない非同期実行には `SOLOMON_CLAUSE_ASYNC(...)` と `SOLOMON_SYNCHRONIZE(...)` などを推奨する．バックエンドによらないキューの待ちには `SOLOMON_CLAUSE_WAIT_QUEUE(...)` を使う <br> 同じ指示文の `wait` 節は `device_type` の区切りごとにまとめる．引数なしの `wait` があれば今のデバイスの待ちはその `wait` 1 つに，無ければ番号の並びを 1 つの `wait(..., ...)` にする．`devnum:` / `queues:`（大文字・小文字を問わない）の待ちは残す．空の `wait()` は `wait` と同じ．`wait` 節を 2 つ以上書くときは，引数の先頭を名前・整数・括弧にする．まとめた引数は 32 個までで，超えると指示文に `SOLOMON_ERROR_TOO_MANY_MERGED_WAIT_ARGUMENTS` が残る（[Solomon の拡張方法](#solomon-の拡張方法コードジェネレータを用いた更新方法) を参照）．1 つの `wait` 節に書いた引数はまとめないので，この上限を受けない <br> 引数に空の要素があると（余分なカンマ．`ACC_CLAUSE_WAIT(1, )`・`ACC_CLAUSE_WAIT(, 2)` など），OpenACC と OpenMP target 使用時に指示文に `SOLOMON_ERROR_EMPTY_ARGUMENT_IN_ACC_CLAUSE_WAIT` が残り，コンパイラがそこで止まる．余分なカンマを消し，並び無しで待つなら `ACC_CLAUSE_WAIT()` と書く（v2.0.0 以降．33 個の引数まで調べる） <br> data 構文（`PRAGMA_ACC_DATA`，`SOLOMON_DATA_ACCESS_BY_DEVICE`）にも付く（OpenACC 3.2 以降．GCC 13 は拒否する）．OpenMP target の `target data` には付かない（`nowait` / `depend` が無い）ので，待ちが要るときは data 構文の前に `SOLOMON_SYNCHRONIZE()` を置く |
  | `ACC_CLAUSE_FINALIZE` | `finalize` | N/A（OpenMP target 使用時には無視される） | OpenMP のバックエンドでは，`ACC_CLAUSE_DELETE(...)` に `ACC_CLAUSE_FINALIZE` を付けても参照数を 1 減らすだけになる |
  | `ACC_CLAUSE_NUM_GANGS(n)` | `num_gangs(n)` | `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` | v2.0.0 で `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` に変換されるよう変更 |
  | `ACC_CLAUSE_NUM_WORKERS(n)` | `num_workers(n)` | N/A（OpenMP target 使用時には無視される） | v2.0.0 で OpenMP target 使用時に無視されるよう変更 |
  | `ACC_CLAUSE_VECTOR_LENGTH(n)` | `vector_length(n)` | `OMP_TARGET_CLAUSE_THREAD_LIMIT(n)` | |
  | `ACC_CLAUSE_REDUCTION(...)` | `reduction(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_REDUCTION(__VA_ARGS__)` | |
  | `ACC_CLAUSE_PRIVATE(...)` | `private(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_PRIVATE(__VA_ARGS__)` | |
  | `ACC_CLAUSE_FIRSTPRIVATE(...)` | `firstprivate(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | |
  | `ACC_CLAUSE_COPY(...)` | `copy(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_TOFROM(__VA_ARGS__)` | |
  | `ACC_CLAUSE_COPYIN(...)` | `copyin(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_TO(__VA_ARGS__)` | `readonly:`（copyin）と `zero:`（copyout・create）の修飾子は OpenACC だけ．OpenMP target 使用時には `map` 節に残り，コンパイラが拒否する |
  | `ACC_CLAUSE_COPYOUT(...)` | `copyout(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_FROM(__VA_ARGS__)` | `zero:` の修飾子は OpenACC だけ（`ACC_CLAUSE_COPYIN(...)` を参照） |
  | `ACC_CLAUSE_CREATE(...)` | `create(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_ALLOC(__VA_ARGS__)` | `zero:` の修飾子は OpenACC だけ（`ACC_CLAUSE_COPYIN(...)` を参照） |
  | `ACC_CLAUSE_NO_CREATE(...)` | `no_create(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_DELETE(...)` | `delete(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_RELEASE(__VA_ARGS__)` | OpenACC の `delete` と同じく参照数を 1 減らす（v2.0.0 以降）．`ACC_CLAUSE_FINALIZE` は OpenMP のバックエンドでは無視される |
  | `ACC_CLAUSE_PRESENT(...)` | `present(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_DEVICEPTR(...)` | `deviceptr(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(__VA_ARGS__)` | |
  | `ACC_CLAUSE_ATTACH(...)` | `attach(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_DETACH(...)` | `detach(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_USE_DEVICE(...)` | `use_device(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_USE_DEVICE_ADDR(__VA_ARGS__)` | |
  | `ACC_CLAUSE_IF_PRESENT` | `if_present` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_COLLAPSE(n)` | `collapse(n)` | `OMP_TARGET_CLAUSE_COLLAPSE(n)` | |
  | `ACC_CLAUSE_SEQ` | `seq` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_AUTO` | `auto` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_INDEPENDENT` | `independent` | `OMP_TARGET_CLAUSE_SIMD` | |
  | `ACC_CLAUSE_TILE(...)` | `tile(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_GANG` | `gang` | N/A（OpenMP target 使用時には無視される） | `gang(n)` という記法は提供しない（`ACC_CLAUSE_NUM_GANGS(n)`が使用可能） |
  | `ACC_CLAUSE_WORKER` | `worker` | N/A（OpenMP target 使用時には無視される） | `worker(n)` という記法は提供しない（`ACC_CLAUSE_NUM_WORKERS(n)`が使用可能） |
  | `ACC_CLAUSE_VECTOR` | `vector` | N/A（OpenMP target 使用時には無視される） | `vector(n)` という記法は提供しない（`ACC_CLAUSE_VECTOR_LENGTH(n)`が使用可能） |
  | `ACC_CLAUSE_READ` | `read` | `OMP_TARGET_CLAUSE_READ` | |
  | `ACC_CLAUSE_WRITE` | `write` | `OMP_TARGET_CLAUSE_WRITE` | |
  | `ACC_CLAUSE_UPDATE` | `update` | `OMP_TARGET_CLAUSE_UPDATE` | |
  | `ACC_CLAUSE_CAPTURE` | `capture` | `OMP_TARGET_CLAUSE_CAPTURE` | |
  | `ACC_CLAUSE_HOST(...)` | `host(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_FROM(__VA_ARGS__)` | |
  | `ACC_CLAUSE_DEVICE(...)` | `device(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_TO(__VA_ARGS__)` | |
  | `ACC_PASS_LIST(...)` | `(__VA_ARGS__)` | `OMP_TARGET_PASS_LIST(__VA_ARGS__)` | |
  | `ACC_CLAUSE_BIND(arg)` | `bind(arg)` | N/A（OpenMP target 使用時には無視される） | gfortran 13 は `bind` を `routine` で拒否する（nvfortran 26.9 は受理する） |
  | `ACC_CLAUSE_NOHOST` | `nohost` | N/A（OpenMP のバックエンドでは無視される） | OpenMP target では手続がホスト向けにも作られる（v2.0.0 以降） |
  | `ACC_CLAUSE_DEVICE_RESIDENT(...)` | `device_resident(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_LINK(...)` | `link(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） | |

  </details>

* <details><summary>OpenMP target</summary>

  | 入力 | 出力 | OpenACC 使用時の出力 | 縮退モード（演算加速器を用いないCPU実行）での出力 | 備考 |
  | ---- | ---- | ---- | ---- | ---- |
  | `OMP_TARGET_CLAUSE_ALIGNED(...)` | `OMP_CLAUSE_ALIGNED(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ALIGNED(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_SIMDLEN(length)` | `OMP_CLAUSE_SIMDLEN(length)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SIMDLEN(length)` | |
  | `OMP_TARGET_CLAUSE_DEVICE_TYPE(nohost)` | `device_type(nohost)` | `ACC_CLAUSE_NOHOST`（`nohost` のとき）．`host`・`any` は無視（v2.0.0 以降） | N/A（縮退モードでは無視される） | 引数は `host`・`nohost`・`any` のどれか．ほかの語は OpenACC 使用時に指示文に `SOLOMON_ERROR_UNKNOWN_DEVICE_TYPE <word>` を残す（C/C++ は `Solomon: the argument of OMP_TARGET_CLAUSE_DEVICE_TYPE must be host / nohost / any: <word>`） |
  | `OMP_TARGET_CLAUSE_ENTER(...)` | `enter(__VA_ARGS__)` | N/A（OpenACC 使用時には指示節が捨てられ，指示文は名前の無い `!$acc routine` として残る．手続の中ではその手続に付き，モジュールの宣言部ではコンパイルエラーになる．変数には `SOLOMON_DECLARE_ON_DEVICE(...)` / `SOLOMON_DECLARE_ON_DEVICE_LINKED(...)`，関数には `SOLOMON_DECLARE_OFFLOADED(SOLOMON_CLAUSE_TARGETS(...))` を使う） | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_INDIRECT(...)` | 引数なしは `indirect`，引数ありは後ろに括弧で付く `(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） | GCC 13 と NVIDIA HPC SDK 26.9 は `indirect` を受けない |
  | `OMP_TARGET_CLAUSE_LINK(...)` | `link(__VA_ARGS__)` | N/A（OpenACC 使用時には指示節が捨てられ，指示文は名前の無い `!$acc routine` として残る．手続の中ではその手続に付き，モジュールの宣言部ではコンパイルエラーになる．変数には `SOLOMON_DECLARE_ON_DEVICE(...)` / `SOLOMON_DECLARE_ON_DEVICE_LINKED(...)`，関数には `SOLOMON_DECLARE_OFFLOADED(SOLOMON_CLAUSE_TARGETS(...))` を使う） | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_COPYIN(...)` | `OMP_CLAUSE_COPYIN(__VA_ARGS__)` | N/A（OpenACC 使用時には捨てる．OpenACC に `threadprivate` は無く，`copyin` はデバイスへの転送） | `OMP_CLAUSE_COPYIN(__VA_ARGS__)` | `PRAGMA_OMP_TARGET_*` の複合構文では捨てられる（OpenMP 5.2 の 17.4 節）．縮退モードでは残る．ホストからデバイスへ転送するには `SOLOMON_CLAUSE_COPY_H2D_BEFORE_EXEC(...)`（または `ACC_CLAUSE_COPYIN(...)` / `OMP_TARGET_CLAUSE_MAP_TO(...)`）と書く |
  | `OMP_TARGET_CLAUSE_NUM_THREADS(nthreads)` | `OMP_CLAUSE_NUM_THREADS(nthreads)` | `ACC_CLAUSE_VECTOR_LENGTH(nthreads)` | `OMP_CLAUSE_NUM_THREADS(nthreads)` | |
  | `OMP_TARGET_CLAUSE_PROC_BIND(attr)` | `OMP_CLAUSE_PROC_BIND(attr)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_PROC_BIND(attr)` | |
  | `OMP_TARGET_CLAUSE_NUM_TEAMS(...)` | `OMP_CLAUSE_NUM_TEAMS(__VA_ARGS__)` | `ACC_CLAUSE_NUM_GANGS(__VA_ARGS__)` | `OMP_CLAUSE_NUM_TEAMS(__VA_ARGS__)` | v2.0.0 で変換先を `ACC_CLAUSE_NUM_WORKERS(__VA_ARGS__)` から `ACC_CLAUSE_NUM_GANGS(__VA_ARGS__)` に変更．OpenACC 使用時は上限だけを渡す（`num_gangs` に下限は無い） |
  | `OMP_TARGET_CLAUSE_THREAD_LIMIT(num)` | `OMP_CLAUSE_THREAD_LIMIT(num)` | `ACC_CLAUSE_VECTOR_LENGTH(num)` | `OMP_CLAUSE_THREAD_LIMIT(num)` | 縮退モードでは `PRAGMA_OMP_TARGET_TEAMS(...)` だけが受け取る（ほかの指示文は `parallel` 系になり捨てる）．`SOLOMON_CLAUSE_NUM_THREADS(n)` と `ACC_CLAUSE_VECTOR_LENGTH(n)` は縮退モードで `num_threads(n)`（`parallel` 系）と `thread_limit(n)`（`PRAGMA_OMP_TARGET_TEAMS(...)`）になる <br> OpenACC 使用時，`PRAGMA_OMP_TARGET_*` の指示文は，同じ指示文に `OMP_TARGET_CLAUSE_NUM_THREADS(...)` があると捨てる（どちらも `vector_length` になるため）．`SOLOMON_OFFLOAD(...)`・`PRAGMA_ACC_KERNELS_LOOP(...)`・`PRAGMA_ACC_PARALLEL_LOOP(...)` は両方を残して `vector_length` を 2 つ出すので，そこではどちらか一方だけを渡す |
  | `OMP_TARGET_CLAUSE_NONTEMPORAL(...)` | `OMP_CLAUSE_NONTEMPORAL(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_NONTEMPORAL(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_SAFELEN(length)` | `OMP_CLAUSE_SAFELEN(length)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SAFELEN(length)` | |
  | `OMP_TARGET_CLAUSE_ORDERED(...)` | `OMP_CLAUSE_ORDERED(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ORDERED(__VA_ARGS__)` | `PRAGMA_OMP_ORDERED()` の区画を持つループには引数なしで渡す（clang 22 と Intel oneAPI 2026 はそこで `ordered(1)` を拒否する） |
  | `OMP_TARGET_CLAUSE_SCHEDULE(...)` | `OMP_CLAUSE_SCHEDULE(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SCHEDULE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_DIST_SCHEDULE(...)` | `OMP_CLAUSE_DIST_SCHEDULE(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DIST_SCHEDULE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_BIND(binding)` | `OMP_CLAUSE_BIND(binding)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_BIND(binding)` | |
  | `OMP_TARGET_CLAUSE_USE_DEVICE_PTR(...)` | `use_device_ptr(__VA_ARGS__)` | `ACC_CLAUSE_USE_DEVICE(__VA_ARGS__)` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_USE_DEVICE_ADDR(...)` | `use_device_addr(__VA_ARGS__)` | `ACC_CLAUSE_USE_DEVICE(__VA_ARGS__)` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP(...)` | `defaultmap(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP_NONE` | `OMP_TARGET_CLAUSE_DEFAULTMAP(none)` | `ACC_CLAUSE_DEFAULT_NONE` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT` | `OMP_TARGET_CLAUSE_DEFAULTMAP(present)` | `ACC_CLAUSE_DEFAULT_PRESENT` | N/A（縮退モードでは無視される） | gfortran 13 と nvfortran 26.9（`-mp=gpu`，`presents cannot be used with defaultmap clause`）は `defaultmap(present)` を拒否し，`map(present, ...)` も拒否する（ifx 2026.1 はどちらも受理する．`OMP_TARGET_MAP_MODIFIER_PRESENT` の行を参照） |
  | `OMP_TARGET_CLAUSE_HAS_DEVICE_ADDR(...)` | `has_device_addr(__VA_ARGS__)` | `ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(...)` | `is_device_ptr(__VA_ARGS__)` | `ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_USES_ALLOCATORS(...)` | `uses_allocators(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_FROM(...)` | `from(__VA_ARGS__)` | `ACC_CLAUSE_HOST(__VA_ARGS__)` | N/A（縮退モードでは無視される） | `present` 修飾子は `OMP_TARGET_CLAUSE_FROM_PRESENT(...)` で書く（生の修飾子には対応しない） |
  | `OMP_TARGET_CLAUSE_TO(...)` | `to(__VA_ARGS__)` | `ACC_CLAUSE_DEVICE(__VA_ARGS__)` | N/A（縮退モードでは無視される） | `present` 修飾子は `OMP_TARGET_CLAUSE_TO_PRESENT(...)` で書く（生の修飾子には対応しない） |
  | `OMP_TARGET_CLAUSE_FROM_PRESENT(...)` | `from(present : __VA_ARGS__)` | `ACC_CLAUSE_HOST(__VA_ARGS__)` | N/A（縮退モードでは無視される） | `update` 用．`OMP_TARGET_CLAUSE_MAP(...)` の入れ子にすると `map(present, from : ...)`（OpenACC では `present(...)`）になる．NVIDIA HPC SDK 26.9 は OpenMP target で `present` の付いた `to` / `from` を `update` に書くと受け付けない．nvfortran 26.9 と gfortran 13 は `map(present, from : ...)` を拒否する（ifx 2026.1 は受理する） |
  | `OMP_TARGET_CLAUSE_TO_PRESENT(...)` | `to(present : __VA_ARGS__)` | `ACC_CLAUSE_DEVICE(__VA_ARGS__)` | N/A（縮退モードでは無視される） | `update` 用．`OMP_TARGET_CLAUSE_MAP(...)` の入れ子にすると `map(present, to : ...)`（OpenACC では `present(...)`）になる．NVIDIA HPC SDK 26.9 は OpenMP target で `present` の付いた `to` / `from` を `update` に書くと受け付けない．nvfortran 26.9 と gfortran 13 は `map(present, to : ...)` を拒否する（ifx 2026.1 は受理する） |
  | `OMP_TARGET_MAP_MODIFIER_ALWAYS` | map の修飾子 always．OMP_TARGET_CLAUSE_MAP(...) の中だけに書く（OMP_TARGET_CLAUSE_MAP の備考） | 止まる（OpenACC に対応物が無い） | 出力されない（map が出力されない） | |
  | `OMP_TARGET_MAP_MODIFIER_CLOSE` | map の修飾子 close．OMP_TARGET_CLAUSE_MAP(...) の中だけに書く（OMP_TARGET_CLAUSE_MAP の備考） | 捨てる（修飾子が無いときと同じ copyin / copyout） | 出力されない（map が出力されない） | |
  | `OMP_TARGET_MAP_MODIFIER_PRESENT` | map の修飾子 present．OMP_TARGET_CLAUSE_MAP(...) の中だけに書く（OMP_TARGET_CLAUSE_MAP の備考） | data 構文と計算構文では present(list)．enter data / exit data では止まる | 出力されない（map が出力されない） | nvfortran 26.9 と gfortran 13 は `map(present, ...)` を拒否する（ifx 2026.1 は受理する） |
  | `OMP_TARGET_MAP_MODIFIER_MAPPER(id)` | map の修飾子 mapper(id)．OMP_TARGET_CLAUSE_MAP(...) の中だけに書く（OMP_TARGET_CLAUSE_MAP の備考） | 止まる（OpenACC に対応物が無い） | 出力されない（map が出力されない） | |
  | `OMP_TARGET_MAP_MODIFIER_ITERATOR(spec)` | map の修飾子 iterator(spec)．OMP_TARGET_CLAUSE_MAP(...) の中だけに書く（OMP_TARGET_CLAUSE_MAP の備考） | 止まる（OpenACC に対応物が無い） | 出力されない（map が出力されない） | |
  | `OMP_TARGET_PASS_LIST(...)` | `OMP_PASS_LIST(__VA_ARGS__)` | `ACC_PASS_LIST(__VA_ARGS__)` | `OMP_PASS_LIST(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_SEQ_CST` | `OMP_CLAUSE_SEQ_CST` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SEQ_CST` | |
  | `OMP_TARGET_CLAUSE_ACQ_REL` | `OMP_CLAUSE_ACQ_REL` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ACQ_REL` | |
  | `OMP_TARGET_CLAUSE_RELEASE` | `OMP_CLAUSE_RELEASE` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_RELEASE` | |
  | `OMP_TARGET_CLAUSE_ACQUIRE` | `OMP_CLAUSE_ACQUIRE` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ACQUIRE` | |
  | `OMP_TARGET_CLAUSE_RELAXED` | `OMP_CLAUSE_RELAXED` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_RELAXED` | |
  | `OMP_TARGET_CLAUSE_READ` | `OMP_CLAUSE_READ` | `ACC_CLAUSE_READ` | `OMP_CLAUSE_READ` | |
  | `OMP_TARGET_CLAUSE_WRITE` | `OMP_CLAUSE_WRITE` | `ACC_CLAUSE_WRITE` | `OMP_CLAUSE_WRITE` | |
  | `OMP_TARGET_CLAUSE_UPDATE` | `OMP_CLAUSE_UPDATE` | `ACC_CLAUSE_UPDATE` | `OMP_CLAUSE_UPDATE` | |
  | `OMP_TARGET_CLAUSE_CAPTURE` | `OMP_CLAUSE_CAPTURE` | `ACC_CLAUSE_CAPTURE` | `OMP_CLAUSE_CAPTURE` | |
  | `OMP_TARGET_CLAUSE_COMPARE` | `OMP_CLAUSE_COMPARE` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_COMPARE` | |
  | `OMP_TARGET_CLAUSE_FAIL(...)` | `OMP_CLAUSE_FAIL(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_FAIL(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_WEAK` | `OMP_CLAUSE_WEAK` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_WEAK` | |
  | `OMP_TARGET_CLAUSE_HINT(expression)` | `OMP_CLAUSE_HINT(expression)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_HINT(expression)` | |
  | `OMP_TARGET_CLAUSE_SIMD` | `OMP_CLAUSE_SIMD` | `ACC_CLAUSE_INDEPENDENT` | `OMP_CLAUSE_SIMD` | |
  | `OMP_TARGET_CLAUSE_DEFAULT_SHARED` | `OMP_CLAUSE_DEFAULT_SHARED` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DEFAULT_SHARED` | |
  | `OMP_TARGET_CLAUSE_DEFAULT_FIRSTPRIVATE` | `OMP_CLAUSE_DEFAULT_FIRSTPRIVATE` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DEFAULT_FIRSTPRIVATE` | |
  | `OMP_TARGET_CLAUSE_DEFAULT_PRIVATE` | `OMP_CLAUSE_DEFAULT_PRIVATE` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DEFAULT_PRIVATE` | |
  | `OMP_TARGET_CLAUSE_DEFAULT_NONE` | `OMP_CLAUSE_DEFAULT_NONE` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DEFAULT_NONE` | |
  | `OMP_TARGET_CLAUSE_SHARED(...)` | `OMP_CLAUSE_SHARED(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SHARED(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_PRIVATE(...)` | `OMP_CLAUSE_PRIVATE(__VA_ARGS__)` | `ACC_CLAUSE_PRIVATE(__VA_ARGS__)` | `OMP_CLAUSE_PRIVATE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_FIRSTPRIVATE(...)` | `OMP_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | `ACC_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | `OMP_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_LASTPRIVATE(...)` | `OMP_CLAUSE_LASTPRIVATE(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_LASTPRIVATE(__VA_ARGS__)` | OpenACC に対応物が無く，OpenACC 使用時はループの後の値が保証されない．OpenACC でも動かすコードでは頼らない |
  | `OMP_TARGET_CLAUSE_LINEAR(...)` | `OMP_CLAUSE_LINEAR(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_LINEAR(__VA_ARGS__)` | OpenACC に対応物が無い．OpenACC 使用時は，`simd` を含む指示文（`loop independent` を出す），指示節の `SOLOMON_CLAUSE_INDEPENDENT`・`ACC_CLAUSE_INDEPENDENT`・`OMP_TARGET_CLAUSE_SIMD`，`SOLOMON_OFFLOAD_BY_OPENACC_PARALLEL`（OpenACC 3.3 の 2.9.6 節）のどれかがあると，ループは independent（反復をどの順にも，同時にも実行してよい）とみなされる．それ以外（既定の `kernels` 構文）はコンパイラが判断する．OpenACC でも動かすコードでは，変数を各反復で更新せずループの添字から計算する（NVIDIA HPC SDK 26.9 は `j = j + 1` の形を正しく扱った．ほかの OpenACC のコンパイラは未確認） |
  | `OMP_TARGET_CLAUSE_ALLOCATE(...)` | `OMP_CLAUSE_ALLOCATE(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ALLOCATE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_COLLAPSE(n)` | `OMP_CLAUSE_COLLAPSE(n)` | `ACC_CLAUSE_COLLAPSE(n)` | `OMP_CLAUSE_COLLAPSE(n)` | |
  | `OMP_TARGET_CLAUSE_DEPEND(...)` | `OMP_CLAUSE_DEPEND(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DEPEND(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_DEPEND_IN(...)` | `OMP_CLAUSE_DEPEND_IN(__VA_ARGS__)` | `wait`（引数は捨てられ，全てのキューを待つ） | `OMP_CLAUSE_DEPEND_IN(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_DEVICE(...)` | `device(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `device(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(condition)` | `ACC_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(condition)` | |
  | `OMP_TARGET_CLAUSE_IF_TARGET(condition)` | `OMP_CLAUSE_IF(target : condition)` | `ACC_CLAUSE_IF(condition)` | N/A（縮退モードでは無視される） | `target` 構文に付ける指示節．`OMP_TARGET_CLAUSE_IF(condition)` を `PRAGMA_OMP_TARGET_DATA(...)`，`..._ENTER_DATA(...)`，`..._EXIT_DATA(...)`，`..._UPDATE(...)` に使う |
  | `OMP_TARGET_CLAUSE_MAP(...)` | `OMP_CLAUSE_MAP(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される．入れ子の節を書いたときは備考） | `OMP_CLAUSE_MAP(__VA_ARGS__)` | 入れ子の `OMP_TARGET_CLAUSE_TO(...)` / `OMP_TARGET_CLAUSE_FROM(...)`（と `OMP_TARGET_CLAUSE_TO_PRESENT(...)` / `OMP_TARGET_CLAUSE_FROM_PRESENT(...)`）は，1 つずつ OpenMP では `map(to : ...)` / `map(from : ...)`，OpenACC では `copyin(...)` / `copyout(...)` になる．先頭に書いた修飾子 `OMP_TARGET_MAP_MODIFIER_*` はその map の入れ子すべてに付く（`map(present, to : ...)`）．OpenACC では `present` は data 構文と計算構文で `present(...)` になり enter data / exit data では止まる．`close` は捨て，`always` / `mapper` / `iterator` は止まる．入れ子の後ろの生の並びは止まる．入れ子の節が無ければ OpenACC 使用時には無視される（OpenMP の data 系の指示文は止まる）．入れ子でない並びの 3 番目以降の要素が，1 引数で呼べない関数形式のマクロの名前（例: `#define fm2(x, y) ...` の下の `OMP_TARGET_CLAUSE_MAP(a, b, fm2)`）だと前処理が止まり，1 引数で呼ぶとトップレベルのカンマを含む形に展開される関数形式のマクロの名前（例: `#define fc1(x) x, x`）だと内部のマクロの名前を挙げる Solomon のエラーでビルドが止まる．その要素を 1 番目か 2 番目に置く（例: `OMP_TARGET_CLAUSE_MAP(fm2, a, b)`）．`update` では `OMP_TARGET_CLAUSE_TO(...)` / `OMP_TARGET_CLAUSE_FROM(...)` を本マクロで包まない（update 指示文に `map` 節は無く，包んだ節は捨てられる．`SOLOMON_MEMCPY_H2D(list)` / `SOLOMON_MEMCPY_D2H(list)` を推奨） |
  | `OMP_TARGET_CLAUSE_MAP_ALLOC(...)` | `OMP_CLAUSE_MAP_ALLOC(__VA_ARGS__)` | `ACC_CLAUSE_CREATE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_ALLOC(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_TO(...)` | `OMP_CLAUSE_MAP_TO(__VA_ARGS__)` | `ACC_CLAUSE_COPYIN(__VA_ARGS__)` | `OMP_CLAUSE_MAP_TO(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_FROM(...)` | `OMP_CLAUSE_MAP_FROM(__VA_ARGS__)` | `ACC_CLAUSE_COPYOUT(__VA_ARGS__)` | `OMP_CLAUSE_MAP_FROM(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_TOFROM(...)` | `OMP_CLAUSE_MAP_TOFROM(__VA_ARGS__)` | `ACC_CLAUSE_COPY(__VA_ARGS__)` | `OMP_CLAUSE_MAP_TOFROM(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_RELEASE(...)` | `OMP_CLAUSE_MAP_RELEASE(__VA_ARGS__)` | `ACC_CLAUSE_DELETE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_RELEASE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_DELETE(...)` | `OMP_CLAUSE_MAP_DELETE(__VA_ARGS__)` | `ACC_CLAUSE_DELETE(__VA_ARGS__)` と，exit data に `finalize` を 1 つ（v2.0.0 以降） | `OMP_CLAUSE_MAP_DELETE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_ORDER(...)` | `OMP_CLAUSE_ORDER(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ORDER(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_NOWAIT` | `OMP_CLAUSE_NOWAIT` | `ACC_CLAUSE_ASYNC()` | `OMP_CLAUSE_NOWAIT` | |
  | `OMP_TARGET_CLAUSE_REDUCTION(...)` | `OMP_CLAUSE_REDUCTION(__VA_ARGS__)` | `ACC_CLAUSE_REDUCTION(__VA_ARGS__)` | `OMP_CLAUSE_REDUCTION(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_IN_REDUCTION(...)` | `OMP_CLAUSE_IN_REDUCTION(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_IN_REDUCTION(__VA_ARGS__)` | |

  </details>

* <details><summary>OpenMP</summary>

  | 入力 | 出力 |
  | ---- | ---- |
  | `OMP_CLAUSE_EXCLUSIVE(...)` | `exclusive(__VA_ARGS__)` |
  | `OMP_CLAUSE_INCLUSIVE(...)` | `inclusive(__VA_ARGS__)` |
  | `OMP_CLAUSE_ALIGNED(...)` | `aligned(__VA_ARGS__)` |
  | `OMP_CLAUSE_INBRANCH` | `inbranch` |
  | `OMP_CLAUSE_NOTINBRANCH` | `notinbranch` |
  | `OMP_CLAUSE_SIMDLEN(length)` | `simdlen(length)` |
  | `OMP_CLAUSE_UNIFORM(...)` | `uniform(__VA_ARGS__)` |
  | `OMP_CLAUSE_SIZES(...)` | `sizes(__VA_ARGS__)` |
  | `OMP_CLAUSE_FULL` | `full` |
  | `OMP_CLAUSE_PARTIAL(...)` | 引数なしは `partial`，引数ありは後ろに括弧で付く `(__VA_ARGS__)` |
  | `OMP_CLAUSE_COPYIN(...)` | `copyin(__VA_ARGS__)` |
  | `OMP_CLAUSE_NUM_THREADS(nthreads)` | `num_threads(nthreads)` |
  | `OMP_CLAUSE_PROC_BIND(attr)` | `proc_bind(attr)` |
  | `OMP_CLAUSE_NUM_TEAMS(...)` | `num_teams(__VA_ARGS__)` |
  | `OMP_CLAUSE_THREAD_LIMIT(num)` | `thread_limit(num)` |
  | `OMP_CLAUSE_NONTEMPORAL(...)` | `nontemporal(__VA_ARGS__)` |
  | `OMP_CLAUSE_SAFELEN(length)` | `safelen(length)` |
  | `OMP_CLAUSE_FILTER(thread_num)` | `filter(thread_num)` |
  | `OMP_CLAUSE_COPYPRIVATE(...)` | `copyprivate(__VA_ARGS__)` |
  | `OMP_CLAUSE_ORDERED(...)` | 引数なしは `ordered`，引数ありは後ろに括弧で付く `(__VA_ARGS__)` |
  | `OMP_CLAUSE_SCHEDULE(...)` | `schedule(__VA_ARGS__)` |
  | `OMP_CLAUSE_DIST_SCHEDULE(...)` | `dist_schedule(__VA_ARGS__)` |
  | `OMP_CLAUSE_BIND(binding)` | `bind(binding)` |
  | `OMP_CLAUSE_AFFINITY(...)` | `affinity(__VA_ARGS__)` |
  | `OMP_CLAUSE_DETACH(event_handle)` | `detach(event_handle)` |
  | `OMP_CLAUSE_FINAL(expression)` | `final(expression)` |
  | `OMP_CLAUSE_MERGEABLE` | `mergeable` |
  | `OMP_CLAUSE_PRIORITY(value)` | `priority(value)` |
  | `OMP_CLAUSE_UNTIED` | `untied` |
  | `OMP_CLAUSE_GRAINSIZE(...)` | `grainsize(__VA_ARGS__)` |
  | `OMP_CLAUSE_NOGROUP` | `nogroup` |
  | `OMP_CLAUSE_NUM_TASKS(...)` | `num_tasks(__VA_ARGS__)` |
  | `OMP_CLAUSE_INIT(...)` | `init(__VA_ARGS__)` |
  | `OMP_CLAUSE_USE(var)` | `use(var)` |
  | `OMP_CLAUSE_TASK_REDUCTION(...)` | `task_reduction(__VA_ARGS__)` |
  | `OMP_CLAUSE_DESTROY(...)` | 引数なしは `destroy`，引数ありは後ろに括弧で付く `(__VA_ARGS__)` |
  | `OMP_PASS_LIST(...)` | `(__VA_ARGS__)` |
  | `OMP_CLAUSE_SEQ_CST` | `seq_cst` |
  | `OMP_CLAUSE_ACQ_REL` | `acq_rel` |
  | `OMP_CLAUSE_RELEASE` | `release` |
  | `OMP_CLAUSE_ACQUIRE` | `acquire` |
  | `OMP_CLAUSE_RELAXED` | `relaxed` |
  | `OMP_CLAUSE_READ` | `read` |
  | `OMP_CLAUSE_WRITE` | `write` |
  | `OMP_CLAUSE_UPDATE` | `update` |
  | `OMP_CLAUSE_DEPOBJ_UPDATE(...)` | `update(__VA_ARGS__)`: `PRAGMA_OMP_DEPOBJ(...)` の `update` 節（v2.0.0 以降） |
  | `OMP_CLAUSE_CAPTURE` | `capture` |
  | `OMP_CLAUSE_COMPARE` | `compare` |
  | `OMP_CLAUSE_FAIL(...)` | `fail(__VA_ARGS__)`（引数は `OMP_CLAUSE_SEQ_CST`・`OMP_CLAUSE_ACQUIRE`・`OMP_CLAUSE_RELAXED` のどれか: OpenMP 5.2 §15.8.3） |
  | `OMP_CLAUSE_WEAK` | `weak` |
  | `OMP_CLAUSE_HINT(expression)` | `hint(expression)` |
  | `OMP_CLAUSE_THREADS` | `threads` |
  | `OMP_CLAUSE_SIMD` | `simd` <br> 名前に `simd` の無いループの指示文では名前に続く（`PRAGMA_OMP_DO(OMP_CLAUSE_SIMD)` → `do simd`）．名前に `simd` を含む指示文では捨てる（v2.0.0 以降） |
  | `OMP_CLAUSE_DOACROSS(...)` | `doacross(__VA_ARGS__)` <br> Clang 23 と Intel 2026.1（C/C++）は `doacross` を拒否する．そこでは `PRAGMA_OMP_ORDERED(...)` が出す `OMP_CLAUSE_DEPEND(source)` / `OMP_CLAUSE_DEPEND(sink: ...)` を使う（OpenMP 5.2 で非推奨，6.0 で削除．v2.0.0 以降．gfortran 13 と ifx 2026.1 は正しく実行する）．NVIDIA HPC SDK 26.9 は `doacross` を拒否し，nvfortran 26.9 は `ordered` の `depend` を拒否する（nvc++ 26.9 は節を警告だけで無視して誤った結果になる）．ROCm 10.0 の amdflang（flang 23）は `ordered` の `depend` で止まり（`not yet implemented`），`doacross(source:)` を構文解析できない |
  | `OMP_CLAUSE_DEFAULT(attr)` | `default(attr)` |
  | `OMP_CLAUSE_DEFAULT_SHARED` | `OMP_CLAUSE_DEFAULT(shared)` |
  | `OMP_CLAUSE_DEFAULT_FIRSTPRIVATE` | `OMP_CLAUSE_DEFAULT(firstprivate)` |
  | `OMP_CLAUSE_DEFAULT_PRIVATE` | `OMP_CLAUSE_DEFAULT(private)` |
  | `OMP_CLAUSE_DEFAULT_NONE` | `OMP_CLAUSE_DEFAULT(none)` |
  | `OMP_CLAUSE_SHARED(...)` | `shared(__VA_ARGS__)` |
  | `OMP_CLAUSE_PRIVATE(...)` | `private(__VA_ARGS__)` |
  | `OMP_CLAUSE_FIRSTPRIVATE(...)` | `firstprivate(__VA_ARGS__)` |
  | `OMP_CLAUSE_LASTPRIVATE(...)` | `lastprivate(__VA_ARGS__)` |
  | `OMP_CLAUSE_LINEAR(...)` | `linear(__VA_ARGS__)` |
  | `OMP_CLAUSE_ALLOCATE(...)` | `allocate(__VA_ARGS__)` |
  | `OMP_CLAUSE_COLLAPSE(n)` | `collapse(n)` |
  | `OMP_CLAUSE_DEPEND(...)` | `depend(__VA_ARGS__)` |
  | `OMP_CLAUSE_DEPEND_IN(...)` | `OMP_CLAUSE_DEPEND(in : __VA_ARGS__)` |
  | `OMP_CLAUSE_IF(...)` | `if(__VA_ARGS__)` <br> `teams` の `if`（`PRAGMA_OMP_TEAMS(...)`・`..._TEAMS_DISTRIBUTE(...)`・`..._TEAMS_LOOP(...)`）は OpenMP 6.0 の節．Fortran では Solomon は版によらず書いたとおりに出す: 指示文としては正しい書き方だが，6.0 より前のコンパイラは拒否することがある（gfortran 13 と nvfortran 26.9 は拒否する）．Intel ifx 2026.1 は受け付けて正しく動く（C/C++ では `_OPENMP >= 202411` のときだけ出す．C/C++ 版を参照） |
  | `OMP_CLAUSE_MAP(...)` | `map(__VA_ARGS__)` |
  | `OMP_CLAUSE_MAP_ALLOC(...)` | `OMP_CLAUSE_MAP(alloc : __VA_ARGS__)` |
  | `OMP_CLAUSE_MAP_TO(...)` | `OMP_CLAUSE_MAP(to : __VA_ARGS__)` |
  | `OMP_CLAUSE_MAP_FROM(...)` | `OMP_CLAUSE_MAP(from : __VA_ARGS__)` |
  | `OMP_CLAUSE_MAP_TOFROM(...)` | `OMP_CLAUSE_MAP(tofrom : __VA_ARGS__)` |
  | `OMP_CLAUSE_MAP_RELEASE(...)` | `OMP_CLAUSE_MAP(release : __VA_ARGS__)` |
  | `OMP_CLAUSE_MAP_DELETE(...)` | `OMP_CLAUSE_MAP(delete : __VA_ARGS__)` |
  | `OMP_CLAUSE_ORDER(...)` | `order(__VA_ARGS__ concurrent)` |
  | `OMP_CLAUSE_NOWAIT` | `nowait` |
  | `OMP_CLAUSE_REDUCTION(...)` | `reduction(__VA_ARGS__)` |
  | `OMP_CLAUSE_IN_REDUCTION(...)` | `in_reduction(__VA_ARGS__)` |

  </details>

### 使用可能なランタイム関数

* デバイスを選ぶ関数と，オフロードのバックエンドのストリームを得る関数です（バックエンドのランタイムルーチンを呼びます）
* 呼び出すモジュール・プログラムに `USE_SOLOMON_RUNTIME` を書いてください（[Solomon を用いたコードの開発方法](#solomon-を用いたコードの開発方法)）．`set_default_device_for_offloading(id)` はサブルーチンとして `call` で呼びます
* シリアルモードでは，`set_default_device_for_offloading(id)` は出力の引数を持たない組み込みの `get_command_argument(int(id))` になります．効果は無く，`call set_default_device_for_offloading(id)` が正しい文のまま残ります．同じスコープに `get_command_argument` という名前の変数や手続を宣言しないでください: シリアルモードでは，その名前の変数はコンパイルエラーになり，引数 1 個のその名前の手続は黙って呼ばれます．`id` は既定の種別の整数で渡してください: `integer(8)` の引数は OpenACC のバックエンドでコンパイルエラーになります（gfortran と nvfortran）．シリアルモードはどの種別の整数でも受け付けます（`int(id)`）
* `get_offloaded_stream_id*` は CUDA のストリームを返すので，NVIDIA HPC SDK でだけ使えます:
  * OpenACC: `acc_get_cuda_stream(...)` を呼びます．コンパイラがこのルーチンを提供している必要があります（gfortran の `openacc` モジュールにはありません）．nvfortran ではストリームを `integer(8)` の変数で受けてください
  * OpenMP target: Fortran では定義されません．`spp.sh` / `fortran.mk` は GNU の `cpp` で前処理し，GNU の `cpp` は `__NVCOMPILER` を定義しないためです（C/C++ では nvc / nvc++ のときだけ定義されます）
  * 縮退モードとシリアルモードでは定義されません（デバイスのストリームが無いため）
  * AMD・Intel の GPU でも定義されません．ifx は `interop` 構文（`PRAGMA_OMP_INTEROP(...)`）と `omp_lib` の `omp_get_interop_ptr` を受け付け，Level Zero のハンドルを返します（`target` 構文が使うキューではありません．C/C++ の README を参照）．amdflang は `interop` 構文を拒否し，nvfortran 26.9 は警告 `NVFORTRAN-W-0287-Unrecognized OpenMP directive - interop` を出して無視します（構文は何もしません）

  ```Fortran
  integer :: n, id
  n = get_num_devices_for_offloading()
  call set_default_device_for_offloading(n - 1)
  id = get_default_device_for_offloading()
  ```

  | 入力 | 出力 | バックエンド |
  | ---- | ---- | ---- |
  | `get_num_devices_for_offloading()` | `acc_get_num_devices(acc_device_default)` <br> `omp_get_num_devices()` <br> `omp_get_num_devices()` <br> `0` | OpenACC <br> OpenMP target <br> OpenMP（縮退モード） <br> シリアルモード |
  | `set_default_device_for_offloading(id)` | `acc_set_device_num(int(id), acc_device_default)` <br> `omp_set_default_device(int(id))` <br> `omp_set_default_device(int(id))` <br> `get_command_argument(int(id))`（効果なし） | OpenACC <br> OpenMP target <br> OpenMP（縮退モード） <br> シリアルモード |
  | `get_default_device_for_offloading()` | `acc_get_device_num(acc_device_default)` <br> `omp_get_default_device()` <br> `omp_get_default_device()` <br> `0` | OpenACC <br> OpenMP target <br> OpenMP（縮退モード） <br> シリアルモード |
  | `get_offloaded_stream_id(async)` | `acc_get_cuda_stream(async)` | OpenACC（のみ） |
  | `get_offloaded_stream_id_async()` | `get_offloaded_stream_id(acc_async_noval)` | OpenACC（のみ） |
  | `get_offloaded_stream_id_sync()` | `get_offloaded_stream_id(acc_async_sync)` | OpenACC（のみ） |

## 謝辞

本ライブラリの開発の一部はJSPS科研費JP23K11123，文部科学省「HPCI整備計画調査研究事業」，文部科学省「次世代HPC・AI開発支援拠点形成事業」および文部科学省次世代計算科学グランドリーチプログラム「次世代多物理・多階層宇宙計算基盤の確立」(JPMXP1920260203) の支援を受けたものです
