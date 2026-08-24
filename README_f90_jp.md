# Solomon: Simple Off-LOading Macros Orchestrating multiple Notations

| | English | 日本語 |
| ---- | ---- | ---- |
| Fortran（**v2.0.0から追加**） | [README](README_f90.md) | [このファイル](README_f90_jp.md) |
| C/C++ | [README](README.md) | [README](README_jp.md) |

## 概要

* GPU向け指示文（OpenACC, OpenMP target）統合マクロ
* 詳細については [Miki & Hanawa (2024, IEEE Access, vol. 12, pp. 181644-181665)](https://doi.org/10.1109/ACCESS.2024.3509380) を参照してください
* 使用した際には [Miki & Hanawa (2024, IEEE Access, vol. 12, pp. 181644-181665)](https://doi.org/10.1109/ACCESS.2024.3509380) を引用してください
* Released under the MIT license, see LICENSE.txt
* Copyright (c) 2024 Yohei MIKI

## v1.x との後方互換性（v2.0.0 以降）

* v2.0.0 では，ユーザーコードや他ライブラリとの名前衝突を避けるため，ユーザー向けマクロを全て改名しました:
  * 指示文マクロには `SOLOMON_` 接頭辞が付きます（例: `OFFLOAD(...)` は `SOLOMON_OFFLOAD(...)` になりました）
  * 引数トークンには `SOLOMON_CLAUSE_` 接頭辞が付きます（例: `AS_INDEPENDENT` は `SOLOMON_CLAUSE_INDEPENDENT`，`COLLAPSE(n)` は `SOLOMON_CLAUSE_COLLAPSE(n)` になりました）
* v1.x 向けに書かれたコードは，`SOLOMON_WITH_SHORT_NAMES` を定義する（例: コンパイルフラグに `-DSOLOMON_WITH_SHORT_NAMES` を追加する）ことで従来の接頭辞なしの綴りが有効になり，そのまま動作します（デフォルト: OFF）
* 設定マクロには新しい綴り `-DSOLOMON_OFFLOAD_BY_*` が追加されました．従来の `-DOFFLOAD_BY_*` は無条件で使用し続けられます（`SOLOMON_WITH_SHORT_NAMES` の定義は不要です）
* 新旧 API の完全な対応表は [misc/migrate/MIGRATION.md](misc/migrate/MIGRATION.md) を参照してください
* 移行スクリプト `misc/migrate/solomon_migrate_v1_to_v2.sh` を同梱しています．v1.x のソースコード（およびファイル引数として明示したビルドスクリプト）を v2.0.0 の綴りに書き換えます．デフォルトは dry-run（diff 表示のみ）で，`--apply` を付けるとバックアップ（`*.v1.bak`）を残して書き換えます

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

1. Solomon のモジュールファイルをインクルードする

   各ソースファイルの冒頭でSolomonのインクルードファイルをインクルードしてください
   続いて、各モジュール、プログラムの冒頭で`USE_SOLOMON_RUNTIME`としてください

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
     * `PRAGMA_OMP_TARGET_DATA(...)` という記法は使わないでください
       * 演算加速器（GPU）からアクセスするデータについては `SOLOMON_DATA_ACCESS_BY_DEVICE(...)` か `PRAGMA_ACC_DATA(...)`，ホスト（CPU）からアクセスするデータについては `SOLOMON_DATA_ACCESS_BY_HOST(...)` か `PRAGMA_ACC_HOST_DATA(...)` をお使いください
     * `PRAGMA_ACC_DECLARE(...)` という記法は使わないでください（OpenMP target バックエンドへは変換されません）
       * デバイス常駐変数には `SOLOMON_DECLARE_ON_DEVICE(...)` を，link 意味論が必要な場合には `SOLOMON_DECLARE_ON_DEVICE_LINKED(...)` をお使いください（v2.0.0 以降）
     * Fortran では手続内に挿入した `SOLOMON_DECLARE_OFFLOADED(...)`（や `PRAGMA_ACC_ROUTINE(...)`）は自己完結するため，`SOLOMON_DECLARE_OFFLOADED_END` の挿入は不要です（空に展開されるため書いても無害です）（v2.0.0 以降）
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
         !$acc kernels private(i,j)
         !$acc loop
         do i=1, num

           do j=1, 16
             ! computation
           end do
         end do
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
        * **[更新 v1.1.0]** この制約は自動的に処理されるようになりました．どの順番で記述されていても，`SOLOMON_CLAUSE_INDEPENDENT`（及びその同義語）を Solomon が自動的に先頭に並び替えます
      * 指示文に対応していない指示節・指示句については，Solomon が自動的に無視します
   * OpenACC/OpenMP target間の互換性向上のため，（個々のマクロを直接使うよりも）下記に示す統合マクロの使用をおすすめします

     | 推奨実装 | 対応実装（非推奨） |
     | ---- | ---- |
     | **`SOLOMON_OFFLOAD(...)`** <br> `PRAGMA_ACC_KERNELS_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL_LOOP(...)` | `PRAGMA_ACC_KERNELS(...) PRAGMA_ACC_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL(...) PRAGMA_ACC_LOOP(...)` |
     | **`SOLOMON_DECLARE_DATA_ON_DEVICE(...)`** <br> `PRAGMA_ACC_DATA_PRESENT(...)` | `PRAGMA_ACC_DATA(ACC_CLAUSE_PRESENT(...))` |
     | `OMP_TARGET_CLAUSE_MAP_TO(...)` | `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_CLAUSE_TO(...))` |

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

     | 使用可能なマクロ | 出力 | 使用されるバックエンド | 備考 |
     | ---- | ---- | ---- | ---- |
     | **`SOLOMON_CLAUSE_ASYNC(...)`** <br> `ACC_CLAUSE_ASYNC(...)` <br> `OMP_TARGET_CLAUSE_NOWAIT` | `async(__VA_ARGS__)` <br> `nowait` | OpenACC <br> OpenMP | 両バックエンドで共通して非同期実行される <br> OpenACC においてはキューIDを指定可能 <br> OpenMP においてはキューIDは無視される |
     | **`SOLOMON_SYNCHRONIZE(...)`** <br> `PRAGMA_ACC_WAIT(...)` <br> `PRAGMA_OMP_TARGET_TASKWAIT(...)` | `!$acc wait __VA_ARGS__` <br> `!$omp taskwait __VA_ARGS__` | OpenACC <br> OpenMP | 両バックエンドで共通して同期処理が実行される．`SOLOMON_CLAUSE_ASYNC(...)` に対応して記述する |
     | **`SOLOMON_CLAUSE_ASYNC_QUEUE(id)`** <br> `ACC_CLAUSE_ASYNC(id)` | `async(id)` <br> N/A（OpenMP target 使用時には無視される） | OpenACC <br> OpenMP | OpenACC でのみキューIDを指定して非同期実行 <br> キューIDの指定が必須 <br> OpenMP ではキューIDを指定した非同期実行がサポートされていないため，無視される |
     | **`SOLOMON_WAIT_QUEUE(id)`** <br> `PRAGMA_ACC_WAIT(id)` | `wait(id)` <br> N/A（OpenMP target 使用時には無視される） | OpenACC <br> OpenMP | OpenACC でのみキューIDを指定して同期処理を実行．`SOLOMON_CLAUSE_ASYNC_QUEUE(id)` に対応して記述する <br> キューIDの指定が必須 <br> OpenMP ではキューIDを指定した同期処理がサポートされていないため，無視される |

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

* Solomon の動作モードを決めて、そのプリプロセスフラグを下記から選んでください

  | プリプロセスフラグ | 使用されるバックエンド | 備考 |
  | ---- | ---- | ---- |
  | `-DSOLOMON_OFFLOAD_BY_OPENACC` | OpenACC | デフォルトでは `kernels` 構文を使用 |
  | `-DSOLOMON_OFFLOAD_BY_OPENACC -DSOLOMON_OFFLOAD_BY_OPENACC_PARALLEL` | OpenACC | デフォルトでは `parallel` 構文を使用 |
  | `-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET` | OpenMP target | デフォルトでは `loop` 指示文を使用 |
  | `-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE` | OpenMP target | デフォルトでは `distribute` 指示文を使用 |
  | | 縮退モード | OpenMP を用いたマルチコアCPU向けのスレッド並列 |
  | | シリアルモード | OpenACC・OpenMP のいずれも有効化されていない場合は，全ての指示文が除去されシリアルコードとしてコンパイルされます（v2.0.0 以降） |

* **半自動コード生成・コンパイル方法**

  1. Makefile中で以下のようにFortranのコンパイラとそのオプション、さらにコンパイルの対象のファイル(.f90)を実施している箇所を探してください

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

     include $(SOLOMON_DIR)/fortran.mk
     ```

     * Solomonのパスを`SOLOMON_DIR`として指定
     * Makefile中の変数`FLAGS`にOpenACC または OpenMP target の機能を有効化するオプションを追加
     * コンパイルオプションの変数`FLAGS`にSolomonの動作モードのプリプロセスフラグを追加
     * `FC`および`FLAGS`の内容を変数`SOLOMON_FC`と`SOLOMON_FLAGS`にそれぞれ指定
       * `fortran.mk` は `SOLOMON_FC` と `SOLOMON_FLAGS` を用いてコンパイラを起動し `_OPENACC` / `_OPENMP` を自動検出するため，OpenACC/OpenMP を有効化するフラグ（`-acc=gpu`, `-mp=gpu`, `-fopenmp`, `-fiopenmp` など）を `SOLOMON_FC` または `SOLOMON_FLAGS` のいずれかに必ず含めてください
       * `_OPENACC` / `_OPENMP` のいずれも検出されなかった場合は注意メッセージ（`solomon: note: ...`）が表示され，全ての指示文はシリアルコードに展開されます．オフロードを意図していた場合は有効化フラグを追加してください（v2.0.0 以降）
     * ソースファイル中の Fortran の文字列連結（`//`）はプリプロセス時に保護されるため，自由に使用できます（v2.0.0 以降，`fortran.mk`・`spp.sh` の両方）
       * この保護のために `__SOLOMON_FC_CONCAT__` というトークンを Solomon が予約しています．ユーザーコード中に書かないでください
     * コンパイルの対象のファイルをsppディレクトリの下にある.f90に変更
     * Solomonの補助Makefileである`fortran.mk`をincludeするように絶対パスあるいは相対パスで指定

  3. 以上の変更により，makeコマンドでSolomonを用いて実装したコードから，バックエンドとしてOpenACCまたはOpenMP targetを用いてGPU化したプログラムが生成され，コンパイル・リンクされるようになります

* 手動コード生成・コンパイル方法
  * 前述の半自動コード生成・コンパイル方法において `include $(SOLOMON_DIR)/fortran.mk` によって簡略化していた手順を，全て自分で実施するという内容です
    * 下記手順（2, 3）については，事前に手順1で出力先ディレクトリ（以下の例では`spp`）を作成した上で，簡易スクリプト（試作版）を用いて実行することもできます：

      ```sh
      export SOLOMON_DIR=/path/to/Solomon
      $(SOLOMON_DIR)/spp.sh -compiler=nvfortran -acc=gpu -mp=gpu -I$(SOLOMON_DIR) -DSOLOMON_OFFLOAD_BY_OPENACC main.f90 > spp/main.f90 # NVIDIA HPC SDK の場合
      $(SOLOMON_DIR)/spp.sh -compiler=amdflang -fopenmp -I$(SOLOMON_DIR) -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET main.f90 > spp/main.f90 # AMD ROCm の場合
      $(SOLOMON_DIR)/spp.sh -compiler=ifx -fiopenmp -I$(SOLOMON_DIR) -DSOLOMON_OFFLOAD_BY_OPENMP_TARGET main.f90 > spp/main.f90 # Intel oneAPI の場合
      ```

      * `-compiler=...` の引数およびコンパイラフラグは使用する環境に応じて適宜置き換えてください
      * `-compiler=...`, `-I...`, `-D...` 以外のコンパイラフラグ（`-acc=gpu`, `-mp=gpu`, `-fopenmp`, `-fiopenmp` など）は `_OPENACC` / `_OPENMP` の自動検出時にコンパイラへ渡されるため，`-D_OPENACC=...` / `-D_OPENMP=...` を手動で指定する必要はありません（v2.0.0 以降．従来の `-compiler="nvfortran -acc=gpu"` という書き方も引き続き使用できます）
      * GPU アーキテクチャを指定するフラグ（`-gpu=...`, `--offload-arch=...`, `-Xs "-device ..."`）は `spp.sh` ではプリプロセスとマクロ検出のみを行うため不要で，最終的な compile/link 時に指定してください
      * `$(SOLOMON_DIR)/spp.sh` については，Solomonのパスに`PATH`を通した上で`spp.sh`として実行することもできます

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
       * 有効化できない場合は`OPENMP=202011 OPENACC=_OPENACC` のように元の文字列のまま表示されます

  3. FortranのソースファイルをC言語用のプリプロセッサを用いた以下の2つのコマンドでプリプロセスを実行してください

     ```sh
     cpp -P -DSOLOMON_FORTRAN -D_OPENACC=201711 -I../../../solomon -DSOLOMON_OFFLOAD_BY_OPENACC mysrc.f90 > spp/mysrc.i.f90
     sed 's/^#pragma /!$/g' spp/mysrc.i.f90 > spp/mysrc.f90
     ```

  4. 最後に，以下のようにコンパイルを実行してください

     ```sh
     nvfortran -O3 -acc=gpu -gpu=[target GPU architecture (e.g., cc90)] spp/mysrc.f90 -o myprog # NVIDIA HPC SDK（OpenACC）の場合
     nvfortran -O3 -mp=gpu -gpu=[target GPU architecture (e.g., cc90)] spp/mysrc.f90 -o myprog # NVIDIA HPC SDK（OpenMP target）の場合
     amdflang -O3 -fopenmp --offload-arch=[target GPU architecture (e.g., gfx942)] spp/mysrc.f90 -o myprog  # AMD ROCm（OpenMP target）の場合
     ifx -O3 -fiopenmp -fopenmp-targets=spir64_gen -Xs "-device [target GPU architecture (e.g., pvc)]" spp/mysrc.f90 -o myprog  # Intel oneAPI（OpenMP target）の場合
     ```

* 実際に生成される指示文を確認する方法
  * 中間ファイル `spp/*.f90` が生成されているので，このファイルを開くと実際に生成された指示文が確認できます
  * 先のプリプロセスの際にプリプロセスフラグとして`-DPRINT_GENERATED_PRAGMA`を追加すると，実際に生成される指示文をコンパイル時メッセージに出力できます
    * LLVMではwarning扱いとなるため，コンパイルで`-Werror`を指定している場合には`-Wno-error=pragma-messages`も渡してこのメッセージがエラー扱いにならないようにしてください

* 使用例：[拡散方程式用の Makefile](samples/F/diffusion/Makefile)
  * 半自動コード生成・コンパイル方法の手順例です

### Solomon の拡張方法（コードジェネレータを用いた更新方法）

* Solomon では，1つの指示文ごとに 32 個の指示節・指示句（候補）を受け付ける仕様となっています
  * 32 という上限値では不足する場合には，下記の手順で上限値を増やしてください

    ```sh
    cd solomon/util # このディレクトリには，jl/*.jl が格納されています
    julia jl/check_clause.jl --max 64 # この例は，上限値を 64 に増やす場合です
    ```

  * 同様の制限が他の内部マクロに存在することがありますが，同様の手順で上限値を増やせます
  * 提供済みのコードジェネレータは下記の通りで，すべて `solomon/util/jl/` に配置されています

  | ジェネレータ | 目的 | デフォルト最大値 |
  | ------------ | ---- | ---------------- |
  | `check_clause.jl` | 節チェックマクロの生成 | 32 |
  | `num_args.jl` | 入力数読取マクロの生成 | 1024 |
  | `pickup_clause.jl` | 節フィルタリングマクロの生成 | 99 |
  | `retrieve_args.jl` | 入力フィルタリングマクロの生成 | 128 |
  | `sort_clause.jl` | 節ソートマクロの生成 | 32 |

### プロファイラタグ（NVTX, rocTX, ITT）

Solomon はベンダー中立なプロファイラタグを発行できます（v2.0.0 以降）．バックエンド（NVIDIA NVTX / AMD rocTX / Intel ITT）は使用中のコンパイラから自動選択されるため，ベンダー固有名がコードに現れることはありません（強制したい場合は `SOLOMON_TAGGED_PROFILE_WITH_NVTX` / `..._WITH_ROCTX` / `..._WITH_ITT` を定義してください）．

* `-DSOLOMON_TAGGED_PROFILE` で手動タグ用マクロが有効になります（他の Solomon マクロと同様，行末セミコロンなしで記述します）:
  * `SOLOMON_PROFILE_RANGE_BEGIN("name")` / `SOLOMON_PROFILE_RANGE_END`: 名前付き区間
  * `SOLOMON_PROFILE_MARK("name")`: 瞬間イベント
* `-DSOLOMON_TAGGED_PROFILE_AUTO` を定義すると，さらに Solomon の指示文マクロに `"FILE:LINE MACRO"` 形式のタグが自動で付きます:
  * データ転送・同期系（`SOLOMON_MEMCPY_*`, `SOLOMON_MALLOC_ON_DEVICE`, `SOLOMON_FREE_FROM_DEVICE`, `SOLOMON_SYNCHRONIZE` など）は操作全体を覆う区間になります
  * カーネル起動系ループマクロ（`SOLOMON_OFFLOAD`, `SOLOMON_OFFLOAD_OUTER_LOOP`）はマクロからループの終端が見えないため瞬間マークのみです．カーネル実行時間の計測には手動区間マクロ（またはプロファイラ自身のカーネルトレース）をお使いください
  * `SOLOMON_OFFLOAD_SERIAL`〜`SOLOMON_END_OFFLOAD_SERIAL` は実区間になります
* リンク: NVTX は NVIDIA HPC SDK のオフロードフラグがあれば追加フラグ不要です．rocTX は `-lroctx64`，ITT は `-littnotify` を追加でリンクしてください
* Fortran: 同梱ヘルパーを同じコンパイラ・フラグでコンパイルしてリンクしてください（例: `nvc -c -mp=gpu -DSOLOMON_TAGGED_PROFILE $(SOLOMON_DIR)/profile/solomon_profile.c`．define なしでコンパイルすると no-op になります）．自動タグの `FILE` は `spp.sh`/`fortran.mk` の中間ファイル名になる点にご注意ください

### エディタ支援（シンタックスハイライトと clang-format）

Solomon は単なるプリプロセッサマクロのため，エディタは既定ではハイライトしません．`SOLOMON_*` マクロと2段記法（`PRAGMA_ACC_*`, `PRAGMA_OMP_*`, `ACC_CLAUSE_*`, `OMP_CLAUSE_*`, `OMP_TARGET_*`）を指示文らしく見せる設定ファイルを `misc/editor/` に同梱しています（v2.0.0 以降）:

* vim / neovim: `misc/editor/vim/after/syntax/{c,cpp,fortran}.vim` を `~/.vim/after/syntax/` へコピー（または symlink）してください（neovim は `~/.config/nvim/after/syntax/`）
* emacs: `init.el` に `(load "/path/to/solomon/misc/editor/emacs/solomon-highlight.el")` を追加してください
* VS Code: フォルダ `misc/editor/vscode/solomon-highlight/` を `~/.vscode/extensions/` へコピー（または symlink）し，VS Code を再起動してください
  * これはビルド不要のローカル injection grammar（テキスト2ファイル）です．VS Code は `settings.json` だけでは正規表現ベースのハイライトを追加できないため，展開済みローカル拡張として読み込ませる方式を採っています（マーケットプレイスへの公開は行いません）
* clang-format: リポジトリ直下の `.clang-format` に Solomon の指示文マクロを `StatementMacros`/`AttributeMacros` として登録してあり，整形時に指示文行が崩れません．参考設定（強制ではありません）なので，マクロのリストをご自身の `.clang-format` へコピーしてお使いください

## サンプルコード

### diffusion: メモリ律速な問題の実装例

* [3次元拡散方程式](samples/F/diffusion)
  * [OpenACC を用いた実装（名古屋大学の星野哲也氏による実装）](https://github.com/hoshino-UTokyo/lecture_openacc) を改変したものです
  * Solomon 化以外に，簡単な性能最適化やコード改編も施してあります

## Solomon が提供する API

### 使用可能な指示文

* <details><summary>演算に関する指示文</summary>

  | 入力 | 出力 | バックエンドとして用いる指示文 |
  | ---- | ---- | ---- |
  | **`SOLOMON_OFFLOAD(...)`** <br> `PRAGMA_ACC_KERNELS_LOOP(...)` <br> `PRAGMA_ACC_PARALLEL_LOOP(...)` <br> `PRAGMA_OMP_TARGET_TEAMS_LOOP(...)` <br> `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO(...)` | `!$acc kernels __VA_ARGS__` <br> `!$acc loop __VA_ARGS__` <br> `!$acc parallel __VA_ARGS__` <br> `!$acc loop __VA_ARGS__` <br> `!$omp target teams loop __VA_ARGS__` <br> `!$omp target teams distribute parallel do __VA_ARGS__` | OpenACC (kernels) <br><br> OpenACC (parallel) <br><br> OpenMP (loop) <br> OpenMP (distribute) |
  | **`SOLOMON_OFFLOAD_OUTER_LOOP(...)`** | `!$acc parallel __VA_ARGS__ !$acc loop gang __VA_ARGS__` <br> `!$omp target teams distribute __VA_ARGS__` <br> `!$omp parallel do __VA_ARGS__` | OpenACC <br> OpenMP target <br> OpenMP（縮退モード） |
  | **`SOLOMON_PARALLELIZE_INNER_LOOP(...)`** | `!$acc loop vector __VA_ARGS__` <br> `!$omp parallel do __VA_ARGS__` <br> 無視される（外側ループが並列化済みのため） | OpenACC <br> OpenMP target <br> OpenMP（縮退モード） |
  | **`SOLOMON_OFFLOAD_SERIAL(...)`** <br> `PRAGMA_ACC_SERIAL(...)` <br> `PRAGMA_OMP_TARGET(...)` | <br> `!$acc serial __VA_ARGS__` <br> `!$omp target __VA_ARGS__` | <br> OpenACC <br> OpenMP <br> 直後の構造化ブロックをデバイス上で単一スレッド実行する．reduction 結果をデバイス上に保持したい場合などに有用（v2.0.0 以降） |
  | **`SOLOMON_END_OFFLOAD_SERIAL`** | `!$acc end serial` <br> `!$omp end target` | OpenACC <br> OpenMP <br> serial 領域を終端する（v2.0.0 以降） |
  | **`SOLOMON_SYNCHRONIZE(...)`** <br> `PRAGMA_ACC_WAIT(...)` <br> `PRAGMA_OMP_TARGET_TASKWAIT(...)` | `!$acc wait __VA_ARGS__` <br> `!$omp taskwait __VA_ARGS__` | OpenACC <br> OpenMP |
  | **`SOLOMON_WAIT_QUEUE(id)`** <br> `PRAGMA_ACC_WAIT(id)` | `!$acc wait id` | OpenACC (only) |
  | **`SOLOMON_DECLARE_OFFLOADED(...)`** <br> `PRAGMA_ACC_ROUTINE(...)` <br> `PRAGMA_OMP_DECLARE_TARGET(...)` | `!$acc routine __VA_ARGS__` <br> `!$omp declare target __VA_ARGS__` | OpenACC <br> OpenMP |
  | **`SOLOMON_DECLARE_OFFLOADED_END`** <br> `PRAGMA_OMP_END_DECLARE_TARGET` | （何も出力しない） | OpenMP (only) <br> Fortran では手続内の `SOLOMON_DECLARE_OFFLOADED(...)` が自己完結するため本マクロは空に展開される．書いても無害だが不要（v2.0.0 以降） |
  | **`SOLOMON_CLAUSE_TARGETS(...)`** | `(__VA_ARGS__)` | OpenACC/OpenMP <br> 手続を名前で指定する: `SOLOMON_DECLARE_OFFLOADED(SOLOMON_CLAUSE_TARGETS(func), ...)` は `!$acc routine (func) ...` / `!$omp declare target (func)` に展開される（v2.0.0 以降） |
  | **`SOLOMON_DECLARE_ON_DEVICE(...)`** | `!$acc declare create(__VA_ARGS__)` <br> `!$omp declare target (__VA_ARGS__)` | OpenACC <br> OpenMP <br> モジュール・手続の宣言部でデバイス常駐変数を宣言する（v2.0.0 以降） |
  | **`SOLOMON_DECLARE_ON_DEVICE_LINKED(...)`** | `!$acc declare link(__VA_ARGS__)` <br> `!$omp declare target link(__VA_ARGS__)` | OpenACC <br> OpenMP <br> link 意味論でデバイス常駐変数を宣言する（v2.0.0 以降） |
  | **`SOLOMON_ATOMIC(...)`** <br> `PRAGMA_ACC_ATOMIC(...)` <br> `PRAGMA_OMP_TARGET_ATOMIC(...)` | `!$acc atomic __VA_ARGS__` <br> `!$omp atomic __VA_ARGS__` | OpenACC <br> OpenMP |
  | **`SOLOMON_ATOMIC_UPDATE`** <br> `PRAGMA_ACC_ATOMIC_UPDATE` <br> `PRAGMA_OMP_TARGET_ATOMIC_UPDATE` | `!$acc atomic update` <br> `!$omp atomic update` | OpenACC <br> OpenMP |
  | **`SOLOMON_ATOMIC_READ`** <br> `PRAGMA_ACC_ATOMIC_READ` <br> `PRAGMA_OMP_TARGET_ATOMIC_READ` | `!$acc atomic read` <br> `!$omp atomic read` | OpenACC <br> OpenMP |
  | **`SOLOMON_ATOMIC_WRITE`** <br> `PRAGMA_ACC_ATOMIC_WRITE` <br> `PRAGMA_OMP_TARGET_ATOMIC_WRITE` | `!$acc atomic write` <br> `!$omp atomic write` | OpenACC <br> OpenMP |
  | **`SOLOMON_ATOMIC_CAPTURE`** <br> `PRAGMA_ACC_ATOMIC_CAPTURE` <br> `PRAGMA_OMP_TARGET_ATOMIC_CAPTURE` | `!$acc atomic capture` <br> `!$omp atomic capture` | OpenACC <br> OpenMP |
  | **`SOLOMON_END_OFFLOAD`** | `!$acc end parallel` | OpenACC (only) |
  | **`SOLOMON_END_OFFLOAD_OUTER_LOOP`** | `!$acc end parallel` | OpenACC (only) |
  | **`PRAGMA_ACC_END_PARALLEL`** | `!$acc end parallel` | OpenACC (only) |
  | **`PRAGMA_ACC_END_KERNELS`** | `!$acc end kernels` | OpenACC (only) |
  | **`PRAGMA_ACC_END_SERIAL`** | `!$acc end serial` | OpenACC (only) |
  | **`PRAGMA_ACC_END_DATA`** | `!$acc end data` <br> `!$omp end target data` | OpenACC <br> OpenMP |
  | **`PRAGMA_ACC_END_HOST_DATA`** | `!$acc end host_data` <br> `!$omp end target data` | OpenACC <br> OpenMP |
  | **`PRAGMA_ACC_END_ROUTINE`** | `!$omp end declare target` | OpenMP (only) |
  | **`PRAGMA_OMP_END_PARALLEL`** | `!$omp end parallel` | OpenMP (only) |
  | **`PRAGMA_OMP_END_TEAMS`** | `!$omp end teams` | OpenMP (only) |
  | **`PRAGMA_OMP_END_SIMD`** | `!$omp end simd` | OpenMP (only) |
  | **`PRAGMA_OMP_END_SINGLE`** | `!$omp end single` | OpenMP (only) |
  | **`PRAGMA_OMP_END_SECTIONS`** | `!$omp end sections` | OpenMP (only) |
  | **`PRAGMA_OMP_END_CRITICAL`** | `!$omp end critical` | OpenMP (only) |
  | **`PRAGMA_OMP_END_PARALLEL_SECTIONS`** | `!$omp end parallel sections` | OpenMP (only) |
  | **`PRAGMA_OMP_END_DECLARE_TARGET`** | `!$omp end declare target` | OpenMP (only) |
  | **`PRAGMA_OMP_END_TARGET_PARALLEL`** | `!$acc end parallel` <br> `!$omp end target parallel` | OpenACC <br> OpenMP |
  | **`PRAGMA_OMP_END_TARGET_SIMD`** | `!$acc end parallel` <br> `!$omp end target simd` | OpenACC <br> OpenMP |
  | **`PRAGMA_OMP_END_TARGET_TEAMS`** | `!$acc end parallel` <br> `!$omp end target teams` | OpenACC <br> OpenMP |
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
    | `PRAGMA_ACC_WAIT(...)` | `!$acc wait __VA_ARGS__` | `PRAGMA_OMP_TARGET_TASKWAIT(__VA_ARGS__)` |
    | `PRAGMA_ACC_ROUTINE(...)` | `!$acc routine __VA_ARGS__` | `PRAGMA_OMP_DECLARE_TARGET(__VA_ARGS__)` |
    | `PRAGMA_ACC_DECLARE(...)` | `!$acc declare __VA_ARGS__` | N/A（OpenMP target 使用時には無視される．`SOLOMON_DECLARE_ON_DEVICE(...)` をお使いください） |

    </details>

  * <details><summary>OpenMP target</summary>

    | 入力 | OpenMP target 使用時の出力 | OpenACC 使用時の出力 | 縮退モード（演算加速器を用いないCPU実行）での出力 |
    | ---- | ---- | ---- | ---- |
    | `PRAGMA_OMP_TARGET(...)` | `!$omp target __VA_ARGS__` | `PRAGMA_ACC_SERIAL(__VA_ARGS__)` (v2.0.0 or later) | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_TARGET_PARALLEL(...)` | `!$omp target parallel __VA_ARGS__` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_DO(...)` | `!$omp target parallel do __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_DO(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_DO_SIMD(...)` | `!$omp target parallel do simd __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_DO_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_PARALLEL_LOOP(...)` | `!$omp target parallel loop __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_PARALLEL_LOOP(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_SIMD(...)` | `!$omp target simd __VA_ARGS__` | `PRAGMA_ACC_LAUNCH_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS(...)` | `!$omp target teams __VA_ARGS__` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE(...)` | `!$omp target teams distribute __VA_ARGS__` | `PRAGMA_ACC_LAUNCH_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_DISTRIBUTE(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_SIMD(...)` | `!$omp target teams distribute simd __VA_ARGS__` | `PRAGMA_ACC_LAUNCH_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_DISTRIBUTE_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_LOOP(...)` | `!$omp target teams loop __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_LOOP(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO(...)` | `!$omp target teams distribute parallel do __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_DO(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD(...)` | `!$omp target teams distribute parallel do simd __VA_ARGS__` | `PRAGMA_ACC_OFFLOADING_DEFAULT(ACC_CLAUSE_INDEPENDENT, ##__VA_ARGS__)` | `PRAGMA_OMP_TEAMS_DISTRIBUTE_PARALLEL_DO_SIMD(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_ATOMIC(...)` | `PRAGMA_OMP_ATOMIC(__VA_ARGS__)` | `PRAGMA_ACC_ATOMIC(__VA_ARGS__)` | `PRAGMA_OMP_ATOMIC(__VA_ARGS__)` |
    | `PRAGMA_OMP_TARGET_TASKWAIT(...)` | `PRAGMA_OMP_TASKWAIT(__VA_ARGS__)` | `PRAGMA_ACC_WAIT(__VA_ARGS__)` | `PRAGMA_OMP_TASKWAIT(__VA_ARGS__)` |
    | `PRAGMA_OMP_DECLARE_TARGET(...)` | `!$omp declare target __VA_ARGS__` | `PRAGMA_ACC_ROUTINE(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_BEGIN_DECLARE_TARGET(...)` | `!$omp begin declare target __VA_ARGS__` | `PRAGMA_ACC_ROUTINE(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_END_DECLARE_TARGET` | `!$omp end declare target` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） |

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
    | `PRAGMA_OMP_WORKSHARE(...)` | `!$omp workshare __VA_ARGS__` |
    | `PRAGMA_OMP_SCOPE(...)` | `!$omp scope __VA_ARGS__` |
    | `PRAGMA_OMP_SECTIONS(...)` | `!$omp sections __VA_ARGS__` |
    | `PRAGMA_OMP_SECTION` | `!$omp section` |
    | `PRAGMA_OMP_DO(...)` | `!$omp do __VA_ARGS__` |
    | `PRAGMA_OMP_DISTRIBUTE(...)` | `!$omp distribute __VA_ARGS__` |
    | `PRAGMA_OMP_LOOP(...)` | `!$omp loop __VA_ARGS__` |
    | `PRAGMA_OMP_TASK(...)` | `!$omp task __VA_ARGS__` |
    | `PRAGMA_OMP_TASKLOOP(...)` | `!$omp taskloop __VA_ARGS__` |
    | `PRAGMA_OMP_TASKYIELD` | `!$omp taskyield` |
    | `PRAGMA_OMP_INTEROP(...)` | `!$omp interop __VA_ARGS__` |
    | `PRAGMA_OMP_CRITICAL(...)` | `!$omp critical __VA_ARGS__` |
    | `PRAGMA_OMP_BARRIER` | `!$omp barrier` |
    | `PRAGMA_OMP_TASKGROUP(...)` | `!$omp taskgroup __VA_ARGS__` |
    | `PRAGMA_OMP_TASKWAIT(...)` | `!$omp taskwait __VA_ARGS__` |
    | `PRAGMA_OMP_FLUSH(...)` | `!$omp flush __VA_ARGS__` |
    | `PRAGMA_OMP_DEPOBJ(...)` | `!$omp depobj __VA_ARGS__` |
    | `PRAGMA_OMP_ATOMIC(...)` | `!$omp atomic __VA_ARGS__` |
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
  | **`SOLOMON_ALLOCATE_ON_DEVICE(...)`** <br> `PRAGMA_ACC_ENTER_DATA_CREATE(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA_MAP_ALLOC(...)` | `!$acc enter data create(__VA_ARGS__)` <br> `!$omp target enter data map(alloc: __VA_ARGS__)` | OpenACC <br> OpenMP |
  | **`SOLOMON_DEALLOCATE_ON_DEVICE(...)`** <br> `PRAGMA_ACC_EXIT_DATA_DELETE(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA_MAP_DELETE(...)` | `!$acc exit data delete(__VA_ARGS__)` <br> `!$omp target exit data map(delete: __VA_ARGS__)` | OpenACC <br> OpenMP |
  | **`SOLOMON_MEMCPY_D2H(...)`** <br> `PRAGMA_ACC_UPDATE_HOST(...)` <br> `PRAGMA_OMP_TARGET_UPDATE_FROM(...)` | `!$acc update host(__VA_ARGS__)` <br> `!$omp target update from(__VA_ARGS__)` | OpenACC <br> OpenMP |
  | **`SOLOMON_MEMCPY_H2D(...)`** <br> `PRAGMA_ACC_UPDATE_DEVICE(...)` <br> `PRAGMA_OMP_TARGET_UPDATE_TO(...)` | `!$acc update device(__VA_ARGS__)` <br> `!$omp target update to(__VA_ARGS__)` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_ENTER_DATA(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA(...)` | `!$acc enter data __VA_ARGS__` <br> `!$omp target enter data __VA_ARGS__` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_ENTER_DATA_COPYIN(...)` <br> `PRAGMA_OMP_TARGET_ENTER_DATA_MAP_TO(...)` | `!$acc enter data copyin(__VA_ARGS__)` <br> `!$omp target enter data map(to: __VA_ARGS__)` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_EXIT_DATA(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA(...)` | `!$acc exit data __VA_ARGS__` <br> `!$omp target exit data __VA_ARGS__` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_EXIT_DATA_COPYOUT(...)` <br> `PRAGMA_OMP_TARGET_EXIT_DATA_MAP_FROM(...)` | `!$acc exit data copyout(__VA_ARGS__)` <br> `!$omp target exit data map(from: __VA_ARGS__)` | OpenACC <br> OpenMP |
  | `PRAGMA_ACC_UPDATE(...)` <br> `PRAGMA_OMP_TARGET_UPDATE(...)` | `!$acc update __VA_ARGS__` <br> `!$omp target update __VA_ARGS__` | OpenACC <br> OpenMP |
  | **`SOLOMON_DATA_ACCESS_BY_DEVICE(...)`** <br> `PRAGMA_ACC_DATA(...)` <br> `PRAGMA_OMP_TARGET_DATA(...)` | `!$acc data __VA_ARGS__` <br> `!$omp target data __VA_ARGS__` | OpenACC <br> OpenMP |
  | **`SOLOMON_DATA_ACCESS_BY_HOST(...)`** <br> `PRAGMA_ACC_HOST_DATA(...)` <br> `PRAGMA_OMP_TARGET_DATA(...)` | `!$acc host_data __VA_ARGS__` <br> `!$omp target data __VA_ARGS__` | OpenACC <br> OpenMP |
  | **`SOLOMON_USE_DEVICE_DATA_FROM_HOST(...)`** <br> `PRAGMA_ACC_HOST_DATA_USE_DEVICE(...)` <br> `PRAGMA_OMP_TARGET_DATA_USE_DEVICE_ADDR(...)` | `!$acc host_data use_device(__VA_ARGS__)` <br> `!$omp target data use_device_addr(__VA_ARGS__)` | OpenACC <br> OpenMP |
  | **`SOLOMON_DECLARE_DATA_ON_DEVICE(...)`** <br> `PRAGMA_ACC_DATA_PRESENT(...)` | `!$acc data present(__VA_ARGS__)` | OpenACC (only) |

  </details>

  * <details><summary>OpenACC</summary>

    | 入力 | 出力 | OpenMP target 使用時の出力 |
    | ---- | ---- | ---- |
    | `PRAGMA_ACC_DATA(...)` | `!$acc data __VA_ARGS__` | `PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_ENTER_DATA(...)` | `!$acc enter data __VA_ARGS__` | `PRAGMA_OMP_TARGET_ENTER_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_EXIT_DATA(...)` | `!$acc exit data __VA_ARGS__` | `PRAGMA_OMP_TARGET_EXIT_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_HOST_DATA(...)` | `!$acc host_data __VA_ARGS__` | `PRAGMA_OMP_TARGET_DATA(__VA_ARGS__)` |
    | `PRAGMA_ACC_UPDATE(...)` | `!$acc update __VA_ARGS__` | `PRAGMA_OMP_TARGET_UPDATE(__VA_ARGS__)` |

    </details>

  * <details><summary>OpenMP target</summary>

    | 入力 | 出力 | OpenACC 使用時の出力 | 縮退モード（演算加速器を用いないCPU実行）での出力 |
    | ---- | ---- | ---- | ---- |
    | `PRAGMA_OMP_TARGET_DATA(...)` | `!$omp target data __VA_ARGS__` | `PRAGMA_ACC_DATA(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_TARGET_ENTER_DATA(...)` | `!$omp target enter data __VA_ARGS__` | `PRAGMA_ACC_ENTER_DATA(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_TARGET_EXIT_DATA(...)` | `!$omp target exit data __VA_ARGS__` | `PRAGMA_ACC_EXIT_DATA(__VA_ARGS__)` | N/A（縮退モードでは無視される） |
    | `PRAGMA_OMP_TARGET_UPDATE(...)` | `!$omp target update __VA_ARGS__` | `PRAGMA_ACC_UPDATE(__VA_ARGS__)` | N/A（縮退モードでは無視される） |

    </details>

### 使用可能な指示節・指示句

* <details><summary>簡易記法</summary>

  | 入力 | 出力 | バックエンドとして用いる指示文 | 備考 |
  | ---- | ---- | ---- | ---- |
  | **`SOLOMON_CLAUSE_INDEPENDENT`** <br> `ACC_CLAUSE_INDEPENDENT` <br> `OMP_TARGET_CLAUSE_SIMD` | `independent` <br> `simd` | OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_SEQUENTIAL`** <br> `ACC_CLAUSE_SEQ` | `seq` | OpenACC (only) | |
  | **`SOLOMON_CLAUSE_NUM_THREADS(n)`** <br> `ACC_CLAUSE_VECTOR_LENGTH(n)` <br> `OMP_TARGET_CLAUSE_THREAD_LIMIT(n)` | `vector_length(n)` <br> `thread_limit(n)` | OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_NUM_BLOCKS(n)`** <br> `ACC_CLAUSE_NUM_GANGS(n)` <br> `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` | <br> `num_gangs(n)` <br> `num_teams(n)` | <br> OpenACC <br> OpenMP | v2.0.0 で `ACC_CLAUSE_NUM_WORKERS(n)` から `ACC_CLAUSE_NUM_GANGS(n)` へと変更 |
  | ~~`SOLOMON_CLAUSE_NUM_GRIDS(n)`~~ <br> ~~`ACC_CLAUSE_NUM_GANGS(n)`~~ | <br> ~~`num_gangs(n)`~~ | <br> OpenACC (only) | v2.0.0 で非推奨．以降は `SOLOMON_CLAUSE_NUM_BLOCKS(n)` を使用 |
  | **`SOLOMON_CLAUSE_THREAD`** <br> `ACC_CLAUSE_VECTOR` | `vector` | OpenACC (only) | |
  | **`SOLOMON_CLAUSE_BLOCK`** <br> `ACC_CLAUSE_GANG` | <br> `gang` | <br> OpenACC (only) | v2.0.0 で `ACC_CLAUSE_WORKER` から `ACC_CLAUSE_GANG` へと変更 |
  | ~~`SOLOMON_CLAUSE_GRID`~~ <br> ~~`ACC_CLAUSE_GANG`~~ | <br> ~~`gang`~~ | <br> OpenACC (only) | v2.0.0 で非推奨．以降は `SOLOMON_CLAUSE_BLOCK` を使用 |
  | **`SOLOMON_CLAUSE_COLLAPSE(n)`** <br> `ACC_CLAUSE_COLLAPSE(n)` <br> `OMP_TARGET_CLAUSE_COLLAPSE(n)` | `collapse(n)` <br> `collapse(n)` | OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_ASYNC(...)`** <br> `ACC_CLAUSE_ASYNC(...)` <br> `OMP_TARGET_CLAUSE_NOWAIT` | `async(__VA_ARGS__)` <br> `nowait` | OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_ASYNC_QUEUE(id)`** <br> `ACC_CLAUSE_ASYNC(id)` | `async(id)` | OpenACC (only) | |
  | **`SOLOMON_CLAUSE_REDUCTION(...)`** <br> `ACC_CLAUSE_REDUCTION(...)` <br> `OMP_TARGET_CLAUSE_REDUCTION(...)` | `reduction(__VA_ARGS__)` <br> `reduction(__VA_ARGS__)` | OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_IF(condition)`** <br> `ACC_CLAUSE_IF(condition)` <br> `OMP_TARGET_CLAUSE_IF(condition)` | `if(condition)` <br> `if(condition)` | OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_PRIVATE(...)`** <br> `ACC_CLAUSE_PRIVATE(...)` <br> `OMP_TARGET_CLAUSE_PRIVATE(...)` | `private(__VA_ARGS__)` <br> `private(__VA_ARGS__)` | OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_FIRSTPRIVATE(...)`** <br> `ACC_CLAUSE_FIRSTPRIVATE(...)` <br> `OMP_TARGET_CLAUSE_FIRSTPRIVATE(...)` | `firstprivate(__VA_ARGS__)` <br> `firstprivate(__VA_ARGS__)` | OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_DEVICE_PTR(...)`** <br> `ACC_CLAUSE_DEVICEPTR(...)` <br> `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(...)` | `deviceptr(__VA_ARGS__)` <br> `is_device_ptr(__VA_ARGS__)` | OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_COPY_BEFORE_AND_AFTER_EXEC(...)`** <br> `ACC_CLAUSE_COPY(...)` <br> `OMP_TARGET_CLAUSE_MAP_TOFROM(...)` | `copy(__VA_ARGS__)` <br> `map(tofrom: __VA_ARGS__)` | OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_COPY_H2D_BEFORE_EXEC(...)`** <br> `ACC_CLAUSE_COPYIN(...)` <br> `OMP_TARGET_CLAUSE_MAP_TO(...)` | `copyin(__VA_ARGS__)` <br> `map(to: __VA_ARGS__)` | OpenACC <br> OpenMP | |
  | **`SOLOMON_CLAUSE_COPY_D2H_AFTER_EXEC(...)`** <br> `ACC_CLAUSE_COPYOUT(...)` <br> `OMP_TARGET_CLAUSE_MAP_FROM(...)` | `copyout(__VA_ARGS__)` <br> `map(from: __VA_ARGS__)` | OpenACC <br> OpenMP | |

  </details>

* <details><summary>OpenACC</summary>

  | 入力 | 出力 | OpenMP target 使用時の出力 | 備考 |
  | ---- | ---- | ---- | ---- |
  | `ACC_CLAUSE_IF(condition)` | `if(condition)` | `OMP_TARGET_CLAUSE_IF(condition)` | |
  | `ACC_CLAUSE_SELF(...)` | `self(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_DEFAULT(mode)` | `default(mode)` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_DEFAULT_NONE` | `default(none)` | `OMP_TARGET_CLAUSE_DEFAULTMAP_NONE` | |
  | `ACC_CLAUSE_DEFAULT_PRESENT` | `default(present)` | `OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT` | |
  | `ACC_CLAUSE_DEVICE_TYPE(...)` | `device_type(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_DEVICE_TYPE(__VA_ARGS__)` | |
  | `ACC_CLAUSE_ASYNC(...)` | `async(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_NOWAIT` | |
  | `ACC_CLAUSE_WAIT(...)` | `wait(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_DEPEND_IN(__VA_ARGS__)` | |
  | `ACC_CLAUSE_FINALIZE` | `finalize` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_NUM_GANGS(n)` | `num_gangs(n)` | `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` | v2.0.0 で `OMP_TARGET_CLAUSE_NUM_TEAMS(n)` に変換されるよう変更 |
  | `ACC_CLAUSE_NUM_WORKERS(n)` | `num_workers(n)` | N/A（OpenMP target 使用時には無視される） | v2.0.0 で OpenMP target 使用時に無視されるよう変更 |
  | `ACC_CLAUSE_VECTOR_LENGTH(n)` | `vector_length(n)` | `OMP_TARGET_CLAUSE_THREAD_LIMIT(n)` | |
  | `ACC_CLAUSE_REDUCTION(...)` | `reduction(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_REDUCTION(__VA_ARGS__)` | |
  | `ACC_CLAUSE_PRIVATE(...)` | `private(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_PRIVATE(__VA_ARGS__)` | |
  | `ACC_CLAUSE_FIRSTPRIVATE(...)` | `firstprivate(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_FIRSTPRIVATE(__VA_ARGS__)` | |
  | `ACC_CLAUSE_COPY(...)` | `copy(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_TOFROM(__VA_ARGS__)` | |
  | `ACC_CLAUSE_COPYIN(...)` | `copyin(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_TO(__VA_ARGS__)` | |
  | `ACC_CLAUSE_COPYOUT(...)` | `copyout(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_FROM(__VA_ARGS__)` | |
  | `ACC_CLAUSE_CREATE(...)` | `create(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_ALLOC(__VA_ARGS__)` | |
  | `ACC_CLAUSE_NO_CREATE(...)` | `no_create(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_DELETE(...)` | `delete(__VA_ARGS__)` | `OMP_TARGET_CLAUSE_MAP_DELETE(__VA_ARGS__)` | |
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
  | `ACC_CLAUSE_BIND(arg)` | `bind(arg)` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_NOHOST` | `nohost` | `OMP_TARGET_CLAUSE_DEVICE_TYPE(nohost)` | |
  | `ACC_CLAUSE_DEVICE_RESIDENT(...)` | `device_resident(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） | |
  | `ACC_CLAUSE_LINK(...)` | `link(__VA_ARGS__)` | N/A（OpenMP target 使用時には無視される） | |

  </details>

* <details><summary>OpenMP target</summary>

  | 入力 | 出力 | OpenACC 使用時の出力 | 縮退モード（演算加速器を用いないCPU実行）での出力 | 備考 |
  | ---- | ---- | ---- | ---- | ---- |
  | `OMP_TARGET_CLAUSE_ALIGNED(...)` | `OMP_CLAUSE_ALIGNED(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ALIGNED(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_SIMDLEN(length)` | `OMP_CLAUSE_SIMDLEN(length)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SIMDLEN(length)` | |
  | `OMP_TARGET_CLAUSE_DEVICE_TYPE(type)` | `device_type(type)` | `ACC_CLAUSE_DEVICE_TYPE(type)` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_ENTER(...)` | `enter(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_INDIRECT(...)` | `indirect(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_LINK(...)` | `link(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_COPYIN(...)` | `OMP_CLAUSE_COPYIN(__VA_ARGS__)` | `ACC_CLAUSE_COPYIN(__VA_ARGS__)` | `OMP_CLAUSE_COPYIN(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_NUM_THREADS(nthreads)` | `OMP_CLAUSE_NUM_THREADS(nthreads)` | `ACC_CLAUSE_VECTOR_LENGTH(nthreads)` | `OMP_CLAUSE_NUM_THREADS(nthreads)` | |
  | `OMP_TARGET_CLAUSE_PROC_BIND(attr)` | `OMP_CLAUSE_PROC_BIND(attr)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_PROC_BIND(attr)` | |
  | `OMP_TARGET_CLAUSE_NUM_TEAMS(...)` | `OMP_CLAUSE_NUM_TEAMS(__VA_ARGS__)` | `ACC_CLAUSE_NUM_GANGS(__VA_ARGS__)` | `OMP_CLAUSE_NUM_TEAMS(__VA_ARGS__)` | v2.0.0 で変換先を `ACC_CLAUSE_NUM_WORKERS(__VA_ARGS__)` から `ACC_CLAUSE_NUM_GANGS(__VA_ARGS__)` に変更 |
  | `OMP_TARGET_CLAUSE_THREAD_LIMIT(num)` | `OMP_CLAUSE_THREAD_LIMIT(num)` | `ACC_CLAUSE_VECTOR_LENGTH(num)` | `OMP_CLAUSE_THREAD_LIMIT(num)` | |
  | `OMP_TARGET_CLAUSE_NONTEMPORAL(...)` | `OMP_CLAUSE_NONTEMPORAL(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_NONTEMPORAL(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_SAFELEN(length)` | `OMP_CLAUSE_SAFELEN(length)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SAFELEN(length)` | |
  | `OMP_TARGET_CLAUSE_ORDERED(...)` | `OMP_CLAUSE_ORDERED(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ORDERED(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_SCHEDULE(...)` | `OMP_CLAUSE_SCHEDULE(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_SCHEDULE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_DIST_SCHEDULE(...)` | `OMP_CLAUSE_DIST_SCHEDULE(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DIST_SCHEDULE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_BIND(binding)` | `OMP_CLAUSE_BIND(binding)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_BIND(binding)` | |
  | `OMP_TARGET_CLAUSE_USE_DEVICE_PTR(...)` | `use_device_ptr(__VA_ARGS__)` | `ACC_CLAUSE_USE_DEVICE(__VA_ARGS__)` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_USE_DEVICE_ADDR(...)` | `use_device_addr(__VA_ARGS__)` | `ACC_CLAUSE_USE_DEVICE(__VA_ARGS__)` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP(...)` | `defaultmap(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP_NONE` | `OMP_TARGET_CLAUSE_DEFAULTMAP(none)` | `ACC_CLAUSE_DEFAULT_NONE` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_DEFAULTMAP_PRESENT` | `OMP_TARGET_CLAUSE_DEFAULTMAP(present)` | `ACC_CLAUSE_DEFAULT_PRESENT` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_HAS_DEVICE_ADDR(...)` | `has_device_addr(__VA_ARGS__)` | `ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_IS_DEVICE_PTR(...)` | `is_device_ptr(__VA_ARGS__)` | `ACC_CLAUSE_DEVICEPTR(__VA_ARGS__)` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_USES_ALLOCATORS(...)` | `uses_allocators(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_FROM(...)` | `from(__VA_ARGS__)` | `ACC_CLAUSE_HOST(__VA_ARGS__)` | N/A（縮退モードでは無視される） | |
  | `OMP_TARGET_CLAUSE_TO(...)` | `to(__VA_ARGS__)` | `ACC_CLAUSE_DEVICE(__VA_ARGS__)` | N/A（縮退モードでは無視される） | |
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
  | `OMP_TARGET_CLAUSE_LASTPRIVATE(...)` | `OMP_CLAUSE_LASTPRIVATE(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_LASTPRIVATE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_LINEAR(...)` | `OMP_CLAUSE_LINEAR(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_LINEAR(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_ALLOCATE(...)` | `OMP_CLAUSE_ALLOCATE(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_ALLOCATE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_COLLAPSE(n)` | `OMP_CLAUSE_COLLAPSE(n)` | `ACC_CLAUSE_COLLAPSE(n)` | `OMP_CLAUSE_COLLAPSE(n)` | |
  | `OMP_TARGET_CLAUSE_DEPEND(...)` | `OMP_CLAUSE_DEPEND(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_DEPEND(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_DEPEND_IN(...)` | `OMP_CLAUSE_DEPEND_IN(__VA_ARGS__)` | `ACC_CLAUSE_WAIT(__VA_ARGS__)` | `OMP_CLAUSE_DEPEND_IN(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_DEVICE(...)` | `device(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `device(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(condition)` | `ACC_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(condition)` | |
  | `OMP_TARGET_CLAUSE_IF_TARGET(condition)` | `OMP_CLAUSE_IF(target : condition)` | `ACC_CLAUSE_IF(condition)` | `OMP_CLAUSE_IF(target : condition)` | |
  | `OMP_TARGET_CLAUSE_MAP(...)` | `OMP_CLAUSE_MAP(__VA_ARGS__)` | N/A（OpenACC 使用時には無視される） | `OMP_CLAUSE_MAP(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_ALLOC(...)` | `OMP_CLAUSE_MAP_ALLOC(__VA_ARGS__)` | `ACC_CLAUSE_CREATE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_ALLOC(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_TO(...)` | `OMP_CLAUSE_MAP_TO(__VA_ARGS__)` | `ACC_CLAUSE_COPYIN(__VA_ARGS__)` | `OMP_CLAUSE_MAP_TO(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_FROM(...)` | `OMP_CLAUSE_MAP_FROM(__VA_ARGS__)` | `ACC_CLAUSE_COPYOUT(__VA_ARGS__)` | `OMP_CLAUSE_MAP_FROM(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_TOFROM(...)` | `OMP_CLAUSE_MAP_TOFROM(__VA_ARGS__)` | `ACC_CLAUSE_COPY(__VA_ARGS__)` | `OMP_CLAUSE_MAP_TOFROM(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_RELEASE(...)` | `OMP_CLAUSE_MAP_RELEASE(__VA_ARGS__)` | `ACC_CLAUSE_DELETE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_RELEASE(__VA_ARGS__)` | |
  | `OMP_TARGET_CLAUSE_MAP_DELETE(...)` | `OMP_CLAUSE_MAP_DELETE(__VA_ARGS__)` | `ACC_CLAUSE_DELETE(__VA_ARGS__)` | `OMP_CLAUSE_MAP_DELETE(__VA_ARGS__)` | |
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

## 謝辞

本ライブラリの開発の一部はJSPS科研費JP23K11123，文部科学省「HPCI整備計画調査研究事業」および文部科学省「次世代HPC・AI開発支援拠点形成事業」の助成を受けたものです
