# Solomon: Simple Off-LOading Macros Orchestrating multiple Notations

[English](README.md) | [日本語](README_jp.md)

## 概要

* GPU向け指示文（OpenACC, OpenMP target）統合マクロ
* 詳細については [Miki & Hanawa (2024, IEEE Access, vol. 12, pp. 181644-181665)](https://doi.org/10.1109/ACCESS.2024.3509380) を参照してください
* 使用した際には [Miki & Hanawa (2024, IEEE Access, vol. 12, pp. 181644-181665)](https://doi.org/10.1109/ACCESS.2024.3509380) を引用してください
* Released under the MIT license, see LICENSE.txt
* Copyright (c) 2024 Yohei MIKI

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

## 使い方

### Solomon を用いたコードの開発方法

1. Solomon のヘッダファイルをインクルードする

   ```c++
   #include <solomon.hpp>
   ```

2. OpenACC/OpenMP target 指示文のかわりに Solomon が提供するマクロを挿入する

   * GPU化を始めたばかりの方には，`OFFLOAD(...)` などの簡易記法の利用をおすすめします
   * OpenACC あるいは OpenMP target を用いた開発に慣れている方は，OpenACC/OpenMP 的な記法を使うこともできます
     * OpenMP 的記法を使った場合には，`PRAGMA_OMP_TARGET_*` や `OMP_TARGET_CLAUSE_*` のように `_TARGET_` をつけたものだけが OpenACC 使用時における変換対象となります（例えば `PRAGMA_OMP_ATOMIC(...)` は `_Pragma("omp atomic __VA_ARGS__")` へと変換されるため，`_Pragma("acc atomic __VA_ARGS__")` には変換されません）
     * `PRAGMA_OMP_TARGET_DATA(...)` という記法は使わないでください
       * 演算加速器（GPU）からアクセスするデータについては `DATA_ACCESS_BY_DEVICE(...)` か `PRAGMA_ACC_DATA(...)`，ホスト（CPU）からアクセスするデータについては `DATA_ACCESS_BY_HOST(...)` か `PRAGMA_ACC_HOST_DATA(...)` をお使いください
     * OpenACC 的記法においてを使った際に`PRAGMA_ACC_ROUTINE(...)`（や対応するマクロ）を使用した際には，対象リージョンの最後に`DECLARE_OFFLOADED_END` か `PRAGMA_OMP_END_DECLARE_TARGET` も挿入してください
   * GPU実行時には無視してほしい指示文については，`IF_NOT_OFFLOADED(arg)` の中に記入してください
     * <details><summary> 実装例: `arg` については，縮退モード（OpenACC と OpenMP target 両方を無効化した場合）のみ実体化されます</summary>

       ```c++
       OFFLOAD()
       for(int i = 0; i < num; i++){
         IF_NOT_OFFLOADED(PRAGMA_OMP_SIMD())
         for(int j = 0; j < 16; j++){
           // computation
         }
       }
       ```

       * OpenACC 使用時

         ```c++
         _Pragma("acc kernels")
         _Pragma("acc loop")
         for(int i = 0; i < num; i++){

           for(int j = 0; j < 16; j++){
             // computation
           }
         }
         ```

       * OpenMP target 使用時

         ```c++
         _Pragma("omp target teams loop")
         for(int i = 0; i < num; i++){

           for(int j = 0; j < 16; j++){
             // computation
           }
         }
         ```

       * 縮退モード

         ```c++
         _Pragma("omp parallel for")
         for(int i = 0; i < num; i++){
           _Pragma("omp simd")
           for(int j = 0; j < 16; j++){
             // computation
           }
         }
         ```

     </details>

   * 指示文に付与する指示節・指示句については，下記の例のようにカンマ区切りで渡してください：

      ```c++
      OFFLOAD(AS_INDEPENDENT, ACC_CLAUSE_VECTOR_LENGTH(128), OMP_TARGET_CLAUSE_COLLAPSE(3))
      ```

      * 簡易記法，OpenACC/OpenMP的記法を混ぜても問題ありません
      * `AS_INDEPENDENT`（及びこれに対応する `ACC_CLAUSE_INDEPENDENT` や `OMP_TARGET_CLAUSE_SIMD`）については，全ての指示節・指示句の先頭に記載してください
      * 指示文に対応していない指示節・指示句については，Solomon が自動的に無視します
   * OpenACC/OpenMP target間の互換性向上のため，（個々のマクロを直接使うよりも）下記に示す統合マクロの使用をおすすめします

     | 推奨実装 | 対応実装（非推奨） |
     | ---- | ---- |
     | **`OFFLOAD(...)`** <br> `PRAGMA_ACC_KERNELS_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL_LOOP(...)` | <br> `PRAGMA_ACC_KERNELS(...) PRAGMA_ACC_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL(...) PRAGMA_ACC_LOOP(...)` |
     | **`DECLARE_DATA_ON_DEVICE(...)`** <br> `PRAGMA_ACC_DATA_PRESENT(...)` | <br> `PRAGMA_ACC_DATA(ACC_CLAUSE_PRESENT(...))` |
     | `OMP_TARGET_CLAUSE_MAP_TO(...)` | `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_CLAUSE_TO(...))` |

   * カーネルの非同期実行および同期処理については下記のマクロが提供されているので，用途に合わせて使い分けてください
     * OpenACC/OpenMP target間の互換性向上のため，非同期実行および同期処理についてもSolomonが提供する簡易記法の使用をおすすめします
     * バックエンドに依らず非同期処理を実行したい場合には`AS_ASYNC(...)`および`SYNCHRONIZE(...)`を用いてください．キューIDの指定は無視される場合がある点にご注意ください
     * キューIDを指定した細やかな非同期処理を実行したい場合には`ASYNC_QUEUE(id)`および`WAIT_QUEUE(id)`を用いてください．非同期実行がなされない場合がある点にご注意ください

     | 使用可能なマクロ | 出力 | 使用されるバックエンド | 備考 |
     | ---- | ---- | ---- | ---- |
     | **`AS_ASYNC(...)`** <br> `ACC_CLAUSE_ASYNC(...)` <br> `OMP_TARGET_CLAUSE_NOWAIT` | <br> `async(__VA_ARGS__)` <br> `nowait` | <br> OpenACC <br> OpenMP | 両バックエンドで共通して非同期実行される <br> OpenACC においてはキューIDを指定可能 <br> OpenMP においてはキューIDは無視される |
     | **`SYNCHRONIZE(...)`** <br> `PRAGMA_ACC_WAIT(...)` <br> `PRAGMA_OMP_TARGET_TASKWAIT(...)` | <br> `_Pragma("acc wait __VA_ARGS__")` <br> `_Pragma("omp taskwait __VA_ARGS__")` | <br> OpenACC <br> OpenMP | 両バックエンドで共通して同期処理が実行される．`AS_ASYNC(...)` に対応して記述する |
     | **`ASYNC_QUEUE(id)`** <br> `ACC_CLAUSE_ASYNC(id)` | <br> `async(id)` <br> N/A（OpenMP target 使用時には無視される） | <br> OpenACC <br> OpenMP | OpenACC でのみキューIDを指定して非同期実行 <br> キューIDの指定が必須 <br> OpenMP ではキューIDを指定した非同期実行がサポートされていないため，無視される |
     | **`WAIT_QUEUE(id)`** <br> `PRAGMA_ACC_WAIT(id)` | <br> `wait(id)` <br> N/A（OpenMP target 使用時には無視される） | <br> OpenACC <br> OpenMP | OpenACC でのみキューIDを指定して同期処理を実行．`ASYNC_QUEUE(id)` に対応して記述する <br> キューIDの指定が必須 <br> OpenMP ではキューIDを指定した同期処理がサポートされていないため，無視される |

### Solomon を使ったコードのコンパイル方法

* コンパイラオプションを用いて，OpenACC または OpenMP target を有効化してください
* Solomon のパス（`solomon.hpp` があるディレクトリ）を `-I/path/to/solomon` などとして指示してください
* 下記のコンパイルフラグを用いて，Solomon の動作モードを指定してください

  | コンパイルフラグ | 使用されるバックエンド | 備考 |
  | ---- | ---- | ---- |
  | `-DOFFLOAD_BY_OPENACC` | OpenACC | デフォルトでは `kernels` 構文を使用 |
  | `-DOFFLOAD_BY_OPENACC -DOFFLOAD_BY_OPENACC_PARALLEL` | OpenACC | デフォルトでは `parallel` 構文を使用 |
  | `-DOFFLOAD_BY_OPENMP_TARGET` | OpenMP target | デフォルトでは `loop` 指示文を使用 |
  | `-DOFFLOAD_BY_OPENMP_TARGET -DOFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE` | OpenMP target | デフォルトでは `distribute` 指示文を使用 |
  | | 縮退モード | OpenMP を用いたマルチコアCPU向けのスレッド並列 |

* コンパイルフラグとして`-DPRINT_GENERATED_PRAGMA`を追加すると，実際に生成される指示文をコンパイル時メッセージに出力できます
  * LLVMではwarning扱いとなるため，`-Werror`を指定している際には`-Wno-error=pragma-messages`も渡してこのメッセージがエラー扱いにならないようにしてください
* 使用例： [N体計算用の Makefile](samples/nbody/Makefile) および [拡散方程式用の Makefile](samples/diffusion/Makefile)

### Solomon の拡張方法

* Solomon では，1つの指示文ごとに 32 個の指示節・指示句（候補）を受け付ける仕様となっています
  * 32 という上限値では不足する場合には，下記の手順で上限値を増やしてください

    ```sh
    cd solomon/util # このディレクトリには，jl/ および pickup.hpp が格納されています
    julia jl/check_clause.jl --max 64 >> pickup.hpp # この例は，上限値を 64 に増やす場合です
    # pickup.hpp の中身を適切に編集してください（古い CHECK_CLAUSE_* および APPEND_CLAUSE を削除し，新たに追加された CHECK_CLAUSE_* および APPEND_CLAUSE を使用してください）
    ```

  * 同様の制限が他の内部マクロに存在することがありますが，同様の手順で上限値を増やせます

## サンプルコード

### nbody: 演算律速な問題の実装例

* [直接法に基づく無衝突系$N$体計算](samples/nbody/)
  * 簡易記法を用いた実装例: `samples/nbody/src/[nbody gravity].cpp`
  * OpenACC 的記法を用いた実装例: `samples/nbody/src/[nbody gravity]_acc.cpp`
  * OpenMP 的記法を用いた実装例: `samples/nbody/src/[nbody gravity]_omp.cpp`

### diffusion: メモリ律速な問題の実装例

* [3次元拡散方程式](samples/diffusion/)
  * [OpenACC を用いた実装（名古屋大学の星野哲也氏による実装）](https://github.com/hoshino-UTokyo/lecture_openacc) を改変したものです
  * Solomon 化以外に，簡単な性能最適化やコード改編も施してあります

## Solomon が提供する API

### 使用可能な指示文

* <details><summary>演算に関する指示文</summary>

  | 入力 | 出力 | バックエンドとして用いる指示文 |
  | ---- | ---- | ---- |
  | **`OFFLOAD(...)`** <br> `PRAGMA_ACC_KERNELS_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL_LOOP(...)` <br> `PRAGMA_OMP_TARGET_TEAMS_LOOP(...)` <br> `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...)` | <br> `_Pragma("acc kernels __VA_ARGS__") _Pragma("acc loop __VA_ARGS__")` <br> `_Pragma("acc parallel __VA_ARGS__") _Pragma("acc loop __VA_ARGS__")` <br> `_Pragma("omp target teams loop __VA_ARGS__")` <br> `_Pragma("omp target teams distribute parallel for __VA_ARGS__")` | <br> OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP (loop) <br> OpenMP (distribute) |
  | **`SYNCHRONIZE(...)`** <br> `PRAGMA_ACC_WAIT(...)` <br> `PRAGMA_OMP_TARGET_TASKWAIT(...)` | <br> `_Pragma("acc wait __VA_ARGS__")` <br> `_Pragma("omp taskwait __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`WAIT_QUEUE(id)`** <br> `PRAGMA_ACC_WAIT(id)` | <br> `_Pragma("acc wait id")` | <br> OpenACC (only) |
  | **`DECLARE_OFFLOADED(...)`** <br> `PRAGMA_ACC_ROUTINE(...)` <br> `PRAGMA_OMP_DECLARE_TARGET(...)` | <br> `_Pragma("acc routine __VA_ARGS__")` <br> `_Pragma("omp declare target __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`DECLARE_OFFLOADED_END`** <br> `PRAGMA_OMP_END_DECLARE_TARGET` | <br> `_Pragma("omp end declare target")` | <br> OpenMP (only) |
  | **`ATOMIC(...)`** <br> `PRAGMA_ACC_ATOMIC(...)` <br> `PRAGMA_OMP_TARGET_ATOMIC(...)` | <br> `_Pragma("acc atomic __VA_ARGS__")` <br> `_Pragma("omp atomic __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`ATOMIC_UPDATE`** <br> `PRAGMA_ACC_ATOMIC_UPDATE` <br> `PRAGMA_OMP_TARGET_ATOMIC_UPDATE` | <br> `_Pragma("acc atomic update")` <br> `_Pragma("omp atomic update")` | <br> OpenACC <br> OpenMP |
  | **`ATOMIC_READ`** <br> `PRAGMA_ACC_ATOMIC_READ` <br> `PRAGMA_OMP_TARGET_ATOMIC_READ` | <br> `_Pragma("acc atomic read")` <br> `_Pragma("omp atomic read")` | <br> OpenACC <br> OpenMP |
  | **`ATOMIC_WRITE`** <br> `PRAGMA_ACC_ATOMIC_WRITE` <br> `PRAGMA_OMP_TARGET_ATOMIC_WRITE` | <br> `_Pragma("acc atomic write")` <br> `_Pragma("omp atomic write")` | <br> OpenACC <br> OpenMP |
  | **`ATOMIC_CAPTURE`** <br> `PRAGMA_ACC_ATOMIC_CAPTURE` <br> `PRAGMA_OMP_TARGET_ATOMIC_CAPTURE` | <br> `_Pragma("acc atomic capture")` <br> `_Pragma("omp atomic capture")` | <br> OpenACC <br> OpenMP |

  </details>

  * <details><summary>抽象化マクロ</summary>

    | 入力 | （中間）出力 | バックエンドとして用いる指示文 |
    | ---- | ---- | ---- |
    | `PRAGMA_ACC_LAUNCH_DEFAULT(...)` <br> `PRAGMA_OMP_TARGET_LAUNCH_DEFAULT(...)` | `PRAGMA_ACC_KERNELS(__VA_ARGS__)` <br> `PRAGMA_ACC_PARALLEL(__VA_ARGS__)` <br> `PRAGMA_OMP_TARGET_TEAMS(__VA_ARGS__)` | OpenACC (kernels) <br> OpenACC (parallel) <br> OpenMP |
    | `PRAGMA_ACC_OFFLOADING_DEFAULT(...)` <br> `PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(...)` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__) PRAGMA_ACC_LOOP(__VA_ARGS__)` <br> `PRAGMA_OMP_TARGET_TEAMS_LOOP(__VA_ARGS__)` <br> `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(__VA_ARGS__)` | OpenACC <br> OpenMP (loop) <br> OpenMP (distribute) |

    </details>

  * <details><summary>OpenACC</summary>

    | 入力 | OpenACC 使用時の出力 | OpenMP target 使用時の出力 |
    | ---- | ---- | ---- |
    | `PRAGMA_ACC_PARALLEL(...)` | `_Pragma("acc parallel __VA_ARGS__")` | `PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(__VA_ARGS__)` |
    | `PRAGMA_ACC_KERNELS(...)` | `_Pragma("acc kernels __VA_ARGS__")` | `PRAGMA_OMP_TARGET_OFFLOADING_DEFAULT(__VA_ARGS__)` |
    | `PRAGMA_ACC_SERIAL(...)` | `_Pragma("acc serial __VA_ARGS__")` | N/A（OpenMP target 使用時には無視される） |
    | `PRAGMA_ACC_LOOP(...)` | `_Pragma("acc loop __VA_ARGS__")` | N/A（OpenMP target 使用時には無視される） |
    | `PRAGMA_ACC_CACHE(...)` | `_Pragma("acc cache(__VA_ARGS__)")` | N/A（OpenMP target 使用時には無視される） |
    | `PRAGMA_ACC_ATOMIC(...)` | `_Pragma("acc atomic __VA_ARGS__")` | `PRAGMA_OMP_TARGET_ATOMIC(__VA_ARGS__)` |
    | `PRAGMA_ACC_WAIT(...)` | `_Pragma("acc wait __VA_ARGS__")` | `PRAGMA_OMP_TARGET_TASKWAIT(__VA_ARGS__)` |
    | `PRAGMA_ACC_ROUTINE(...)` | `_Pragma("acc routine __VA_ARGS__")` | `PRAGMA_OMP_DECLARE_TARGET(__VA_ARGS__)` |
    | `PRAGMA_ACC_DECLARE(...)` | `_Pragma("acc declare __VA_ARGS__")` | N/A（OpenMP target 使用時には無視される） |

    </details>

  * <details><summary>OpenMP target</summary>

    | 入力 | OpenMP target 使用時の出力 | OpenACC 使用時の出力 | 縮退モード（演算加速器を用いないCPU実行）での出力 |
    | ---- | ---- | ---- | ---- |
    | `PRAGMA_OMP_TARGET(...)` | `_Pragma("omp target __VA_ARGS__")` | `PRAGMA_ACC(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_TARGET_PARALLEL(...)` | `_Pragma("omp target parallel __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_FOR(...)` | `_Pragma("omp target parallel for __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_FOR(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_FOR_SIMD(...)` | `_Pragma("omp target parallel for simd __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_FOR_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_LOOP(...)` | `_Pragma("omp target parallel loop __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_LOOP(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_SIMD(...)` | `_Pragma("omp target simd __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS(...)` | `_Pragma("omp target teams __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)` | `_Pragma("omp target teams distribute __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_DISTRIBUTE(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...)` | `_Pragma("omp target teams distribute simd __VA_ARGS__")` | `PRAGMA_ACC_LAUNCH_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_DISTRIBUTE_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_LOOP(...)` | `_Pragma("omp target teams loop __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_LOOP(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR(...)` | `_Pragma("omp target teams distribute parallel for __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_FOR(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...)` | `_Pragma("omp target teams distribute parallel for simd __VA_ARGS__")` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_ATOMIC(...)` | `PRAGMA_OMP_ATOMIC(__VA_ARGS__)` | `PRAGMA_ACC_ATOMIC(__VA_ARGS__)` | `PRAGMA_OMP_ATOMIC(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TASKWAIT(...)` | `PRAGMA_OMP_TASKWAIT(__VA_ARGS__)` | `PRAGMA_ACC_WAIT(__VA_ARGS__)` | `PRAGMA_OMP_TASKWAIT(__VA_ARGS__)` |
    | `PRAGMA_OMP_DECLARE_TARGET(...)` | `_Pragma("omp declare target __VA_ARGS__")` | `PRAGMA_ACC_ROUTINE(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_BEGIN_DECLARE_TARGET(...)` | `_Pragma("omp begin declare target __VA_ARGS__")` | `PRAGMA_ACC_ROUTINE(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_END_DECLARE_TARGET` | `_Pragma("omp end declare target")` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） |

    </details>

  * <details><summary>OpenMP</summary>

    | 入力 | 出力 |
    | ---- | ---- |
    | `PRAGMA_OMP_THREADPRIVATE(...)` | `_Pragma("omp threadprivate(__VA_ARGS__)")` |
    | `PRAGMA_OMP_SCAN(...)` | `_Pragma("omp scan __VA_ARGS__")` |
    | `PRAGMA_OMP_DECLARE_SIMD(...)` | `_Pragma("omp declare simd __VA_ARGS__")` |
    | `PRAGMA_OMP_TILE(...)` | `_Pragma("omp tile __VA_ARGS__")` |
    | `PRAGMA_OMP_UNROLL(...)` | `_Pragma("omp unroll __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL(...)` | `_Pragma("omp parallel __VA_ARGS__")` |
    | `PRAGMA_OMP_TEAMS(...)` | `_Pragma("omp teams __VA_ARGS__")` |
    | `PRAGMA_OMP_SIMD(...)` | `_Pragma("omp simd __VA_ARGS__")` |
    | `PRAGMA_OMP_MASKED(...)` | `_Pragma("omp masked __VA_ARGS__")` |
    | `PRAGMA_OMP_SINGLE(...)` | `_Pragma("omp single __VA_ARGS__")` |
    | `PRAGMA_OMP_WORKSHARE(...)` | `_Pragma("omp workshare __VA_ARGS__")` |
    | `PRAGMA_OMP_SCOPE(...)` | `_Pragma("omp scope __VA_ARGS__")` |
    | `PRAGMA_OMP_SECTIONS(...)` | `_Pragma("omp sections __VA_ARGS__")` |
    | `PRAGMA_OMP_SECTION` | `_Pragma("omp section")` |
    | `PRAGMA_OMP_FOR(...)` | `_Pragma("omp for __VA_ARGS__")` |
    | `PRAGMA_OMP_DISTRIBUTE(...)` | `_Pragma("omp distribute __VA_ARGS__")` |
    | `PRAGMA_OMP_LOOP(...)` | `_Pragma("omp loop __VA_ARGS__")` |
    | `PRAGMA_OMP_TASK(...)` | `_Pragma("omp task __VA_ARGS__")` |
    | `PRAGMA_OMP_TASKLOOP(...)` | `_Pragma("omp taskloop __VA_ARGS__")` |
    | `PRAGMA_OMP_TASKYIELD` | `_Pragma("omp taskyield")` |
    | `PRAGMA_OMP_INTEROP(...)` | `_Pragma("omp interop __VA_ARGS__")` |
    | `PRAGMA_OMP_CRITICAL(...)` | `_Pragma("omp critical __VA_ARGS__")` |
    | `PRAGMA_OMP_BARRIER` | `_Pragma("omp barrier")` |
    | `PRAGMA_OMP_TASKGROUP(...)` | `_Pragma("omp taskgroup __VA_ARGS__")` |
    | `PRAGMA_OMP_TASKWAIT(...)` | `_Pragma("omp taskwait __VA_ARGS__")` |
    | `PRAGMA_OMP_FLUSH(...)` | `_Pragma("omp flush __VA_ARGS__")` |
    | `PRAGMA_OMP_DEPOBJ(...)` | `_Pragma("omp depobj __VA_ARGS__")` |
    | `PRAGMA_OMP_ATOMIC(...)` | `_Pragma("omp atomic __VA_ARGS__")` |
    | `PRAGMA_OMP_ORDERED(...)` | `_Pragma("omp ordered __VA_ARGS__")` |
    | `PRAGMA_OMP_FOR_SIMD(...)` | `_Pragma("omp for simd __VA_ARGS__")` |
    | `PRAGMA_OMP_DISTRIBUTE_SIMD(...)` | `_Pragma("omp distribute simd __VA_ARGS__")` |
    | `PRAGMA_OMP_DISTRIBUTE_PARALLEL_FOR(...)` | `_Pragma("omp distribute parallel for __VA_ARGS__")` |
    | `PRAGMA_OMP_DISTRIBUTE_PARALLEL_FOR_SIMD(...)` | `_Pragma("omp distribute parallel for simd __VA_ARGS__")` |
    | `PRAGMA_OMP_TASKLOOP_SIMD(...)` | `_Pragma("omp taskloop simd __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL_FOR(...)` | `_Pragma("omp parallel for __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL_LOOP(...)` | `_Pragma("omp parallel loop __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL_SECTIONS(...)` | `_Pragma("omp parallel sections __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL_FOR_SIMD(...)` | `_Pragma("omp parallel for simd __VA_ARGS__")` |
    | `PRAGMA_OMP_MASKED_TASKLOOP(...)` | `_Pragma("omp masked taskloop __VA_ARGS__")` |
    | `PRAGMA_OMP_MASKED_TASKLOOP_SIMD(...)` | `_Pragma("omp masked taskloop simd __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL_MASKED_TASKLOOP(...)` | `_Pragma("omp parallel masked taskloop __VA_ARGS__")` |
    | `PRAGMA_OMP_PARALLEL_MASKED_TASKLOOP_SIMD(...)` | `_Pragma("omp parallel masked taskloop simd __VA_ARGS__")` |
    | `PRAGMA_OMP_TEAMS_DISTRIBUTE(...)` | `_Pragma("omp teams distribute __VA_ARGS__")` |
    | `PRAGMA_OMP_TEAMS_DISTRIBUTE_SIMD(...)` | `_Pragma("omp teams distribute simd __VA_ARGS__")` |
    | `PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_FOR(...)` | `_Pragma("omp teams distribute parallel for __VA_ARGS__")` |
    | `PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_FOR_SIMD(...)` | `_Pragma("omp teams distribute parallel for simd __VA_ARGS__")` |
    | `PRAGMA_OMP_TEAMS_LOOP(...)` | `_Pragma("omp teams loop __VA_ARGS__")` |

    </details>

* <details><summary>メモリ操作，データ転送</summary>

  | 入力 | 出力 | バックエンドとして用いる指示文 |
  | ---- | ---- | ---- |
  | **`MALLOC_ON_DEVICE(...)`** <br> `PRAGMA_ACC_ENTER_DATA_CREATE(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA_MAP_ALLOC(...)` | <br> `_Pragma("acc enter data create(__VA_ARGS__)")` <br> `_Pragma("omp target enter data map(alloc: __VA_ARGS__)")` | <br> OpenACC <br> OpenMP |
  | **`FREE_FROM_DEVICE(...)`** <br> `PRAGMA_ACC_EXIT_DATA_DELETE(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA_MAP_DELETE(...)` | <br> `_Pragma("acc exit data delete(__VA_ARGS__)")` <br> `_Pragma("omp target exit data map(delete: __VA_ARGS__)")` | <br> OpenACC <br> OpenMP |
  | **`MEMCPY_D2H(...)`** <br> `PRAGMA_ACC_UPDATE_HOST(...)` <br> `PRAGMA_OMP_TARGET_UPDATE_FROM(...)` | <br> `_Pragma("acc update host(__VA_ARGS__)")` <br> `_Pragma("omp target update from(__VA_ARGS__)")` | <br> OpenACC <br> OpenMP |
  | **`MEMCPY_H2D(...)`** <br> `PRAGMA_ACC_UPDATE_DEVICE(...)` <br> `PRAGMA_OMP_TARGET_UPDATE_TO(...)` | <br> `_Pragma("acc update device(__VA_ARGS__)")` <br> `_Pragma("omp target update to(__VA_ARGS__)")` | <br> OpenACC <br> OpenMP |
  | `PRAGMA_ACC_ENTER_DATA(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA(...)` | `_Pragma("acc enter data __VA_ARGS__")` <br> `_Pragma("omp target enter data __VA_ARGS__")` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_ENTER_DATA_COPYIN(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA_MAP_TO(...)` | `_Pragma("acc enter data copyin(__VA_ARGS__)")` <br> `_Pragma("omp target enter data map(to: __VA_ARGS__)")` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_EXIT_DATA(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA(...)` | `_Pragma("acc exit data __VA_ARGS__")` <br> `_Pragma("omp target exit data __VA_ARGS__")` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_EXIT_DATA_COPYOUT(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA_MAP_FROM(...)` | `_Pragma("acc exit data copyout(__VA_ARGS__)")` <br> `_Pragma("omp target exit data map(from: __VA_ARGS__)")` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_UPDATE(...)` <br> `PRAGMA_OMP_TARGET_UPDATE(...)` | `_Pragma("acc update __VA_ARGS__")` <br> `_Pragma("omp target update __VA_ARGS__")` | OpenACC <br> OpenMP |
  | **`DATA_ACCESS_BY_DEVICE(...)`** <br> `PRAGMA_ACC_DATA(...)` <br> `PRAGMA_OMP_TARGET_DATA(...)` | <br> `_Pragma("acc data __VA_ARGS__")` <br> `_Pragma("omp target data __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`DATA_ACCESS_BY_HOST(...)`** <br> `PRAGMA_ACC_HOST_DATA(...)` <br> `PRAGMA_OMP_TARGET_DATA(...)` | <br> `_Pragma("acc host_data __VA_ARGS__")` <br> `_Pragma("omp target data __VA_ARGS__")` | <br> OpenACC <br> OpenMP |
  | **`USE_DEVICE_DATA_FROM_HOST(...)`** <br> `PRAGMA_ACC_HOST_DATA_USE_DEVICE(...)` <br> `PRAGMA_OMP_TARGET_DATA_USE_DEVICE_PTR(...)` | <br> `_Pragma("acc host_data use_device(__VA_ARGS__)")` <br> `_Pragma("omp target data use_device_ptr(__VA_ARGS__)")` | <br> OpenACC <br> OpenMP |
  | **`DECLARE_DATA_ON_DEVICE(...)`** <br> `PRAGMA_ACC_DATA_PRESENT(...)` | <br> `_Pragma("acc data present(__VA_ARGS__)")` | <br> OpenACC (only) |

  </details>

  * <details><summary>OpenACC</summary>

    | 入力 | 出力 | OpenMP target 使用時の出力 |
    | ---- | ---- | ---- |
    | `PRAGMA_ACC_DATA(...)` | `_Pragma("acc data __VA_ARGS__")` | `PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_ENTER_DATA(...)` | `_Pragma("acc enter data __VA_ARGS__")` | `PRAGMA_OMP_TARGET_ENTER_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_EXIT_DATA(...)` | `_Pragma("acc exit data __VA_ARGS__")` | `PRAGMA_OMP_TARGET_EXIT_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_HOST_DATA(...)` | `_Pragma("acc host_data __VA_ARGS__")` | `PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_UPDATE(...)` | `_Pragma("acc update __VA_ARGS__")` | `PRAGMA_OMP_TARGET_UPDATE(__VA_ARGS__)` |

    </details>

  * <details><summary>OpenMP target</summary>

    | 入力 | 出力 | OpenACC 使用時の出力 | 縮退モード（演算加速器を用いないCPU実行）での出力 |
    | ---- | ---- | ---- | ---- |
    | `PRAGMA_OMP_TARGET_DATA(...)` | `_Pragma("omp target data __VA_ARGS__")` | `PRAGMA_ACC_DATA(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_TARGET_ENTER_DATA(...)` | `_Pragma("omp target enter data __VA_ARGS__")` | `PRAGMA_ACC_ENTER_DATA(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_TARGET_EXIT_DATA(...)` | `_Pragma("omp target exit data __VA_ARGS__")` | `PRAGMA_ACC_EXIT_DATA(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_TARGET_UPDATE(...)` | `_Pragma("omp target update __VA_ARGS__")` | `PRAGMA_ACC_UPDATE(__VA_ARGS__)` | N/A（縮退モードでは無視される） |

    </details>

### 使用可能な指示節・指示句

* <details><summary>簡易記法</summary>

  | 入力 | 出力 | バックエンドとして用いる指示文 |
  | ---- | ---- | ---- |
  | **`AS_INDEPENDENT`** <br> `ACC_CLAUSE_INDEPENDENT` <br> `OMP_TARGET_CLAUSE_SIMD` | <br> `independent` <br> `simd` | <br> OpenACC <br> OpenMP |
  | **`AS_SEQUENTIAL`** <br> `ACC_CLAUSE_SEQ` | <br> `seq` | <br> OpenACC (only) |
  | **`NUM_THREADS(n)`** <br> `ACC_CLAUSE_VECTOR_LENGTH(n)` <br> `OMP_TARGET_CLAUSE_THREAD_LIMIT(n)` | <br> `vector_length(n)` <br> `thread_limit(n)` | <br> OpenACC <br> OpenMP |
  | **`NUM_BLOCKS(n)`** <br> `ACC_CLAUSE_NUM_WORKERS(n)` <br> `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` | <br> `num_workers(n)` <br> `num_teams(n)` | <br> OpenACC <br> OpenMP |
  | **`NUM_GRIDS(n)`** <br> `ACC_CLAUSE_NUM_GANGS(n)` | <br> `num_gang(n)` | <br> OpenACC (only) |
  | **`AS_THREAD`** <br> `ACC_CLAUSE_VECTOR` | <br> `vector` | <br> OpenACC (only) |
  | **`AS_BLOCK`** <br> `ACC_CLAUSE_WORKER` | <br> `worker` | <br> OpenACC (only) |
  | **`AS_GRID`** <br> `ACC_CLAUSE_GANG` | <br> `gang` | <br> OpenACC (only) |
  | **`COLLAPSE(n)`** <br> `ACC_CLAUSE_COLLAPSE(n)` <br> `OMP_TARGET_CLAUSE_COLLAPSE(n)` | <br> `collapse(n)` <br> `collapse(n)` | <br> OpenACC <br> OpenMP |
  | **`AS_ASYNC(...)`** <br> `ACC_CLAUSE_ASYNC(...)` <br> `OMP_TARGET_CLAUSE_NOWAIT` | <br> `async(__VA_ARGS__)` <br> `nowait` | <br> OpenACC <br> OpenMP |
  | **`ASYNC_QUEUE(id)`** <br> `ACC_CLAUSE_ASYNC(id)` | <br> `async(id)` | <br> OpenACC (only) |
  | **`REDUCTION(...)`** <br> `ACC_CLAUSE_REDUCTION(...)` <br> `OMP_TARGET_CLAUSE_REDUCTION(...)` | <br> `reduction(__VA_ARGS__)` <br> `reduction(__VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`ENABLE_IF(condition)`** <br> `ACC_CLAUSE_IF(condition)` <br> `OMP_TARGET_CLAUSE_IF(condition)` | <br> `if(condition)` <br> `if(condition)` | <br> OpenACC <br> OpenMP |
  | **`AS_PRIVATE(...)`** <br> `ACC_CLAUSE_PRIVATE(...)` <br> `OMP_TARGET_CLAUSE_PRIVATE(...)` | <br> `private(__VA_ARGS__)` <br> `private(__VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`AS_FIRSTPRIVATE(...)`** <br> `ACC_CLAUSE_FIRSTPRIVATE(...)` <br> `OMP_TARGET_CLAUSE_FIRSTPRIVATE(...)` | <br> `firstprivate(__VA_ARGS__)` <br> `firstprivate(__VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`AS_DEVICE_PTR(...)`** <br> `ACC_CLAUSE_DEVICEPTR(...)` <br> `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(...)` | <br> `deviceptr(__VA_ARGS__)` <br> `is_device_ptr(__VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`COPY_BEFORE_AND_AFTER_EXEC(...)`** <br> `ACC_CLAUSE_COPY(...)` <br> `OMP_TARGET_CLAUSE_MAP_TOFROM(...)` | <br> `copy(__VA_ARGS__)` <br> `map(tofrom: __VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`COPY_H2D_BEFORE_EXEC(...)`** <br> `ACC_CLAUSE_COPYIN(...)` <br> `OMP_TARGET_CLAUSE_MAP_TO(...)` | <br> `copyin(__VA_ARGS__)` <br> `map(to: __VA_ARGS__)` | <br> OpenACC <br> OpenMP |
  | **`COPY_D2H_AFTER_EXEC(...)`** <br> `ACC_CLAUSE_COPYOUT(...)` <br> `OMP_TARGET_CLAUSE_MAP_FROM(...)` | <br> `copyout(__VA_ARGS__)` <br> `map(from: __VA_ARGS__)` | <br> OpenACC <br> OpenMP |

  </details>

* <details><summary>OpenACC</summary>

  | 入力 | 出力 | OpenMP target 使用時の出力 | 備考 |
  | ---- | ---- | ---- | ---- |
  | `ACC_CLAUSE_IF(condition)` | `if(condition)` | `OMP_TARGET_CLAUSE_IF(condition)` ||
  | `ACC_CLAUSE_SELF(...)` | `self(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_DEFAULT(mode)` | `default(mode)` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_DEFAULT_NONE` | `default(none)` | `OMP_TARGET_CLAUSE_DEFAULTMAP_NONE` ||
  | `ACC_CLAUSE_DEFAULT_PRESENT` | `default(present)` | `OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT` ||
  | `ACC_CLAUSE_DEVICE_TYPE(...)` | `device_type(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_DEVICE_TYPE(__VA_ARGS__)` ||
  | `ACC_CLAUSE_ASYNC(...)` | `async(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_NOWAIT` ||
  | `ACC_CLAUSE_WAIT(...)` | `wait(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_DEPEND_IN(__VA_ARGS__)` ||
  | `ACC_CLAUSE_FINALIZE` | `finalize` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_NUM_GANGS(n)` | `num_gangs(n)` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_NUM_WORKERS(n)` | `num_workers(n)` | `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` ||
  | `ACC_CLAUSE_VECTOR_LENGTH(n)` | `vector_length(n)` | `OMP_TARGET_CLAUSE_THREAD_LIMIT(n)` ||
  | `ACC_CLAUSE_REDUCTION(...)` | `reduction(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_REDUCTION(__VA_ARGS__)` ||
  | `ACC_CLAUSE_PRIVATE(...)` | `private(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_PRIVATE(__VA_ARGS__)` ||
  | `ACC_CLAUSE_FIRSTPRIVATE(...)` | `firstprivate(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` ||
  | `ACC_CLAUSE_COPY(...)` | `copy(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_TOFROM(__VA_ARGS__)` ||
  | `ACC_CLAUSE_COPYIN(...)` | `copyin(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_TO(__VA_ARGS__)` ||
  | `ACC_CLAUSE_COPYOUT(...)` | `copyout(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_FROM(__VA_ARGS__)` ||
  | `ACC_CLAUSE_CREATE(...)` | `create(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_ALLOC(__VA_ARGS__)` ||
  | `ACC_CLAUSE_NO_CREATE(...)` | `no_create(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_DELETE(...)` | `delete(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_DELETE(__VA_ARGS__)` ||
  | `ACC_CLAUSE_PRESENT(...)` | `present(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_DEVICEPTR(...)` | `deviceptr(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(__VA_ARGS__)` ||
  | `ACC_CLAUSE_ATTACH(...)` | `attach(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_DETACH(...)` | `detach(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_USE_DEVICE(...)` | `use_device(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_USE_DEVICE_PTR(__VA_ARGS__)` ||
  | `ACC_CLAUSE_IF_PRESENT` | `if_present` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_COLLAPSE(n)` | `collapse(n)` | `OMP_TARGET_CLAUSE_COLLAPSE(n)` ||
  | `ACC_CLAUSE_SEQ` | `seq` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_AUTO` | `auto` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_INDEPENDENT` | `independent` | `OMP_TARGET_CLAUSE_SIMD` ||
  | `ACC_CLAUSE_TILE(...)` | `tile(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_GANG` | `gang` | N/A（OpenMP target 使用時には無視される） | `gang(n)` という記法は提供しない（`ACC_CLAUSE_NUM_GANGS(n)`が使用可能） |
  | `ACC_CLAUSE_WORKER` | `worker` | N/A（OpenMP target 使用時には無視される） | `worker(n)` という記法は提供しない（`ACC_CLAUSE_NUM_WORKERS(n)`が使用可能） |
  | `ACC_CLAUSE_VECTOR` | `vector` | N/A（OpenMP target 使用時には無視される） | `vector(n)` という記法は提供しない（`ACC_CLAUSE_VECTOR_LENGTH(n)`が使用可能） |
  | `ACC_CLAUSE_READ` | `read` | `OMP_TARGET_CLAUSE_READ` ||
  | `ACC_CLAUSE_WRITE` | `write` | `OMP_TARGET_CLAUSE_WRITE` ||
  | `ACC_CLAUSE_UPDATE` | `update` | `OMP_TARGET_CLAUSE_UPDATE` ||
  | `ACC_CLAUSE_CAPTURE` | `capture` | `OMP_TARGET_CLAUSE_CAPTURE` ||
  | `ACC_CLAUSE_HOST(...)` | `host(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_FROM(__VA_ARGS__)` ||
  | `ACC_CLAUSE_DEVICE(...)` | `device(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_TO(__VA_ARGS__)` ||
  | `ACC_PASS_LIST(...)` | `(__VA_ARGS__)` | `OMP_TARGET_PASS_LIST(__VA_ARGS__)` ||
  | `ACC_CLAUSE_BIND(arg)` | `bind(arg)` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_NOHOST` | `nohost` | `OMP_TARGET_CLAUSE_DEVICE_TYPE(nohost)` ||
  | `ACC_CLAUSE_DEVICE_RESIDENT(...)` | `device_resident(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） ||
  | `ACC_CLAUSE_LINK(...)` | `link(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） ||

  </details>

* <details><summary>OpenMP target</summary>

  | 入力 | 出力 | OpenACC 使用時の出力 | 縮退モード（演算加速器を用いないCPU実行）での出力 |
  | ---- | ---- | ---- | ---- |
  | `OMP_TARGET_CLAUSE_ALIGNED(...)` | `OMP_CLAUSE_ALIGNED(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ALIGNED(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_SIMDLEN(length)` | `OMP_CLAUSE_SIMDLEN(length)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SIMDLEN(length)` |
  | `OMP_TARGET_CLAUSE_DEVICE_TYPE(type)` | `device_type(type)` | `ACC_CLAUSE_DEVICE_TYPE(type)` | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_ENTER(...)` | `enter(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_INDIRECT(...)` | `indirect(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_LINK(...)` | `link(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_COPYIN(...)` | `OMP_CLAUSE_COPYIN(__VA_ARGS__)` | `ACC_CLAUSE_COPYIN(__VA_ARGS__)` | `OMP_CLAUSE_COPYIN(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_NUM_THREADS(nthreads)` | `OMP_CLAUSE_NUM_THREADS(nthreads)` | `ACC_CLAUSE_VECTOR_LENGTH(nthreads)` | `OMP_CLAUSE_NUM_THREADS(nthreads)` |
  | `OMP_TARGET_CLAUSE_PROC_BIND(attr)` | `OMP_CLAUSE_PROC_BIND(attr)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_PROC_BIND(attr)` |
  | `OMP_TARGET_CLAUSE_NUM_TEAMS(...)` | `OMP_CLAUSE_NUM_TEAMS(__VA_ARGS__)` | `ACC_CLAUSE_NUM_WORKERS(__VA_ARGS__)` | `OMP_CLAUSE_NUM_TEAMS(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_THREAD_LIMIT(num)` | `OMP_CLAUSE_THREAD_LIMIT(num)` | `ACC_CLAUSE_VECTOR_LENGTH(num)` | `OMP_CLAUSE_THREAD_LIMIT(num)` |
  | `OMP_TARGET_CLAUSE_NONTEMPORAL(...)` | `OMP_CLAUSE_NONTEMPORAL(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_NONTEMPORAL(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_SAFELEN(length)` | `OMP_CLAUSE_SAFELEN(length)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SAFELEN(length)` |
  | `OMP_TARGET_CLAUSE_ORDERED(...)` | `OMP_CLAUSE_ORDERED(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ORDERED(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_SCHEDULE(...)` | `OMP_CLAUSE_SCHEDULE(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SCHEDULE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_DIST_SCHEDULE(...)` | `OMP_CLAUSE_DIST_SCHEDULE(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DIST_SCHEDULE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_BIND(binding)` | `OMP_CLAUSE_BIND(binding)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_BIND(binding)` |
  | `OMP_TARGET_CLAUSE_USE_DEVICE_PTR(...)` | `use_device_ptr(__VA_ARGS__)` | `ACC_CLAUSE_USE_DEVICE(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_USE_DEVICE_ADDR(...)` | `use_device_addr(__VA_ARGS__)` | `ACC_CLAUSE_USE_DEVICE(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP(...)` | `defaultmap(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP_NONE` | `OMP_TARGET_CLAUSE_DEFAULTMAP(none)` | `ACC_CLAUSE_DEFAULT_NONE` | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT` | `OMP_TARGET_CLAUSE_DEFAULTMAP(present)` | `ACC_CLAUSE_DEFAULT_PRESENT` | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_HAS_DEVICE_ADDR(...)` | `has_device_addr(__VA_ARGS__)` | `ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(...)` | `is_device_ptr(__VA_ARGS__)` | `ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_USES_ALLOCATORS(...)` | `uses_allocators(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_FROM(...)` | `from(__VA_ARGS__)` | `ACC_CLAUSE_HOST(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_CLAUSE_TO(...)` | `to(__VA_ARGS__)` | `ACC_CLAUSE_DEVICE(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
  | `OMP_TARGET_PASS_LIST(...)` | `OMP_PASS_LIST(__VA_ARGS__)` | `ACC_PASS_LIST(__VA_ARGS__)` | `OMP_PASS_LIST(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_SEQ_CST` | `OMP_CLAUSE_SEQ_CST` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SEQ_CST` |
  | `OMP_TARGET_CLAUSE_ACQ_REL` | `OMP_CLAUSE_ACQ_REL` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ACQ_REL` |
  | `OMP_TARGET_CLAUSE_RELEASE` | `OMP_CLAUSE_RELEASE` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_RELEASE` |
  | `OMP_TARGET_CLAUSE_ACQUIRE` | `OMP_CLAUSE_ACQUIRE` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ACQUIRE` |
  | `OMP_TARGET_CLAUSE_RELAXED` | `OMP_CLAUSE_RELAXED` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_RELAXED` |
  | `OMP_TARGET_CLAUSE_READ` | `OMP_CLAUSE_READ` | `ACC_CLAUSE_READ` | `OMP_CLAUSE_READ` |
  | `OMP_TARGET_CLAUSE_WRITE` | `OMP_CLAUSE_WRITE` | `ACC_CLAUSE_WRITE` | `OMP_CLAUSE_WRITE` |
  | `OMP_TARGET_CLAUSE_UPDATE` | `OMP_CLAUSE_UPDATE` | `ACC_CLAUSE_UPDATE` | `OMP_CLAUSE_UPDATE` |
  | `OMP_TARGET_CLAUSE_CAPTURE` | `OMP_CLAUSE_CAPTURE` | `ACC_CLAUSE_CAPTURE` | `OMP_CLAUSE_CAPTURE` |
  | `OMP_TARGET_CLAUSE_COMPARE` | `OMP_CLAUSE_COMPARE` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_COMPARE` |
  | `OMP_TARGET_CLAUSE_FAIL(...)` | `OMP_CLAUSE_FAIL(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_FAIL(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_WEAK` | `OMP_CLAUSE_WEAK` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_WEAK` |
  | `OMP_TARGET_CLAUSE_HINT(expression)` | `OMP_CLAUSE_HINT(expression)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_HINT(expression)` |
  | `OMP_TARGET_CLAUSE_SIMD` | `OMP_CLAUSE_SIMD` | `ACC_CLAUSE_INDEPENDENT` | `OMP_CLAUSE_SIMD` |
  | `OMP_TARGET_CLAUSE_DEFAULT_SHARED` | `OMP_CLAUSE_DEFAULT_SHARED` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DEFAULT_SHARED` |
  | `OMP_TARGET_CLAUSE_DEFAULT_FIRSTPRIVATE` | `OMP_CLAUSE_DEFAULT_FIRSTPRIVATE` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DEFAULT_FIRSTPRIVATE` |
  | `OMP_TARGET_CLAUSE_DEFAULT_PRIVATE` | `OMP_CLAUSE_DEFAULT_PRIVATE` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DEFAULT_PRIVATE` |
  | `OMP_TARGET_CLAUSE_DEFAULT_NONE` | `OMP_CLAUSE_DEFAULT_NONE` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DEFAULT_NONE` |
  | `OMP_TARGET_CLAUSE_SHARED(...)` | `OMP_CLAUSE_SHARED(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SHARED(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_PRIVATE(...)` | `OMP_CLAUSE_PRIVATE(__VA_ARGS__)` | `ACC_CLAUSE_PRIVATE(__VA_ARGS__)` | `OMP_CLAUSE_PRIVATE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_FIRSTPRIVATE(...)` | `OMP_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | `ACC_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | `OMP_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_LASTPRIVATE(...)` | `OMP_CLAUSE_LASTPRIVATE(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_LASTPRIVATE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_LINEAR(...)` | `OMP_CLAUSE_LINEAR(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_LINEAR(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_ALLOCATE(...)` | `OMP_CLAUSE_ALLOCATE(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ALLOCATE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_COLLAPSE(n)` | `OMP_CLAUSE_COLLAPSE(n)` | `ACC_CLAUSE_COLLAPSE(n)` | `OMP_CLAUSE_COLLAPSE(n)` |
  | `OMP_TARGET_CLAUSE_DEPEND(...)` | `OMP_CLAUSE_DEPEND(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DEPEND(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_DEPEND_IN(...)` | `OMP_CLAUSE_DEPEND_IN(__VA_ARGS__)` | `ACC_CLAUSE_WAIT(__VA_ARGS__)` | `OMP_CLAUSE_DEPEND_IN(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_DEVICE(...)` | `device(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `device(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(condition)` | `ACC_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(condition)` |
  | `OMP_TARGET_CLAUSE_IF_TARGET(condition)` | `OMP_CLAUSE_IF(target : condition)` | `ACC_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(target : condition)` |
  | `OMP_TARGET_CLAUSE_MAP(...)` | `OMP_CLAUSE_MAP(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_MAP(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_MAP_ALLOC(...)` | `OMP_CLAUSE_MAP_ALLOC(__VA_ARGS__)` | `ACC_CLAUSE_CREATE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_ALLOC(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_MAP_TO(...)` | `OMP_CLAUSE_MAP_TO(__VA_ARGS__)` | `ACC_CLAUSE_COPYIN(__VA_ARGS__)` | `OMP_CLAUSE_MAP_TO(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_MAP_FROM(...)` | `OMP_CLAUSE_MAP_FROM(__VA_ARGS__)` | `ACC_CLAUSE_COPYOUT(__VA_ARGS__)` | `OMP_CLAUSE_MAP_FROM(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_MAP_TOFROM(...)` | `OMP_CLAUSE_MAP_TOFROM(__VA_ARGS__)` | `ACC_CLAUSE_COPY(__VA_ARGS__)` | `OMP_CLAUSE_MAP_TOFROM(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_MAP_RELEASE(...)` | `OMP_CLAUSE_MAP_RELEASE(__VA_ARGS__)` | `ACC_CLAUSE_DELETE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_RELEASE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_MAP_DELETE(...)` | `OMP_CLAUSE_MAP_DELETE(__VA_ARGS__)` | `ACC_CLAUSE_DELETE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_DELETE(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_ORDER(...)` | `OMP_CLAUSE_ORDER(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ORDER(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_NOWAIT` | `OMP_CLAUSE_NOWAIT` | `ACC_CLAUSE_ASYNC()` | `OMP_CLAUSE_NOWAIT` |
  | `OMP_TARGET_CLAUSE_REDUCTION(...)` | `OMP_CLAUSE_REDUCTION(__VA_ARGS__)` | `ACC_CLAUSE_REDUCTION(__VA_ARGS__)` | `OMP_CLAUSE_REDUCTION(__VA_ARGS__)` |
  | `OMP_TARGET_CLAUSE_IN_REDUCTION(...)` | `OMP_CLAUSE_IN_REDUCTION(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_IN_REDUCTION(__VA_ARGS__)` |

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
  | `OMP_CLAUSE_PARTIAL(...)` | `partial(__VA_ARGS__)` |
  | `OMP_CLAUSE_COPYIN(...)` | `copyin(__VA_ARGS__)` |
  | `OMP_CLAUSE_NUM_THREADS(nthreads)` | `num_threads(nthreads)` |
  | `OMP_CLAUSE_PROC_BIND(attr)` | `proc_bind(attr)` |
  | `OMP_CLAUSE_NUM_TEAMS(...)` | `num_teams(__VA_ARGS__)` |
  | `OMP_CLAUSE_THREAD_LIMIT(num)` | `thread_limit(num)` |
  | `OMP_CLAUSE_NONTEMPORAL(...)` | `nontemporal(__VA_ARGS__)` |
  | `OMP_CLAUSE_SAFELEN(length)` | `safelen(length)` |
  | `OMP_CLAUSE_FILTER(thread_num)` | `filter(thread_num)` |
  | `OMP_CLAUSE_COPYPRIVATE(...)` | `copyprivate(__VA_ARGS__)` |
  | `OMP_CLAUSE_ORDERED(...)` | `ordered(__VA_ARGS__)` |
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
  | `OMP_CLAUSE_DESTROY(...)` | `destroy(__VA_ARGS__)` |
  | `OMP_PASS_LIST(...)` | `(__VA_ARGS__)` |
  | `OMP_CLAUSE_SEQ_CST` | `seq_cst` |
  | `OMP_CLAUSE_ACQ_REL` | `acq_rel` |
  | `OMP_CLAUSE_RELEASE` | `release` |
  | `OMP_CLAUSE_ACQUIRE` | `acquire` |
  | `OMP_CLAUSE_RELAXED` | `relaxed` |
  | `OMP_CLAUSE_READ` | `read` |
  | `OMP_CLAUSE_WRITE` | `write` |
  | `OMP_CLAUSE_UPDATE` | `update` |
  | `OMP_CLAUSE_CAPTURE` | `capture` |
  | `OMP_CLAUSE_COMPARE` | `compare` |
  | `OMP_CLAUSE_FAIL(...)` | `fail(__VA_ARGS__)` |
  | `OMP_CLAUSE_WEAK` | `weak` |
  | `OMP_CLAUSE_HINT(expression)` | `hint(expression)` |
  | `OMP_CLAUSE_THREADS` | `threads` |
  | `OMP_CLAUSE_SIMD` | `simd` |
  | `OMP_CLAUSE_DOACROSS(...)` | `doacross(__VA_ARGS__)` |
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
  | `OMP_CLAUSE_IF(...)` | `if(__VA_ARGS__)` |
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
