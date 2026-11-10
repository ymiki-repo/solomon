# Migration guide from v1.x to v2.0.0 / v1.x から v2.0.0 への移行ガイド

In v2.0.0, all user-facing macros were renamed to avoid name collisions: directive macros carry the `SOLOMON_` prefix and clause tokens carry the `SOLOMON_CLAUSE_` prefix. Codes written for v1.x keep working by defining `SOLOMON_WITH_SHORT_NAMES`. This file lists the complete correspondence.

v2.0.0 では名前衝突を避けるため，ユーザー向けマクロを全て改名しました（指示文マクロは `SOLOMON_` 接頭辞，引数トークンは `SOLOMON_CLAUSE_` 接頭辞）．v1.x 向けのコードは `SOLOMON_WITH_SHORT_NAMES` を定義すればそのまま動作します．本ファイルは完全な対応表です．

A migration script is bundled; it needs bash and GNU sed (as `gsed` on macOS / BSD); it was tested with bash 4 or later and the GNU tools (`diff`, `mktemp`, `cp`) and on Ubuntu 26.04, where `mkdir`, `mktemp` and `cat` are the uutils coreutils, not with bash 3.2 or the BSD / macOS tools / 移行スクリプトを同梱しています（bash と GNU sed（macOS / BSD では `gsed`）が必要です．bash 4 以降と GNU の道具（`diff`，`mktemp`，`cp`），および `mkdir`・`mktemp`・`cat` が uutils coreutils である Ubuntu 26.04 で確かめており，bash 3.2 と BSD / macOS の道具では確かめていません）:

```sh
misc/migrate/solomon_migrate_v1_to_v2.sh <file|directory> ...          # dry-run (show diff; exit status 1 when something would change)
misc/migrate/solomon_migrate_v1_to_v2.sh --apply <file|directory> ...  # rewrite in place (*.v1.bak backups)
misc/migrate/solomon_migrate_v1_to_v2.sh --help                        # options (--all, --exclude <pattern> or --exclude=<pattern>) and exit status
```

Directories are searched for C/C++/Fortran sources (including `.fpp`). By default only the sources that `#include` `solomon.hpp` (`<solomon.hpp>`, `"solomon.hpp"`, or a path ending in it such as `<solomon/solomon.hpp>`) are converted; the others, including headers that use the v1.x names without including `solomon.hpp`, are skipped with a note, and `--all` converts them as well. Build files (Makefile, `*.mk`, `*.inc`, including Fortran include files named `*.inc`) are examined only when given explicitly; a file given explicitly whose extension is not in the list of sources (e.g., `*.inl`, `*.cuh`) is treated as a build file (the rule below). Names in comments and string literals are replaced as well, so please review the diff. Running the script again, or on v2.0.0 code, changes nothing. The script leaves the following unchanged:
ディレクトリは C/C++/Fortran のソース（`.fpp` を含む）を探します．既定では `solomon.hpp` を `#include` するソース（`<solomon.hpp>`，`"solomon.hpp"`，`<solomon/solomon.hpp>` のように道が `solomon.hpp` で終わるもの）だけを書き換え，それ以外（`solomon.hpp` を読み込まずに v1.x の名前を使うヘッダを含む）は飛ばして表示します．`--all` を付けるとそれらも書き換えます．ビルドのファイル（Makefile，`*.mk`，`*.inc`．Fortran の include ファイルの `*.inc` も）は明示して渡したときだけ見ます．明示して渡したファイルの拡張子がソースの一覧に無いとき（`*.inl`，`*.cuh` など）は，ビルドのファイルとして扱います（下の規則）．コメントと文字列リテラルの中の名前も置換するので，差分を確認してください．もう一度実行しても，v2.0.0 のコードに掛けても何も変わりません．次のものは書き換えません:

* the configuration macros `OFFLOAD_BY_*` and `-DOFFLOAD_BY_*`: they keep working in v2.0.0, and `defined(OFFLOAD_BY_*)` in sources tells the backend actually in use (`solomon.hpp` removes the unprefixed name when the backend is not available), so renaming it would change the meaning / 設定マクロ `OFFLOAD_BY_*` と `-DOFFLOAD_BY_*`（v2.0.0 でも有効です．ソース中の `defined(OFFLOAD_BY_*)` は実際に使われる backend を表す（使えない backend の接頭辞の無い名前は `solomon.hpp` が取り消す）ので，書き換えると意味が変わります）
* raw directive lines: `#pragma` with its continuation lines (a line ending in `\`, also with spaces or tabs after the `\`, as gcc reads it), Fortran `!$omp` / `!$acc` in any letter case, and `c$omp` / `*$omp` / `c$acc` / `*$acc` in column 1; the script prints a note listing such lines that hold v1.x names, so review them by hand (in Fortran directives, `ATOMIC`, `COLLAPSE`, `NUM_THREADS`, `REDUCTION`, and `APPEND_ARGS` are taken as the OpenMP / OpenACC words of the same names and not listed) / 生の指示文の行（`#pragma` とその継続行（`\` で終わる行．gcc と同じく `\` の後ろに空白やタブがあっても継続とみなす），大文字小文字を問わず Fortran の `!$omp` / `!$acc`，1 桁目の `c$omp` / `*$omp` / `c$acc` / `*$acc`）．v1.x の名前を含むそのような行は行番号を表示するので，手で確認してください（Fortran の指示文の `ATOMIC`，`COLLAPSE`，`NUM_THREADS`，`REDUCTION`，`APPEND_ARGS` は同じ名前の OpenMP / OpenACC の語とみなして表示しません）
* in build files, the lines with make / shell variables or assignments (`$(`, `${`, `$NAME`, `:=`, `::=`, `?=`, `+=`, `!=`, `NAME =` at the start of a line, `export` / `unexport` / `override` / `undefine` / `ifdef` / `ifndef` / `unset`); the script prints a note listing such lines that hold v1.x names, so review them by hand / ビルドのファイルで，make / shell の変数や代入を含む行（上の字面）．v1.x の名前を含むそのような行は行番号を表示するので，手で確認してください
* Solomon itself (a directory holding Solomon's `solomon.hpp` is skipped), symbolic links to directories and to sources, and the paths matched by `--exclude` (a shell pattern matched against the path and each of its trailing parts, so `--exclude src/gen` works whether the tree is given as `.` or `app`; a leading `./` in the pattern is ignored, and a trailing `/` makes it match directories only); each is reported / Solomon 本体（Solomon の `solomon.hpp` を持つディレクトリは飛ばす），ディレクトリとソースへのシンボリックリンク，`--exclude` に当たるパス（シェルのパターンを，パスとその後ろ側の各部分に照合するので，`--exclude src/gen` は木を `.` で渡しても `app` で渡しても効く．パターンの先頭の `./` は無視し，末尾の `/` はディレクトリだけに当てる）．いずれも表示します

To verify a migration, compare the preprocessed sources before and after (C/C++: compiler `-E` output; Fortran: the spp output), preprocessing the original sources with the v2.0.0 headers and `-DSOLOMON_WITH_SHORT_NAMES` and the converted ones with the v2.0.0 headers alone; they must be identical, except for names in string literals and `DECLARE_END_OFFLOADED` (which `-DSOLOMON_WITH_SHORT_NAMES` does not define). With the v1.x headers on the original side, the files using `AS_BLOCK` / `NUM_BLOCKS` differ (see below).
移行の検証には，変換前後の前処理済みソース（C/C++ はコンパイラの `-E` 出力，Fortran は spp 出力）を diff 比較してください．変換前は v2.0.0 のヘッダと `-DSOLOMON_WITH_SHORT_NAMES` で，変換後は v2.0.0 のヘッダだけで前処理します．文字列リテラルの中の名前と `DECLARE_END_OFFLOADED`（`-DSOLOMON_WITH_SHORT_NAMES` でも定義されない）を除き，完全一致するはずです．変換前を v1.x のヘッダで前処理すると，`AS_BLOCK` / `NUM_BLOCKS` を使うファイルは一致しません（下記）．

## Directive macros / 指示文マクロ

The spellings marked "pre-release only" were not in v1.1.0; they existed only in the pre-release version of v2.0.0 (the `fortran` branch) and are listed, and rewritten by the migration script, for the codes written against it.
「公開前の版のみ」と記した綴りは v1.1.0 には無く，v2.0.0 の公開前の版（`fortran` ブランチ）にだけあったものです．その版向けに書かれたコードのために載せており，移行スクリプトも書き換えます．

| v1.x | v2.0.0 |
| ---- | ---- |
| `OFFLOAD(...)` | `SOLOMON_OFFLOAD(...)` |
| `END_OFFLOAD` (pre-release only / 公開前の版のみ) | `SOLOMON_END_OFFLOAD` |
| `OFFLOAD_OUTER_LOOP(...)` (pre-release only / 公開前の版のみ) | `SOLOMON_OFFLOAD_OUTER_LOOP(...)` |
| `PARALLELIZE_INNER_LOOP(...)` (pre-release only / 公開前の版のみ) | `SOLOMON_PARALLELIZE_INNER_LOOP(...)` |
| `END_OFFLOAD_OUTER_LOOP` (pre-release only / 公開前の版のみ) | `SOLOMON_END_OFFLOAD_OUTER_LOOP` |
| `IF_NOT_OFFLOADED(arg)` | `SOLOMON_IF_NOT_OFFLOADED(arg)` |
| `DECLARE_OFFLOADED(...)` | `SOLOMON_DECLARE_OFFLOADED(...)` |
| `DECLARE_OFFLOADED_END` | `SOLOMON_DECLARE_OFFLOADED_END` (not required in Fortran / Fortran では不要) |
| `DECLARE_END_OFFLOADED` (pre-release only / 公開前の版のみ) | (removed; use `SOLOMON_DECLARE_OFFLOADED_END` / 削除．`SOLOMON_DECLARE_OFFLOADED_END` を使用) |
| `SYNCHRONIZE(...)` | `SOLOMON_SYNCHRONIZE(...)` |
| `WAIT_QUEUE(id)` | `SOLOMON_WAIT_QUEUE(id)` |
| `ATOMIC(...)` | `SOLOMON_ATOMIC(...)` |
| `ATOMIC_UPDATE` | `SOLOMON_ATOMIC_UPDATE` |
| `ATOMIC_READ` | `SOLOMON_ATOMIC_READ` |
| `ATOMIC_WRITE` | `SOLOMON_ATOMIC_WRITE` |
| `ATOMIC_CAPTURE` | `SOLOMON_ATOMIC_CAPTURE` |
| `DATA_ACCESS_BY_HOST(...)` | `SOLOMON_DATA_ACCESS_BY_HOST(...)` |
| `DATA_ACCESS_BY_DEVICE(...)` | `SOLOMON_DATA_ACCESS_BY_DEVICE(...)` |
| `USE_DEVICE_DATA_FROM_HOST(...)` | `SOLOMON_USE_DEVICE_DATA_FROM_HOST(...)` |
| `MALLOC_ON_DEVICE(...)` | `SOLOMON_MALLOC_ON_DEVICE(...)` |
| `ALLOCATE_ON_DEVICE(...)` (pre-release only / 公開前の版のみ) | `SOLOMON_ALLOCATE_ON_DEVICE(...)` |
| `FREE_FROM_DEVICE(...)` | `SOLOMON_FREE_FROM_DEVICE(...)` |
| `DEALLOCATE_ON_DEVICE(...)` (pre-release only / 公開前の版のみ) | `SOLOMON_DEALLOCATE_ON_DEVICE(...)` |
| `MEMCPY_D2H(...)` | `SOLOMON_MEMCPY_D2H(...)` |
| `MEMCPY_H2D(...)` | `SOLOMON_MEMCPY_H2D(...)` |
| `DECLARE_DATA_ON_DEVICE(...)` | `SOLOMON_DECLARE_DATA_ON_DEVICE(...)` |

## Clause tokens / 引数トークン

| v1.x | v2.0.0 |
| ---- | ---- |
| `AS_INDEPENDENT` | `SOLOMON_CLAUSE_INDEPENDENT` |
| `AS_SEQUENTIAL` | `SOLOMON_CLAUSE_SEQUENTIAL` |
| `NUM_THREADS(n)` | `SOLOMON_CLAUSE_NUM_THREADS(n)` |
| `NUM_BLOCKS(n)` | `SOLOMON_CLAUSE_NUM_BLOCKS(n)` |
| `NUM_GRIDS(n)` (deprecated) | `SOLOMON_CLAUSE_NUM_GRIDS(n)` (deprecated; use `SOLOMON_CLAUSE_NUM_BLOCKS(n)`) |
| `AS_THREAD` | `SOLOMON_CLAUSE_THREAD` |
| `AS_BLOCK` | `SOLOMON_CLAUSE_BLOCK` |
| `AS_GRID` (deprecated) | `SOLOMON_CLAUSE_GRID` (deprecated; use `SOLOMON_CLAUSE_BLOCK`) |
| `COLLAPSE(n)` | `SOLOMON_CLAUSE_COLLAPSE(n)` |
| `AS_ASYNC(...)` | `SOLOMON_CLAUSE_ASYNC(...)` |
| `ASYNC_QUEUE(id)` | `SOLOMON_CLAUSE_ASYNC_QUEUE(id)` |
| `REDUCTION(...)` | `SOLOMON_CLAUSE_REDUCTION(...)` |
| `ENABLE_IF(condition)` | `SOLOMON_CLAUSE_IF(condition)` |
| `AS_PRIVATE(...)` | `SOLOMON_CLAUSE_PRIVATE(...)` |
| `AS_FIRSTPRIVATE(...)` | `SOLOMON_CLAUSE_FIRSTPRIVATE(...)` |
| `AS_DEVICE_PTR(...)` | `SOLOMON_CLAUSE_DEVICE_PTR(...)` |
| `COPY_BEFORE_AND_AFTER_EXEC(...)` | `SOLOMON_CLAUSE_COPY_BEFORE_AND_AFTER_EXEC(...)` |
| `COPY_H2D_BEFORE_EXEC(...)` | `SOLOMON_CLAUSE_COPY_H2D_BEFORE_EXEC(...)` |
| `COPY_D2H_AFTER_EXEC(...)` | `SOLOMON_CLAUSE_COPY_D2H_AFTER_EXEC(...)` |

## Clauses whose meaning changed / 意味が変わった指示節

The block clauses were restructured so that a block is a gang in OpenACC and a team in OpenMP target. The renaming keeps the names, but the output in the OpenACC backend changes; the migration script prints a note for the files that hold `AS_BLOCK` / `NUM_BLOCKS` (also when they are only in raw directive lines, which it leaves unchanged).
ブロック系の指示節は，ブロックを OpenACC では gang に，OpenMP target では team にそろえる形に再設計されました．改名は名前を保ちますが，OpenACC での出力は変わります．移行スクリプトは `AS_BLOCK` / `NUM_BLOCKS` を含むファイルについて注意を表示します（書き換えずに残す生の指示文の行にだけある場合も）．

| v1.x | v1.x output (OpenACC / OpenMP target) | v2.0.0 output (OpenACC / OpenMP target) | to keep the v1.x meaning in OpenACC / OpenACC で v1.x の意味を保つには |
| ---- | ---- | ---- | ---- |
| `AS_BLOCK` | `worker` / (nothing) | `gang` / (nothing) | `ACC_CLAUSE_WORKER` |
| `NUM_BLOCKS(n)` | `num_workers(n)` / `num_teams(n)` | `num_gangs(n)` / `num_teams(n)` | `ACC_CLAUSE_NUM_WORKERS(n)` (it emits nothing in OpenMP target, where v1.x emitted `num_teams(n)` / OpenMP target では何も出力しない．v1.x は `num_teams(n)`) |
| `NUM_GRIDS(n)` (deprecated) | `num_gangs(n)` / (nothing) | `num_gangs(n)` / `num_teams(n)` | (unchanged in OpenACC / OpenACC では同じ) |

Using `NUM_GRIDS(n)` together with `NUM_BLOCKS(m)` (or `AS_GRID` together with `AS_BLOCK`) gives the same clause twice in v2.0.0, since both now mean the gang / team level.
v2.0.0 では `NUM_GRIDS(n)` と `NUM_BLOCKS(m)`（または `AS_GRID` と `AS_BLOCK`）を併用すると，どちらも gang / team の階層を指すので同じ指示節が 2 つ出力されます．

```c
OFFLOAD(NUM_GRIDS(128), NUM_BLOCKS(4))
// v1.x  : #pragma acc kernels num_gangs(128) num_workers(4)  /  #pragma omp target teams loop num_teams(4)
// v2.0.0: #pragma acc kernels num_gangs(128) num_gangs(4)    /  #pragma omp target teams loop num_teams(128) num_teams(4)
OFFLOAD(AS_GRID, AS_BLOCK)
// v1.x  : #pragma acc loop gang worker
// v2.0.0: #pragma acc loop gang gang
```

To keep the v1.x OpenACC mapping, replace `NUM_BLOCKS(m)` / `AS_BLOCK` in such lists with `ACC_CLAUSE_NUM_WORKERS(m)` / `ACC_CLAUSE_WORKER`; otherwise keep only one of the two.
v1.x の OpenACC での対応付けを保つには，このような並びの `NUM_BLOCKS(m)` / `AS_BLOCK` を `ACC_CLAUSE_NUM_WORKERS(m)` / `ACC_CLAUSE_WORKER` に置き換えてください．そうでなければ 2 つのうち 1 つだけを残してください．

## Configuration macros and utilities / 設定マクロ・ユーティリティ

| v1.x | v2.0.0 |
| ---- | ---- |
| `-DOFFLOAD_BY_OPENACC` | `-DSOLOMON_OFFLOAD_BY_OPENACC` (old spelling keeps working unconditionally, and the migration script leaves it unchanged / 旧綴りも無条件で有効．移行スクリプトは書き換えない) |
| `-DOFFLOAD_BY_OPENACC_PARALLEL` | `-DSOLOMON_OFFLOAD_BY_OPENACC_PARALLEL` (same / 同上) |
| `-DOFFLOAD_BY_OPENMP_TARGET` | `-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET` (same / 同上) |
| `-DOFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE` | `-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE` (same / 同上) |
| `APPEND_ARGS(...)` | `SOLOMON_APPEND_ARGS(...)` (expands to nothing in serial execution, where neither OpenACC nor OpenMP is enabled; the arguments are dropped / シリアル実行（OpenACC・OpenMP のいずれも無効）では空に展開され，引数は消える) |
| `PRAGMA(...)`, `PRAGMA_STR(x)`, `PRAGMA_STR1(x)`, `SHOW_PRAGMA(...)` | internal (`SOLOMON_INTERNAL_*`); the old names are defined only with `-DSOLOMON_WITH_SHORT_NAMES` and are not rewritten by the migration script. If your own directive macros use them, copy their definitions (`solomon/short_names.hpp`) / 内部名（`SOLOMON_INTERNAL_*`）．旧名は `-DSOLOMON_WITH_SHORT_NAMES` のときだけ定義され，移行スクリプトは書き換えない．自前の指示文マクロで使っている場合は定義（`solomon/short_names.hpp`）をコピーする |

The names of the two-level notations (`PRAGMA_ACC_*`, `PRAGMA_OMP_*`, `ACC_CLAUSE_*`, `OMP_CLAUSE_*`, `OMP_TARGET_CLAUSE_*`) are unchanged; some of their outputs changed (the "Bug fixes" in the Changelog of README.md), and one input form stops preprocessing (below).
2段記法（`PRAGMA_ACC_*`, `PRAGMA_OMP_*`, `ACC_CLAUSE_*`, `OMP_CLAUSE_*`, `OMP_TARGET_CLAUSE_*`）の名前は変更ありません．出力が変わったもの（README の Changelog の「Bug fixes」）と，前処理が止まるようになった書き方（下記）があります．

## Inputs that stop preprocessing in v2.0.0 / v2.0.0 で前処理が止まる書き方

* `OMP_TARGET_CLAUSE_MAP(...)` with OpenACC or OpenMP target (serial and fallback mode drop the clause and are not affected): if the third or a later element of a non-nested list is the name of a function-like macro that cannot be invoked with one argument (e.g., `#define fm2(x, y) ...` and `OMP_TARGET_CLAUSE_MAP(a, b, fm2)`), preprocessing now fails, and if it is the name of one whose expansion with one argument has a comma at the top level (e.g., `#define fc1(x) x, x`), the build stops with a Solomon error that names internal macros. Place that element first or second (e.g., `OMP_TARGET_CLAUSE_MAP(fm2, a, b)`), or write it as an array section (`fm2[0:n]`; C and C++ only). The migration script does not rewrite it.
  OpenACC と OpenMP target のとき（縮退モードとシリアルモードでは節を出さないので起きません），入れ子でない並びの 3 番目以降の要素が，引数 1 個では呼べない関数形式マクロの名前のとき（例: `#define fm2(x, y) ...` と `OMP_TARGET_CLAUSE_MAP(a, b, fm2)`）は前処理が失敗し，引数 1 個で呼ぶとトップレベルのカンマを含む形に展開される関数形式マクロの名前のとき（例: `#define fc1(x) x, x`）は内部のマクロの名前を挙げる Solomon のエラーでビルドが止まるようになりました．その要素を 1 番目か 2 番目に置く（例: `OMP_TARGET_CLAUSE_MAP(fm2, a, b)`）か，配列区間（`fm2[0:n]`．C と C++ のみ）で書いてください．移行スクリプトはこれを書き換えません．
* `ACC_CLAUSE_WAIT(...)`, `ACC_CLAUSE_ASYNC(...)`, `ACC_CLAUSE_SELF(...)` (and the clauses of the same names in the other notations) with gcc / g++ in a strict mode before C23 / C++20 (`-std=c99`, `-std=c++17`, ...): when the last token of the arguments is the name of a function-like macro with two or more named parameters (e.g., `fm2` after `#define fm2(a, b) ...`), preprocessing now fails. Put that name in parentheses (`ACC_CLAUSE_ASYNC((fm2))`) or not at the end, or use a GNU mode or C23 / C++20
  gcc / g++ の厳密モードで C23 / C++20 より前の版（`-std=c99`，`-std=c++17` など）では，`ACC_CLAUSE_WAIT(...)`・`ACC_CLAUSE_ASYNC(...)`・`ACC_CLAUSE_SELF(...)`（とほかの記法の同じ名前の指示節）の引数の最後の字句が，名前付きの引数を 2 個以上とる関数形式マクロの名前のとき（例: `#define fm2(a, b) ...` の下の `fm2`），前処理が失敗するようになりました．その名前を括弧で囲む（`ACC_CLAUSE_ASYNC((fm2))`）か最後に置かない，または GNU モードか C23 / C++20 を使ってください

## New macros in v2.0.0 / v2.0.0 の新規マクロ

| macro | description |
| ---- | ---- |
| `SOLOMON_OFFLOAD_SERIAL(...)`, `SOLOMON_END_OFFLOAD_SERIAL` | offload the following structured block for single-thread execution on the device (`acc serial` / `omp target`) / 直後の構造化ブロックをデバイス上で 1 スレッド実行 |
| `SOLOMON_CLAUSE_WAIT_QUEUE(...)` | make a construct wait for the given queues before it starts (OpenACC `wait(...)`; ignored in OpenMP target) / 指定したキューの完了を待ってから開始（OpenACC の `wait(...)`．OpenMP target では無視） |
| `SOLOMON_CLAUSE_TARGETS(...)` | name the target procedures of `SOLOMON_DECLARE_OFFLOADED(...)` / 手続を名前指定 |
| `SOLOMON_DECLARE_ON_DEVICE(...)` | declare device-resident variables / デバイス常駐変数の宣言 |
| `SOLOMON_DECLARE_ON_DEVICE_LINKED(...)` | device-resident variables with link semantics / link 意味論の常駐変数 |
| `SOLOMON_PROFILE_RANGE_BEGIN("name")`, `SOLOMON_PROFILE_RANGE_END`, `SOLOMON_PROFILE_MARK("name")` | vendor-neutral profiler tags (NVTX / rocTX / ITT), enabled by `-DSOLOMON_TAGGED_PROFILE`; `-DSOLOMON_TAGGED_PROFILE_AUTO` also tags the directive macros, and `SOLOMON_TAGGED_PROFILE_WITH_NVTX` / `_WITH_ROCTX` / `_WITH_ITT` force a backend (see README.md) / ベンダー中立のプロファイラタグ（`-DSOLOMON_TAGGED_PROFILE` で有効．`_AUTO` は指示文マクロにも自動でタグを付ける） |
| `OMP_TARGET_MAP_MODIFIER_*` | map modifiers written in front of the nested clauses of `OMP_TARGET_CLAUSE_MAP(...)` (e.g., `OMP_TARGET_CLAUSE_MAP(OMP_TARGET_MAP_MODIFIER_PRESENT, OMP_TARGET_CLAUSE_TO(a))`) / `OMP_TARGET_CLAUSE_MAP(...)` の入れ子の節の前に書く map 修飾子 |
| `OMP_TARGET_CLAUSE_TO_PRESENT(...)`, `OMP_TARGET_CLAUSE_FROM_PRESENT(...)` | `to(present : ...)` / `from(present : ...)` for `update` / `update` 用 |
| `-DSOLOMON_OFFLOAD_BY_*` | prefixed spellings of the configuration macros (the unprefixed ones keep working) / 設定マクロの接頭辞つきの綴り（接頭辞の無い綴りも有効） |
| `SOLOMON_WITH_SHORT_NAMES` | enable the v1.x spellings / v1.x の綴りを有効化（デフォルト OFF） |
| `PRAGMA_OMP_END_CRITICAL_WITH_NAME(name)` | close a named `critical` (Fortran: `!$omp end critical(name)`; nothing in C/C++) / 名前つきの `critical` を閉じる（Fortran は `!$omp end critical(name)`．C/C++ では何も出さない） |
| `PRAGMA_OMP_END_TARGET_PARALLEL_DO` and the other 7 loop forms | close an OpenMP loop directive in the OpenACC backend of Fortran (nothing in OpenMP; `SOLOMON_END_OFFLOAD` is recommended) / Fortran の OpenACC のバックエンドで OpenMP の記法のループ系の指示文を閉じる（OpenMP では何も出さない．`SOLOMON_END_OFFLOAD` を推奨） |
